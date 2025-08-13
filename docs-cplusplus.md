### `Serialize::`

---

**`class CompoundNode{...}`** (AKA `serialize::compound_node`)

A serializable json-like data structure. Contains key-value pairs for the following in discrete hash tables:

- Generic non-referencing types and arrays (generic blocks of homogeneous type)
- "child" structures (child CompoundNodes)
- arrays of child structures (arrays of CompoundNodes)

The type of the tag is not preserved through serialization. However, the tag can be retrieved as any compatible type (i.e. any type that has the same size).

For human-readable serialization, there exists a function `SizedBlock::assign_meta` to hint a datatype to represent the data.

---

**`class SizedBlock{...}`** (AKA `serialize::sized_block`)

Not intended for logical use outside of the library except `assign_meta(uint8_t meta)`

---

#### `SizedBlock::`

---

`void assign_meta(uint8_t)`

Assigns meta to data for use in human-readable serializations. Meta is preserved throughout all serializations. Available values are:  
```
SB_META_UNDEFINED
SB_META_INT_STYLE
SB_META_FLOAT_STYLE
SB_META_BOOLEAN
SB_META_STRING
```
Default is `SB_META_UNDEFINED`. Undefined meta will cause a tag to be skipped when in human-readable serializations.

---

#### `CompoundNode::`

`CompoundNode(){}`

Constructor. Constructs an empty node.

---

`template<typename T> void put(std::string key, T var)`

Insert a tag with the respective key and value. This will safely override and replace an existing tag, if present.

---

`template<typename T> void put_string(std::string key, size_t amount, T* vars)`

Insert a c-style string with the respective key and values. Strings are stored in the same hash table space as single tags.

---

`template<typename T> void put_string(std::string key, std::vector<T>& vars)`

Insert a vector (string) with the respective key and values.

---

`SizedBlock* put(std::string key, CompoundNode& node)`

Insert a copy of the given node as a child.

---

`SizedBlock* put(std::string key, std::vector<CompoundNode*& nodes)`

Insert a copy of the vector of the nodes as a child array.

---

`void put_back(std::string key, CompoundNode& node)`

Append a copy of the given node to the child array.

---

`template<typename T> bool has_compat(std::string key)`

Determine whether there is a tag that can be retrieved as [a single instance of] the given variable.

---

`template<typename T> bool has_compat_string(std::string key)`

Determines whether there is a tag that can be retrieved as a string (of any length, including 1) of the given variable.

---

`bool has_node(std::string key)`

Determines whether a child node exists.

---


`bool has_node_list(std::string key)`

Determines whether a child node list exists.

---


`template<typename T> T get(std::string key)`

Get a copy of the value of the given tag

---


`template<typename T> std::vector<T> get_string(std::string key)`

Get a copy of the given tag as a vector

---

`template<typename T> T* get_ref(std::string key)`

Retrieve a pointer to the actual data contained with a tag

---


`size_t get_string_length(std::string key)`

Calculate the number of elements in a tag

---

`CompoundNode* get_node(std::string key)`

Retrieve a reference to a child node

---

`std::vector<CompoundNode*> get_node_list(std::string key)`

Retrieve a list of references to child nodes in a list

---

`size_t get_node_list_length(std::string key)`

Get the length of a node list

---

`void copy_to(CompoundNode* node)`

Recursively copy to another node. This erases the target node first.

---

**[DEPRECATED]** `std::string similar_json()`

For debugging, it returns a pseudo-code representation of the data structure. Please note that some tags may display as the pointer address.

---

`std::vector<char> serialize()`

Generate a binary serialization of the tag and its descendants.

---

`bool deserialize(std::vector<char>* data, size_t start_index, size_t* end_index)`

Deserializes from the given buffer. Returns `true` if successful.
 
 ---

`void destroy_children()`

Delete the node's tags and its children, resulting in an empty node.

---

`bool operator[]`

Returns whether there is a tag of the name contained within the brackets.

---

`std::string serialize_encode()`

Returns a string-compatible serialization in base64 encoding.

---

`bool decode_deserialize(std::string data)`

Deserializes from a base64 encoded serialization.

---

`std::string serialize_readable()`

Serialize into a human-readable notation. There is no defined guarantee that the data within generic tags with no defined meta is preserved.

---

`bool deserialize_readable(std::string data)`

Deserialize from a human-readable notation.
