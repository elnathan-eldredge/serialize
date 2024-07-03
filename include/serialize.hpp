#pragma once
#include <unordered_map>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
//only for this file, so things don't crash for 32-bit systems
#define size_t uint32_t

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
    
    SizedBlock(uint16_t element_size, size_t element_count, void* data){
      contents_native = malloc(element_size * element_count);
      span = element_count * element_size;
      element_span = element_size;
      memcpy(contents_native, data, element_size * element_count);
    }

    SizedBlock(){};

    std::vector<char> lower(){
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

    char* upper(char* data){ //returns the address AFTER all the data used
      if(span) dump();
      uint16_t element_size;
      memcpy(&element_size, data, sizeof(uint16_t));
      element_span = little_endian<uint16_t>(element_size);// also goes from little to native
      size_t total_size;
      memcpy(&total_size, data + sizeof(uint16_t), sizeof(size_t));
      span = little_endian<size_t>(total_size);
      char* data_after_header = data + sizeof(uint16_t) + sizeof(size_t);
      if(is_big_endian()){
        contents_native = invert_endian_h(element_span, span/element_span, data_after_header);
      } else {
        contents_native = malloc(span);
        memcpy(contents_native, data_after_header, span);
      }
      return data_after_header + span;
    }

    uint64_t upper(std::vector<char>& data, uint64_t starting_index){
      return (upper(data.data() + starting_index) - data.data());
    }

    void dump(){
      if(!span) return;
      free(contents_native);
      span = 0;
      element_span = 0;
    }

    ~SizedBlock(){
      dump();
    }

  };

}

#undef size_t

