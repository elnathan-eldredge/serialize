let arr1 = new Uint16Array([0x0001,0x0002,0x0003,0x0004])
let arr2 = new Uint32Array(arr1.buffer)
let arr3 = new Uint8Array(arr1.buffer)
console.log(arr1,arr2,arr3)
invert_endian(arr1,arr1.BYTES_PER_ELEMENT)
console.log(arr1,arr2,arr3)
invert_endian(arr2,arr2.BYTES_PER_ELEMENT)
console.log(arr1,arr2,arr3)

console.log("=======sizedblocks=======")
let block = new SizedBlock(2, new Uint16Array([0x04,0x03,0x02,0x01,0x011,0x12]));
console.log("block:", block);
console.log("block element 0:", new Uint16Array(block.contents_native.buffer)[0])
console.log("block contents native:",block.contents_native)
let lowered = block.lower()
console.log("block lowered:",lowered)
block.upper(lowered);
console.log("block after upper:",block);
console.log("block element 0:", new Uint16Array(block.contents_native.buffer)[0])
let block2 = new SizedBlock();
block.copy_to(block2);
console.log("copied block contents:",block2.contents_native)
console.log("copied block:",block2);
console.log("copied block element 0:",new Uint16Array(block2.contents_native.buffer)[0])
console.log("=======sizedblocks (storing float)=======")
block = new SizedBlock(2, new Float32Array([1.1,2.2,3.3,3.6,0.1,9.81]));
console.log("block:", block);
console.log("block element 0:", new Float32Array(block.contents_native.buffer)[0])
console.log("block contents native:",block.contents_native)
lowered = block.lower()
console.log("block lowered:",lowered)
block.upper(lowered);
console.log("block after upper:",block);
console.log("block element 0:", new Float32Array(block.contents_native.buffer)[0])
block2 = new SizedBlock();
block.copy_to(block2);
console.log("copied block contents:",block2.contents_native)
console.log("copied block:",block2);
console.log("copied block element 0:",new Float32Array(block2.contents_native.buffer)[0])

console.log("=======compound nodes=======")
let node = new CompoundNode();

node.put("keyforfloat64array", new Float64Array([1.1,2.2,3.3,3.6,0.1,9.81]));
node.put("keyforuint16array",new Uint16Array(node.get("keyforfloat64array").buffer));


let childnode = new CompoundNode();
childnode.put("a string","this is a string");
childnode.put("a number",5,undefined,Uint16Array)

node.put_node("a child node", childnode);

childnode.put("index",0,undefined,Uint8Array)
node.put_back("an array of nodes", childnode);
childnode.put("index",1,undefined,Uint8Array)
node.put_back("an array of nodes", childnode)
childnode.put("index",2,undefined,Uint8Array)
childnode.put("a string","this is a diffrent string")
node.put_back("an array of nodes", childnode)


console.log(node.has_compat("keyforfloat64array",Float64Array),
            node.has_compat_array("keyforfloat64array",Float64Array),
            node.has_compat_array("keyforfloat64array",Uint16Array))
console.log(node.has_node("nonexistant"),
            node.has_node("a child node"),
            node.has_node_list("an array of nodes"))
console.log(node.get("keyforfloat64array",Float64Array),
            node.get("keyforuint16array",Uint32Array));
console.log(node);

