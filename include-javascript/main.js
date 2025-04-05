// The code up until the compound node demonstration is mainly for debugging if something goes wrong downstream

let ser = serialize
/*
let arr1 = new Uint16Array([0x0001,0x0002,0x0003,0x0004])
let arr2 = new Uint32Array(arr1.buffer)
let arr3 = new Uint8Array(arr1.buffer)
console.log(arr1,arr2,arr3)
ser.invert_endian(arr1,arr1.BYTES_PER_ELEMENT)
console.log(arr1,arr2,arr3)
ser.invert_endian(arr2,arr2.BYTES_PER_ELEMENT)
console.log(arr1,arr2,arr3)

console.log("=======sizedblocks=======")
let block = new ser.SizedBlock(2, new Uint16Array([0x04,0x03,0x02,0x01,0x11,0x1012]));
console.log("block:", block);
console.log("block element 0:", new Uint16Array(block.contents_native.buffer)[0])
console.log("block contents native:",block.contents_native)
let lowered = block.lower()
console.log("block lowered:",lowered)
block.upper(lowered);
console.log("block after upper:",block);
console.log("block element 0:", new Uint16Array(block.contents_native.buffer)[0])
let block2 = new ser.SizedBlock();
block.copyTo(block2);
console.log("copied block contents:",block2.contents_native)
console.log("copied block:",block2);
console.log("copied block element 0:",new Uint16Array(block2.contents_native.buffer)[0])

block.assign_meta(ser.SB_META_INT_STYLE)
console.log("value strings: ", ser._value_string(block))
block.assign_meta(ser.SB_META_BOOLEAN)
console.log("value strings: ", ser._value_string(block))

console.log("=======sizedblocks (storing float)=======")
block = new ser.SizedBlock(4, new Float32Array([1.1,2.2,3.3,3.6,0.1,9.81]));
console.log("block:", block);
console.log("block element 0:", new Float32Array(block.contents_native.buffer)[0])
console.log("block contents native:",block.contents_native)
lowered = block.lower()
console.log("block lowered:",lowered)
block.upper(lowered);
console.log("block after upper:",block);
console.log("block element 0:", new Float32Array(block.contents_native.buffer)[0])
block2 = new ser.SizedBlock();
block.copyTo(block2);
console.log("copied block contents:",block2.contents_native)
console.log("copied block:",block2);
console.log("copied block element 0:",new Float32Array(block2.contents_native.buffer)[0])

block.assign_meta(ser.SB_META_FLOAT_STYLE)
console.log("value strings: ", ser._value_string(block))
*/

// Compound node demo
console.log("=======compound nodes=======")
let mnode = new ser.CompoundNode()

mnode.put("keyforfloat64array(with some escapes) : %s $", new Float64Array([1.1,2.2,3.3,3.6,0.1,9.81]));
mnode.put("keyforuint16array",new Uint16Array(mnode.get("keyforfloat64array(with some escapes) : %s $",Float64Array).buffer));


let childnode = new ser.CompoundNode();
let childchildnode = new ser.CompoundNode();
childnode.put("a string","this is a string");
childnode.put("a number",5,undefined,Uint16Array)
childnode.put_back("nested",childchildnode)

mnode.put_node("a child node", childnode);

childnode.put("array index",0,undefined,Uint8Array)
mnode.put_back("an array of nodes", childnode);
childnode.put("array index",1,undefined,Uint8Array)
mnode.put_back("an array of nodes", childnode)
childnode.put("array index",2,undefined,Uint8Array)
childnode.put("a string","this is a diffrent string")
mnode.put_back("an array of nodes", childnode)

console.log(mnode.has_compat("keyforfloat64array(with some escapes) : %s $",Float64Array),
            mnode.has_compat_array("keyforfloat64array",Float64Array),
            mnode.has_compat_array("keyforfloat64array",Uint16Array))
console.log(mnode.has_node("nonexistant"),
            mnode.has_node("a child node"),
            mnode.has_node_list("an array of nodes"))
console.log(mnode.get("keyforfloat64array",Float64Array),
            mnode.get("keyforuint16array",Uint32Array));
console.log(mnode);
console.log("serialization: ", mnode.serialize())
console.log("encoded serialization:", mnode.serialize_encode())

let parser = new ser.BasicPushdownParser();

let arr = mnode.serialize();
let enc = mnode.serialize_encode();

let state = ser.BasicParserState.AwaitBegin;

for(let l = 0; l < arr.length; l++){
    state = parser.consume(arr[l]);
//    console.log(state, String.fromCharCode(arr[l]), arr[l])
//    console.log();
    if(state == undefined){
	console.log("state is undefined please report")
	break;
    }

 if(state == ser.BasicParserState.Error){
	console.log("parser failure")
	break;
    }
    if(state == ser.BasicParserState.Success){
	console.log("parser success")
	break;
    }
}
console.log(parser.state.node)
console.log("parsing stopped")
mnode.deserialize(arr)
console.log(mnode)
mnode.deserialize_decode(enc)
console.log(mnode)

let generatedByOtherProgram ="eyxyZXNlcnZlZCBlc2NhcGVzIGNhbiBiZSB1c2VkIGFzIHdlbGwgXHt9Oi0MAQAMAAAAAAAAADpcIlw0e306XDoAACxzb21lIGJvb2xlYW5zOi0LAQAQAAAAAAAAAAgHBgUEAwL/AA4NDAsKCQAsdHdvIG51bWJlcjY0czotAQgAEAAAAAAAAAAIBwYFBAMC/wAODQwLCgkALGZvdXIgbnVtYmVyMzJzOi0BBAAQAAAAAAAAAAgHBgUEAwL/AA4NDAsKCQAsbnVtYmVyODotAQEAAQAAAAAAAACBLHNvbWUgc3RyaW5nMjotDAEAHQAAAAAAAAB0aGlzIGlzIGEgc3RyaW5nIG9mIGxldHRlcnMyACxzaXh0ZWVuIG51bWJlcjhzOi0BAQAQAAAAAAAAAAgHBgUEAwL/AA4NDAsKCQAsc29tZSBzdHJpbmc6LQwBABwAAAAAAAAAdGhpcyBpcyBhIHN0cmluZyBvZiBsZXR0ZXJzACxlaWdodCBudW1iZXIxNnM6LQECABAAAAAAAAAACAcGBQQDAv8ADg0MCwoJACxudW1iZXI2NDotAQgACAAAAAAAAADvzauJZ0UjASx3cm9uZyBtZXRhJ2Qgc3RyaW5nOi0JAQAFAAAAAAAAAHd4eXoALGRvdWJsZTotCQgACAAAAAAAAADfd+jIhRvJvyxjaGlsZDp7LGxldHRlcnM6LQwBAAgAAAAAAAAAaGlna2xtbgB9LGNoaWxkIGFycmF5Olt7LGxldHRlcnM6LQwBAAgAAAAAAAAAb3BxcnN0dQB9eyxsZXR0ZXJzOi0MAQAIAAAAAAAAAGFiY2RlZmcAfXssbGV0dGVyczotDAEACAAAAAAAAABhYmNkZWZnAH1dfQ=="

mnode.deserialize_decode(generatedByOtherProgram)

console.log("deserialized from other program:", mnode.serialize_readable())

let generatedByOtherProgramR = '{ "reserved escapes can be used as well \{}" : s":\\"\\4{}:\:", "some booleans" : n[ true,true,true,true,true,true,true,true,false,true,true,true,true,true,true,false], "two number64s" : l[ -71491328285473016,2544317353496064], "four number32s" : i[ 84281096,-16645372,202182144,592395], "number8" : b[ -127], "some string2" : s"this is a string of letters2", "sixteen number8s" : b[ 8,7,6,5,4,3,2,-1,0,14,13,12,11,10,9,0], "some string" : s"this is a string of letters", "eight number16s" : m[ 1800,1286,772,-254,3584,3085,2571,9], "number64" : l[ 81985529216486895], "wrong meta\'d string" : x[ ], "double" : d[ -0.196152], "child" : { "letters" : s"higklmn"}, "child array" : [{ "letters" : s"opqrstu"}, { "letters" : s"abcdefg"}, { "letters" : s"abcdefg"}]}'

let parserR = new ser.ReadablePushdownParser();

state = ser.BasicParserState.AwaitStart;

for(let c of generatedByOtherProgramR){
    let state = parserR.consume(c);
//    console.log(state, c)
    if(state == ser.ParserState.Error){
        console.log("parser error (readable)")
        break;
        
    }
    if(state == ser.ParserState.Success){
        console.log("parser success (readable)")
        break;        
    }
    if(state == undefined){
        console.log("state became undefined, please report")
        break;
    }
}

console.log(parserR.state.node)

console.log("deserialized from other program (readable):", mnode.serialize_readable())

mnode.deserialize_readable(generatedByOtherProgramR)

console.log(mnode)
