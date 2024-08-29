# Serialize

Arbitrary type serialization library

## This library operates under the principals:

- Functions always have referenced value inputs as pointers

## Incorporation

This library is a single header library. Including the header file is sufficient

## Dependencies

The STL library

## LIMITATIONS

Individual elements in tag elements have a maximum size of 65,535 bytes. 
Individual tags have a maximum size of 4,294,967,295 bytes. 
Serializations temporarily reserve 8kb per nested element during serialization. Compile with SERIALIZE_NORESERVE defined to disable.

## Available utilities intended for use by the programmer

### class CompoundNode (also compound_node)
**within the namespace "Serialize" (also serialize)**

A serializable json-like data structure. Contains key-value pairs for the following in discrete hashmaps:

- Generic non-referencing types and arrays (tags)
- "child" structures (child nodes)
- arrays of child structures (arrays of child nodes)

The type of the tag is not preserved through serialization. However, the tag can be retrieved as any compatible type (i.e. any type that has the same size). The serialized data structure is stored as a binary vector of bytes in the little endian format. Note that conversion to the host architecture is automatic when deserializing.

#### Methods:

#### CompoundNode(){};

Constructor

#### template<typename T> void put(std::string key, T var);

Insert a tag with the respective key and value. This will safley override and replace an existing tag, if present.


#### template<typename T> void put_string(std::string key, size_t amount, T* vars);

Insert a c-style string with the respective key and values. Strings are stored in the same hash table space as scalars.


#### template<typename T> void put_string(std::string key, std::vector<T>* vars);

Insert a vector with the respective key and values.


#### void put(std::string key, CompoundNode* node);

Insert a copy of the given node as a child.


#### void put(std::string key, std::vector<CompoundNode*>* nodes);

Insert a copy of the vector of the nodes as a child array.


#### void put_back(std::string key, CompoundNode* node);

Append a copy of the given node to the child array.


#### template<typename T> bool has_compat(std::string key);

Determine whether there is a tag that can be retrieved as [a single instance of] the given variable.


#### template<typename T> bool has_compat_string(std::string key);

Determines whether there is a tag that can be retrieved as a string (of any length, including 1) of the given variable.


#### bool has_tag(std::string key);

Determines whether a tag exists.


#### bool has_tag_list(std::string key);

Determines whether a tag exists, including string types.


#### template<typename T> T get(std::string key);

Get a copy of the value of the given tag


#### template<typename T> std::vector<T> get_string(std::string key);

Get a copy of the given tag as a vector


#### template<typename T> T* get_ref(std::string key);

Get a reference to the tag as a c-style string


#### size_t get_string_length(std::string key);

Get the length of the tag in number of elements


#### CompoundNode* get_node(std::string key);

Get a child node


#### std::vector<CompoundNode*> get_node_list(std::string key);

Get a vector of references to the nodes in the list


#### size_t get_node_list_length(std::string key);

Get the length of a node list


#### void copy_to(CompoundNode* node);

Copy the node and it's descendants to the given node


#### std::string similair_json();

Exclusively for debugging, returns a json pseudo-equivalent of the data structure. Please note that some tags may display as the pointer address

#### std::vector<char> serialize();

Return a block of data that fully expresses the data structure


#### bool deserialize(std::vector<char>* data, size_t start_index, size_t* end_index);

Initialize the node as the data structure expressed in the block. The end index may be nullptr.
 

#### void destroy_children();

Destroy the node and the child data structure.

#### bool operator [];

Returns wether there is a child or tag of the name

### Other

There are other public types and methods for internal use.

## Resources

### Bug tracker:

https://github.com/RawSteak0/serialize/issues
Please submit issues in the Stackoverflow style, reading prior issues to avoid duplicating.

### Example:

./example/main.cpp
No linking should be necessary
