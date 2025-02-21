let is_big_endian = () => {
    let uInt32 = new Uint32Array([0x01000000]);
    let uInt8 = new Uint8Array(uInt32.buffer);
    return uInt8[0] == 0x01;
};
