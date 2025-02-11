//#include <vector>
//#include <stdlib.h>
//#include <stdio.h>
//#include "../include/serialize.hpp"
//#include <iostream>
#include <emscripten.h>

#include <vector>
#include <unordered_map>
//#include <map>

//#include <new> // bad_alloc, bad_array_new_length
template <class T> struct Mallocator {
  typedef T value_type;
  Mallocator() noexcept { } // default ctor not required
  template <class U> Mallocator(const Mallocator<U>&) noexcept {} 
  template <class U> bool operator==(
    const Mallocator<U>&) const noexcept { return true; }
  template <class U> bool operator!=(
    const Mallocator<U>&) const noexcept { return false; }

  T * allocate(const size_t n) const {
    if (n == 0) { return nullptr; }
    if (n > static_cast<size_t>(-1) / sizeof(T)) {
      //      throw std::bad_array_new_length();
      return nullptr;
    }
    void * const pv = malloc(n * sizeof(T));
    if (!pv) { return nullptr;}// throw std::bad_alloc(); }
    return static_cast<T *>(malloc(n));
  }
  void deallocate(T * const p, size_t) const noexcept {
      free(p);
  }
};

    /*if (n == 0) { return nullptr; }
      if (n > static_cast<size_t>(-1) / sizeof(T)) {
          throw std::bad_array_new_length();
      }
      void * const pv = malloc(n * sizeof(T));
      if (!pv) { throw std::bad_alloc(); }*/
//using namespace Serialize;

//std::vector<int> nodes;


extern "C" {
  EMSCRIPTEN_KEEPALIVE 
  int versiont(){
    emscripten_run_script("alert('0')");
    std::vector<char, Mallocator<char>> str;
    std::unordered_map<char, char, std::hash<char>, std::equal_to<char>, Mallocator<std::pair<const char, char>>> mapc;
    emscripten_run_script("alert('1')");
    str.push_back(*"a");
    mapc[1] = 2;
    emscripten_run_script("alert('2')");
    return str[0];
  }

  /*  EMSCRIPTEN_KEEPALIVE
  int test() {    
    char *previous, *current;
    previous = (char*)malloc(0);
    for(int i=0; i<32; ++i) {
      current = (char*)malloc(i+1);
      std::cout << "malloc(" << i << ") consumed " << (current-previous) << " bytes\n";
      previous = current;
    }
    std::cout << "\n";
    previous = (char*)malloc(1);
    for(int i=0; i<12; ++i) {
      current = (char*)malloc( 1<<(i+1) );
      std::cout << "malloc(" << (1<<i) << ") consumed " << (current-previous) << " bytes\n";
      previous = current;
    }    
    return 0;
    }*/
  /*
  int tmalloc(){
    char* i = (char*)malloc(1);
    int j = *i;
    free(i);
    return j;
  }
  */
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
