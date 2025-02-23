let arr1 = new Uint16Array([0x0001,0x0002,0x0003,0x0004])
let arr2 = new Uint32Array(arr1.buffer)
let arr3 = new Uint8Array(arr1.buffer)
console.log(arr1,arr2,arr3)
invert_endian(arr1,arr1.BYTES_PER_ELEMENT)
console.log(arr1,arr2,arr3)
invert_endian(arr2,arr2.BYTES_PER_ELEMENT)
console.log(arr1,arr2,arr3)

let block = new SizedBlock(2, new Uint16Array([0x04,0x03,0x02,0x01,0x011,0x12]));
console.log(block);
let lowered = block.lower()
console.log(lowered)
block.upper(lowered);
console.log(block);
let block2 = new SizedBlock();
block.copy_to(block2);
console.log(block2);

