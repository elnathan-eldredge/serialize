let serialize = {

    NORESERVE: true,

    is_big_endian: () => {
        let uInt32 = new Uint32Array([0x01000000]);
        let uInt8 = new Uint8Array(uInt32.buffer);
        return uInt8[0] == 0x01;
    },

    convert_typed_array: (array, newtype) => {
        return new newtype(array.buffer.slice());
    },
    
    invert_endian: (array, unit_size) => {
        if(array.byteLength % unit_size != 0)
            throw new Error("serialize.invert_endian: array bytelength " + array.byteLength.toString()
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
    },

    enforce_endian: (array, unit_size_optional) => {
        if(serialize.is_big_endian)
            serialize.invert_endian(array,unit_size_optional==undefined?array.BYTES_PER_ELEMENT:unit_size_optional);
        return array
    },
    //left off
    
    COMPOUND_NODE_BEGIN_FLAG : 123,
    COMPOUND_NODE_END_FLAG : 125,
    COMPOUND_NODE_KEY_ESCAPE_FLAG : 92,
    COMPOUND_NODE_BEGIN_STRING_FLAG  : 44,
    COMPOUND_NODE_BEGIN_ELEMENT_FLAG  : 58,
    COMPOUND_NODE_BEGIN_ELEMENT_FLAG_S : ':',
    COMPOUND_NODE_BEGIN_BLOCK_FLAG  : 45,
    COMPOUND_NODE_BEGIN_LIST_FLAG  : 91,
    COMPOUND_NODE_END_LIST_FLAG  : 93,

    SB_META_UNDEFINED : 0,
    SB_META_INT_STYLE : 1,
    SB_META_FLOAT_STYLE : 9,
    SB_META_BOOLEAN : 11,
    SB_META_STRING : 12,
    SB_META_MAX : 127,

    COMPOUND_NODE_BEGIN_FLAG_R : "{",
    COMPOUND_NODE_ESCAPE_STRING_R : "\\",
    COMPOUND_NODE_BEGIN_STRING_FLAG_R : "\"",
    COMPOUND_NODE_END_STRING_FLAG_R : "\"",
    COMPOUND_NODE_KV_SEPERATOR_R : ":",
    COMPOUND_NODE_BEGIN_LIST_R : "[",
    COMPOUND_NODE_END_LIST_R : "]",
    COMPOUND_NODE_VALUE_SEPERATOR_R : ",",
    COMPOUND_NODE_END_FLAG_R : "}",

    SB_FLAG_UNDEFINED : "x",
    SB_FLAG_I8 : "b",
    SB_FLAG_I16 : "m",
    SB_FLAG_I32 : "i",
    SB_FLAG_I64 : "l",
    SB_FLAG_FLOAT : "f",
    SB_FLAG_DOUBLE : "d",
    SB_FLAG_LONG_DOUBLE : "q",
    SB_FLAG_BOOLEAN : "n",
    SB_FLAG_STRING : "s",

    /*vector*/

    uVector : class{
        constructor(data){
            this.data = data==undefined?new Uint8Array():new Uint8Array(data.buffer.slice());
            this.size = data==undefined?0:data.byteLength;
        }
        reserve(num){
            if(typeof(num) != "number")
                throw new Error("serialize.uVector: reserve: must be number")
            let arr = new Uint8Array(num);
            arr.set(this.data.slice(0,Math.min(this.data.byteLength,num)))
            this.data = arr;
        }
        push_back(uint){
            if(typeof(uint) != "number")
                throw new Error("serialize.uVector: reserve: must be number")
            if(this.size + 1 > this.data.byteLength)
                this.reserve(this.data.byteLength*2+1)
            this.data[this.size] = uint;
            this.size++;
        }
        insert_back(arr){
            if(!ArrayBuffer.isView(arr))
                throw new Error("serialize.uVector: insert: must be typed array");
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
    },

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

    SizedBlock : class{
        contents_native = null;
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
            serialize.invert_endian(meta_a,1);
            serialize.invert_endian(span_a,8);
            serialize.invert_endian(elemspan_a,2);
            serialize.invert_endian(contents_little,this.element_span);
            serialize.enforce_endian(meta_a,1);
            serialize.enforce_endian(elemspan_a,2);
            serialize.enforce_endian(span_a,8);
            serialize.enforce_endian(contents_little,this.element_span);
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
            serialize.invert_endian(meta_a,1);
            serialize.invert_endian(span_a,8);
            serialize.invert_endian(elemspan_a,2);
            serialize.enforce_endian(meta_a, 1);
            serialize.enforce_endian(elemspan_a, 2);
            serialize.enforce_endian(span_a, 8);
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
            let errcannot = new Error("serialize.SizedBlock.upper: error deserializing")
            if(uint8array.byteLength < 11){
                throw errcannot;
            }
            let meta_a = new Uint8Array(uint8array.slice(startindex,startindex+1).buffer);
            let elemspan_a = new Uint16Array(uint8array.slice(startindex+1,startindex+3).buffer);
            let span_a = new BigUint64Array(uint8array.slice(startindex+3,startindex+11).buffer);
            serialize.invert_endian(meta_a,1);
            serialize.invert_endian(span_a,8);
            serialize.invert_endian(elemspan_a,2);
            serialize.enforce_endian(meta_a, 1);
            serialize.enforce_endian(elemspan_a, 2);
            serialize.enforce_endian(span_a, 8);
            this.meta = meta_a[0];
            this.element_span = elemspan_a[0];
            this.span = Number(span_a[0]);
            if(this.span > (uint8array.bytelength - 11 - startindex)){
                dump();
                throw errcannot;
            }
            let contents_little = uint8array.slice(startindex + 11);
            serialize.enforce_endian(contents_little);
            this.contents_native = contents_little;
        }

        dump(){
            if(this.isdumped())
                return;
            this.contents_native = null;
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

        isdumped(){return this.contents_native==null};
        
    },

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
    ArrayBufferToString : (buffer) => {
        return serialize.BinaryToString(String.fromCharCode.apply(null, Array.prototype.slice.apply(new Uint8Array(buffer))));
    },

    StringToArrayBuffer : (string) => {
        return serialize.StringToUint8Array(string).buffer;
    },

    StringToUint8Array : (str) => {
        return new TextEncoder().encode(str);
    },

    BinaryToString : (binary) => {
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
    },

    _get_flag : (bloc) => {
        switch(bloc.meta) {
        case serialize.SB_META_BOOLEAN:
            return serialize.SB_FLAG_BOOLEAN;
            break;
        case serialize.SB_META_STRING:
            return serialize.SB_FLAG_STRING;
            break;
        case serialize.SB_META_INT_STYLE:{
            switch(bloc.element_span){
            case 1:
                return serialize.SB_FLAG_I8
                break;
            case 2:
                return serialize.SB_FLAG_I16
                break;
            case 4:
                return serialize.SB_FLAG_I32
                break;
            case 8:
                return serialize.SB_FLAG_I64
                break;
            default:
                return serialize.SB_FLAG_UNDEFINED
                break;
            }
            break;
        }
        case serialize.SB_META_FLOAT_STYLE: {
            switch (bloc.element_span) {
            case 4:
                return serialize.SB_FLAG_FLOAT;
                break;
            case 8:
                return serialize.SB_FLAG_DOUBLE;
                break;
            case 16:
                return serialize.SB_FLAG_LONG_DOUBLE;
                break;
            default:
                return serialize.SB_FLAG_UNDEFINED;
                break;
            }
            break;
        }
        default:
            return serialize.SB_FLAG_UNDEFINED;
            break;
        }
        return serialize.SB_FLAG_UNDEFINED
    },

    getfloatarray : (bloc) => {
        switch(bloc.element_span){
        case 4:
            //        console.log("ff:", bloc.span,bloc.element_span,bloc.meta)
            return new Float32Array(bloc.contents_native.buffer.slice())
            break;
        case 8:
            return new Float64Array(bloc.contents_native.buffer.slice())
            break;
        case 16:
            return new Float64Array(bloc.contents_native.buffer.slice())
            break;
        default:
            //        console.log("bad block: ", bloc)
        }
    },

    getintarray : (bloc) => {
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
    },

    _value_string : (bloc) => {
        let d = serialize._get_flag(bloc);
        //    console.log(bloc.span,bloc.element_span,bloc.meta)
        switch(d){
        default:
        case serialize.SB_FLAG_UNDEFINED:
            d += serialize.COMPOUND_NODE_BEGIN_LIST_R
            d += serialize.COMPOUND_NODE_END_LIST_R
            break;
        case serialize.SB_FLAG_STRING:{
            d += serialize.COMPOUND_NODE_BEGIN_STRING_FLAG_R
            for(let num of bloc.contents_native){
                if(num == 0)
                    break
                d += String.fromCharCode(num)
            }
            d += serialize.COMPOUND_NODE_END_STRING_FLAG_R
            break;
        }
        case serialize.SB_FLAG_BOOLEAN:{
            d += serialize.COMPOUND_NODE_BEGIN_LIST_R
            for(let num of bloc.contents_native){
                if(num == 0)
                    d += "false"
                else
                    d += "true"
                d += ","
            }
            if(d.slice(-1) == ",")
                d = d.slice(0,-1)
            d += serialize.COMPOUND_NODE_END_LIST_R
            break;
        }
        case serialize.SB_FLAG_I8:
        case serialize.SB_FLAG_I16:
        case serialize.SB_FLAG_I32:
        case serialize.SB_FLAG_I64:{
            d += serialize.COMPOUND_NODE_BEGIN_LIST_R
            let arr = serialize.getintarray(bloc);
            for(let integer of arr){
                d += Math.round(Number(integer)).toString()
                d += serialize.COMPOUND_NODE_VALUE_SEPERATOR_R
            }
            if(d.slice(-1) == ",")
                d = d.slice(0,-1)
            d += serialize.COMPOUND_NODE_END_LIST_R;
            break;
        }
        case serialize.SB_FLAG_FLOAT:
        case serialize.SB_FLAG_DOUBLE:
        case serialize.SB_FLAG_LONG_DOUBLE:{
            d += serialize.COMPOUND_NODE_BEGIN_LIST_R
            let arr = serialize.getfloatarray(bloc);
            for(let floating of arr){
                d += Number(floating).toString()
                d += serialize.COMPOUND_NODE_VALUE_SEPERATOR_R
            }
            if(d.slice(-1) == ",")
                d = d.slice(0,-1)
            d += serialize.COMPOUND_NODE_END_LIST_R;
            break;
        }
        }
        return d
    },

    _add_escapes_to_string : (str) => {
        return str.replace(':', '\\:')
    },

    _add_escapes_to_readable : (str) => {
        return str.replace('"', '\\"')
    },

    base64ToArrayBuffer : (base64) => {
        var binaryString = atob(base64);
        var bytes = new Uint8Array(binaryString.length);
        for (var i = 0; i < binaryString.length; i++) {
            bytes[i] = binaryString.charCodeAt(i);
        }
        return bytes.buffer;
    },

    BasicParserState : Object.freeze({
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
    }),

    BasicPushdownParserData : class{
        node = new serialize.CompoundNode();
        currentState = serialize.BasicParserState.AwaitBegin;
        curstring = "";
        curKey = "";
        constructor(){};
        nodeData = [];
        nodeDataCounter = 0;
        nodeDataLeft = 0;
    },

    BasicPushdownParser : class{
        stateStack = [];
        state = new serialize.BasicPushdownParserData();
        awaitCounter = 0;
        constructor(){};
        writeToNode(otherNode){
            this.state.node.merge_to(otherNode);
        };
        consume(c){

            switch(this.state.currentState){

            case serialize.BasicParserState.AwaitNodeArrayNode:{
                if(c == serialize.COMPOUND_NODE_END_LIST_FLAG){
                    this.state.currentState = serialize.BasicParserState.AwaitKeyStart
                    break;
                }
                if(c == serialize.COMPOUND_NODE_BEGIN_FLAG){
		    this.stateStack.push(this.state);
		    this.state = new serialize.BasicPushdownParserData();
                    this.state.currentState = serialize.BasicParserState.AwaitKeyStart;
		    break;
                }
                break;
            }

            case serialize.BasicParserState.AquireNodeData: {
                //console.log("data left: ", this.state.nodeDataLeft)
                this.state.nodeData.push(c);
                this.state.nodeDataLeft--;
                if(this.state.nodeDataLeft <= 0){
                    let newBlock = new serialize.SizedBlock();
                    try {
                        newBlock.upper(new Uint8Array(this.state.nodeData));
                    } catch (e) {
                        this.state.currentState = serialize.BasicParserState.Error;
                        break;
                    }
                    this.state.node.generic_tags[this.state.curKey] = newBlock;
                    this.state.currentState = serialize.BasicParserState.AwaitKeyStart;
                    //console.log("upper'd node");
                    break;
                }
                break;
            }

            case serialize.BasicParserState.AquireNodeHeader: {
                let counterval = this.state.nodeDataCounter;
                let headersize = serialize.SizedBlock.headerSizeBytes();
                this.state.nodeData.push(c)
                this.state.nodeDataCounter ++;
                if(counterval < headersize - 1)
                    break;
                if(counterval == headersize - 1){ // if the parser is on the last header byte
                    this.state.nodeDataLeft = serialize.SizedBlock.interpretSizeFromHeader(new Uint8Array(this.state.nodeData));
                    //console.log("read header, data left: ", this.state.nodeDataLeft)
                    if(this.state.nodeDataLeft == 0){
                        let newBlock = new serialize.SizedBlock();
                        try {
                            newBlock.upper(new Uint8Array(this.state.nodeData));
                        } catch (e) {
                            this.state.currentState = serialize.BasicParserState.Error;
                            break;
                        }
                        this.state.node.generic_tags[this.state.curKey] = newBlock;
                        this.state.currentState = serialize.BasicParserState.AwaitKeyStart;
                        //console.log("upper'd node");
                        break;
                    }
                    this.state.currentState = serialize.BasicParserState.AquireNodeData;
                    break;
                }
                this.state.currentState = serialize.BasicParserState.Error;
                break;
            }

	    case serialize.BasicParserState.GetIndicator: {
	        if(c == serialize.COMPOUND_NODE_BEGIN_BLOCK_FLAG){
                    this.state.currentState = serialize.BasicParserState.AquireNodeHeader;
                    this.state.nodeData = [];
                    this.state.nodeDataCounter = 0;
                    this.state.nodeDataLeft = 0;
		    break;
	        }
	        if(c == serialize.COMPOUND_NODE_BEGIN_FLAG){
		    this.state.currentState = serialize.BasicParserState.AwaitKeyStart;
		    this.stateStack.push(this.state);
		    this.state = new serialize.BasicPushdownParserData();
                    this.state.currentState = serialize.BasicParserState.AwaitKeyStart;
		    break;
	        }
	        if(c == serialize.COMPOUND_NODE_BEGIN_LIST_FLAG){
		    this.state.currentState = serialize.BasicParserState.AwaitNodeArrayNode;
		    break;
	        }
	        break;
	    }

	    case serialize.BasicParserState.ConstructKeyEscape:{
	        if(c == serialize.COMPOUND_NODE_BEGIN_ELEMENT_FLAG){
	            this.state.curstring += String.fromCharCode(c);
		    this.state.currentState = serialize.BasicParserState.ConstructKey;
		    break;
	        }
	        this.state.curstring += String.fromCharCode(serialize.COMPOUND_NODE_BEGIN_ELEMENT_FLAG);
                this.state.curstring += String.fromCharCode(c);
	        this.state.currentState = serialize.BasicParserState.ConstructKey;
	        break;
	    }

	    case serialize.BasicParserState.ConstructKey:{
	        if(c == serialize.COMPOUND_NODE_KEY_ESCAPE_FLAG){
		    this.state.currentState = serialize.BasicParserState.ConstructKeyEscape;
		    break;
	        }
	        if(c == serialize.COMPOUND_NODE_BEGIN_ELEMENT_FLAG){
		    this.state.curKey = this.state.curstring;
		    this.state.curstring = "";
		    this.state.currentState = serialize.BasicParserState.GetIndicator;
		    break;
	        }
                this.state.curstring += String.fromCharCode(c);
	        break;
	    }
	        
	    case serialize.BasicParserState.AwaitKeyStart:{
	        if(c == serialize.COMPOUND_NODE_BEGIN_STRING_FLAG){
		    this.state.currentState = serialize.BasicParserState.ConstructKey;
		    this.state.curstring = "";
		    break;
	        }
	        if(c == serialize.COMPOUND_NODE_END_FLAG){
		    if(this.stateStack.length == 0){
		        this.state.currentState = serialize.BasicParserState.Success;
		        break;
		    }
		    if(this.stateStack.length >= 0){
		        let topState = this.stateStack[this.stateStack.length-1].currentState;
		        let topKey = this.stateStack[this.stateStack.length-1].curKey;
		        if(topState == serialize.BasicParserState.AwaitKeyStart){
			    this.stateStack[this.stateStack.length-1].node.put_node(topKey, this.state.node);
		        } else if (topState == serialize.BasicParserState.AwaitNodeArrayNode){
			    this.stateStack[this.stateStack.length-1].node.put_back(topKey, this.state.node);
		        } else {
			    this.state = serialize.BasicParserState.Error;
			    break;
		        }
                        //console.log("popping from stack")
		        this.state = this.stateStack.pop();
		        break;
		    }
		    this.state.currentState = serialize.BasicParserState.Error;
	        }
	        break;
	    }
	        
            case serialize.BasicParserState.AwaitBegin:{
                if(c == serialize.COMPOUND_NODE_BEGIN_FLAG){
                    this.state.currentState = serialize.BasicParserState.AwaitKeyStart;
                }
                break;
            }
                
            default:{
                this.state.currentState = serialize.BasicParserState.Error;
                break;
            }
                
            }

	    return this.state.currentState;
            
        }
    },

    _is_ascii_whitespace : (character) => {
        let c = character.charCodeAt(0);
        return c == 32 || c == 9 || c == 10 || c == 13;
    },

    ParserState : Object.freeze({
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
    }), //left off

    ReadableParserData : class {
        currentState = serialize.ParserState.AwaitStart
        nextState = serialize.ParserState.Error
        node = new serialize.CompoundNode()
        currentConstruction = ""
        currentValueType = 0
        currentKey = ""
        valueConstruction = []
        constructor(){}
    },

    ReadablePushdownParser: class{
        state = new serialize.ReadableParserData();
        stateStack = [];
        constructor(){}
        
        consume(c){
            switch(this.state.currentState){

            case serialize.ParserState.Warning: {
                this.state.currentState = serialize.ParserState.Error;
            }
                
            case serialize.ParserState.AwaitValueParsableSeperator: {
                if (c == serialize.COMPOUND_NODE_ITEM_SEPERATOR_R) {
                    this.state.currentState = serialize.ParserState.AwaitValueParsable;
                    break;
                }
                if (c == serialize.COMPOUND_NODE_END_ARRAY_R) {
                    this.state.valueConstructions.push(this.state.currentConstruction);
                    this.state.currentConstruction = "";
                    if (!serialize.parse_insert_generic(this.state.node, this.state.currentKey,
                                                        this.state.valueConstructions,
                                                        this.state.currentValueType)) {
                        this.state.currentState = serialize.ParserState.Error;
                        this.state.valueConstruction = [];
                        break;
                    }
                    this.state.currentState = serialize.ParserState.AwaitItemSeperator;
                    this.state.valueConstructions = [];
                    break;
                }
                if (!serialize._is_ascii_whitespace(c)) {
                    state.current_state = Error;
                    break;
                }
                break;
            }

            case serialize.ParserState.ConstructValueParsable: {
                if (c == serialize.COMPOUND_NODE_END_LIST_R) {
                    this.state.valueConstruction.push(this.state.currentConstruction);
                    this.state.currentConstruction = "";
                    if (!serialize.parse_insert_generic(this.state.node, this.state.currentKey,
                                                        this.state.valueConstruction,
                                                        this.state.currentValueType)) {
                        this.state.currentState = serialize.ParserState.Error;
                        this.state.valueConstruction = []
                        break;
                    }
                    this.state.currentState = serialize.ParserState.AwaitItemSeperator;
                    this.state.valueConstruction = []
                    break;
                }
                if (serialize._is_ascii_whitespace(c)) {
                    this.state.valueConstructions.push(this.state.currentConstruction);
                    this.state.currentConstruction = "";
                    this.state.currentState = serialize.ParserState.AwaitValueParsableSeperator;
                    break;
                }
                if (c == serialize.COMPOUND_NODE_VALUE_SEPERATOR_R) {
                    this.state.valueConstruction.push(this.state.currentConstruction);
                    this.state.currentConstruction = "";
                    this.state.currentState = serialize.ParserState.AwaitValueParsable;
                    break;
                }
                this.state.currentConstruction += c;
                break;
            }
                
            case serialize.ParserState.AwaitValueParsable: {
                if (c == serialize.COMPOUND_NODE_END_LIST_R) {
                    if (this.state.valueConstruction.length != 0) {
                        this.state.currentState = serialize.ParserState.Error;
                        break;
                    }
                    if (!serialize.parse_insert_generic(this.state.node, this.state.currentKey, [],
                                                        this.state.currentValueType)) {
                        this.state.currentState = serialize.ParserState.Error;
                    } else {
                        this.state.currentState = serialize.ParserState.AwaitItemSeperator;
                    }
                    this.state.currentState = serialize.ParserState.AwaitItemSeperator;
                    break;
                }
                if (c == serialize.COMPOUND_NODE_VALUE_SEPERATOR_R) {
                    this.state.currentState = serialize.ParserState.Error;
                    break;
                }
                if (!serialize._is_ascii_whitespace(c)) {
                    this.state.currentState = serialize.ParserState.ConstructValueParsable;
                    this.state.currentConstruction += c;
                }
                break;
            }
                
            case serialize.ParserState.ConstructValueString: {
                if (c == serialize.COMPOUND_NODE_ESCAPE_STRING_R) {
                    this.state.currentState = serialize.ParserState.ConstructValueStringEscape;
                    break;
                }
                if (c == serialize.COMPOUND_NODE_END_STRING_FLAG_R) {
                    this.state.node.put(this.state.currentKey, this.state.currentConstruction);
                    
                    this.state.currentKey = "";
                    this.state.currentConstruction = "";
                    this.state.currentState = serialize.ParserState.AwaitItemSeperator;
                    break;
                }
                this.state.currentConstruction += c;
                break;
            }

            case serialize.ParserState.ConstructValueStringEscape: {
                if (c == serialize.COMPOUND_NODE_END_STRING_FLAG_R) {
                    this.state.currentConstruction += c;
                    break;
                }
                if (c == serialize.COMPOUND_NODE_ESCAPE_STRING_R) {
                    this.state.currentConstruction += c;
                    break;
                }
                this.state.currentConstruction += serialize.COMPOUND_NODE_ESCAPE_STRING_R;
                this.state.currentConstruction += c;
                this.state.currentState = serialize.ParserState.ConstructValueString;
                break;
            }

            case serialize.ParserState.AwaitValue: {
                if (c == serialize.COMPOUND_NODE_BEGIN_LIST_R && this.state.currentValueType != serialize.SB_FLAG_STRING) {
                    this.state.currentState = serialize.ParserState.AwaitValueParsable;
                    break;
                }
                if (c == serialize.COMPOUND_NODE_BEGIN_STRING_FLAG_R && this.state.currentValueType == serialize.SB_FLAG_STRING) {
                    this.state.currentConstruction = "";
                    this.state.currentState = serialize.ParserState.ConstructValueString;
                    break;
                }
                if (!serialize._is_ascii_whitespace(c))
                    this.state.currentState = serialize.ParserState.Error;
                break;
            }

            case serialize.ParserState.AwaitItemSeperator: {
                if (c == serialize.COMPOUND_NODE_VALUE_SEPERATOR_R) {
                    this.state.currentState = serialize.ParserState.AwaitKey;
                    break;
                }
                if (c == serialize.COMPOUND_NODE_END_FLAG_R && this.stateStack.length == 0) {
                    this.state.currentState = serialize.ParserState.Success;
                    break;
                }
                if (c == serialize.COMPOUND_NODE_END_FLAG_R && !this.stateStack.length==0) {
                    if (this.stateStack[this.stateStack.length-1].currentState == serialize.ParserState.AwaitItemSeperator) {
                        this.stateStack[this.stateStack.length-1].node.put_node(this.stateStack[this.stateStack.length-1].currentKey, this.state.node);
                    } else if (this.stateStack[this.stateStack.length-1].currentState == serialize.ParserState.ConstructNodeArrayAwaitSeperator) {
                        this.stateStack[this.stateStack.length-1].node.put_back(this.stateStack[this.stateStack.length-1].currentKey, this.state.node);
                    }
                    this.state = this.stateStack.pop();
                    break;
                }
                if (!serialize._is_ascii_whitespace(c))
                    this.state.currentState = serialize.ParserState.Error;
                break;
            }
                
	    case serialize.ParserState.ConstructNodeArrayAwaitSeperator: { 
                if (c == serialize.COMPOUND_NODE_VALUE_SEPERATOR_R) {
                    this.state.currentState = serialize.ParserState.ConstructNodeArrayAwaitNode;
                    break;
                }
                if (c == serialize.COMPOUND_NODE_END_LIST_R) {
                    this.state.currentKey = "";
                    this.state.currentState = serialize.ParserState.AwaitItemSeperator;
                    break;
                }
                if (!serialize._is_ascii_whitespace(c))
                    this.state.currentState = Error;
                break;
            }
	        
	    case serialize.ParserState.ConstructNodeArrayAwaitNode: {
                if (c == serialize.COMPOUND_NODE_END_LIST_R && this.state.node.get_node_list_length(this.state.currentKey) == 0) {
		    this.state.currentKey = "";
		    this.state.currentState = serialize.ParserState.AwaitItemSeperator;
		    break;
                }
                if (c == serialize.COMPOUND_NODE_BEGIN_FLAG_R) {
		    this.state.currentState = serialize.ParserState.ConstructNodeArrayAwaitSeperator;
		    this.stateStack.push(this.state);
		    this.state = new serialize.ReadableParserData();
		    this.state.currentState = serialize.ParserState.AwaitKey;
		    break;
                }
                if(!serialize._is_ascii_whitespace(c))
		    this.state.currentState = serialize.ParserState.Error;
                break;
	    }
	        
            case serialize.ParserState.AwaitValueTypeIdentifier: {
                this.state.currentState = serialize.get_value_type_state(c);
                if (this.state.currentState == serialize.ParserState.Error)
                    break;
                if (this.state.currentState == serialize.ParserState.AwaitValue)
                    this.state.currentValueType = c;
                if (c == serialize.COMPOUND_NODE_BEGIN_FLAG_R) {
                    this.state.currentState = serialize.ParserState.AwaitItemSeperator;
                    this.stateStack.push(this.state);
                    this.state = new serialize.ReadableParserData();
                    this.state.currentState = serialize.ParserState.AwaitKey;
                    break;
                }
                if (c == serialize.COMPOUND_NODE_BEGIN_LIST_R) {
                    this.state.currentState = serialize.ParserState.ConstructNodeArrayAwaitNode;
                    break;
                }
                break;
            }
                
            case serialize.ParserState.AwaitKeyValueSeperator: {
                if (c == serialize.COMPOUND_NODE_KV_SEPERATOR_R) {
                    this.state.currentState = serialize.ParserState.AwaitValueTypeIdentifier;
                    break;
                }
                if (!serialize._is_ascii_whitespace(c))
                    state.currentState = serialize.ParserState.Error;
                break;
            }

            case serialize.ParserState.ConstructKeyEscape: {
                if (c == serialize.COMPOUND_NODE_END_STRING_FLAG_R) {
                    this.state.currentConstruction += c;
                    this.state.currentState = serialize.ParserState.ConstructKey;
                    break;
                }
                if (c == serialize.COMPOUND_NODE_ESCAPE_STRING_R) {
                    this.state.currentConstruction += c;
                    break;
                }
                this.state.currentConstruction += serialize.COMPOUND_NODE_ESCAPE_STRING_R;
                this.state.currentConstruction += c;
                this.state.currentState = serialize.ParserState.ConstructKey;
                break;
            }

            case serialize.ParserState.ConstructKey: {
                if(c == serialize.COMPOUND_NODE_END_STRING_FLAG_R) {
                    this.state.currentState = serialize.ParserState.AwaitKeyValueSeperator;
                    this.state.currentKey = this.state.currentConstruction;
                    this.state.currentConstruction = "";
                    break;
                }
                if(c == serialize.COMPOUND_NODE_ESCAPE_STRING_R) {
                    this.state.currentState = serialize.ParserState.ConstructKeyEscape
                    break;
                }
                this.state.currentConstruction += c;
                break;
            }
                
            case serialize.ParserState.AwaitKey:{
                if(c == serialize.COMPOUND_NODE_BEGIN_STRING_FLAG_R){
                    this.state.currentState = serialize.ParserState.ConstructKey;
                    break;
                }
                if(c == serialize.COMPOUND_NODE_END_FLAG_R && this.stateStack.length == 0 && this.state.node.empty()){
                    this.state.currentState = serialize.ParserState.Success;
                    break;
                }
                if(c == serialize.COMPOUND_NODE_END_FLAG_R && !this.stateStack.length==0 && this.state.node.empty()){
                    if(this.stateStack[this.stateStack.length-1].currentState == serialize.ParserState.AwaitItemSeperator){
                        this.stateStack[this.stateStack.length-1].node.put_node(this.stateStack[this.stateStack.length-1].currentKey,this.state.node)
                    } else if (this.stateStack[this.stateStack.length-1].currentState == serialize.ParserState.ConstructNodeArrayAwaitSeperator){
                        this.stateStack[this.stateStack.length-1].node.put_back(this.stateStack[this.stateStack.length-1].currentKet,this.state.node)
                    } else {
                        this.state.currentState = Error
                    }
                    this.state = this.stateStack.pop()
                    break;
                }
                if (!serialize._is_ascii_whitespace(c))
                    this.state.current_state = serialize.ParserState.Error;
                break;
            }

            case serialize.ParserState.AwaitStart:{
                if(c == serialize.COMPOUND_NODE_BEGIN_FLAG_R){
                    this.state.currentState = serialize.ParserState.AwaitKey;
                    break;
                }
                if(!serialize._is_ascii_whitespace(c))
                    this.state.currentState = serialize.ParserState.Error;
                break;
            }
                
            default:
                this.state.currentState = serialize.ParserState.Error;
                break;
            }
	    return this.state.currentState;
        }
    },

    get_value_type_state : (c) => {
        let state = serialize.ParserState.Error;
        if (c == serialize.SB_FLAG_UNDEFINED || c == serialize.SB_FLAG_I8 || c == serialize.SB_FLAG_I16 || c == serialize.SB_FLAG_I32 ||
            c == serialize.SB_FLAG_I64 || c == serialize.SB_FLAG_FLOAT || c == serialize.SB_FLAG_DOUBLE ||
            c == serialize.SB_FLAG_LONG_DOUBLE || c == serialize.SB_FLAG_BOOLEAN ||
            c == serialize.SB_FLAG_STRING || c == serialize.COMPOUND_NODE_BEGIN_FLAG_R || c == serialize.COMPOUND_NODE_BEGIN_LIST_R)
            state = serialize.ParserState.AwaitValue;
        if (serialize._is_ascii_whitespace(c))
            state = serialize.ParserState.AwaitValueTypeIdentifier;
        return state;
    },

    elem_size : (flag) => {
        switch(flag){
        case serialize.SB_FLAG_UNDEFINED:
            return 1
            break;
        case serialize.SB_FLAG_I8:
            return 1
            break;
        case serialize.SB_FLAG_I16:
            return 2
            break;
        case serialize.SB_FLAG_I32:
            return 4
            break;
        case serialize.SB_FLAG_I64:
            return 8
            break;
        case serialize.SB_FLAG_FLOAT:
            return 4
            break;
        case serialize.SB_FLAG_DOUBLE:
            return 8
            break;
        case serialize.SB_FLAG_LONG_DOUBLE:
            return 16
            break;
        case serialize.SB_FLAG_BOOLEAN:
            return 1
            break;
        case serialize.SB_FLAG_STRING:
            return 1
            break;
        }
        return -1
    },

    _safe_iparse : (str,successcontainer) => {
        let num;
        num = parseInt(str)
        if(num == NaN)
            successcontainer["success"] = false
        successcontainer["success"] = true
        return num
    },

    _safe_biparse : (str,successcontainer) => {
        let num;
        try{
            num = BigInt(str)
        } catch(e) {
            successcontainer["success"] = false
            return;
        }
        successcontainer["success"] = true
        return num
    },

    _safe_fparse : (str,successcontainer) => {
        let num;
        num = parseFloat(str)
        if(num == NaN)
            successcontainer["success"] = false
        successcontainer["success"] = true
        return num    
    },

    parse_insert_generic : (noderef, key, array, parse_type) => {
        switch(parse_type){
        case serialize.SB_FLAG_UNDEFINED: {
            noderef.put(key, new Uint8Array(), 1, Uint8Array).assign_meta(serialize.SB_META_UNDEFINED);
            return true;
            break;
        }
        case serialize.SB_FLAG_I8: {
            let arr = []
            for(let str of array){
                let result = {}
                arr.push(serialize._safe_iparse(str,result))
                if(result.success == false)
                    return false
            }
            noderef.put(key,new Int8Array(arr), 1, Int8Array).assign_meta(serialize.SB_META_INT_STYLE)
            return true
            break;
        }
        case serialize.SB_FLAG_I16: {
            let arr = []
            for(let str of array){
                let result = {}
                arr.push(serialize._safe_iparse(str,result))
                if(result.success == false)
                    return false
            }
            noderef.put(key,new Int16Array(arr), 2, Int16Array).assign_meta(serialize.SB_META_INT_STYLE)
            return true
            break;        
        }
        case serialize.SB_FLAG_I32: {
            let arr = []
            for(let str of array){
                let result = {}
                arr.push(serialize._safe_iparse(str,result))
                if(result.success == false)
                    return false
            }
            noderef.put(key,new Int32Array(arr), 4, Int32Array).assign_meta(serialize.SB_META_INT_STYLE)
            return true
            break;        
        }
        case serialize.SB_FLAG_I64: {
            let arr = []
            for(let str of array){
                let result = {}
                arr.push(serialize._safe_biparse(str,result))
                if(result.success == false)
                    return false
            }
            noderef.put(key,new BigInt64Array(arr), 8, BigInt64Array).assign_meta(serialize.SB_META_INT_STYLE)
            return true
            break;        
        }
        case serialize.SB_FLAG_FLOAT: {
            let arr = []
            for(let str of array){
                let result = {}
                arr.push(serialize._safe_iparse(str,result))
                if(result.success == false)
                    return false
            }
            noderef.put(key,new Float32Array(arr), 4, Float32Array).assign_meta(serialize.SB_META_FLOAT_STYLE)
            return true
            break;        
        }
        case serialize.SB_FLAG_DOUBLE: {
            let arr = []
            for(let str of array){
                let result = {}
                arr.push(serialize._safe_iparse(str,result))
                if(result.success == false)
                    return false
            }
            noderef.put(key,new Float64Array(arr), 8, Float64Array).assign_meta(serialize.SB_META_FLOAT_STYLE)
            return true
            break;        
        }
        case serialize.SB_FLAG_LONG_DOUBLE: {
            let arr = []
            for(let str of array){
                let result = {}
                arr.push(serialize._safe_iparse(str,result))
                if(result.success == false)
                    return false
            }
            noderef.put(key,new Float64Array(arr), 8, Float64Array).assign_meta(serialize.SB_META_FLOAT_STYLE)
            return true;
            break;        
        }
        case serialize.SB_FLAG_BOOLEAN: {
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
            noderef.put(key,new Uint8Array(arr),Uint8Array).assign_meta(serialize.SB_META_BOOLEAN)
            return true;
            break;
        }
        default:
            break;
        }
        return false
    },

    // put(string key, array_or_number value, int bytesperelement, class arraytype)
    // put_node(string key, node/array_of_nodes node_or_array)
    // put_back(string key, node anode)
    // has_compat(string key, class arrtype)
    // has_compat_array(string key, class arrtype)
    // has_node(string key)
    // has_node_list(string key)
    // empty()
    // get(string key, class optionalcast)
    // get_ref(string key, class optionalcast)
    // get_node(string key)
    // get_node_list(string key)
    // get_node_list_length(string key)
    // merge_to(node target)
    // has_symbol(string key)
    // serialize()
    // serialize_encode()
    // serialize_readable()
    // deserialize(numberarray content)
    // deserialize_decode(string content)
    // destroy_children()
    // deserialize_readable(string content)

    CompoundNode : class {
        generic_tags = {};
        child_nodes = {};
        child_node_lists = {};

        constructor(){};

        put(key, value, bytesperelement, arraytype){
            let sb;
            switch(typeof(value)){
            case "object":{
                if(!ArrayBuffer.isView(value))
                    throw new Error("serialize.CompoundNode.put : if object, value must be typed array")
                sb = new serialize.SizedBlock(bytesperelement!=undefined?bytesperelement:value.BYTES_PER_ELEMENT, value);
                break;
            }
            case "bigint":{
                sb = new serialize.SizedBlock(8,new BigInt64Array([value]));
                break;
            }
            case "number":{
                if(arraytype==undefined)
                    throw new Error("serialize.CompoundNode.put : array type reqired for insertion of number values");
                sb = new serialize.SizedBlock(arraytype.BYTES_PER_ELEMENT,new arraytype([value]));
                break;
            }
            case "string":{
                sb = new serialize.SizedBlock(1,new Uint8Array(value.split("").map(x=>x.charCodeAt())));
                break;
            }
            case "boolean":{
                sb = new serialize.SizedBlock(1,new Uint8Array([value?1:0]));
                break;
            }
            default:{
                throw new Error("serialize.CompoundNode.put : cannot insert variables of type: " + typeof(value));
                break;
            }
            }
            this.generic_tags[key] = sb;
            return sb;
        }
        put_node(key, node_or_array){
            if(typeof(node_or_array) == "object"){
                let ncn = new serialize.CompoundNode()
                node_or_array.merge_to(ncn);
                this.child_nodes[key] = ncn;
                return;
            }
            child_node_lists[key] = []
            for (let node in node_or_array){
                let ncn = new serialize.CompoundNode();
                code.copyTo(ncn);
                child_node_lists[key].push(node_or_array);
            }
            return node_or_array
        }
        put_back(key, anode){
            if(this.child_node_lists[key] == undefined)
                this.child_node_lists[key] = [];
            let ncn = new serialize.CompoundNode();
            anode.merge_to(ncn);
            this.child_node_lists[key].push(ncn);
            return ncn
        }

        has_compat(key, arrtype){
            if(this.generic_tags[key] == undefined)
                return false;
            if(this.generic_tags[key].span != (typeof(arrtype)=="number"?arrtype:arrtype.BYTES_PER_ELEMENT))
                return false;
            return true;
        }
        has_compat_array(key, arrtype){
            if(this.generic_tags[key] == undefined)
                return false;
            if(this.generic_tags[key].element_span != (typeof(arrtype)=="number"?arrtype:arrtype.BYTES_PER_ELEMENT))
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
            let contents = this.generic_tags[key]?.contents_native.buffer.slice();
            if(contents == undefined)
                return undefined;
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

        merge_to(target){
            for(let key in this.generic_tags){
                let nsb = new serialize.SizedBlock();
                this.generic_tags[key].copyTo(nsb);
                target.generic_tags[key] = nsb;
            }
            for(let key in this.child_nodes){
                let ncn = new serialize.CompoundNode();
                this.child_nodes[key].merge_to(ncn);
                target.put_node("key",ncn);
            }
            for(let key in this.child_node_lists){
                let list = this.child_node_lists[key];
                for(let lnode in list){
                    let ncn = new serialize.CompoundNode();
                    list[lnode].merge_to(ncn);
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
            let arr = new serialize.uVector();
            if(!serialize.NORESERVE)
                arr.reserve(8192);
            arr.push_back(serialize.COMPOUND_NODE_BEGIN_FLAG);
            for(const key in this.generic_tags){
                let esc = serialize._add_escapes_to_string(key);
                arr.push_back(serialize.COMPOUND_NODE_BEGIN_STRING_FLAG);
                arr.insert_back(Uint8Array.from(esc.split("").map(x => x.charCodeAt())));
                arr.push_back(serialize.COMPOUND_NODE_BEGIN_ELEMENT_FLAG);
                arr.push_back(serialize.COMPOUND_NODE_BEGIN_BLOCK_FLAG);
                arr.insert_back(this.generic_tags[key].lower());
            }
            for(const key in this.child_nodes){
                let esc = serialize._add_escapes_to_string(key);
                arr.push_back(serialize.COMPOUND_NODE_BEGIN_STRING_FLAG);
                arr.insert_back(Uint8Array.from(esc.split("").map(x => x.charCodeAt())));
                arr.push_back(serialize.COMPOUND_NODE_BEGIN_ELEMENT_FLAG);
                arr.insert_back(this.child_nodes[key].serialize());
            }
            for(const key in this.child_node_lists){
                let esc = serialize._add_escapes_to_string(key);
                arr.push_back(serialize.COMPOUND_NODE_BEGIN_STRING_FLAG);
                arr.insert_back(Uint8Array.from(esc.split("").map(x => x.charCodeAt())));
                arr.push_back(serialize.COMPOUND_NODE_BEGIN_ELEMENT_FLAG);
                arr.push_back(serialize.COMPOUND_NODE_BEGIN_LIST_FLAG);
                for (let n = 0; n < this.child_node_lists[key].length; n ++){
                    arr.insert_back(this.child_node_lists[key][n].serialize())
                }
                arr.push_back(serialize.COMPOUND_NODE_END_LIST_FLAG);
            }
            arr.push_back(serialize.COMPOUND_NODE_END_FLAG);
            arr.shrink_to_fit()
            return arr.array()
        }
        serialize_encode(){
            let arrb = this.serialize();
            let str = btoa(serialize.ArrayBufferToString(arrb));
            return str
        }
        
        serialize_readable(){
            let serialization = serialize.COMPOUND_NODE_BEGIN_FLAG_R
            let loop = 0;
            for(const key in this.generic_tags){
                serialization += serialize.COMPOUND_NODE_BEGIN_STRING_FLAG_R
                serialization += serialize._add_escapes_to_readable(key)
                serialization += serialize.COMPOUND_NODE_END_STRING_FLAG_R
                serialization += serialize.COMPOUND_NODE_KV_SEPERATOR_R
                serialization += serialize._value_string(this.generic_tags[key])
                if(++loop < Object.keys(this.generic_tags).length || !Object.keys(this.child_nodes).length==0 || !Object.keys(this.child_node_lists).length==0)
                    serialization += serialize.COMPOUND_NODE_VALUE_SEPERATOR_R
            }
            loop = 0
            for(const key in this.child_nodes){
                serialization += serialize.COMPOUND_NODE_BEGIN_STRING_FLAG_R
                serialization += serialize._add_escapes_to_readable(key)
                serialization += serialize.COMPOUND_NODE_END_STRING_FLAG_R
                serialization += serialize.COMPOUND_NODE_KV_SEPERATOR_R
                serialization += this.child_nodes[key].serialize_readable()
                if(++loop < Object.keys(this.child_nodes).length || !Object.keys(this.child_node_lists).length==0)
                    serialization += serialize.COMPOUND_NODE_VALUE_SEPERATOR_R
            }
            loop = 0
            for(const key in this.child_node_lists){
                serialization += serialize.COMPOUND_NODE_BEGIN_STRING_FLAG_R
                serialization += serialize._add_escapes_to_readable(key)
                serialization += serialize.COMPOUND_NODE_END_STRING_FLAG_R
                serialization += serialize.COMPOUND_NODE_KV_SEPERATOR_R
                serialization += serialize.COMPOUND_NODE_BEGIN_LIST_R
                let loop2 = 0
                for(const node of this.child_node_lists[key]){
                    serialization += node.serialize_readable();
                    if(++loop2 < this.child_node_lists[key].length)
                        serialization += serialize.COMPOUND_NODE_VALUE_SEPERATOR_R
                }
                serialization += serialize.COMPOUND_NODE_END_LIST_R           
                if(++loop < Object.keys(this.child_node_lists).length)
                    serialization += serialize.COMPOUND_NODE_VALUE_SEPERATOR_R
            }
            serialization += serialize.COMPOUND_NODE_END_FLAG_R
            return serialization
        }

        deserialize(content){
            let parser = new serialize.BasicPushdownParser();
            let state = serialize.BasicParserState.AwaitBegin;
            for(let i = 0; i < content.length; i++){
                state = parser.consume(content[i])
                if(state == serialize.BasicParserState.Error)
                    return false;
                if(state == serialize.BasicParserState.Warning)
                    parser.state.currentState = parser.state.nextState
            }
            if(state != serialize.BasicParserState.Success)
                return false;
            this.destroy_children()
            parser.state.node.merge_to(this)
            return true;
        }
        deserialize_decode(content){
            let str = content;
            let arrb = new Uint8Array(serialize.base64ToArrayBuffer(content))
            this.deserialize(arrb)
        }

        destroy_children(){
            this.generic_tags = {};
            this.child_nodes = {};
            this.child_node_lists = {};
        }
        deserialize_readable(content){
            let parser = new serialize.ReadablePushdownParser;
            let str = content;
            let state = serialize.ParserState.AwaitStart;
            for (let i = 0; i < str.length; i ++){
                state = parser.consume(content[i])
                if(state == serialize.ParserState.Error)
                    return false;
                if(state == serialize.ParserState.Warning)
                    parser.state.currentState = parser.state.nextState;
            }
            if(state != serialize.BasicParserState.Success)
                return false;
            this.destroy_children();
            parser.state.node.merge_to(this)
            return true;
        }
    }
}
