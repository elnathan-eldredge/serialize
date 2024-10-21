#pragma once
#include <cstdint>
#include <istream>
#include <unordered_map>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <string.h>

//this definition is limited to this file, to ensure cross-compatibility with system bitness.
#define un_size_t uint64_t

#define COMPOUND_NODE_BEGIN_FLAG (char)123
#define COMPOUND_NODE_END_FLAG (char)125
#define COMPOUND_NODE_BEGIN_STRING_FLAG (char)44
#define COMPOUND_NODE_BEGIN_ELEMENT_FLAG (char)58
#define COMPOUND_NODE_BEGIN_BLOCK_FLAG (char)45
#define COMPOUND_NODE_BEGIN_LIST_FLAG (char)91
#define COMPOUND_NODE_END_LIST_FLAG (char)93

#define SB_META_UNDEFINED 0
#define SB_META_INT_STYLE 1
#define SB_META_FLOAT_STYLE 9
#define SB_META_BOOLEAN 11
#define SB_META_STRING 12
#define SB_META_MAX 127

#define SB_FLAG_UNDEFINED *"x"
#define SB_FLAG_I8 *"b"
#define SB_FLAG_I16 *"m"
#define SB_FLAG_I32 *"i"
#define SB_FLAG_I64 *"l"
#define SB_FLAG_FLOAT *"f"
#define SB_FLAG_DOUBLE *"d"
#define SB_FLAG_LONG_DOUBLE *"q"
#define SB_FLAG_BOOLEAN *"n"
#define SB_FLAG_STRING *"s"

#define serialize Serialize
#define compound_node CompoundNode

//If not already included in a seprate library
#ifndef EE_base64
#define EE_base64

namespace Serialize{
  namespace base64{
    namespace detail {
      const unsigned char base64table[66] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
#define SRLS_EQ_ESCAPE_CODE 254 //a non-255 number above 63
      const unsigned char numtable[256] = {
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255, //15
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255, //31
        255,255,255,255,255,255,255,255,255,255,255,62 ,255,255,255, 63, //47
        52 ,53 ,54 ,55 ,56 ,57 ,58 ,59 ,60 ,61 ,255,255,255,SRLS_EQ_ESCAPE_CODE,255,255, //63the parser will stop if 254 (=)
        255,0  ,1  ,2  ,3  ,4  ,5  ,6  ,7  ,8  ,9  ,10 ,11 ,12 ,13 ,14 , //79
        15 ,16 ,17 ,18 ,19 ,20 ,21 ,22 ,23 ,24 ,25 ,255,255,255,255,255, //95
        255,26 ,27 ,28 ,29 ,30 ,31 ,32 ,33 ,34 ,35 ,36 ,37 ,38 ,39 ,40 , //111
        41 ,42 ,43 ,44 ,45 ,46 ,47 ,48 ,49 ,50 ,51 ,255,255,255,255,255, //127
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255, //143
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255, //159
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255, //175
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255, //191
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255, //207
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255, //223
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255, //239
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255  //255
      };
      const char m1_1_mask = 0b11111100; //8 bit, byte 1, 6bit section 1
      const char m1_2_mask = 0b00000011; //8 bit, byte 1, 6bit section 2
      const char m2_2_mask = 0b11110000; //8 bit, byte 2, 6bit section 2
      const char m2_3_mask = 0b00001111; //8 bit, byte 2, 6bit section 3
      const char m3_3_mask = 0b11000000; //8 bit, byte 3, 6bit section 3
      const char m3_4_mask = 0b00111111; //8 bit, byte 3, 6bit section 4
      const char d1_1_mask = 0b00111111;
      const char d2_1_mask = 0b00110000;
      const char d2_2_mask = 0b00001111;
      const char d3_2_mask = 0b00111100;
      const char d3_3_mask = 0b00000011;
      const char d4_3_mask = 0b00111111;
      const char s1_1_rlshift = 2; //shift right for encoding, shift left for decoding
      const char s1_2_lrshift = 4; //shift left for encoding, shift right for decoding
      const char s2_2_rlshift = 4;
      const char s2_3_lrshift = 2;
      const char s3_3_rlshift = 6;
      const char s3_4_lrshift = 0;
    }

    un_size_t diagnose(std::string data){
      un_size_t idx;
      for(char c : data){
        ++idx;
        if(!(detail::numtable[c] + 1))
          return idx;
      }
      return 0;
    }
  
    std::vector<char> decode(std::string data){ //will return an empty vector if error
      std::vector<char> b64raw;
      //    printf("size: %d, string: %s\n", data.size(), data.c_str());
      if(data.size() % 4 != 0) return std::vector<char>();
      b64raw.reserve(data.size());
      char* dat = (char*)data.c_str();
      for(un_size_t i = 0; i < data.size(); i++){
        char ans = detail::numtable[dat[i]];
        //      printf("I: %d, %c\n", ans, data[i]);
        if(ans == SRLS_EQ_ESCAPE_CODE) break;
        if(ans == 255){
          //        printf("illegal char: %d, idx: %d\n", dat[i],i);
          return std::vector<char>();
        };
        b64raw.push_back(ans);
      }
      b64raw.shrink_to_fit(); //we now have the array
      un_size_t cur64idx = 0;
      un_size_t nextemptyidx  = 0;
      std::vector<char> output;
      output.reserve((data.size()/4)*3); //the b64 array is a multiple of 4
      for(char byte : b64raw){
        if(byte==SRLS_EQ_ESCAPE_CODE) break;
        output.push_back(0);
        switch(cur64idx % 4){
        case 0:
          output[nextemptyidx] |= (byte & detail::d1_1_mask) << detail::s1_1_rlshift;
          nextemptyidx += 1;
          break;
        case 1:
          output[nextemptyidx - 1] |= (byte & detail::d2_1_mask) >> detail::s1_2_lrshift;
          output[nextemptyidx] |= (byte & detail::d2_2_mask) << detail::s2_2_rlshift;
          nextemptyidx += 1;
          break;
        case 2:
          output[nextemptyidx - 1] |= (byte & detail::d3_2_mask) >> detail::s2_3_lrshift;
          output[nextemptyidx] |= (byte & detail::d3_3_mask) << detail::s3_3_rlshift;
          nextemptyidx += 1;
          break;
        case 3:
          output[nextemptyidx - 1] |= (byte & detail::d4_3_mask) >> detail::s3_4_lrshift;
          break;
        }
        ++cur64idx;
      }
      output.shrink_to_fit();
      return output;
    }

    std::string encode(std::vector<char> data){
      un_size_t curbase256index = 0;
      un_size_t nextemptybase64idx = 0;
      std::vector<char> base64raw;
      un_size_t num64bytes = ((data.size()+2)/3)*4; //the celing division of the size and 3, multiplied by 4
      base64raw.resize(num64bytes);
      memset(base64raw.data(), 0, num64bytes);
      for(char sbyte : data){
        unsigned char byte = (sbyte + 256) % 256;
        switch (curbase256index % 3) {
        case 0:
          base64raw[nextemptybase64idx] |= ((byte & detail::m1_1_mask) >> detail::s1_1_rlshift);
          base64raw[nextemptybase64idx + 1] |= ((byte & detail::m1_2_mask) << detail::s1_2_lrshift);
          nextemptybase64idx += 2;
          break;
        case 1:
          base64raw[nextemptybase64idx - 1] |= ((byte & detail::m2_2_mask) >> detail::s2_2_rlshift);
          base64raw[nextemptybase64idx] |= ((byte & detail::m2_3_mask) << detail::s2_3_lrshift);
          nextemptybase64idx += 1;
          break;
        case 2:
          base64raw[nextemptybase64idx - 1] |= ((byte & detail::m3_3_mask) >> detail::s3_3_rlshift);
          base64raw[nextemptybase64idx] |= ((byte & detail::m3_4_mask) << detail::s3_4_lrshift);
          nextemptybase64idx += 1;
          break;
        }
        ++curbase256index;
      }
      --nextemptybase64idx;
      while(++nextemptybase64idx < num64bytes){
        base64raw[nextemptybase64idx] = 64; //extra code for = character
      }
      std::string output;
      output.reserve(num64bytes + 1);
      for(char b64raw : base64raw){
        output += detail::base64table[(b64raw+256)%256];
        //      printf("%d, %d\n",detail::base64table[(b64raw+256)%256],b64raw);
      }
      return output;
    }
  }
}
#endif

namespace Serialize{
  
  bool is_big_endian(void){
    union {
      uint32_t i;
      char c[4];
    } e = { 0x01000000 };
    return e.c[0];
  }

  void* invert_endian_h(uint16_t element_size, un_size_t element_count, void* data_reg){
    char* data_invert = (char*)malloc(element_size * element_count);
    char* data = (char*)data_reg;
    for(un_size_t i = 0; i < element_count * element_size; i += element_size){
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
    uint16_t element_span = 0;
    un_size_t span = 0;
    uint8_t meta = 0;
    
    SizedBlock(uint16_t element_size, un_size_t element_count, void* data);

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

    void assign_meta(uint8_t meta);

    ~SizedBlock();

  };

  class CompoundNode {
  public:
    std::unordered_map<std::string,SizedBlock*> generic_tags;
    std::unordered_map<std::string,CompoundNode*> child_nodes;
    std::unordered_map<std::string,std::vector<CompoundNode*>> child_node_lists;

    CompoundNode(){};

    template<typename T> SizedBlock* put(std::string key, T var);

    template<typename T> SizedBlock* put_string(std::string key, un_size_t amount, T* vars);

    template<typename T> SizedBlock* put_string(std::string key, std::vector<T>* vars);

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

    un_size_t get_string_length(std::string key);

    CompoundNode* get_node(std::string key);

    std::vector<CompoundNode*> get_node_list(std::string key);

    un_size_t get_node_list_length(std::string key);

    void copy_to(CompoundNode* node);
    
    void burninate_generic_if_exists(std::string key);

    std::string similair_json();

    std::vector<char> serialize();

    bool operator[](std::string key);

    bool deserialize(std::vector<char>* data, un_size_t start_index, un_size_t* end_index);

    std::string serialize_encode();

    bool decode_deserialize(std::string data);

    std::string serialize_readable(bool omit_undefined);

    bool deserialize_readable(std::vector<char> *data, un_size_t start_index,
                              un_size_t *end_index);
    bool deserialize_readable(std::string data);
    
    ~CompoundNode();

    void destroy_children();
  };
}

#undef un_size_t


