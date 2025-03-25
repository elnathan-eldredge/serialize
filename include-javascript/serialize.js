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
const  COMPOUND_NODE_KEY_ESCAPE_FLAG = 92
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

const  COMPOUND_NODE_BEGIN_FLAG_R = "{"
const  COMPOUND_NODE_ESCAPE_STRING_R = "\\"
const  COMPOUND_NODE_BEGIN_STRING_FLAG_R = "\""
const  COMPOUND_NODE_END_STRING_FLAG_R = "\""
const  COMPOUND_NODE_KV_SEPERATOR_R = ":"
const  COMPOUND_NODE_BEGIN_LIST_R = "["
const  COMPOUND_NODE_END_LIST_R = "]"
const  COMPOUND_NODE_VALUE_SEPERATOR_R = ","
const  COMPOUND_NODE_END_FLAG_R = "}"

const  SB_FLAG_UNDEFINED = "x"
const  SB_FLAG_I8 = "b"
const  SB_FLAG_I16 = "m"
const  SB_FLAG_I32 = "i"
const  SB_FLAG_I64 = "l"
const  SB_FLAG_FLOAT = "f"
const  SB_FLAG_DOUBLE = "d"
const  SB_FLAG_LONG_DOUBLE = "q"
const  SB_FLAG_BOOLEAN = "n"
const  SB_FLAG_STRING = "s"

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

    static headerSizeBytes(){
        return Uint8Array.BYTES_PER_ELEMENT +
               Uint16Array.BYTES_PER_ELEMENT +
               BigUint64Array.BYTES_PER_ELEMENT;
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

    static interpretSizeFromHeader(uint8array,type){
        let meta_a = new Uint8Array(uint8array.slice(0,1).buffer);
        let elemspan_a = new Uint16Array(uint8array.slice(1,3).buffer);
        let span_a = new BigUint64Array(uint8array.slice(3,11).buffer);
        invert_endian(meta_a,1);
        invert_endian(span_a,8);
        invert_endian(elemspan_a,2);
        enforce_endian(meta_a, 1);
        enforce_endian(elemspan_a, 2);
        enforce_endian(span_a, 8);
        switch (type){
        default:
        case 0:
            return Number(span_a[0]);
            break;
        case 1:
            return elemspan_a[0]
            break;
        case 2:
            return meta_a[0]
            break;
        }
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

    copyTo(target){
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

function StringToUint8Array(str){
    return new TextEncoder().encode(str);
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

let _get_flag = (bloc) => {
    switch(bloc.meta) {
    case SB_META_BOOLEAN:
        return SB_FLAG_BOOLEAN;
        break;
    case SB_META_STRING:
        return SB_FLAG_STRING;
        break;
    case SB_META_INT_STYLE:{
        switch(bloc.element_span){
        case 1:
            return SB_FLAG_I8
            break;
        case 2:
            return SB_FLAG_I16
            break;
        case 4:
            return SB_FLAG_I32
            break;
        case 8:
            return SB_FLAG_I64
            break;
        default:
            return SB_FLAG_UNDEFINED
            break;
        }
        break;
    }
    case SB_META_FLOAT_STYLE: {
        switch (bloc.element_span) {
        case 4:
            return SB_FLAG_FLOAT;
            break;
        case 8:
            return SB_FLAG_DOUBLE;
            break;
        case 16:
            return SB_FLAG_LONG_DOUBLE;
            break;
        default:
            return SB_FLAG_UNDEFINED;
            break;
        }
        break;
    }
    default:
        return SB_FLAG_UNDEFINED;
        break;
    }
    return SB_FLAG_UNDEFINED
}

let getfloatarray = (bloc) => {
    switch(bloc.element_span){
    case 4:
        console.log("ff:", bloc.span,bloc.element_span,bloc.meta)
        return new Float32Array(bloc.contents_native.buffer.slice())
        break;
    case 8:
        return new Float64Array(bloc.contents_native.buffer.slice())
        break;
    case 16:
        return new Float64Array(bloc.contents_native.buffer.slice())
        break;
    default:
        console.log("bad block: ", bloc)
    }
}

let getintarray = (bloc) => {
    switch(bloc.element_span){
    default:
    case 1:
        return new Int8Array(bloc.contents_native.buffer.slice())
        break;
    case 2:
        return new Int16Array(bloc.contents_native.buffer.slice())
        break;
    case 4:
        return new Int32Array(bloc.contents_native.buffer.slice())
        break;
    case 8:
        return new BigInt64Array(bloc.contents_native.buffer.slice())
        break;
    }
}

let _value_string = (bloc) => {
    let d = _get_flag(bloc);
//    console.log(bloc.span,bloc.element_span,bloc.meta)
    switch(d){
    default:
    case SB_FLAG_UNDEFINED:
        d += COMPOUND_NODE_BEGIN_LIST_R
        d += COMPOUND_NODE_END_LIST_R
        break;
    case SB_FLAG_STRING:{
        d += COMPOUND_NODE_BEGIN_STRING_FLAG_R
        for(let num of bloc.contents_native){
            if(num == 0)
                break
            d += String.fromCharCode(num)
        }
        d += COMPOUND_NODE_END_STRING_FLAG_R
        break;
    }
    case SB_FLAG_BOOLEAN:{
        d += COMPOUND_NODE_BEGIN_LIST_R
        for(let num of bloc.contents_native){
            if(num == 0)
                d += "false"
            else
                d += "true"
            d += ","
        }
        if(d.slice(-1) == ",")
            d = d.slice(0,-1)
        d += COMPOUND_NODE_END_LIST_R
        break;
    }
    case SB_FLAG_I8:
    case SB_FLAG_I16:
    case SB_FLAG_I32:
    case SB_FLAG_I64:{
        d += COMPOUND_NODE_BEGIN_LIST_R
        let arr = getintarray(bloc);
        for(let integer of arr){
            d += Math.round(Number(integer)).toString()
            d += COMPOUND_NODE_VALUE_SEPERATOR_R
        }
        if(d.slice(-1) == ",")
            d = d.slice(0,-1)
        d += COMPOUND_NODE_END_LIST_R;
        break;
    }
    case SB_FLAG_FLOAT:
    case SB_FLAG_DOUBLE:
    case SB_FLAG_LONG_DOUBLE:{
        d += COMPOUND_NODE_BEGIN_LIST_R
        let arr = getfloatarray(bloc);
        for(let floating of arr){
            d += Number(floating).toString()
            d += COMPOUND_NODE_VALUE_SEPERATOR_R
        }
        if(d.slice(-1) == ",")
            d = d.slice(0,-1)
        d += COMPOUND_NODE_END_LIST_R;
        break;
    }
    }
    return d
}

let _add_escapes_to_string = (str) => {
    return str.replace(':', '\\:')
}

let _add_escapes_to_readable = (str) => {
    return str.replace('"', '\\"')
}

let base64ToArrayBuffer = (base64) => {
    var binaryString = atob(base64);
    var bytes = new Uint8Array(binaryString.length);
    for (var i = 0; i < binaryString.length; i++) {
        bytes[i] = binaryString.charCodeAt(i);
    }
    return bytes.buffer;
}

const BasicParserState = Object.freeze({
    AwaitBegin         : 0,
    AwaitKeyStart      : 1,
    ConstructKey       : 2,
    ConstructKeyEscape : 3,
    GetIndicator       : 4,
    AwaitNodeArrayNode : 5,
    AquireNodeHeader   : 6,
    AquireNodeData     : 7,
    Success            : 253,
    Error              : 254,
    Warning            : 255
})

class BasicPushdownParserData{
    node = new CompoundNode();
    currentState = BasicParserState.AwaitBegin;
    curstring = "";
    curKey = "";
    constructor(){};
    nodeData = [];
    nodeDataCounter = 0;
    nodeDataLeft = 0;
}

class BasicPushdownParser{
    stateStack = [];
    state = new BasicPushdownParserData();
    awaitCounter = 0;
    constructor(){};
    writeToNode(otherNode){
        this.state.node.copyTo(otherNode);
    };
    consume(c){

        switch(this.state.currentState){

        case BasicParserState.AwaitNodeArrayNode:{
            if(c == COMPOUND_NODE_END_LIST_FLAG){
                this.state.currentState = BasicParserState.AwaitKeyStart
                break;
            }
            if(c == COMPOUND_NODE_BEGIN_FLAG){
		this.stateStack.push(this.state);
		this.state = new BasicPushdownParserData();
                this.state.currentState = BasicParserState.AwaitKeyStart;
		break;
            }
            break;
        }

        case BasicParserState.AquireNodeData: {
            //console.log("data left: ", this.state.nodeDataLeft)
            this.state.nodeData.push(c);
            this.state.nodeDataLeft--;
            if(this.state.nodeDataLeft <= 0){
                let newBlock = new SizedBlock();
                try {
                    newBlock.upper(new Uint8Array(this.state.nodeData));
                } catch (e) {
                    this.state.currentState = BasicParserState.Error;
                    break;
                }
                this.state.node.generic_tags[this.state.curKey] = newBlock;
                this.state.currentState = BasicParserState.AwaitKeyStart;
                //console.log("upper'd node");
                break;
            }
            break;
        }

        case BasicParserState.AquireNodeHeader: {
            let counterval = this.state.nodeDataCounter;
            let headersize = SizedBlock.headerSizeBytes();
            this.state.nodeData.push(c)
            this.state.nodeDataCounter ++;
            if(counterval < headersize - 1)
                break;
            if(counterval == headersize - 1){ // if the parser is on the last header byte
                this.state.nodeDataLeft = SizedBlock.interpretSizeFromHeader(new Uint8Array(this.state.nodeData));
                //console.log("read header, data left: ", this.state.nodeDataLeft)
                if(this.state.nodeDataLeft == 0){
                    let newBlock = new SizedBlock();
                    try {
                        newBlock.upper(new Uint8Array(this.state.nodeData));
                    } catch (e) {
                        this.state.currentState = BasicParserState.Error;
                        break;
                    }
                    this.state.node.generic_tags[this.state.curKey] = newBlock;
                    this.state.currentState = BasicParserState.AwaitKeyStart;
                    //console.log("upper'd node");
                    break;
                }
                this.state.currentState = BasicParserState.AquireNodeData;
                break;
            }
            this.state.currentState = BasicParserState.Error;
            break;
        }

	case BasicParserState.GetIndicator: {
	    if(c == COMPOUND_NODE_BEGIN_BLOCK_FLAG){
                this.state.currentState = BasicParserState.AquireNodeHeader;
                this.state.nodeData = [];
                this.state.nodeDataCounter = 0;
                this.state.nodeDataLeft = 0;
		break;
	    }
	    if(c == COMPOUND_NODE_BEGIN_FLAG){
		this.state.currentState = BasicParserState.AwaitKeyStart;
		this.stateStack.push(this.state);
		this.state = new BasicPushdownParserData();
                this.state.currentState = BasicParserState.AwaitKeyStart;
		break;
	    }
	    if(c == COMPOUND_NODE_BEGIN_LIST_FLAG){
		this.state.currentState = BasicParserState.AwaitNodeArrayNode;
		break;
	    }
	    break;
	}

	case BasicParserState.ConstructKeyEscape:{
	    if(c == COMPOUND_NODE_BEGIN_ELEMENT_FLAG){
	        this.state.curstring += String.fromCharCode(c);
		this.state.currentState = BasicParserState.ConstructKey;
		break;
	    }
	    this.state.curstring += String.fromCharCode(COMPOUND_NODE_BEGIN_ELEMENT_FLAG);
            this.state.curstring += String.fromCharCode(c);
	    this.state.currentState = BasicParserState.ConstructKey;
	    break;
	}

	case BasicParserState.ConstructKey:{
	    if(c == COMPOUND_NODE_KEY_ESCAPE_FLAG){
		this.state.currentState = BasicParserState.ConstructKeyEscape;
		break;
	    }
	    if(c == COMPOUND_NODE_BEGIN_ELEMENT_FLAG){
		this.state.curKey = this.state.curstring;
		this.state.curstring = "";
		this.state.currentState = BasicParserState.GetIndicator;
		break;
	    }
            this.state.curstring += String.fromCharCode(c);
	    break;
	}
	    
	case BasicParserState.AwaitKeyStart:{
	    if(c == COMPOUND_NODE_BEGIN_STRING_FLAG){
		this.state.currentState = BasicParserState.ConstructKey;
		this.state.curstring = "";
		break;
	    }
	    if(c == COMPOUND_NODE_END_FLAG){
		if(this.stateStack.length == 0){
		    this.state.currentState = BasicParserState.Success;
		    break;
		}
		if(this.stateStack.length >= 0){
		    let topState = this.stateStack[this.stateStack.length-1].currentState;
		    let topKey = this.stateStack[this.stateStack.length-1].curKey;
		    if(topState == BasicParserState.AwaitKeyStart){
			this.stateStack[this.stateStack.length-1].node.put_node(topKey, this.state.node);
		    } else if (topState == BasicParserState.AwaitNodeArrayNode){
			this.stateStack[this.stateStack.length-1].node.put_back(topKey, this.state.node);
		    } else {
			this.state = BasicParserState.Error;
			break;
		    }
                    //console.log("popping from stack")
		    this.state = this.stateStack.pop();
		    break;
		}
		this.state.currentState = BasicParserState.Error;
	    }
	    break;
	}
	    
        case BasicParserState.AwaitBegin:{
            if(c == COMPOUND_NODE_BEGIN_FLAG){
                this.state.currentState = BasicParserState.AwaitKeyStart;
            }
            break;
        }
            
        default:{
            this.state.currentState = BasicParserState.Error;
            break;
        }
            
        }

	return this.state.currentState;
        
    }
}

let _is_ascii_whitespace = (character) => {
    let c = character.charCodeAt(0);
    return c == 32 || c == 9 || c == 10 || c == 13;
}

const ParserState = Object.freeze({
    AwaitStart                       : 0,
    AwaitKey                         : 1,
    ConstructKey                     : 2,
    ConstructKeyEscape               : 3,
    AwaitKeyValueSeperator           : 4,
    AwaitValueTypeIdentifier         : 5,
    AwaitValue                       : 6,
    ConstructValueStringEscape       : 7,
    ConstructValueString             : 8,
    AwaitValueParsable               : 9,
    ConstructValueParsable           : 10,
    AwaitValueParsableSeperator      : 11,
    ConstructNodeArrayAwaitNode      : 12,
    ConstructNodeArrayAwaitSeperator : 13,
    AwaitItemSeperator               : 14,
    Success                          : 253,
    Error                            : 254,
    Warning                          : 255,
})

class ReadableParserData{
    currentState = ParserState.AwaitStart
    nextState = ParserState.Error
    node = new CompoundNode()
    currentConstruction = ""
    currentValueType = 0
    currentKey = ""
    value_construction = []
    constructor(){}
}

class ReadablePushdownParser{
    state = new ReadableParserData();
    stateStack = [];
    constructor(){}
    
    consume(c){
        switch(this.state.currentState){

	 
	    
	case ParserState.ConstructNodeArrayAwaitNode: {
            if (c == COMPOUND_NODE_END_LIST_R && this.state.node.get_node_list_length(this.state.currentKey) == 0) {
		this.state.current_key = "";
		this.state.current_state = ParserState.AwaitItemSeperator;
		break;
            }
            if (c == COMPOUND_NODE_BEGIN_FLAG_R) {
		this.state.current_state = ParserState.ConstructNodeArrayAwaitSeperator;
		this.stateStack.push(this.state);
		this.state = new ReadableParserData();
		this.state.currentState = ParserState.AwaitKey;
		break;
            }
            if(!_is_ascii_whitespace(c))
		this.state.currentState = ParserState.Error;
            break;
	}
	    
        case ParserState.AwaitValueTypeIdentifier: {
            this.state.currentState = get_value_type_state(c);
            if (this.state.currentState == ParserState.Error)
                break;
            if (this.state.currentState == ParserState.AwaitValue)
                this.state.currentValueType = c;
            if (c == COMPOUND_NODE_BEGIN_FLAG_R) {
                this.state.currentState = ParserState.AwaitItemSeperator;
                this.stateStack.push(this.state);
                this.state = new ReadableParserData;
                this.state.currentState = ParserState.AwaitKey;
                break;
            }
            if (c == COMPOUND_NODE_BEGIN_LIST_R) {
                this.state.currentState = ParserState.ConstructNodeArrayAwaitNode;
                break;
            }
            break;
        }
            
        case ParserState.AwaitKeyValueSeperator: {
            if (c == COMPOUND_NODE_KV_SEPERATOR_R) {
                this.state.currentState = ParserState.AwaitValueTypeIdentifier;
                break;
            }
            if (!_is_ascii_whitespace(c))
                state.currentState = ParserState.Error;
            break;
        }

        case ParserState.ConstructKeyEscape: {
            if (c == COMPOUND_NODE_END_STRING_FLAG_R) {
                this.state.currentConstruction += c;
                this.state.currentState = ParserState.ConstructKey;
                break;
            }
            if (c == COMPOUND_NODE_ESCAPE_STRING_R) {
                this.state.currentConstruction += c;
                break;
            }
            this.state.currentConstruction += COMPOUND_NODE_ESCAPE_STRING_R;
            this.state.currentConstruction += c;
            this.state.currentState = ParserState.ConstructKey;
            break;
        }

        case ParserState.ConstructKey: {
            if(c == COMPOUND_NODE_END_STRING_FLAG_R) {
                this.state.currentState = ParserState.AwaitKeyValueSeperator;
                this.state.currentKey = this.state.current_construction;
                this.state.currentConstruction = "";
                break;
            }
            if(c == COMPOUND_NODE_ESCAPE_STRING_R) {
                this.state.currentState = ParserState.ConstructKeyEscape
                break;
            }
            this.state.currentConstruction += c;
            break;
        }
            
        case ParserState.AwaitKey:{
            if(c == COMPOUND_NODE_BEGIN_STRING_FLAG_R){
                this.state.currentState = ParserState.ConstructKey;
                break;
            }
            if(c == COMPOUND_NODE_END_FLAG_R && this.stateStack.length == 0 && this.state.node.empty()){
                this.state.currentState = ParserState.Success;
                break;
            }
            if(c == COMPOUND_NODE_END_FLAG_R && !this.stateStack.length==0 && this.state.node.empty()){
                if(this.stateStack[this.stateStack.length-1].currentState == ParserState.AwaitItemSeperator){
                    this.stateStack[this.stateStack.length-1].node.put_node(this.stateStack[this.stateStack.length-1].currentKey,this.state.node)
                } else if (this.stateStack[this.stateStack.length-1].currentState == ParserState.ConstructNodeArrayAwaitSeperator){
                    this.stateStack[this.stateStack.length-1].node.put_back(this.stateStack[this.stateStack.length-1].currentKet,this.state.node)
                } else {
                    this.state.currentState = Error
                }
                this.state = this.stateStack.pop()
                break;
            }
            if (!_is_ascii_whitespace(c))
                this.state.current_state = ParserState.Error;
            break;
        }

        case ParserState.AwaitStart:{
            if(c == COMPOUND_NODE_BEGIN_FLAG_R){
                this.state.currentState = ParserState.AwaitKey;
                break;
            }
            if(!_is_ascii_whitespace(c))
                this.state.currentState = ParserState.Error;
            break;
        }
            
        default:
            this.state.currentState = ParserState.Error;
            break;
        }
	return this.state.currentState;
    }
}

let get_value_type_state = (c) => {
    let state = ParserState.Error;
    if (c == SB_FLAG_UNDEFINED || c == SB_FLAG_I8 || c == SB_FLAG_I16 || c == SB_FLAG_I32 ||
        c == SB_FLAG_I64 || c == SB_FLAG_FLOAT || c == SB_FLAG_DOUBLE ||
        c == SB_FLAG_LONG_DOUBLE || c == SB_FLAG_BOOLEAN ||
        c == SB_FLAG_STRING || c == COMPOUND_NODE_BEGIN_FLAG_R || c == COMPOUND_NODE_BEGIN_LIST_R)
        state = ParserState.AwaitValue;
    if (_is_ascii_whitespace(c))
        state = ParserState.AwaitValueTypeIdentifier;
    return state;
}

let elem_size = (flag) => {
    switch(flag){
    case SB_FLAG_UNDEFINED:
        return 1
        break;
    case SB_FLAG_I8:
        return 1
        break;
    case SB_FLAG_I16:
        return 2
        break;
    case SB_FLAG_I32:
        return 4
        break;
    case SB_FLAG_I64:
        return 8
        break;
    case SB_FLAG_FLOAT:
        return 4
        break;
    case SB_FLAG_DOUBLE:
        return 8
        break;
    case SB_FLAG_LONG_DOUBLE:
        return 16
        break;
    case SB_FLAG_BOOLEAN:
        return 1
        break;
    case SB_FLAG_STRING:
        return 1
        break;
    }
    return -1
}

let _safe_iparse = (str,successcontainer) => {
    let num;
    num = parseInt(str)
    if(num == NaN)
        succescontainer["success"] = false
    succescontainer["success"] = true
    return num
}

let _safe_fparse = (str,successcontainer) => {
    let num;
    num = parseFloat(str)
    if(num == NaN)
        succescontainer["success"] = false
    succescontainer["success"] = true
    return num    
}

let parse_insert_generic = (noderef, key, array, parse_type) => {
    switch(parse_type){
    case SB_FLAG_UNDEFINED: {
        noderef.put(key, new Uint8Array(), 1, Uint8Array).assign_meta(SB_META_UNDEFINE);
        return true;
        break;
    }
    case SB_FLAG_I8: {
        let arr = []
        for(let str of array){
            let result = {}
            arr.push(_safe_iparse(str,result))
            if(result.success == false)
                return false
        }
        node.put(key,new Int8Array(arr), 1, Int8Array).assign_meta(SB_META_INT_STLE)
        return true
        break;
    }
    case SB_FLAG_I16: {
        let arr = []
        for(let str of array){
            let result = {}
            arr.push(_safe_iparse(str,result))
            if(result.success == false)
                return false
        }
        node.put(key,new Int16Array(arr), 2, Int16Array).assign_meta(SB_META_INT_STYLE)
        return true
        break;        
    }
    case SB_FLAG_I32: {
        let arr = []
        for(let str of array){
            let result = {}
            arr.push(_safe_iparse(str,result))
            if(result.success == false)
                return false
        }
        node.put(key,new Int32Array(arr), 4, Int32Array).assign_meta(SB_META_INT_STYLE)
        return true
        break;        
    }
    case SB_FLAG_I64: {
        let arr = []
        for(let str of array){
            let result = {}
            arr.push(_safe_iparse(str,result))
            if(result.success == false)
                return false
        }
        node.put(key,new Int32Array(arr), 8, Int64Array).assign_meta(SB_META_INT_STYLE)
        return true
        break;        
    }
    case SB_FLAG_FLOAT: {
        let arr = []
        for(let str of array){
            let result = {}
            arr.push(_safe_fparse(str,result))
            if(result.success == false)
                return false
        }
        node.put(key,new Float32Array(arr), 4, Float32Array).assign_meta(SB_META_FLOAT_STYLE)
        return true
        break;        
    }
    case SB_FLAG_DOUBLE: {
        let arr = []
        for(let str of array){
            let result = {}
            arr.push(_safe_fparse(str,result))
            if(result.success == false)
                return false
        }
        node.put(key,new Float64Array(arr), 8, Float64Array).assign_meta(SB_META_FLOAT_STYLE)
        return true
        break;        
    }
    case SB_FLAG_LONG_DOUBLE: {
        let arr = []
        for(let str of array){
            let result = {}
            arr.push(_safe_fparse(str,result))
            if(result.success == false)
                return false
        }
        node.put(key,new Float64Array(arr), 8, Float64Array).assign_meta(SB_META_FLOAT_STYLE)
        return true;
        break;        
    }
    case SB_FLAG_BOOLEAN: {
        let arr = []
        for(let str of array){
            if(str == "true"){
                arr.push(1)
            } else if (str == "false"){
                arr.push(0)
            } else {
                return false
            }
        }
        node.put(key,new Uint8Array(arr),Uint8Array).assign_meta(SB_META_BOOLEAN)
        return true;
        break;
    }
    default:
        break;
    }
    return false
}

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
            node_or_array.copyTo(ncn);
            this.child_nodes[key] = ncn;
            return;
        }
        child_node_lists[key] = []
        for (let node in node_or_array){
            let ncn = new CompoundNode();
            code.copyTo(ncn);
            child_node_lists[key].push(node_or_array);
        }
        return node_or_array
    }
    put_back(key, anode){
        if(this.child_node_lists[key] == undefined)
            this.child_node_lists[key] = [];
        let ncn = new CompoundNode();
        anode.copyTo(ncn);
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

    empty(){
	return Object.keys(this.generic_tags).length == 0 && Object.keys(this.child_nodes).length == 0 && Object.keys(this.child_node_lists).length == 0;
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
    get_node_list_length(key){
	if(this.child_node_lists[key] == undefined)
	    this.child_node_lists[key] = []
	return this.child_node_lists[key].length
    }

    copyTo(target){
        target.destroy_children()
        for(let key in this.generic_tags){
            let nsb = new SizedBlock();
            this.generic_tags[key].copyTo(nsb);
            target.generic_tags[key] = nsb;
        }
        for(let key in this.child_nodes){
            let ncn = new CompoundNode();
            this.child_nodes[key].copyTo(ncn);
            target.put_node("key",ncn);
        }
        for(let key in this.child_node_lists){
            let list = this.child_node_lists[key];
            for(let lnode in list){
                let ncn = new CompoundNode();
                list[lnode].copyTo(ncn);
                target.put_back(key,ncn)
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
    
    serialize_readable(){
        let serialization = COMPOUND_NODE_BEGIN_FLAG_R
        let loop = 0;
        for(const key in this.generic_tags){
            serialization += COMPOUND_NODE_BEGIN_STRING_FLAG_R
            serialization += _add_escapes_to_readable(key)
            serialization += COMPOUND_NODE_END_STRING_FLAG_R
            serialization += COMPOUND_NODE_KV_SEPERATOR_R
            serialization += _value_string(this.generic_tags[key])
            if(++loop < Object.keys(this.generic_tags).length || !Object.keys(this.child_nodes).length==0 || !Object.keys(this.child_node_lists).length==0)
                serialization += COMPOUND_NODE_VALUE_SEPERATOR_R
        }
        loop = 0
        for(const key in this.child_nodes){
            serialization += COMPOUND_NODE_BEGIN_STRING_FLAG_R
            serialization += _add_escapes_to_readable(key)
            serialization += COMPOUND_NODE_END_STRING_FLAG_R
            serialization += COMPOUND_NODE_KV_SEPERATOR_R
            serialization += this.child_nodes[key].serialize_readable()
            if(++loop < Object.keys(this.child_nodes).length || !Object.keys(this.child_node_lists).length==0)
                serialization += COMPOUND_NODE_VALUE_SEPERATOR_R
        }
        loop = 0
        for(const key in this.child_node_lists){
            serialization += COMPOUND_NODE_BEGIN_STRING_FLAG_R
            serialization += _add_escapes_to_readable(key)
            serialization += COMPOUND_NODE_END_STRING_FLAG_R
            serialization += COMPOUND_NODE_KV_SEPERATOR_R
            serialization += COMPOUND_NODE_BEGIN_LIST_R
            let loop2 = 0
            for(const node of this.child_node_lists[key]){
                serialization += node.serialize_readable();
                if(++loop2 < this.child_node_lists[key].length)
                    serialization += COMPOUND_NODE_VALUE_SEPERATOR_R
            }
            serialization += COMPOUND_NODE_END_LIST_R           
            if(++loop < Object.keys(this.child_node_lists).length)
                serialization += COMPOUND_NODE_VALUE_SEPERATOR_R
        }
        serialization += COMPOUND_NODE_END_FLAG_R
        return serialization
    }

    deserialize(content){
        let parser = new BasicPushdownParser();
        for(let i = 0; i < content.length; i++){
            state = parser.consume(content[i])
            if(state == BasicParserState.Error)
                break;
            if(state == BasicParserState.Warning)
                parser.state.currentState = parser.state.nextState
        }
        if(state != BasicParserState.Success)
            return false;
        this.destroy_children()
        parser.state.node.copyTo(this)
        return true;
    }
    deserialize_decode(content){
        let str = content;
        let arrb = new Uint8Array(base64ToArrayBuffer(content))
        this.deserialize(arrb)
    }
    deserialize_readable(content){}

    destroy_children(){
        this.generic_tags = {};
        this.child_nodes = {};
        this.child_node_lists = {};
    }
}
