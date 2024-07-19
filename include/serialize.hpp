#pragma once
#include <unordered_map>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
//only for this file, so things don't crash for 32-bit systems
#define size_t uint32_t

#define COMPOUND_NODE_BEGIN_FLAG (char)123
#define COMPOUND_NODE_END_FLAG (char)125
#define COMPOUND_NODE_BEGIN_STRING_FLAG (char)44
#define COMPOUND_NODE_BEGIN_BLOCK_FLAG (char)58
#define COMPOUND_NODE_BEGIN_LIST_FLAG (char)91
#define COMPOUND_NODE_END_LIST_FLAG (char)93

#define serialize Serialize
#define compound_node CompoundNode

namespace Serialize{

  bool is_big_endian(void){
    union {
      uint32_t i;
      char c[4];
    } e = { 0x01000000 };
    return e.c[0];
  }

  void* invert_endian_h(uint16_t element_size, size_t element_count, void* data_reg){
    char* data_invert = (char*)malloc(element_size * element_count);
    char* data = (char*)data_reg;
    for(size_t i = 0; i < element_count * element_size; i += element_size){
      for(uint16_t e = 0; e < element_size; e++){
        data_invert[i + e] = data[i + (element_size - 1) - e];
      }
    }
    return data_invert;
  }

  template<typename T>
  bool exists_key(std::unordered_map<std::string,T>* map, std::string key){
    return map->find(key) != map->end();
  }

  template<typename T>
  T little_endian(T d){
    if(!is_big_endian()) return d;
    T d_copp = d;
    T* d_corr = (T*)invert_endian_h(sizeof(T), 1, &d_copp);
    d_copp = *d_corr;
    free(d_corr);
    return d_copp;
  }

  class SizedBlock{
  public:
    void* contents_native;
    uint16_t element_span;
    size_t span = 0;
    
    SizedBlock(uint16_t element_size, size_t element_count, void* data);

    std::vector<char> lower();
    
    SizedBlock(){
      span = 0;
      element_span = 0;
      contents_native = nullptr;
    };

    char* upper(char* data, char* max);

    uint64_t upper(std::vector<char>* data, uint64_t starting_index);
    
    void dump();

    void copy_to(SizedBlock* target);

    ~SizedBlock();

  };

  class CompoundNode {
  public:
    std::unordered_map<std::string,SizedBlock*> generic_tags;
    std::unordered_map<std::string,CompoundNode*> child_nodes;
    std::unordered_map<std::string,std::vector<CompoundNode*>> child_node_lists;

    CompoundNode(){};

    template<typename T> void put(std::string key, T var);

    template<typename T> void put_string(std::string key, size_t amount, T* vars);

    template<typename T> void put_string(std::string key, std::vector<T>* vars);

    void put(std::string key, CompoundNode* node);

    void put(std::string key, std::vector<CompoundNode*>* nodes);

    void put_back(std::string key, CompoundNode* node);

    template<typename T> bool has_compat(std::string key);

    template<typename T> bool has_compat_string(std::string key);

    bool has_tag(std::string key);

    bool has_tag_list(std::string key);

    template<typename T> T get(std::string key);

    template<typename T> std::vector<T> get_string(std::string key);

    template<typename T> T* get_ref(std::string key);

    size_t get_string_length(std::string key);

    CompoundNode* get_node(std::string key);

    std::vector<CompoundNode*> get_node_list(std::string key);

    size_t get_node_list_length(std::string key);

    void copy_to(CompoundNode* node);
    
    void burninate_generic_if_exists(std::string key);

    std::string similair_json();

    std::vector<char> serialize();

    bool deserialize(std::vector<char>* data, size_t start_index, size_t* end_index);
    
    ~CompoundNode();

    void destroy_children();
  };

  void _skip_to_flag(char flag, std::vector<char>* data, size_t* index, size_t offset){
    *index -= 1;
    while((*data)[++*index] != flag && (*index) < data->size()){}
    *index += offset;
  }

  void _skip_to_flag(char flag, char flag2, std::vector<char>* data, size_t* index, size_t offset){
    *index -= 1;
    while((*data)[++*index] != flag && (*data)[*index] != flag2 && (*index) < data->size()){
    }
    *index += offset;
  }

  void _rewind_to_flag(char flag, std::vector<char>* data, size_t* index, size_t offset){
    *index += 1;
    while((*data)[--*index] != flag && (*index) >= 0){}
    *index += offset;
  }

  std::string _read_as_string_until_block(std::vector<char>* data, size_t* start_index){
    std::string str;
    size_t index = 0;
    if(start_index != nullptr) index = *start_index;
    index -= 1;
    while((*data)[++index] != COMPOUND_NODE_BEGIN_BLOCK_FLAG && index < data->size()){
      if((*data)[index] == *"\\" && (*data)[index+1] == COMPOUND_NODE_BEGIN_BLOCK_FLAG && index < data->size()){
        str += COMPOUND_NODE_BEGIN_BLOCK_FLAG;
        ++index;
        continue;
      }
      str += (*data)[index];
    }
    if(start_index != nullptr) *start_index = index;
    return str;
  }
  
  bool CompoundNode::deserialize(std::vector<char>* data, size_t start_index, size_t* end_index){
    CompoundNode new_node;
    size_t index = start_index;
    size_t max_index = data->size() - 1;
    _skip_to_flag(COMPOUND_NODE_BEGIN_FLAG, data, &index, 1);
    while(true){
      if(index > max_index) return false;
      if((*data)[index] == COMPOUND_NODE_END_FLAG) break;
      _skip_to_flag(COMPOUND_NODE_BEGIN_STRING_FLAG, data, &index, 1); if(index > max_index) return false;
      std::string key = _read_as_string_until_block(data, &index);
      //      printf("got key %s\n", key.c_str());
      index++; if(index > max_index) return false;
      switch ((*data)[index]){
      case COMPOUND_NODE_BEGIN_FLAG:{
        CompoundNode* new_child_node = new CompoundNode();
        if(!new_child_node->deserialize(data, index, &index)){
          free(new_child_node);
          return false;
        }
        --index;
        if(exists_key<CompoundNode*>(&new_node.child_nodes, key)){
          delete new_child_node;
          break;
        }
        new_node.child_nodes[key] = new_child_node;
        break;
      }
      case COMPOUND_NODE_BEGIN_LIST_FLAG:{
        if(!exists_key<std::vector<CompoundNode*>>(&new_node.child_node_lists, key))
          new_node.child_node_lists[key] = std::vector<CompoundNode*>();
        while(true){
          _skip_to_flag(COMPOUND_NODE_BEGIN_FLAG, COMPOUND_NODE_END_LIST_FLAG, data, &index, 0);
          if(index + 1 > max_index){
            return false;
          };
          if((*data)[index] == COMPOUND_NODE_END_LIST_FLAG) break;
          CompoundNode* new_child_node = new CompoundNode();
          if(!new_child_node->deserialize(data, index, &index)){
            free(new_child_node);
            return false;
          }
          new_node.put_back(key, new_child_node);
          delete(new_child_node);
        }
        break;
      }
      default:{
        SizedBlock* new_block = new SizedBlock();
        index = new_block->upper(data, index);
        if(!index){
          delete new_block;
          return false;
        }
        --index;
        if(exists_key<SizedBlock*>(&new_node.generic_tags, key)){
          delete new_block;
          break;
        }
        new_node.generic_tags[key] = new_block;
        break;
      }
      }
      index++;
    }
    if(end_index != nullptr)
      *end_index = ++index;
    destroy_children();
    new_node.copy_to(this);
    return true;
  }

  std::string _add_escapes_to_string(std::string str){
    std::string new_string;
    for(int i = 0; i < str.length(); i ++){
      if(str[i] == COMPOUND_NODE_BEGIN_BLOCK_FLAG)
        new_string += *"\\";
      new_string += str[i];
    }
    return new_string;
  }

  std::vector<char> CompoundNode::serialize(){
    std::vector<char> data;
#ifndef SERIALIZE_NORESERVE
    data.reserve(8192);
#endif
    data.push_back(COMPOUND_NODE_BEGIN_FLAG);
    for(std::pair<std::string, SizedBlock*> pair : generic_tags){
      std::string escaped = _add_escapes_to_string(pair.first);
      data.push_back(COMPOUND_NODE_BEGIN_STRING_FLAG);
      data.insert(data.end(), escaped.begin(), escaped.end());
      data.push_back(COMPOUND_NODE_BEGIN_BLOCK_FLAG);
      std::vector<char> block = pair.second->lower();
      
      data.insert(data.end(), block.begin(), block.end());
    }
    for(std::pair<std::string, CompoundNode*> pair : child_nodes){
      std::string escaped = _add_escapes_to_string(pair.first);      
      data.push_back(COMPOUND_NODE_BEGIN_STRING_FLAG);
      data.insert(data.end(), escaped.begin(), escaped.end());
      data.push_back(COMPOUND_NODE_BEGIN_BLOCK_FLAG);
      std::vector<char> node_serial = pair.second->serialize();
      data.insert(data.end(), node_serial.begin(), node_serial.end());
    }
    for(std::pair<std::string, std::vector<CompoundNode*>> pair: child_node_lists){
      std::string escaped = _add_escapes_to_string(pair.first);
      data.push_back(COMPOUND_NODE_BEGIN_STRING_FLAG);
      data.insert(data.end(), escaped.begin(), escaped.end());
      data.push_back(COMPOUND_NODE_BEGIN_BLOCK_FLAG);
      data.push_back(COMPOUND_NODE_BEGIN_LIST_FLAG);
      for(CompoundNode* node: pair.second){
        std::vector<char> node_serial = node->serialize();
        data.insert(data.end(), node_serial.begin(), node_serial.end());
      }
      data.push_back(COMPOUND_NODE_END_LIST_FLAG);
    }
    data.push_back(COMPOUND_NODE_END_FLAG);
    data.shrink_to_fit();
    return data;
  }

  std::string CompoundNode::similair_json(){
    std::string json = "{";
    int64_t c = 0;
    for(std::pair<std::string, SizedBlock*> pair: generic_tags){
      if(pair.second->span == pair.second->element_span){
        json += "\"" + pair.first + "\": " + std::to_string((long int)pair.second->contents_native);
      }else if(pair.second->element_span == 1){
        json += "\"" + pair.first + "\": \"" + std::string((const char*)pair.second->contents_native) + "\"";
      } else {
        json += "[";
        for(size_t i = 0; i < pair.second->span/pair.second->element_span; i++){
          json += std::to_string(((char*)pair.second->contents_native)[i*pair.second->element_span]);
          if(i + 1 <  pair.second->span/pair.second->element_span)
            json += ",";
        }
        json += "]";
      }
      if(++c < generic_tags.size() || child_nodes.size() || child_node_lists.size())
        json += ",";
    }
    c = 0;
    for(std::pair<std::string, CompoundNode*> pair: child_nodes){
      json += "\"" + pair.first + "\": "+ pair.second->similair_json();
      if(++c < child_nodes.size() || child_node_lists.size())
        json += ",";
    }
    c = 0;
    for(std::pair<std::string, std::vector<CompoundNode*>> pair: child_node_lists){
      json += "\"" + pair.first + "\": [";
      size_t c1 = 0;
      for(CompoundNode* node: pair.second){
        json += node->similair_json();
        if(++c1 < pair.second.size())
          json += ",";
      }
      json += "]";
      if(++c < child_node_lists.size())
        json += ",";
    }
    json += "}";
    return json;
  }

  std::vector<CompoundNode*> CompoundNode::get_node_list(std::string key){
    return child_node_lists[key];
  }

  CompoundNode* CompoundNode::get_node(std::string key){
    return child_nodes[key];
  }

  size_t CompoundNode::get_node_list_length(std::string key){
    return child_node_lists[key].size();
  }

  template<typename T>
  std::vector<T> CompoundNode::get_string(std::string key){
    T* ptr = (T*)generic_tags[key]->contents_native;
    std::vector<T> string;
    string.resize(generic_tags[key]->span/generic_tags[key]->element_span);
    memcpy(string.data(), ptr, generic_tags[key]->span);
    return string;
  }

  size_t CompoundNode::get_string_length(std::string key){
    return generic_tags[key]->span/generic_tags[key]->element_span;
  }

  template<typename T>
  T* CompoundNode::get_ref(std::string key){
    T* ptr = (T*)generic_tags[key]->contents_native;
    return ptr;
  }
  
  template<typename T>
  T CompoundNode::get(std::string key){
    T* ptr = (T*)generic_tags[key]->contents_native;
    return *ptr;
  }

  bool CompoundNode::has_tag_list(std::string key){
    return exists_key<std::vector<CompoundNode*>>(&child_node_lists, key);
  }

  bool CompoundNode::has_tag(std::string key){
    return exists_key<CompoundNode*>(&child_nodes, key);
  }

  template<typename T>
  bool CompoundNode::has_compat_string(std::string key){
    if(!exists_key<SizedBlock*>(&generic_tags, key))
      return false;
    return generic_tags[key]->element_span == sizeof(T);
  }
  
  template<typename T>
  bool CompoundNode::has_compat(std::string key){
    if(!exists_key<SizedBlock*>(&generic_tags, key))
      return false;
    return generic_tags[key]->span == sizeof(T);
  }

  void CompoundNode::put(std::string key, std::vector<CompoundNode*>* nodes){
    if(exists_key<std::vector<CompoundNode*>>(&child_node_lists, key)){
      for(CompoundNode* node: child_node_lists[key]){
        delete node;
      }
      child_node_lists[key].clear();
    }
    child_node_lists[key].reserve(nodes->size());
    for(CompoundNode* node: *nodes){
      CompoundNode* new_node = new CompoundNode();
      node->copy_to(new_node);
      child_node_lists[key].push_back(new_node);      
    }
  }

  void CompoundNode::put_back(std::string key, CompoundNode* node){
    if(!exists_key<std::vector<CompoundNode*>>(&child_node_lists, key))
      child_node_lists[key] = std::vector<CompoundNode*>();
    CompoundNode* new_node = new CompoundNode();
    node->copy_to(new_node);
    child_node_lists[key].push_back(new_node);
  }

  void CompoundNode::put(std::string key, CompoundNode* node){
    if(exists_key<CompoundNode*>(&child_nodes, key))
      delete child_nodes[key];
    CompoundNode* new_node = new CompoundNode();
    node->copy_to(new_node);
    child_nodes[key] = new_node;
  }

  void CompoundNode::copy_to(CompoundNode* target){
    target->destroy_children();
    for(std::pair<std::string,SizedBlock*> pair: generic_tags){
      target->generic_tags[pair.first] = new SizedBlock();
      pair.second->copy_to(target->generic_tags[pair.first]);
    }
    for(std::pair<std::string,CompoundNode*> pair: child_nodes){
      CompoundNode* new_node = new CompoundNode();
      pair.second->copy_to(new_node);
      target->child_nodes[pair.first] = new_node;
    }
    for(std::pair<std::string,std::vector<CompoundNode*>> pair: child_node_lists){
      target->child_node_lists[pair.first] = std::vector<CompoundNode*>();
      target->child_node_lists[pair.first].reserve(pair.second.size());
      for(CompoundNode* source: pair.second){
        CompoundNode* new_node = new CompoundNode();
        source->copy_to(new_node);
        target->child_node_lists[pair.first].push_back(new_node);
      }
    }
  }

  CompoundNode::~CompoundNode(){
    destroy_children();
  }

  void CompoundNode::destroy_children(){
    for(std::pair<std::string, CompoundNode*> pair: child_nodes){
      delete pair.second;
    }
    child_nodes.clear();
    for(std::pair<std::string, std::vector<CompoundNode*>> pair: child_node_lists){
      for(CompoundNode* node: pair.second){
        delete node;
      }
      pair.second.clear();
    }
    child_node_lists.clear();
    for(std::pair<std::string,SizedBlock*> pair: generic_tags){
      delete pair.second;
    }
    generic_tags.clear();
  }

  template<typename T>
  void CompoundNode::put_string(std::string key, std::vector<T>* vars){
    T* p_vars = vars->data();
    size_t amount = vars->size();
    burninate_generic_if_exists(key);
    generic_tags[key] = SizedBlock(sizeof(T), amount, p_vars);
  }

  template<typename T>
  void CompoundNode::put_string (std::string key, size_t amount, T* vars){
    T* p_vars = vars;
    burninate_generic_if_exists(key);
    generic_tags[key] = new SizedBlock(sizeof(T), amount, p_vars);
  }
  
  template<typename T>
  void CompoundNode::put(std::string key, T var){
    T var_copp = var;
    burninate_generic_if_exists(key);
    generic_tags[key] = new SizedBlock(sizeof(T), 1, &var_copp);
  }

  void CompoundNode::burninate_generic_if_exists(std::string key){
    if(exists_key<SizedBlock*>(&generic_tags, key)){
      delete generic_tags[key];
      generic_tags.erase(key);
    }
  }
  
  void SizedBlock::copy_to(SizedBlock* block){
    block->dump();
    if(!span) return;
    block->contents_native = malloc(span);
    memcpy(block->contents_native, contents_native, span);
    block->span = span;
    block->element_span = element_span;
  }

  SizedBlock::SizedBlock(uint16_t element_size, size_t element_count, void* data){
    contents_native = malloc(element_size * element_count);
    span = element_count * element_size;
    element_span = element_size;
    memcpy(contents_native, data, element_size * element_count);
  }

  std::vector<char> SizedBlock::lower(){
    std::vector<char> contents(sizeof(uint16_t) + sizeof(size_t) + span);
    uint16_t element_d = little_endian<uint16_t>(element_span);
    size_t span_d = little_endian<size_t>(span);
    memcpy(contents.data(),(char*)&element_d,sizeof(uint16_t)); //element span 1st 2 bytes
    memcpy(contents.data()+sizeof(uint16_t),(char*)&span_d,sizeof(size_t)); //bytes 2,3,4,5
    void* contents_little_endian = contents_native;
    if(is_big_endian()){
      contents_little_endian = invert_endian_h(element_span, span/element_span, contents_native);
    }
    memcpy(contents.data()+sizeof(uint16_t)+sizeof(size_t),(char*)contents_native,span);
    if(is_big_endian()){
      free(contents_little_endian);
    }
    return contents;
  }

  char* SizedBlock::upper(char* data, char* maxaddress){ //returns the address AFTER all the data used
    if(span) dump();
    char* max = maxaddress + 1;
    if(max - data < sizeof(uint16_t) + sizeof(size_t)) return nullptr;
    uint16_t element_size;
    memcpy(&element_size, data, sizeof(uint16_t));
    element_span = little_endian<uint16_t>(element_size);// also goes from little to native
    size_t total_size;
    memcpy(&total_size, data + sizeof(uint16_t), sizeof(size_t));
    span = little_endian<size_t>(total_size);
    char* data_after_header = data + sizeof(uint16_t) + sizeof(size_t);
    if(max - data < total_size + sizeof(uint16_t) + sizeof(size_t)){
      span = 0;
      element_span = 0;
      return nullptr;
    }
    if(is_big_endian()){
      contents_native = invert_endian_h(element_span, span/element_span, data_after_header);
    } else {
      contents_native = malloc(span);
      memcpy(contents_native, data_after_header, span);
    }
    return data_after_header + span;
  }

  uint64_t SizedBlock::upper(std::vector<char>* data, uint64_t starting_index){
    return (upper(data->data() + starting_index, data->data() + data->size() - 1) - data->data());
  }

  void SizedBlock::dump(){
    if(span == 0) return;
    free(contents_native);
    contents_native = nullptr;
    span = 0;
    element_span = 0;
  }

  SizedBlock::~SizedBlock(){
      dump();
  }


}
#undef size_t

