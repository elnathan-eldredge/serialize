#include "../include/serialize.hpp"
#include <iostream>


int main(){

  printf("##########################\nSizedBlock tests\n##########################\n");

  //although it is not intended to use sizedblocks directly, it can be done. for testing, a 64-bit number will be stored as 2 32-bit numbers, although it is recommended to store as 1 64-bit
  printf("serialize uint64_t as 2 uint32_t\n\n");
  
  uint64_t number = 0x123456789abcdef;

  //the first argument is the size of the elements, and the second is the amount.
  serialize::SizedBlock block = Serialize::SizedBlock(sizeof(uint32_t), 2, &number);

  //this function returns the serialized data, safe to write to disk
  std::vector<char> disk = block.lower();

  printf("serialization contents: ");
  for(unsigned char c : disk){
    printf("%02x ",c);
  }
  printf("\n");
  
  //this function clears the contents of the block
  block.dump();

  //this function initializes from the serialized data
  block.upper(&disk,0);

  printf("(deserialized) element_size: %d, size: %d\n",block.element_span, block.span);
  printf("deserialized number: 0x%lx\n", *((uint64_t*)block.contents_native));
  printf("number: 0x%lx\n", number);
  printf("numbers match: %s\n", (number == *((uint64_t*)block.contents_native))
         ?"TRUE":"FALSE (something has gone terribly wrong)");
  
  printf("##########################\nCompoundNode tests\n##########################\n");

  Serialize::compound_node node;

  node.put<float>("float",5.196152423)->assign_meta(COMPOUND_NODE_BEGIN_LIST_FLAG);
  node.put<uint64_t>("number64",number);  
  node.put_string<char>("some string", 28, "this is a string of letters");
  node.put_string<char>("some string2", 29, "this is a string of letters2");

  printf("node has unsigned long \"number64\" : %s\n", node.has_compat<uint64_t>("number64")?"true":"false");
  printf("node has int \"number64\" : %s\n", node.has_compat<int>("number64")?"true":"false");
  printf("node has member \"number64\" : %s\n", node["number64"]?"true":"false");
  
  Serialize::CompoundNode child_node;
  child_node.put_string<char>("letters", 8, "abcdefg");

  printf("child node member \"letters\" : %s\n", child_node.get_ref<char>("letters"));

  node.put("child", &child_node); //makes a copy

  printf("node has child \"child\": %s\n", node.has_tag("child")?"true":"false");
  printf("node has member \"child\": %s\n", node["child"]?"true":"false");
  printf("changing nodes's child's tag \"letters\", using overriding function\n");

  node.get_node("child")->put_string<char>("letters", 8, "higklmn");

  printf("original node's tag's value: %s\n", child_node.get_ref<char>("letters"));
  printf("node's child's tag's value: %s\n", node.get_node("child")->get_ref<char>("letters"));

  node.put_back("child array", &child_node);
  node.put_back("child array", &child_node);
  node.put_back("child array", &child_node);
  node.get_node_list("child array")[0]->put_string<char>("letters", 8, "opqrstu");
  node.put_string<char>("reserved escapes can be used as well :\\{}[],", 12, ":\\4{}[],:\\:");

  printf("node has tag list \"child array\" : %s\n", node.has_tag_list("child array")?"true":"false");
  printf("node tag list \"child array\" length : %d\n\n", node.get_node_list_length("child array"));
  printf("json representation: \n%s\n", node.similair_json().c_str());
  printf("node.number64: %lx\n", node.get<uint64_t>("number64"));
  printf("node.float: %f\n\n", node.get<float>("float"));

  std::vector<char> serialized = node.serialize();

  printf("serialized contents: \n");
  
  for(char character: serialized){
    putchar(character);
  }
  printf("\n\n");

  std::string serialized_encoded = node.serialize_encode();

  printf("serialized encoded contents: %s\n\n", serialized_encoded.c_str());

  node.destroy_children();

  if(!node.deserialize(&serialized, 0, nullptr))
    puts("unable to deserialize for some reason");

  printf("deserialized json representaion: \n%s\n", node.similair_json().c_str());
  if(node.has_compat<uint64_t>("number64"))
     printf("node.number64: %lx\n", node.get<uint64_t>("number64"));
  if(node.has_compat<float>("float"))
    printf("node.floatf: %f\n\n", node.get<float>("float"));

  node.destroy_children();
  
  if(!node.decode_deserialize(serialized_encoded))
    puts("cannot deserialize decode node");

  printf("decoded deserialized: \n%s\n", node.similair_json().c_str());
  if(node.has_compat<uint64_t>("number64"))
    printf("node.number64: %lx\n", node.get<uint64_t>("number64"));
  if(node.has_compat<float>("float"))
    printf("node.floatf: %f\n\n", node.get<float>("float"));

    printf("PLEASE NOTE: the json representaion will display the pointer adress in some cases instead of the content (because types are arbritrary), so the json representaions may not match. It is recommended to compare data structures algorithmically or by comparing a reserialization of the deserialized node (given the order is preserved)\n\n");
  return 0;
}
