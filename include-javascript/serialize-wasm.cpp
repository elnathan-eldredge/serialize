#include <emscripten.h>

std::unordered_map<int,CompoundNode> nodes;

extern "C" {
  EMSCRIPTEN_KEEPALIVE
  int versiont(){
    return 91800;
  }
  int newCompoundNode(){
    static int index;
    blocks[++index] = CompoundNode();
    return index;
  }
}
