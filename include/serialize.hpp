#pragma once
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
#define un_size_t uint32_t

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

  //these functions prepended with the underscore are not for public use, as segfaults may happen with improper use
  
  void _skip_to_flag(char flag, std::vector<char>* data, un_size_t* index, un_size_t offset){
    *index -= 1;
    while((*data)[++*index] != flag && (*index) < data->size()){}
    *index += offset;
  }

  void _skip_to_flag(char flag, char flag2, std::vector<char>* data, un_size_t* index, un_size_t offset){
    *index -= 1;
    while((*data)[++*index] != flag && (*data)[*index] != flag2 && (*index) < data->size()){
    }
    *index += offset;
  }

  void _rewind_to_flag(char flag, std::vector<char>* data, un_size_t* index, un_size_t offset){
    *index += 1;
    while((*data)[--*index] != flag && (*index) >= 0){}
    *index += offset;
  }

  std::string _read_as_string_until_element(std::vector<char>* data, un_size_t* start_index){
    std::string str;
    un_size_t index = 0;
    if(start_index != nullptr) index = *start_index;
    index -= 1;
    while((*data)[++index] != COMPOUND_NODE_BEGIN_ELEMENT_FLAG && index < data->size()){
      if((*data)[index] == *"\\" && (*data)[index+1] == COMPOUND_NODE_BEGIN_ELEMENT_FLAG && index < data->size()){
        str += COMPOUND_NODE_BEGIN_ELEMENT_FLAG;
        ++index;
        continue;
      }
      str += (*data)[index];
    }
    if(start_index != nullptr) *start_index = index;
    return str;
  }

  bool _is_ascii_whitespace(char c) { return c == 32 || c == 9 || c == 10 || c == 13; }

  void _skip_whitespace(std::vector<char>* data, un_size_t* index,
                        un_size_t offset) {
    *index -= 1;
    while (_is_ascii_whitespace((*data)[++*index])) {}
    *index += offset;
  }

  //All parsing functions require a "\0" at the end
  
  bool _parse_next_enquoted_string(std::vector<char>* data, un_size_t* idx,
                              std::string* str) {
    str->clear();
    _skip_whitespace(data, idx, 0);
    if ((*data)[*idx] != *"\"")
      return false;
    while (((*data)[++*idx] >= 32 && (*data)[*idx] <= 126 ||
            (*data)[*idx] == 9 || (*data)[*idx] == 32) &&
           (*data)[*idx] != *"\"") {
      if ((*data)[*idx] == *"\\" && (*data)[*idx + 1] == *"\"") {
        (*str) += *"\"";
        ++*idx;
        continue;
      }
      (*str) += (*data)[*idx];
      //      printf("%c\n",(*data)[*idx]);
    }
    if ((*data)[*idx] != *"\"")
      return false;
    return true;
  }

  template <typename T>
  bool _parse_immediate_integer(std::vector<char> *data, un_size_t *idx, T* integer) {
    *integer = 0;
    bool neg = false;
    if ((*data)[*idx] == *"-") {
      neg = true;
      ++*idx;
    } else if ((*data)[*idx] < 48 || (*data)[*idx] > 57) {
      return false;
    }
    --*idx;
    while ((*data)[++*idx] >= 48 && (*data)[*idx] <= 57) {
      *integer *= 10;
      *integer += (*data)[*idx] - 48;
    }
    if (neg)
      *integer *= -1; 
    return true;
  }

  template <typename T>
  bool _parse_immediate_float(std::vector<char> *data, un_size_t *idx,
                              T *floater) {
    *floater = 0;
    bool neg = false;
    if ((*data)[*idx] == *"-") {
      neg = true;
      ++*idx;
    } else if ((*data)[*idx] < 48 || (*data)[*idx] > 57) {
      return false;
    }
    --*idx;
    while ((*data)[++*idx] >= 48 && (*data)[*idx] <= 57) {
      *floater *= 10;
      *floater += (*data)[*idx] - 48;
    }
    if ((*data)[*idx] != *".") {
      return false;
    }
    T radix = 1;
    while ((*data)[++*idx] >= 48 && (*data)[*idx] <= 57) {
      radix /= 10.0f;
      *floater += ((*data)[*idx] - 48) * radix;
    }
    if (neg)
      *floater *= -1;
    return true;
  }

  bool _parse_immediate_boolean(std::vector<char> *data, un_size_t *idx,
                                bool *boolin) {
    const char* T = "true";
    const char* F = "false";
    un_size_t lidx = 0;
    bool es = ((*data)[*idx] == *"t");
    --*idx;
    while (es ? T[lidx] : F[lidx]) {
      if (((*data)[++*idx]) != (es ? T[lidx] : F[lidx])) {
        return false;
      }
      ++lidx;
      if (lidx >= strlen(es ? T : F)) {
        break;
      }
    }
    ++*idx;
    *boolin = es;
    return true;
  }

  un_size_t _get_flag(SizedBlock* block) {
    switch (block->meta) {
      case SB_META_BOOLEAN:
        return SB_FLAG_BOOLEAN;
        break;
      case SB_META_STRING:
        return SB_FLAG_STRING;
        break;
      case SB_META_INT_STYLE: {
        switch (block->element_span){
        case 1:
          return SB_FLAG_I8;
          break;
        case 2:
          return SB_FLAG_I16;
          break;
        case 4:
          return SB_FLAG_I32;
          break;
        case 8:
          return SB_FLAG_I64;
          break;
        default:
          return SB_FLAG_UNDEFINED;
          break;
        }
      } break;
      case SB_META_FLOAT_STYLE: {
        switch (block->element_span) {
        case sizeof(float):
          return SB_FLAG_FLOAT;
          break;
        case sizeof(double):
          return SB_FLAG_DOUBLE;
          break;
        case sizeof(long double):
          return SB_FLAG_LONG_DOUBLE;
          break;
        default:
          return SB_FLAG_UNDEFINED;
          break;
        }
      } break;
      default:
        return SB_FLAG_UNDEFINED;
      }
  }

  template<typename T>
  std::string _ptts(T *ptr) {
    std::string str = std::to_string(*ptr);
    return str;
  }

  std::string _add_escapes_to_string_readable(std::string str){
    std::string new_string;
    for(int i = 0; i < str.length(); i ++){
      if(str[i] == *"\"")
        new_string += *"\\";
      new_string += str[i];
    }
    return new_string;
  }

  std::string _value_string(SizedBlock *bloc) { //Needs some drying
    std::string d = "";
    unsigned char meta = bloc->meta;
    char flag = _get_flag(bloc);
    if (flag == SB_FLAG_STRING) {
      d += flag;
      d += "\"";
      d += _add_escapes_to_string_readable(std::string((char*)(bloc->contents_native)));
      d += "\"";
    } else {      
      d += flag;
      d += "[ ";
      switch (meta) {
      case SB_META_INT_STYLE:{
        /* for (un_size_t i = 0; i < bloc->span; i += bloc->element_span) {
          d += _ptis((char*)(bloc->contents_native) + (size_t)i,
          bloc->element_span); if (i + bloc->element_span < bloc->span) d +=
          ",";
            }*/
        switch (bloc->element_span) {
        case sizeof(int8_t):
          for (un_size_t i = 0; i < bloc->span; i += bloc->element_span) {
            d += _ptts<int8_t>((int8_t *)((char *)bloc->contents_native + size_t(i)));
            if (i + bloc->element_span < bloc->span)
              d += ",";
          }
          break;
        case sizeof(uint16_t):
          for (un_size_t i = 0; i < bloc->span; i += bloc->element_span) {
            d += _ptts<int16_t>((int16_t *)((char *)bloc->contents_native + size_t(i)));
            if (i + bloc->element_span < bloc->span)
              d += ",";
          }
          break;
          break;
        case sizeof(uint32_t):
          for (un_size_t i = 0; i < bloc->span; i += bloc->element_span) {
            d += _ptts<int32_t>((int32_t *)((char *)bloc->contents_native + size_t(i)));
            if (i + bloc->element_span < bloc->span)
              d += ",";
          }
          break;
        case sizeof(uint64_t):
          for (un_size_t i = 0; i < bloc->span; i += bloc->element_span) {
            d += _ptts<int64_t>((int64_t *)((char *)bloc->contents_native + size_t(i)));
            if (i + bloc->element_span < bloc->span)
              d += ",";
          }
          break;
        }
        break;
      }
      case SB_META_FLOAT_STYLE:
        if(bloc->element_span == sizeof(float)){
          for (un_size_t i = 0; i < bloc->span; i += bloc->element_span) {
            d += _ptts<float>((float *)((char *)bloc->contents_native + size_t(i)));
            if (i + bloc->element_span < bloc->span)
              d += ",";
          }
        } else if (bloc->element_span == sizeof(double)) {
          for (un_size_t i = 0; i < bloc->span; i += bloc->element_span) {
            d += _ptts<double>((double *)((char *)bloc->contents_native + size_t(i)));
            if (i + bloc->element_span < bloc->span)
              d += ",";
          }
        } else if (bloc->element_span == sizeof(long double)) {
          for (un_size_t i = 0; i < bloc->span; i += bloc->element_span) {
            d += _ptts<long double>((long double *)((char *)bloc->contents_native + size_t(i)));
            if (i + bloc->element_span < bloc->span)
              d += ",";
          }
        }
        break;
      case SB_META_BOOLEAN:
        for (un_size_t i = 0; i < bloc->span; i += bloc->element_span){
          d += (*(bool *)((char *)bloc->contents_native + (size_t)i)) ? "true" : "false";
          if (i + bloc->element_span < bloc->span)
            d += ",";
        }
        break;
      default:
        break;
      }
      d += "]";
    }
    return d;
  }


#define _return_if_EOF(container, idx)                                          \
  if (idx >= container->size())                                                \
    return false;
#define _rassert_token(ptr, idx, tok)                                          \
  if (ptr[idx] != tok)                                                         \
    return false;
#define bp(k)   \
  printf("Breakpoint: %s\n",k);

  bool CompoundNode::deserialize_readable(std::string data) {
    std::vector<char> vec;
    vec.resize(data.size() + 1);
    memcpy(vec.data(), data.data(), data.size() + 1);
    un_size_t t;
    return deserialize_readable(&vec, 0, &t);
  }

  SizedBlock *_parse_value_h(std::vector<char> *vdata, un_size_t *idx) { //needs some drying as well
    SizedBlock* block = new SizedBlock;
    char* data = vdata->data();
    switch (data[*idx]) {
    case SB_FLAG_UNDEFINED:
      --*idx;
      while ((*vdata)[++*idx] != *"]") {};
      return block;
      break;
    case SB_FLAG_I8:{
      *idx += 2;
      std::vector<char> all;
      while (true) {
        _skip_whitespace(vdata, idx, 0);
        char r = 0;
        if (!_parse_immediate_integer<char>(vdata, idx, &r)) {
          delete block;
          return nullptr;
        }
        all.push_back(r);
        _skip_whitespace(vdata, idx, 0);
        //        printf("parsed i8/u8 %d [%ld]\n",r,all.size()-1);
        if (data[*idx] == *","){
          ++*idx;
          continue;
        }
        if (data[*idx] == *"]")
          break;
      }
      block->span = sizeof(uint8_t) * all.size();
      block->element_span = sizeof(uint8_t);
      block->contents_native = malloc(sizeof(uint8_t)*all.size());
      memcpy(block->contents_native, all.data(), sizeof(uint8_t) * all.size());
      block->meta = SB_META_INT_STYLE;
      break;
    }
    case SB_FLAG_I16:{
      *idx += 2;
      std::vector<int16_t> all;
      while (true) {
        _skip_whitespace(vdata, idx, 0);
        uint16_t r = 0;
        if (!_parse_immediate_integer<uint16_t>(vdata, idx, &r)) {
          delete block;
          return nullptr;
        }
        all.push_back(r);
        _skip_whitespace(vdata, idx, 0);
        //        printf("parsed i16/u16 %d [%ld]\n",r,all.size()-1);
        if (data[*idx] == *","){
          ++*idx;
          continue;
        }
        if (data[*idx] == *"]")
          break;
      }
      block->span = sizeof(uint16_t) * all.size();
      block->element_span = sizeof(uint16_t);
      block->contents_native = malloc(sizeof(uint16_t)*all.size());
      memcpy(block->contents_native, all.data(), sizeof(uint16_t) * all.size());
      block->meta = SB_META_INT_STYLE;
      break;
    }
    case SB_FLAG_I32: {
      *idx += 2;
      std::vector<int32_t> all;
      while (true) {
        _skip_whitespace(vdata, idx, 0);
        uint32_t r = 0;
        if (!_parse_immediate_integer<uint32_t>(vdata, idx, &r)) {
          delete block;
          return nullptr;
        }
        all.push_back(r);
        _skip_whitespace(vdata, idx, 0);
        //        printf("parsed i32/u32 %d [%ld]\n",r,all.size()-1);
        if (data[*idx] == *","){
          ++*idx;
          continue;
        }
        if (data[*idx] == *"]")
          break;
      }
      block->span = sizeof(uint32_t) * all.size();
      block->element_span = sizeof(uint32_t);
      block->contents_native = malloc(sizeof(uint32_t)*all.size());
      memcpy(block->contents_native, all.data(), sizeof(uint32_t) * all.size());
      block->meta = SB_META_INT_STYLE;
      break;
    }
    case SB_FLAG_I64: {
      *idx += 2;
      std::vector<int64_t> all;
      while (true) {
        _skip_whitespace(vdata, idx, 0);
        uint64_t r = 0;
        if (!_parse_immediate_integer<uint64_t>(vdata, idx, &r)) {
          delete block;
          return nullptr;
        }
        all.push_back(r);
        _skip_whitespace(vdata, idx, 0);
        //        printf("parsed i64/u64 %ld [%ld]\n",r,all.size()-1);
        if (data[*idx] == *","){
          ++*idx;
          continue;
        }
        if (data[*idx] == *"]")
          break;
      }
      block->span = sizeof(uint64_t) * all.size();
      block->element_span = sizeof(uint64_t);
      block->contents_native = malloc(sizeof(uint64_t)*all.size());
      memcpy(block->contents_native, all.data(), sizeof(uint64_t) * all.size());
      block->meta = SB_META_INT_STYLE;
      break;
    }
    case SB_FLAG_FLOAT:{
      *idx += 2;
      std::vector<float> all;
      while (true) {
        _skip_whitespace(vdata, idx, 0);
        float r = 0;
        if (!_parse_immediate_float<float>(vdata, idx, &r)) {
          delete block;
          return nullptr;
        }
        all.push_back(r);
        _skip_whitespace(vdata, idx, 0);
        if (data[*idx] == *","){
          ++*idx;
          continue;
        }
        if (data[*idx] == *"]")
          break;
      }
      block->span = sizeof(float) * all.size();
      block->element_span = sizeof(float);
      block->contents_native = malloc(sizeof(float)*all.size());
      memcpy(block->contents_native, all.data(), sizeof(float) * all.size());
      block->meta = SB_META_FLOAT_STYLE;
      break;
    }
    case SB_FLAG_DOUBLE:{
      *idx += 2;
      std::vector<double> all;
      while (true) {
        _skip_whitespace(vdata, idx, 0);
        double r = 0;
        if (!_parse_immediate_float<double>(vdata, idx, &r)) {
          delete block;
          return nullptr;
        }
        all.push_back(r);
        _skip_whitespace(vdata, idx, 0);
        if (data[*idx] == *","){
          ++*idx;
          continue;
        }
        if (data[*idx] == *"]")
          break;
      }
      block->span = sizeof(double) * all.size();
      block->element_span = sizeof(double);
      block->contents_native = malloc(sizeof(double)*all.size());
      memcpy(block->contents_native, all.data(), sizeof(double) * all.size());
      block->meta = SB_META_FLOAT_STYLE;
      break;
    }
    case SB_FLAG_LONG_DOUBLE:{
      *idx += 2;
      std::vector<long double> all;
      while (true) {
        _skip_whitespace(vdata, idx, 0);
        long double r = 0;
        if (!_parse_immediate_float<long double>(vdata, idx, &r)) {
          delete block;
          return nullptr;
        }
        all.push_back(r);
        _skip_whitespace(vdata, idx, 0);
        if (data[*idx] == *","){
          ++*idx;
          continue;
        }
        if (data[*idx] == *"]")
          break;
      }
      block->span = sizeof(long double) * all.size();
      block->element_span = sizeof(long double);
      block->contents_native = malloc(sizeof(long double)*all.size());
      memcpy(block->contents_native, all.data(), sizeof(long double) * all.size());
      block->meta = SB_META_FLOAT_STYLE;
      break;
    }
    case SB_FLAG_BOOLEAN:{
      *idx += 2;
      std::vector<uint8_t> all;
      while (true) {
        _skip_whitespace(vdata, idx, 0);
        bool r = 0;
        if (!_parse_immediate_boolean(vdata, idx, &r)) {
          delete block;
          return nullptr;
        }
        all.push_back((uint8_t)r);
        _skip_whitespace(vdata, idx, 0);
        if (data[*idx] == *","){
          ++*idx;
          continue;
        }
        if (data[*idx] == *"]")
          break;
      }
      block->span = sizeof(uint8_t) * all.size();
      block->element_span = sizeof(uint8_t);
      block->contents_native = malloc(sizeof(uint8_t)*all.size());
      memcpy(block->contents_native, all.data(), sizeof(uint8_t) * all.size());
      block->meta = SB_META_BOOLEAN;
      break;
    }
    case SB_FLAG_STRING: {
      ++*idx;
      std::string nstr;
      if (!_parse_next_enquoted_string(vdata, idx, &nstr)) {
        delete block;
        return nullptr;
      }
      block->span = nstr.size() + 1;
      block->element_span = sizeof(char);
      block->contents_native = malloc(nstr.size() + 1);
      memcpy(block->contents_native, nstr.data(), nstr.size());
      ((char*)block->contents_native)[nstr.size()] = 0;
      block->assign_meta(SB_META_STRING);
      break;
    }
    default:
      delete block;
      return nullptr;
    }
    return block;
  }
  
  bool CompoundNode::deserialize_readable(std::vector<char>* vdata, un_size_t sidx, un_size_t* endidx) {
    un_size_t idx = sidx;
    char *data = vdata->data();
    _return_if_EOF(vdata, idx);
    _skip_whitespace(vdata, &idx, 0);
    _rassert_token(data, idx, *"{");
    ++idx;
    _return_if_EOF(vdata, idx);
    _skip_whitespace(vdata, &idx, 0);
    _rassert_token(data, idx, *"\"");
    
    while (true) {
      std::string key;
      //      bp("prekey");
      //      printf("%d\n",idx);
      if (!_parse_next_enquoted_string(vdata, &idx, &key))
        return false;
      //      bp("postkey");
      ++idx;
      _skip_whitespace(vdata, &idx, 0);
      _rassert_token(data, idx, *":");
      ++idx;
      _skip_whitespace(vdata, &idx, 0);
      if (data[idx] == *"{") {
        CompoundNode *newnode = new CompoundNode;
        //        printf("prechar: %c\n", data[idx]);
        if(!newnode->deserialize_readable(vdata, idx, &idx)){
          delete newnode;
          //          printf("nested node is kaka %s %d\n", key.c_str(), idx);
          return false;
        }
        //        printf("ended nodep on index %d\n", idx);
        this->put(key, newnode);
        delete newnode;
      } else if (data[idx] == *"[") {
        ++idx;
        _return_if_EOF(vdata, idx);
        while (true) {
          _skip_whitespace(vdata, &idx, 0);
          _rassert_token(data, idx, *"{");
          //          bp("prenode");
          CompoundNode *newnode = new CompoundNode;
          if (!(newnode->deserialize_readable(vdata, idx, &idx))) {
            delete newnode;
            //            printf("bruh cannot parse array-enclosed node\n");
            return false;
          }
          this->put_back(key, newnode);
          //          printf("ended array->nodep on index %d\n", idx);
          delete newnode;
          ++idx;
          _skip_whitespace(vdata, &idx, 0);
          //          printf("after node parsing: %d\n",idx);
          if (data[idx] == *",") {
            ++idx;
            continue;
          }
          if (data[idx] == *"]")
            break;
          return false;
        }
        //        printf("ended arrayp on index %d\n",idx);
      } else {
        SizedBlock *block = _parse_value_h(vdata, &idx);
        if(block == nullptr) return false;
        if (exists_key(&(this->generic_tags), key)){
            delete block;
            return false;
        };
        this->generic_tags[key] = block;        
      }
      ++idx;
      //      printf("%s\n",data + idx);
      _skip_whitespace(vdata, &idx, 0);
      if (data[idx] == *",") {
        ++idx;
        continue;
      };
      if(data[idx] == *"}") break;
    }
    *endidx = idx;
    return true;
  }

  std::string CompoundNode::serialize_readable(bool omit_undefined) { //Can't think of varnames 
    std::string d = "{ ";
    un_size_t loop = 0;
    for (std::pair<std::string, SizedBlock*> pair : generic_tags) {
      d += "\"";
      d += _add_escapes_to_string_readable(pair.first);
      d += "\" : ";
      d += _value_string(pair.second);
      if (++loop < generic_tags.size() || !child_nodes.empty() || !child_node_lists.empty())
        d += ", ";
    }
    loop = 0;
    for (std::pair<std::string, CompoundNode *> pair : child_nodes) {
      d += "\"";
      d += _add_escapes_to_string_readable(pair.first);
      d += "\" : ";
      d += pair.second->serialize_readable(omit_undefined);
      if (++loop < child_nodes.size() || !child_node_lists.empty())
        d += ", ";
    }
    loop = 0;
    for (std::pair <std::string,std::vector<CompoundNode *>> pair : child_node_lists) {
      d += "\"";
      d += _add_escapes_to_string_readable(pair.first);
      d += "\" : [";
      un_size_t loop2 = 0;
      for (CompoundNode *inode : pair.second) {
        d += inode->serialize_readable(omit_undefined);
        if (++loop2 < pair.second.size())
          d += ", ";
      }
      d += "]";
      if (++loop < child_node_lists.size())
        d += ", ";
    }
    d += "}";
    return d;
  }

  bool CompoundNode::decode_deserialize(std::string data){
    if(base64::diagnose(data) != 0) {
      return false;
    };
    std::vector<char> decoded = base64::decode(data);
    return deserialize(&decoded, 0, nullptr);
  }
  
  std::string CompoundNode::serialize_encode(){
    std::vector<char> to_encode = serialize();
    return base64::encode(to_encode);
  }

  bool CompoundNode::operator[](std::string key){
    return (exists_key<SizedBlock*>(&generic_tags, key) || exists_key<CompoundNode*>(&child_nodes, key));
  }
  
  
  bool CompoundNode::deserialize(std::vector<char>* data, un_size_t start_index, un_size_t* end_index){
    if(data->size() == 0 || data->size() <= start_index) return false;
    CompoundNode new_node;
    un_size_t index = start_index;
    un_size_t max_index = data->size() - 1;
    _skip_to_flag(COMPOUND_NODE_BEGIN_FLAG, data, &index, 1);
    while(true){
      if(index > max_index) return false;
      if((*data)[index] == COMPOUND_NODE_END_FLAG) break;
      _skip_to_flag(COMPOUND_NODE_BEGIN_STRING_FLAG, data, &index, 1); if(index > max_index) return false;
      std::string key = _read_as_string_until_element(data, &index);
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
      case COMPOUND_NODE_BEGIN_BLOCK_FLAG:{
        ++index;
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
      if(str[i] == COMPOUND_NODE_BEGIN_ELEMENT_FLAG)
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
      data.push_back(COMPOUND_NODE_BEGIN_ELEMENT_FLAG);
      data.push_back(COMPOUND_NODE_BEGIN_BLOCK_FLAG);
      std::vector<char> block = pair.second->lower();
      
      data.insert(data.end(), block.begin(), block.end());
    }
    for(std::pair<std::string, CompoundNode*> pair : child_nodes){
      std::string escaped = _add_escapes_to_string(pair.first);      
      data.push_back(COMPOUND_NODE_BEGIN_STRING_FLAG);
      data.insert(data.end(), escaped.begin(), escaped.end());
      data.push_back(COMPOUND_NODE_BEGIN_ELEMENT_FLAG);
      std::vector<char> node_serial = pair.second->serialize();
      data.insert(data.end(), node_serial.begin(), node_serial.end());
    }
    for(std::pair<std::string, std::vector<CompoundNode*>> pair: child_node_lists){
      std::string escaped = _add_escapes_to_string(pair.first);
      data.push_back(COMPOUND_NODE_BEGIN_STRING_FLAG);
      data.insert(data.end(), escaped.begin(), escaped.end());
      data.push_back(COMPOUND_NODE_BEGIN_ELEMENT_FLAG);
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
        json += "\"" + pair.first + "\": " + std::to_string((long int)pair.second->contents_native) + " (" + std::to_string((int)pair.second->meta) + ") ";
      }else if(pair.second->element_span == 1){
        json += "\"" + pair.first + "\": \"" + std::string((const char*)pair.second->contents_native) + "\" (" + std::to_string((int)pair.second->meta) + ") ";
      } else {
        json += "[";
        for(un_size_t i = 0; i < pair.second->span/pair.second->element_span; i++){
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
      un_size_t c1 = 0;
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

  un_size_t CompoundNode::get_node_list_length(std::string key){
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

  un_size_t CompoundNode::get_string_length(std::string key){
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
  SizedBlock* CompoundNode::put_string(std::string key, std::vector<T>* vars){
    T* p_vars = vars->data();
    un_size_t amount = vars->size();
    burninate_generic_if_exists(key);
    SizedBlock* ptr = SizedBlock(sizeof(T), amount, p_vars);
    generic_tags[key] = ptr;
    return ptr;
  }

  template<typename T>
  SizedBlock* CompoundNode::put_string (std::string key, un_size_t amount, T* vars){
    T* p_vars = vars;
    burninate_generic_if_exists(key);
    SizedBlock* ptr = new SizedBlock(sizeof(T), amount, p_vars);
    generic_tags[key] = ptr;
    return ptr;
  }
  
  template<typename T>
  SizedBlock* CompoundNode::put(std::string key, T var){
    T var_copp = var;
    burninate_generic_if_exists(key);
    SizedBlock* ptr = new SizedBlock(sizeof(T), 1, &var_copp);
    generic_tags[key] = ptr;
    return ptr;
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
    block->meta = meta;
  }

  SizedBlock::SizedBlock(uint16_t element_size, un_size_t element_count, void* data){
    contents_native = malloc(element_size * element_count);
    span = element_count * element_size;
    element_span = element_size;
    memcpy(contents_native, data, element_size * element_count);
  }

  std::vector<char> SizedBlock::lower(){
    std::vector<char> contents(sizeof(uint8_t) + sizeof(uint16_t) + sizeof(un_size_t) + span);
    uint16_t element_d = little_endian<uint16_t>(element_span);
    un_size_t span_d = little_endian<un_size_t>(span);
    contents.data()[0] = meta; //byte 1
    memcpy(contents.data()+sizeof(uint8_t),(char*)&element_d,sizeof(uint16_t)); //byte 2,3
    memcpy(contents.data()+sizeof(uint8_t)+sizeof(uint16_t),(char*)&span_d,sizeof(un_size_t)); //bytes 3,4,5,6
    void* contents_little_endian = contents_native;
    if(is_big_endian()){
      contents_little_endian = invert_endian_h(element_span, span/element_span, contents_native);
    }
    memcpy(contents.data()+sizeof(uint8_t)+sizeof(uint16_t)+sizeof(un_size_t),(char*)contents_native,span);
    if(is_big_endian()){
      free(contents_little_endian);
    }
    return contents;
  }

  char* SizedBlock::upper(char* data, char* maxaddress){ //returns the address AFTER all the data used
    if(span) dump();
    char* max = maxaddress + 1;
    if(max - data < sizeof(uint8_t) + sizeof(uint16_t) + sizeof(un_size_t)) return nullptr;
    uint16_t element_size;
    memcpy(&element_size, data + sizeof(uint8_t), sizeof(uint16_t));
    element_span = little_endian<uint16_t>(element_size);// also goes from little to native
    un_size_t total_size;
    memcpy(&total_size, data + sizeof(uint8_t) + sizeof(uint16_t), sizeof(un_size_t));
    span = little_endian<un_size_t>(total_size);
    char* data_after_header = data + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(un_size_t);
    if(max - data < total_size + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(un_size_t)){
      meta = 0;
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
    meta = data[0];
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
    meta = 0;
    element_span = 0;
  }

  void SizedBlock::assign_meta(uint8_t value){
    meta = value;
  }

  SizedBlock::~SizedBlock(){
      dump();
  }


}
#undef un_size_t
#undef _return_if_EOF
#undef _rassert_token
#undef bp
