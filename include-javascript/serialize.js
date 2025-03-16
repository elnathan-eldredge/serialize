let SERIALIZE_NORESERVE = true

let is_big_endian = () => {
    let uInt32 = new Uint32Array([0x01000000]);
    let uInt8 = new Uint8Array(uInt32.buffer);
    return uInt8[0] == 0x01;
};

let convert_typed_array = (array, newtype) => {
    return new newtype(array.buffer.slice());
}

let invert_endian = (array, unit_size) => {
    if(array.byteLength % unit_size != 0)
        throw new Error("invert_endian: array bytelength " + array.byteLength.toString()
                        + " is not a multiple of unit size " + unit_size)
    let u = array.byteLength/unit_size;
    let buff = new Uint8Array(array.buffer);
    while(u--){
        let mini_slice = buff.slice(u*unit_size,(u+1)*unit_size);
        let i = unit_size;
        while(i--){
            buff[(u+1)*unit_size - i - 1] = mini_slice[i];
        }
    }
    return array
}

let enforce_endian = (array, unit_size_optional) => {
    if(is_big_endian)
        invert_endian(array,unit_size_optional==undefined?array.BYTES_PER_ELEMENT:unit_size_optional);
    return array
}

const  COMPOUND_NODE_BEGIN_FLAG = 123
const  COMPOUND_NODE_END_FLAG = 125
const  COMPOUND_NODE_BEGIN_STRING_FLAG  = 44
const  COMPOUND_NODE_BEGIN_ELEMENT_FLAG  = 58
const  COMPOUND_NODE_BEGIN_ELEMENT_FLAG_S = ':'
const  COMPOUND_NODE_BEGIN_BLOCK_FLAG  = 45
const  COMPOUND_NODE_BEGIN_LIST_FLAG  = 91
const  COMPOUND_NODE_END_LIST_FLAG  = 93

const  SB_META_UNDEFINED = 0
const  SB_META_INT_STYLE = 1
const  SB_META_FLOAT_STYLE = 9
const  SB_META_BOOLEAN = 11
const  SB_META_STRING = 12
const  SB_META_MAX = 127

const  SB_FLAG_UNDEFINED = "x".charCodeAt(0)
const  SB_FLAG_I8 = "b".charCodeAt(0)
const  SB_FLAG_I16 = "m".charCodeAt(0)
const  SB_FLAG_I32 = "i".charCodeAt(0)
const  SB_FLAG_I64 = "l".charCodeAt(0)
const  SB_FLAG_FLOAT = "f".charCodeAt(0)
const  SB_FLAG_DOUBLE = "d".charCodeAt(0)
const  SB_FLAG_LONG_DOUBLE = "q".charCodeAt(0)
const  SB_FLAG_BOOLEAN = "n".charCodeAt(0)
const  SB_FLAG_STRING = "s".charCodeAt(0)

/*vector*/

class uVector{
    constructor(data){
        this.data = data==undefined?new Uint8Array():new Uint8Array(data.buffer.slice());
        this.size = data==undefined?0:data.byteLength;
    }
    reserve(num){
        if(typeof(num) != "number")
            throw new Error("uVector: reserve: must be number")
        let arr = new Uint8Array(num);
        arr.set(this.data.slice(0,Math.min(this.data.byteLength,num)))
        this.data = arr;
    }
    push_back(uint){
        if(typeof(uint) != "number")
            throw new Error("uVector: reserve: must be number")
        if(this.size + 1 > this.data.byteLength)
            this.reserve(this.data.byteLength*2+1)
        this.data[this.size] = uint;
        this.size++;
    }
    insert_back(arr){
        if(!ArrayBuffer.isView(arr))
            throw new Error("uVector: insert: must be typed array");
        if(this.size + arr.byteLength >= this.data.byteLength)
            this.reserve(Math.max(this.data.byteLength*2,this.size+arr.byteLength))
        this.data.set(arr.slice(),this.size)
        this.size += arr.byteLength;
    }
    shrink_to_fit(){
        this.reserve(this.size)
    }
    array(){
        return this.data
    }
    used(){
        return this.size
    }
    clear(){
        this.data = new Uint8Array();
        this.size = 0;
    }
}

/*
  class SizedBlock{
  public:
    void* contents_native;
    uint16_t element_span = 0;
    un_size_t span = 0;
    uint8_t meta = 0;
    
    SizedBlock(uint16_t element_size, un_size_t element_count, void* data);

    std::vector<char> lower();
    
    SizedBlock(){
      span = 0;
      element_span = 0;
      contents_native = nullptr;
    };

    char* upper(char* data, char* max);

    uint64_t upper(std::vector<char>& data, uint64_t starting_index);
    
    void dump();

    void copy_to(SizedBlock* target);

    void assign_meta(uint8_t meta);

    ~SizedBlock();

  };
*/

class SizedBlock{
    contents_native = new Uint8Array();
    element_span = 0;
    span = 0;
    meta = 0;

    constructor(element_size, typedarray){
        if(element_size == undefined && typedarray == undefined)
            return;
        this.contents_native = new Uint8Array(typedarray.buffer.slice());
        this.element_span = element_size
        this.span = this.contents_native.byteLength
    }

    lower(){
        if(this.isdumped())
            return new Uint8Array();
        let contents_little = new Uint8Array(this.contents_native.slice());
        let span_b = new BigUint64Array([BigInt(this.span)]);
        let span_a = new Uint8Array(span_b.buffer)
        let elemspan_a = new Uint16Array([this.element_span]);
        let meta_a = new Uint8Array([this.meta]);
        invert_endian(meta_a,1);
        invert_endian(span_a,8);
        invert_endian(elemspan_a,2);
        invert_endian(contents_little,this.element_span);
        enforce_endian(meta_a,1);
        enforce_endian(elemspan_a,2);
        enforce_endian(span_a,8);
        enforce_endian(contents_little,this.element_span);
        let result = new Uint8Array(meta_a.byteLength
                                    + elemspan_a.byteLength
                                    + span_a.byteLength
                                    + contents_little.byteLength);
        result.set(meta_a);
        result.set(new Uint8Array(elemspan_a.buffer), meta_a.byteLength);
        result.set(new Uint8Array(span_a.buffer), meta_a.byteLength + elemspan_a.byteLength);
        result.set(contents_little, meta_a.byteLength + elemspan_a.byteLength + span_a.byteLength);
        return result;
    }

    upper(uint8array, startindex){
        this.dump();
        startindex = startindex==undefined?0:startindex;
        let errcannot = new Error("SizedBlock.upper: error deserializing")
        if(uint8array.byteLength <= 11)
            throw errcannot;
        let meta_a = new Uint8Array(uint8array.slice(startindex,startindex+1).buffer);
        let elemspan_a = new Uint16Array(uint8array.slice(startindex+1,startindex+3).buffer);
        let span_a = new BigUint64Array(uint8array.slice(startindex+3,startindex+11).buffer);
        invert_endian(meta_a,1);
        invert_endian(span_a,8);
        invert_endian(elemspan_a,2);
        enforce_endian(meta_a, 1);
        enforce_endian(elemspan_a, 2);
        enforce_endian(span_a, 8);
        this.meta = meta_a[0];
        this.element_span = elemspan_a[0];
        this.span = Number(span_a[0]);
        if(this.span > (uint8array.bytelength - 11 - startindex)){
            dump();
            throw errcannot;
        }
        let contents_little = uint8array.slice(startindex + 11);
        enforce_endian(contents_little);
        this.contents_native = contents_little;
    }

    dump(){
        if(this.isdumped())
            return;
        this.contents_native = new Uint8Array();
        this.element_span = 0;
        this.span = 0;
        this.meta = 0;        
    };

    copy_to(target){
        target.span = this.span;
        target.element_span = this.element_span;
        target.meta = this.meta;
        target.contents_native = new Uint8Array(this.contents_native.buffer.slice())
    };
    
    assign_meta(meta){
        this.meta = meta;
    };

    isdumped(){return this.span==0};
   
}

/*
  class CompoundNode {
  public:
    std::unordered_map<std::string,SizedBlock*> generic_tags;
    std::unordered_map<std::string,CompoundNode*> child_nodes;
    std::unordered_map<std::string,std::vector<CompoundNode*>> child_node_lists;

    CompoundNode(){};

    template<typename T> SizedBlock* put(std::string key, T var);
    template<typename T> SizedBlock* put_string(std::string key, un_size_t amount, T* vars);
    template<typename T> SizedBlock* put_string(std::string key, std::vector<T>& vars);

    void put(std::string key, CompoundNode& node);
    void put(std::string key, std::vector<CompoundNode*>& nodes);
    void put_back(std::string key, CompoundNode& node);

    template<typename T> bool has_compat(std::string key);
    template<typename T> bool has_compat_string(std::string key);
    bool has_node(std::string key);
    bool has_node_list(std::string key);

    template<typename T> T get(std::string key);
    template<typename T> std::vector<T> get_string(std::string key);
    template<typename T> T* get_ref(std::string key);
    un_size_t get_string_length(std::string key);

    CompoundNode* get_node(std::string key);
    std::vector<CompoundNode*> get_node_list(std::string key);
    un_size_t get_node_list_length(std::string key);

    void copy_to(CompoundNode* node);
    
    void burninate_generic_if_exists(std::string key);

    std::string similair_json();

    bool operator[](std::string key);

    std::vector<char> serialize();
    std::string serialize_encode();
    std::string serialize_readable(bool omit_undefined);

    bool deserialize(std::vector<char>* data, un_size_t start_index, un_size_t* end_index);
    bool decode_deserialize(std::string data);
    bool deserialize_readable(std::string data);

  private:
    bool deserialize_readable(std::vector<char> *data, un_size_t start_index,
                              un_size_t *end_index);

  public:
    
    ~CompoundNode();

    void destroy_children();
  };
*/
function ArrayBufferToString(buffer) {
    return BinaryToString(String.fromCharCode.apply(null, Array.prototype.slice.apply(new Uint8Array(buffer))));
}

function StringToArrayBuffer(string) {
    return StringToUint8Array(string).buffer;
}

function BinaryToString(binary) {
    var error;

    try {
        return decodeURIComponent(escape(binary));
    } catch (_error) {
        error = _error;
        if (error instanceof URIError) {
            return binary;
        } else {
            throw error;
        }
    }
}

let _add_escapes_to_string = (str) => {
    return str.replace(':', '\\:')
}

let _add_escapes_to_readable = (str) => {
    return str.replace('"', '\\"')
}

enum BasicParserState {
    AwaitBegin     0,
    AwaitKeyStart  1,
    ConstructKey   2,
    AwaitIndicator 3,
    
    Success        253,
    Error          254,
    Warning        255
}

class BasicPushdownParserData{
    node = undefined;
    currentState = BasicParserState.AwaitBegin;
    curstring = "";
    constructor(){};
}

class BasicPushdownParser{
    stateStack = [];
    state = new BasicPushdownParserData();
    constructor(){};
    awaitCounter = 0;
    consume(c){
        switch(state.currentState){

        
            
        case AwaitBegin:{
            if(c == COMPOUND_NODE_BEGIN_FLAG){
                awaitCounter = 0;
                state.currentState = AwaitKeyStart;
            }
            awaitCounter++;
            break;
        }
            
        default:{
            state.currentState = BasicParserState.Error;
            break;
        }
            
        }
        
    }
}

class ReadablePushdownParserData{
    node;
    state;
    curstring;
}

class ReadablePushdownParser{}

class CompoundNode{
    generic_tags = {};
    child_nodes = {};
    child_node_lists = {};

    constructor(){};

    put(key, value, bytesperelement, arraytype){
        let sb;
        switch(typeof(value)){
        case "object":{
            if(!ArrayBuffer.isView(value))
                throw new Error("CompoundNode.put : if object, value must be typed array")
            sb = new SizedBlock(bytesperelement!=undefined?bytesperelement:value.BYTES_PER_ELEMENT, value);
            break;
        }
        case "bigint":{
            sb = new SizedBlock(8,new BigInt64Array([value]));
            break;
        }
        case "number":{
            if(arraytype==undefined)
                throw new Error("CompoundNode.put : array type reqired for insertion of number values");
            sb = new SizedBlock(arraytype.BYTES_PER_ELEMENT,new arraytype([value]));
            break;
        }
        case "string":{
            sb = new SizedBlock(1,new Uint8Array(value.split("").map(x=>x.charCodeAt())));
            break;
        }
        case "boolean":{
            sb = new SizedBlock(1,new Uint8Array([value?1:0]));
            break;
        }
        default:{
            throw new Error("CompoundNode.put : cannot insert variables of type: " + typeof(value));
            break;
        }
        }
        this.generic_tags[key] = sb;
        return sb;
    }
    put_node(key, node_or_array){
        if(typeof(node_or_array) == "object"){
            let ncn = new CompoundNode()
            node_or_array.copy_to(ncn);
            this.child_nodes[key] = ncn;
            return;
        }
        child_node_lists[key] = []
        for (let node in node_or_array){
            let ncn = new CompoundNode();
            code.copy_to(ncn);
            child_node_lists[key].push(node_or_array);
        }
        return node_or_array
    }
    put_back(key, node){
        if(this.child_node_lists[key] == undefined)
            this.child_node_lists[key] = [];
        let ncn = new CompoundNode();
        node.copy_to(ncn);
        this.child_node_lists[key].push(ncn);
        return ncn
    }

    has_compat(key, arrtype){
        if(this.generic_tags[key] == undefined)
            return false;
        if(this.generic_tags[key].span != typeof(arrtype)=="number"?arrtype:arrtype.BYTES_PER_ELEMENT)
            return false;
        return true;
    }
    has_compat_array(key, arrtype){
        if(this.generic_tags[key] == undefined)
            return false;
        if(this.generic_tags[key].element_span != typeof(arrtype)=="number"?arrtype:arrtype.BYTES_PER_ELEMENT)
            return false;
        return true;
    }
    has_node(key){
        return this.child_nodes[key]!=undefined;
    }
    has_node_list(key){
        return this.child_node_lists[key]!=undefined;
    }

    get(key, optionalcast){
        if(!this.has_compat_array(key,optionalcast!=undefined?optionalcast.BYTES_PER_ELEMENT:1))
            return undefined;
        let contents = this.generic_tags[key].contents_native.buffer.slice();
        if(optionalcast != undefined)
            return new optionalcast(contents);
        return new Uint8Array(contents);
    }
    get_ref(key, optionalcast){
        if(!this.has_compat_array(key,optionalcast!=undefined?optionalcast.BYTES_PER_ELEMENT:1))
            return undefined;
        if(optionalcast != undefined)
            return new optionalcast(this.generic_tags[key].contents_native.buffer)
        return new Uint8Array(this.generic_tags[key].contents_native.buffer)
    }
    get_node(key){
        return this.child_nodes[key]
    }
    get_node_list(key){
        return this.child_node_lists[key]
    }

    copy_to(target){
        target.destroy_children()
        for(let key in this.generic_tags){
            let nsb = new SizedBlock();
            this.generic_tags[key].copy_to(nsb);
            target.generic_tags[key] = nsb;
        }
        for(let key in this.child_nodes){
            let ncn = new CompoundNode();
            this.child_nodes[key].copy_to(ncn);
            target.child_nodes[key] = ncn;
        }
        for(let key in this.child_node_lists){
            target.child_node_lists[key] = [];
            let list = this.child_node_lists[key];
            for(let lnode in list){
                let ncn = new CompoundNode();
                lnode.copy_to(ncn);
                target.child_node_lists[key].push(ncn);
            }
        }
    }

    has_symbol(key){
        return this.generic_tags[key]!=undefined||
            this.child_nodes[key]!=undefined||
            this.child_node_lists[key]!=undefined;
    }

    serialize(){
        let arr = new uVector();
        SERIALIZE_NORESERVE = false
        if(!SERIALIZE_NORESERVE)
            arr.reserve(8192);
        arr.push_back(COMPOUND_NODE_BEGIN_FLAG);
        for(const key in this.generic_tags){
            let esc = _add_escapes_to_string(key);
            arr.push_back(COMPOUND_NODE_BEGIN_STRING_FLAG);
            arr.insert_back(Uint8Array.from(esc.split("").map(x => x.charCodeAt())));
            arr.push_back(COMPOUND_NODE_BEGIN_ELEMENT_FLAG);
            arr.push_back(COMPOUND_NODE_BEGIN_BLOCK_FLAG);
            arr.insert_back(this.generic_tags[key].lower());
        }
        for(const key in this.child_nodes){
            let esc = _add_escapes_to_string(key);
            arr.push_back(COMPOUND_NODE_BEGIN_STRING_FLAG);
            arr.insert_back(Uint8Array.from(esc.split("").map(x => x.charCodeAt())));
            arr.push_back(COMPOUND_NODE_BEGIN_ELEMENT_FLAG);
            arr.insert_back(this.child_nodes[key].serialize());
        }
        for(const key in this.child_node_lists){
            let esc = _add_escapes_to_string(key);
            arr.push_back(COMPOUND_NODE_BEGIN_STRING_FLAG);
            arr.insert_back(Uint8Array.from(esc.split("").map(x => x.charCodeAt())));
            arr.push_back(COMPOUND_NODE_BEGIN_ELEMENT_FLAG);
            arr.push_back(COMPOUND_NODE_BEGIN_LIST_FLAG);
            for (let n = 0; n < this.child_node_lists[key].length; n ++){
                arr.insert_back(this.child_node_lists[key][n].serialize())
            }
            arr.push_back(COMPOUND_NODE_END_LIST_FLAG);
        }
        arr.push_back(COMPOUND_NODE_END_FLAG);
        arr.shrink_to_fit()
        return arr.array()
    }
    serialize_encode(){
        let arrb = this.serialize();
        let str = btoa(ArrayBufferToString(arrb));
        return str
    }
    serialize_readable(){}

    deserialize(content){}
    deserialize_decode(content){}
    deserialize_readable(content){}

    destroy_children(){
        this.generic_tags = {};
        this.child_nodes = {};
        this.child_node_lists = {};
    }
}
