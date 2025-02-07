#include <emscripten.h>
#include "malloc.c"
//#include <vector>
//#include "../include/serialize.hpp"

//using namespace Serialize;

//std::vector<int> nodes;

extern "C" {
  EMSCRIPTEN_KEEPALIVE 
  int versiont(){
    //char* i = (char*)malloc(1);
    //    int j = *i;
    //    free(i);
    return 8;
  }
  /*
  int newCompoundNode(){
    static int index;
    nodes[++index] = CompoundNode();
    return index;
  }

  bool _unsafe_deleteCompoundNode(int index){
    if(nodes.find(index) == nodes.end())
      return false;
    nodes.erase(index);
      return true;
      }*/
}
