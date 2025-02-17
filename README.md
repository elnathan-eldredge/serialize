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

- Differing from standard convention, (as far as the interface goes), values passed by refrence will not be mutated. Values passed by pointer may be mutated by either party, so long as the mutation is thread-safe.
- There is no SAX interface (yet)

## Incorporation

This library provides two options:

- single-header (./include)

- conventional (./library)

## Dependencies

- The STL library

- System support for uint64_t

## Pseudo-Dependencies

- Auto-injected:

	- Base64 (EE_base64)

- Limited functionality without:

	- (none)

## LIMITATIONS

- Individual elements in tag elements have a maximum size of 65,535 bytes (63.9 Kb).
- Individual tags have a maximum size of 4,294,967,295 bytes (3.9 Gb).
- (Due to be changed) Maximum single-call deserialization will only read up to 18,446,744,073,709,551,615 bytes (~16 Eb) from buffer
- (Due to be changed) Although buffers are read sequentially, there is no SAX interface.
- Serializations temporarily reserve 8kb per nested element during serialization. Compile with SERIALIZE_NORESERVE defined to disable.

## Available utilities intended for use by the programmer

### `Serialize::`

**`class CompoundNode{...}`** (AKA `serialize::compound_node`)

A serializable json-like data structure. Contains key-value pairs for the following in discrete hash tables:

- Generic non-referencing types and arrays (generic blocks of homogeneous type)
- "child" structures (child CompoundNodes)
- arrays of child structures (arrays of CompoundNodes)

The type of the tag is not preserved through serialization. However, the tag can be retrieved as any compatible type (i.e. any type that has the same size).

For human-readable serialization, there exists a function `SizedBlock::assign_meta` to hint a datatype to represent the data.


**`class SizedBlock{...}`** (AKA `serialize::sized_block`)

Not intended for logical use outside of the library except `assign_meta(uint8_t meta)`

#### `SizedBlock::`

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

#### `CompoundNode::`

`CompoundNode(){}`

Constructor. Constructs an empty node.

`template<typename T> void put(std::string key, T var)`

Insert a tag with the respective key and value. This will safley override and replace an existing tag, if present.

`template<typename T> void put_string(std::string key, size_t amount, T* vars)`

Insert a c-style string with the respective key and values. Strings are stored in the same hash table space as single tags.


`template<typename T> void put_string(std::string key, std::vector<T>& vars)`

Insert a vector (string) with the respective key and values.


`SizedBlock* put(std::string key, CompoundNode& node)`

Insert a copy of the given node as a child.

`SizedBlock* put(std::string key, std::vector<CompoundNode*& nodes)`

Insert a copy of the vector of the nodes as a child array.


`SizedBlock* put_back(std::string key, CompoundNode& node)`

Append a copy of the given node to the child array.

`template<typename T> bool has_compat(std::string key)`

Determine whether there is a tag that can be retrieved as [a single instance of] the given variable.

`template<typename T> bool has_compat_string(std::string key)`

Determines whether there is a tag that can be retrieved as a string (of any length, including 1) of the given variable.

`bool has_node(std::string key)`

Determines whether a child node exists.


`bool has_node_list(std::string key)`

Determines whether a child node list exists.


`template<typename T> T get(std::string key)`

Get a copy of the value of the given tag


`template<typename T> std::vector<T> get_string(std::string key)`

Get a copy of the given tag as a vector


`template<typename T> T* get_ref(std::string key)`

Retrieve a pointer to the actual data contained with a tag


`size_t get_string_length(std::string key)`

Calculate the number of elements in a tag


`CompoundNode* get_node(std::string key)`

Retrieve a reference to a child node


`std::vector<CompoundNode*> get_node_list(std::string key)`

Retrieve a list of references to child nodes in a list


`size_t get_node_list_length(std::string key)`

Get the length of a node list


`void copy_to(CompoundNode* node)`

Recursively copy to another node. This erases the target node first.

**[DEPRECATED]** `std::string similair_json()`

For debugging, it returns a pseudocode representation of the data structure. Please note that some tags may display as the pointer address.

`std::vector<char> serialize()`

Generate a binary serialization of the tag and its descendants.


`bool deserialize(std::vector<char>* data, size_t start_index, size_t* end_index)`

Deserializes from the given buffer. Returns `true` if successful.
 

`void destroy_children()`

Delete the node's tags and its children, resulting in an empty node.

`bool operator[]`

Returns whether there is a tag of the name contained within the brackets.

`std::string serialize_encode()`

Returns a string-compatible serialization in base64 encoding.

`bool decode_deserialize()`

Deserializes from a base64 encoded serialization.

`std::string serialize_readable()`

Serialize into a human-readable notation.

`bool deserialize_readable(std::string data)`

Deserialize from a human-readable notation.


## Resources

### Readable Serialization Specification Summary:

![spec drawio](https://github.com/user-attachments/assets/18a0d042-dc48-4f45-ab64-4ff6e9594760)

### Bug tracker:

[https://github.com/RawSteak0/serialize/issues](https://github.com/elnathan-eldredge/serialize/issues)
Please submit issues after reading prior issues to avoid duplicating.

### Example:t

./example/main.cpp
No linking should be necessary

