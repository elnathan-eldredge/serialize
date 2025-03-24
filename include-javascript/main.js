let arr1 = new Uint16Array([0x0001,0x0002,0x0003,0x0004])
let arr2 = new Uint32Array(arr1.buffer)
let arr3 = new Uint8Array(arr1.buffer)
console.log(arr1,arr2,arr3)
invert_endian(arr1,arr1.BYTES_PER_ELEMENT)
console.log(arr1,arr2,arr3)
invert_endian(arr2,arr2.BYTES_PER_ELEMENT)
console.log(arr1,arr2,arr3)

console.log("=======sizedblocks=======")
let block = new SizedBlock(2, new Uint16Array([0x04,0x03,0x02,0x01,0x11,0x1012]));
console.log("block:", block);
console.log("block element 0:", new Uint16Array(block.contents_native.buffer)[0])
console.log("block contents native:",block.contents_native)
let lowered = block.lower()
console.log("block lowered:",lowered)
block.upper(lowered);
console.log("block after upper:",block);
console.log("block element 0:", new Uint16Array(block.contents_native.buffer)[0])
let block2 = new SizedBlock();
block.copyTo(block2);
console.log("copied block contents:",block2.contents_native)
console.log("copied block:",block2);
console.log("copied block element 0:",new Uint16Array(block2.contents_native.buffer)[0])

block.assign_meta(SB_META_INT_STYLE)
console.log("value strings: ", _value_string(block))
block.assign_meta(SB_META_BOOLEAN)
console.log("value strings: ", _value_string(block))

console.log("=======sizedblocks (storing float)=======")
block = new SizedBlock(4, new Float32Array([1.1,2.2,3.3,3.6,0.1,9.81]));
console.log("block:", block);
console.log("block element 0:", new Float32Array(block.contents_native.buffer)[0])
console.log("block contents native:",block.contents_native)
lowered = block.lower()
console.log("block lowered:",lowered)
block.upper(lowered);
console.log("block after upper:",block);
console.log("block element 0:", new Float32Array(block.contents_native.buffer)[0])
block2 = new SizedBlock();
block.copyTo(block2);
console.log("copied block contents:",block2.contents_native)
console.log("copied block:",block2);
console.log("copied block element 0:",new Float32Array(block2.contents_native.buffer)[0])

block.assign_meta(SB_META_FLOAT_STYLE)
console.log("value strings: ", _value_string(block))

console.log("=======compound nodes=======")
let node = new CompoundNode()

node.put("keyforfloat64array(with some escapes) : %s $", new Float64Array([1.1,2.2,3.3,3.6,0.1,9.81]));
node.put("keyforuint16array",new Uint16Array(node.get("keyforfloat64array(with some escapes) : %s $").buffer));


let childnode = new CompoundNode();
let childchildnode = new CompoundNode();
childnode.put("a string","this is a string");
childnode.put("a number",5,undefined,Uint16Array)
childnode.put_back("nested",childchildnode)

node.put_node("a child node", childnode);

childnode.put("array index",0,undefined,Uint8Array)
node.put_back("an array of nodes", childnode);
childnode.put("array index",1,undefined,Uint8Array)
node.put_back("an array of nodes", childnode)
childnode.put("array index",2,undefined,Uint8Array)
childnode.put("a string","this is a diffrent string")
node.put_back("an array of nodes", childnode)

console.log(node.has_compat("keyforfloat64array(with some escapes) : %s $",Float64Array),
            node.has_compat_array("keyforfloat64array",Float64Array),
            node.has_compat_array("keyforfloat64array",Uint16Array))
console.log(node.has_node("nonexistant"),
            node.has_node("a child node"),
            node.has_node_list("an array of nodes"))
console.log(node.get("keyforfloat64array",Float64Array),
            node.get("keyforuint16array",Uint32Array));
console.log(node);
console.log("serialization: ", node.serialize())
console.log("encoded serialization:", node.serialize_encode())

let parser = new BasicPushdownParser();

let arr = node.serialize();
let enc = node.serialize_encode();

let state = BasicParserState.AwaitBegin;

for(let l = 0; l < arr.length; l++){
    state = parser.consume(arr[l]);
//    console.log(state, String.fromCharCode(arr[l]), arr[l])
//    console.log();
    if(state == undefined){
	console.log("state is undefine")
	break;
    }
    if(state == BasicParserState.Error){
	console.log("parser failure")
	break;
    }
    if(state == BasicParserState.Success){
	console.log("parser success")
	break;
    }
}
console.log(parser.state.node)
console.log("parsing stopped")
node.deserialize(arr)
console.log(node)
node.deserialize_decode(enc)
console.log(node)

let generatedByOtherProgram ="eyxyZXNlcnZlZCBlc2NhcGVzIGNhbiBiZSB1c2VkIGFzIHdlbGwgXHt9Oi0MAQAMAAAAAAAAADpcIlw0e306XDoAACxzb21lIGJvb2xlYW5zOi0LAQAQAAAAAAAAAAgHBgUEAwL/AA4NDAsKCQAsdHdvIG51bWJlcjY0czotAQgAEAAAAAAAAAAIBwYFBAMC/wAODQwLCgkALGZvdXIgbnVtYmVyMzJzOi0BBAAQAAAAAAAAAAgHBgUEAwL/AA4NDAsKCQAsbnVtYmVyODotAQEAAQAAAAAAAACBLHNvbWUgc3RyaW5nMjotDAEAHQAAAAAAAAB0aGlzIGlzIGEgc3RyaW5nIG9mIGxldHRlcnMyACxzaXh0ZWVuIG51bWJlcjhzOi0BAQAQAAAAAAAAAAgHBgUEAwL/AA4NDAsKCQAsc29tZSBzdHJpbmc6LQwBABwAAAAAAAAAdGhpcyBpcyBhIHN0cmluZyBvZiBsZXR0ZXJzACxlaWdodCBudW1iZXIxNnM6LQECABAAAAAAAAAACAcGBQQDAv8ADg0MCwoJACxudW1iZXI2NDotAQgACAAAAAAAAADvzauJZ0UjASx3cm9uZyBtZXRhJ2Qgc3RyaW5nOi0JAQAFAAAAAAAAAHd4eXoALGRvdWJsZTotCQgACAAAAAAAAADfd+jIhRvJvyxjaGlsZDp7LGxldHRlcnM6LQwBAAgAAAAAAAAAaGlna2xtbgB9LGNoaWxkIGFycmF5Olt7LGxldHRlcnM6LQwBAAgAAAAAAAAAb3BxcnN0dQB9eyxsZXR0ZXJzOi0MAQAIAAAAAAAAAGFiY2RlZmcAfXssbGV0dGVyczotDAEACAAAAAAAAABhYmNkZWZnAH1dfQ=="

node.deserialize_decode(generatedByOtherProgram)

console.log("readable:", node.serialize_readable())
