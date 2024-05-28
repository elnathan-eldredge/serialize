#pragma once
#include <unordered_map>
#include <string>
#include <cstring>

bool SystemBigEndian(void){
    union {
        uint32_t i;
        char c[4];
    } int = {0x01020304};

    return int.c[0] == 1;
}

class SizedBlock{
public:
  size_t size;
  void* data;
  
  SizedBlock(size_t size_bytes,const void* input){
    data = malloc(size);
    size = size_bytes;
    memcpy(data,input,size_bytes);
  }

  SizedBlock(SizedBlock* block){
    data = malloc(block->size);
    memcpy(data, block->data, block->size);
    size = block->size;
  }

  void ReverseEndian(size_t span){
    if(size % span){
      #ifndef SERIALIZE_NOWARN
      std::cout << "Serialize: WARN: Cannot reverse endian of size "
                << size
                << " and span"
                << span
                <<". Size must be a multiple of span."
      #endif
      return; 
    }
    unsigned char* bytes = (unsigned char*)data;
    for (i = 0; i < size; i += span){
      for (a = 0; a < (span>>1); a++){
        unsigned char t = bytes[i + a];
        bytes[i + a] = bytes[i + (span-a)];
        bytes[i + (span-a)] = t;
      }
    }
  }

  SizedBlock(){
    size = 0;
  }
  
  ~SizedBlock(){
    free(data);
  }
  
};

  

class CompoundNode{
public:
  CompoundNode(){}

  
  std::unordered_map<std::string, SizedBlock*> tags;
  std::unordered_map<std::string, CompoundNode*> nodes;

  template<typename T>
  void put(T* data, std::string name){
    tags[name] = new SizedBlock(sizeof(T), data);
  }

  template<typename T>
  void put(T data, std::string name){
    T a = data;
    tags[name] = new SizedBlock(sizeof(T), &a);
  }

  template<typename T>
  void put_string(const T* data, size_t amount, std::string name){
    tags[name] = new SizedBlock(sizeof(T)*amount, data);
  }

  void put_node(CompoundNode* node, std::string name){
    if(nodes.find(name) != nodes.end()) delete nodes[name];
    nodes[name] = new CompoundNode();
    node->_copy_to_empty(nodes[name]);
  }

  template<typename T>
  T* retrieve_p(std::string name){
    if(tags.find(name) == tags.end()) return nullptr;
    return static_cast<T*>(tags[name]->data);
  }
  
  template<typename T>
  T retrieve(std::string name){
    return *static_cast<T*>(tags[name]->data);
  }

  template<typename T>
  T* retrieve_string(std::string name){
    return static_cast<T*>(tags[name]->data);
  }

  CompoundNode* retrieve_node(std::string name){
    if(nodes.find(name) == nodes.end()) return nullptr;
    return nodes[name];
  }

  template<typename T>
  bool compat_tag(std::string name){
    if(tags.find(name) == tags.end()) return false;
    return sizeof(T) == tags[name]->size;
  }

  template<typename T>
  bool compat_string(std::string name){
    if(tags.find(name) == tags.end()) return false;
    return tags[name]->size % sizeof(T) == 0;
  }

  bool exists_node(std::string name){
    return nodes.find(name) != nodes.end();
  }

  void _copy_to_empty(CompoundNode* node){
    for(std::pair<std::string, SizedBlock> pair: tags){
      node->tags[pair.first] = new SizedBlock(tags[pair.first]);
    }
    for(std::pair<std::string, CompoundNode*> pair: nodes){
      node->nodes[pair.first] = new CompoundNode();
      pair.second->_copy_to_empty(node->nodes[pair.first]);
    }
  }

  ~CompoundNode(){
    for(std::pair<std::string, SizedBlock*> pair: tags){
      delete pair.second;
    }
    for(std::pair<std::string, CompoundNode*> pair: nodes){
      delete pair.second;
    }
  }

};


