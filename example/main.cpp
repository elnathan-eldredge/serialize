#include "../include/serialize.hpp"
#include <iostream>


int main(){

  printf("##########################\nSizedBlock tests\n##########################\n");

  printf("serialize uint64_t as 2 uint32_t\n\n");
  
  uint64_t number = 0x123456789abcdef;

  Serialize::SizedBlock block = Serialize::SizedBlock(sizeof(uint32_t), 2, &number);

  std::vector<char> disk = block.lower();

  printf("serialization contents: ");
  for(unsigned char c : disk){
    printf("%02x ",c);
  }
  printf("\n");

  block.dump();

  block.upper(disk,0);

  printf("(deserialized) element_size: %d, size: %d\n",block.element_span, block.span);
  printf("deserialized number: 0x%lx\n", *((uint64_t*)block.contents_native));
  printf("number: 0x%lx\n", number);
  printf("numbers match: %s\n", (number == *((uint64_t*)block.contents_native))
         ?"TRUE":"FALSE (something has gone terribly wrong)");
  
  
}
