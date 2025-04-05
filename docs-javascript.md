### `serailize.`

---

**`class SizedBlock`**

A class representing a block of raw data (with some additional segments for size, division, and metadata). The only use for this in the typical use case is for assigning meta to hint a way to represent data in the readable serailziation.

---

**`class CompoundNode`**

A data structure node that contains the following in seperate maps:

- "generic tags" - items used to store raw or otherwise usable data
- "child nodes" - subnodes
- "child node lists" - arrays of subnodes

---

**`SB_META_UNDEFINED`**
**`SB_META_INT_STYLE`**
**`SB_META_FLOAT_STYLE`**
**`SB_META_BOOLEAN`**
**`SB_META_STRING`**

Various constants representing possible values to use in `SizedBlock.assign_meta`, each representing a desired representation to use if the readable serailization is used. **`SB_META_UNDEFINED`** is the default value.

---

**`SB_META_MAX`**

A constant representing the maximum value a reserved constant (used for an assignable meta) will be. The programmer may define his own constants between this and up through the maximum value an unsigned eight bit integer will store.

---
    
#### `SizedBlock.`

---

**`assign_meta(meta)`**

Assign the given meta (converted to an eight bit unsigned integer) to the tag element. The built-in constants can be assigned to hint to the serializer what way the data should be represented in the readable serialization.

---
        
#### `CompoundNode.`

**`constructor()`**

Constructs an empty node

---

**`put(key, value, bytesperelement, arraytype)`**

Inserts a generic tag, `key` and `value` are required arguments. `value` may be many things:

- A typed array
    
    - No other arguments are strictly required, but `bytesperelement` can be used to override the typed array's segmentation.

- A `BigInt`

    - No other arguments are required. Is stored as a signed/unsigned 64 bit integer
    
- A number

    - `arraytype` is required for segmentation and must be one of the typed array types, excluding `BigInt` arrays.
    
- A string

    - No other arguments are required. Stored as a Uint8Array with each element representing a character code
    
- A boolean

    - Stored as an unsigned eight bit integer with the value of 1 or 0, for `true` and `false`.
    
Other cases will generate an error

---

**`put_node(key, node_or_array)`**

Inserts a `CompoundNode` or an array of `CompoundNode`s. Must be either.

---

**`put_back(key, anode)`**

Appends a node to an existing or nonexisting array, creating an array if the case is the latter.

---

**`has_compat(key, arrtype)`**

Returns wether there is a generic tag that can be used as a single value of the array type. I.E. is a singe value with a segmentation identical to `arrtype`.

---

**`has_compat_array(key, arrtype)`**

Returns wether there is a generic tag that can be used as a single or multiple values of the array type. I.E. is a value or array with a segmentation identical to `arrtype`.

---

**`has_node(key)`**

Returns wether there is an immediate child node referred to by the key

---

**`has_node_list(key)`**

Returns wether there is an immediate child node list referred to by the key

---

**`empty()`**

Returns wether the node is empty in all respects

---

**`get(key, optionalcast)`**

Retrieves the value assiociated by the key as an `Uint8Array`, or, if specified by `optional cast`, as the associated typed array. Returns undefined if there is none.

---

**`get_ref(key, optionalcast)`**

Retrieves the value assiociated by the key as a refrence to an `Uint8Array`, or, if specified by `optional cast`, as a refrences to the associated typed array. Returns undefined if there is none.

---

**`get_node(key)`**
    
Retrieves the associated node by refrence. Returns undefined if there is none.

---

**`get_node(key)`**

Retrieves the associated node list by reference. Returns undefined if there is none.

---

**`get_node_list_length(key)`**

Returns the length of the associated node list, creating one if there is none.

---

**`merge_to(target)`**

Creates a copy of its data on the target node, its member(s) taking precedence if there is any collisions, appending to the target's node lists instead of overwriting them.

---

**`has_symbol(key)`**

Returns wether there is a generic tag, child node, or child node list associated with the key.

---

**`serialize()`**

Returns a Uint8Array representing all data within the node.

---

**`serialize_encode()`**

Returns a string, Base64 encoded, representing all data within the node.

---

**`serialize_readable()`**

Returns a string, human readable and editable, representing data within the node. There is no defined guarentee that the data within generic tags is preserved if there is no meta defining a representation in this readable serialization.

---

**`deserialize(content)`**

Deserialize from an `Uint8Array`. Returns wether the deserailization was a success (`true`) or a failure (`false`).

---

**`deserialize_decode(content)`**

Deserialize from a Base64 encoded string. Returns wether the deserailization was a success (`true`) or a failure (`false`).

---

**`destroy_children()`**

Remove all data from the node in all respects. `empty()` will return true.

---

**`deserialize_readable(content)`**{

Deserailize from a string with the human-readable notation. Returns wether the deserailization was a success (`true`) or a failure (`false`).
