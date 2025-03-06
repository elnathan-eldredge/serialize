#include "../include/serialize.hpp"
#include <iostream>


int main(){

  printf("##########################\nSizedBlock tests\n##########################\n");

  //although it is not intended to use sizedblocks directly, it can be done. for testing, a 64-bit number will be stored as 2 32-bit numbers.
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
  block.upper(disk,0);

  //debug
  printf("(deserialized) element_size: %d, size: %d\n",block.element_span, block.span);
  printf("deserialized number: 0x%lx\n", *((uint64_t*)block.contents_native));
  printf("number: 0x%lx\n", number);
  printf("numbers match: %s\n", (number == *((uint64_t*)block.contents_native))
         ?"TRUE":"FALSE (something has gone terribly wrong)");
  

  printf("##########################\nCompoundNode tests\n##########################\n");

  Serialize::compound_node node;

  //Test the CompoundNode store functions
  node.put<double>("double", -0.196152423)->assign_meta(SB_META_FLOAT_STYLE);
  node.put_string<char>("wrong meta'd string", 5, "wxyz")->assign_meta(SB_META_FLOAT_STYLE);
  node.put<uint64_t>("number64",number)->assign_meta(SB_META_INT_STYLE);
  node.put_string<char>("some string", 28, "this is a string of letters")
    ->assign_meta(SB_META_STRING);
  node.put_string<char>("some string2", 29, "this is a string of letters2")
      ->assign_meta(SB_META_STRING);
  node.put<int8_t>("number8", -127)->assign_meta(SB_META_INT_STYLE);

  //Store the same 128 bits in diffrent ways
  int64_t sixteenchars[2] = {(int64_t)0xFF02030405060708, 0x090A0B0C0D0E00};
  node.put_string<int8_t>("sixteen number8s", 16, (int8_t *)sixteenchars)
      ->assign_meta(SB_META_INT_STYLE);
  node.put_string<int16_t>("eight number16s", 8, (int16_t *)sixteenchars)
      ->assign_meta(SB_META_INT_STYLE);
  node.put_string<int32_t>("four number32s", 4, (int32_t *)sixteenchars)
      ->assign_meta(SB_META_INT_STYLE);
  node.put_string<int64_t>("two number64s", 2, (int64_t *)sixteenchars)
      ->assign_meta(SB_META_INT_STYLE);
  node.put_string<bool>("some booleans", (2 * sizeof(uint64_t)) / sizeof(bool), (bool *)sixteenchars)
    ->assign_meta(SB_META_BOOLEAN);

  printf("\n Node evaluation\n\n");
  
  printf("node has unsigned long \"number64\" : %s\n", node.has_compat<uint64_t>("number64")?"true":"false");
  printf("node has int \"number64\" : %s\n", node.has_compat<int>("number64")?"true":"false");
  printf("node has member \"number64\" : %s\n", node["number64"]?"true":"false");
  
  Serialize::CompoundNode child_node;
  child_node.put_string<char>("letters", 8, "abcdefg")->assign_meta(SB_META_STRING);

  printf("child node member \"letters\" : %s\n", child_node.get_ref<char>("letters"));

  node.put("child", child_node); //this function copies the node into itself

  printf("node has child \"child\": %s\n", node.has_node("child")?"true":"false");
  printf("node has member \"child\": %s\n", node["child"]?"true":"false");
  printf("changing nodes's child's tag \"letters\", using overriding function\n");

  node.get_node("child")->put_string<char>("letters", 8, "higklmn") //When nodes are returned by get_node functions, it is by refrence
    ->assign_meta(SB_META_STRING);

  printf("original node's tag's value: %s\n", child_node.get_ref<char>("letters"));
  printf("node's child's tag's value: %s\n", node.get_node("child")->get_ref<char>("letters"));
  
  node.put_back("child array", child_node);
  node.put_back("child array", child_node);
  node.put_back("child array", child_node);
  node.get_node_list("child array")[0]->put_string<char>("letters", 8, "opqrstu") //again, returned nodes are by refrence
    ->assign_meta(SB_META_STRING);
  node.put_string<char>("reserved escapes can be used as well \\{}", 12, ":\\\"\\4{}:\\:")->assign_meta(SB_META_STRING);

  printf("node has tag list \"child array\" : %s\n", node.has_node_list("child array")?"true":"false");
  printf("node tag list \"child array\" length : %d\n\n", node.get_node_list_length("child array"));

  printf("##########################\nSerialization tests\n##########################\n");
  
  printf("\n Node Contents: \n\n%s\n",node.serialize_readable(false).c_str());

  std::vector<char> serialized = node.serialize();

  printf("\nSerialized contents (Binary Format): \n\n");
  
  for(char character: serialized){
    putchar(character);
  }
  printf("\n\n");

  std::string serialized_encoded = node.serialize_encode();

  printf("Serialized contents (Encoded Format):\n\n%s\n\n", serialized_encoded.c_str());

  std::string serialized_hp = node.serialize_readable(false);

  printf("Serialized contents (Human Readable Format) \n\n%s\n\n", serialized_hp.c_str());
  
  node.destroy_children();

  if(!node.deserialize(&serialized, 0, nullptr)){
    puts("unable to deserialize Binary data\n");
  } else {
    printf("Deserialized contents from binary notation: \n\n%s\n\n", node.serialize_readable(false).c_str());
  }
  
  node.destroy_children();
  
  if(!node.decode_deserialize(serialized_encoded)){
    puts("cannot deserialize decode node");
  } else {
    printf("Deserialized contents from encoded notation: \n\n%s\n\n", node.serialize_readable(false).c_str());
  }

  node.destroy_children();
  
  if (node.deserialize_readable(serialized_hp)) {
    printf("Deserialsed conents from human-readable notation: \n\n%s\n\n", node.serialize_readable(false).c_str());
  } else {
    printf("cannot deserialize RSSF\n");
  }

  node.destroy_children();

  std::string invalid_code = "{\r\n"
   "\"incomplete\" : n";

  printf("About to decode invalid code\n\n");
  
  node.destroy_children();
  if (node.deserialize_readable(invalid_code)) {
    printf("from invalid data (this shoud not happen): \n\n%s\n\n", node.serialize_readable(false).c_str());
  } else {
    printf("Cannot decode invalid RSSN\n");
  }
  
  return 0;
}
