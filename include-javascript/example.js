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

class archivable{
    constructor(){}
    archive(node){}
    unpack(node){}
}

class transaction extends archivable{
    static iid = 16
    constructor(party_a,party_b){
        super()
        this.party_a = party_a
        this.party_b = party_b
        this.id = transaction.iid
        transaction.iid++
    }
    archive(node){
        super.archive(node)
        node.put("party_a", String(this.party_a)).assign_meta(ser.SB_META_STRING)
        node.put("party_b", String(this.party_b)).assign_meta(ser.SB_META_STRING)
        node.put("id", new BigUint64Array([BigInt(this.id)]), 8, BigUint64Array).assign_meta(ser.SB_META_INT_STYLE)
        return node;
    }
    unpack(node){
        if(!node.has_compat_array("party_a", Uint8Array) ||
           !node.has_compat_array("party_b", Uint8Array) ||
           !node.has_compat("id", BigUint64Array))
            throw new Error("transaction.unpack: node must have tags \"party_a\", \"party_b\", and \"id\"")
        let coder = new TextDecoder();
        this.party_a = coder.decode(node.get("party_a", Uint8Array))
        this.party_b = coder.decode(node.get("party_b", Uint8Array))
        this.id = Number(node.get("id",BigUint64Array)[0])
    }
}

let transact = new transaction("Coconut","Tomato");
let str = transact.archive(new serialize.CompoundNode()).serialize_readable()
console.log("==========================\nexample.js\n==========================")
console.log(transact)
console.log(str)
let nod = new serialize.CompoundNode()
nod.deserialize_readable(str)
transact.unpack(nod)
console.log(transact)


