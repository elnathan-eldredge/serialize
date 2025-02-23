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
    element_span;
    span;
    meta;

    constructor(element_size, typedarray){
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
        invert_endian(span_a,8);
        enforce_endian(meta_a,1);
        enforce_endian(elemspan_a,2);
        enforce_endian(span_a,8);
        enforce_endian(contents_little,this.element_span);
        let result = new Uint8Array(meta_a.byteLength
                                    + elemspan_a.byteLength
                                    + span_a.byteLength
                                    + contents_little.byteLength);
        result.set(new Uint8Array(elemspan_a.buffer), meta_a.byteLength);
        result.set(new Uint8Array(span_a.buffer), meta_a.byteLength + elemspan_a.byteLength);
        result.set(contents_little, meta_a.byteLength + elemspan_a.byteLength + span_a.byteLength);
        return result;
    }

    upper(uint8array, startindex){
        let errcannot = new Error("SizedBlock.upper: error deserializing")
        if(uint8array.byteLength <= 11)
            throw errcannot;
    }

    dump(){};

    copy_to(target){};
    
    assign_meta(meta){};

    isdumped(){return this.span==0};
   
}

