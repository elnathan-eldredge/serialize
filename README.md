# Serialize

Hashtable style serialization library

## Design Goals:

- Serialize pseudostatically-typed Json-Like data structures
- Serialize into packed binary
- Serialize into string-compatible data
- Serialize into human-readable notation
- Compatible across endiannesses, two's complementnesses, and bitness
- Compatible with all homogeneous data types

## Current notices:

- The standard convention for passing refrences is pointers for when the object is mutated by the called function, refrences for when there is no other purpose than optimization.
- There is no SAX interface (this may change), but there is character-by-character parsing.

## DOCS

- docs-cplusplus.md
- docs-javascript.md

## Examples (c++)


```c++
#include "../include/serialize.hpp"
#include <iostream>
using namespace Serialize;

// An interface similar to something the programmer might find himself writing
class archivable{
    public:
    virtual void archive(CompoundNode* node){}
    virtual void unpack(CompoundNode& node){}
};

// In this example, there may be many classes that can all be declared 
// as inheritors of "archivable" that the programmer has as part of
// a larger structure. 
class transaction : public archivable {
    public:
    std::string party_a;
    std::string party_b;
    long id;
    
    transaction(std::string pa, std::string pb){
        static long sid = 0;
        party_a = pa;
        party_b = pb;
        id = sid;
        sid++;
    }
    
    virtual void archive(CompoundNode* node){
        archivable::archive(node);
        node->put_string<char>("party_a", party_a.size(), (char*)party_a.c_str())->assign_meta(SB_META_STRING);
        node->put_string<char>("party_b", party_b.size(), (char*)party_b.c_str())->assign_meta(SB_META_STRING);
        node->put<uint64_t>("id", id)->assign_meta(SB_META_INT_STYLE);
    }
    
    virtual void unpack(CompoundNode& node){
        archivable::unpack(node);
        if(!node.has_compat_string<char>("party_a") ||
           !node.has_compat_string<char>("party_a") ||
           !node.has_compat<uint64_t>("id"))
           throw std::invalid_argument("transaction::unpack requires tags \"party_a\", \"party_b\", and \"id\".");
      // It may be acceptable to instead include the null character when
      // there is no possibility of using the readable serialization 
      // (while assigning a stringlike meta), but  it is desirable to 
      // instead construct std::strings with size and data.
       party_a = std::string(node.get_ref<char>("party_a"), node.get_string_length("party_a"));
       party_b = std::string(node.get_ref<char>("party_b"), node.get_string_length("party_b"));
       id = node.get<uint64_t>("id");
    }
};

// In this instance, the using structure populates and reads from a node,
// but structures that generate nodes are equally as valid.
int main(){
    transaction t("foo","bar");
    CompoundNode node;
    t.archive(&node);
    std::string data = node.serialize_readable(false);
    std::cout << "node as serailized: " << data << "\n";
    node.deserialize_readable(data);
    t.unpack(node);
    t.archive(&node);
    std::cout << "node after serialized: " << node.serialize_readable(false) << "\n";
  return 0;
}

```

## Examples (javascript)

```javascript
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
```

## Incorporation

- single-header (./include) (C++)
- pretty javascript (./include-javascript/serialize.js)
- minified javascript (./include-javascript/serialize-minified.js)

## Dependencies

- The STL library

- System support for uint64_t

## Pseudo-Dependencies

- Auto-injected:

	- Base64 (EE_base64)

## LIMITATIONS

- Individual elements in tag elements have a maximum size of 65,535 bytes (63.9 Kb).
- Individual tags have a maximum size of 4,294,967,295 bytes (3.9 Gb).
- (Due to be changed) Maximum single-call deserialization will only read up to 18,446,744,073,709,551,615 bytes (~16 Eb) from buffer
- (Due to be changed) Although buffers are read sequentially, there is no SAX interface.
- Serializations temporarily reserve 8kb per nested element during serialization. Compile with SERIALIZE_NORESERVE defined to disable.

## Resources

### Readable Serialization Specification Summary:

**Note: the whitespace markings mark where whitespace must be acceptable in all implementations. There is no gurantee inappropriate whitesapce will result in a parse failure, or, if succeding, have the intended value.*

![spec drawio](https://github.com/user-attachments/assets/18a0d042-dc48-4f45-ab64-4ff6e9594760)

### Bug tracker:

Please be courteous.

### Example:

- ./example/applied.cpp

    - No linking should be necessary

- ./include-javascript/

    - server: ./include-javscript/server
    
        - hosts on 0.0.0.0:8009
    
    - html: ./include-javascript/example.html
    
    - javascript: ./include-javascript/example.js
