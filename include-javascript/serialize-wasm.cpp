#include <emscripten.h>

//std::unordered_map<int,CompoundNode> nodes;

extern "C" {
  EMSCRIPTEN_KEEPALIVE
  int healthcheck() { return 81559; }
  
  int sum(int a, int b) {
    return a+b;
  }
  /* int newCompoundNode(){
    static int index;
    blocks[++index] = CompoundNode();
    return index;
    }*/
}
