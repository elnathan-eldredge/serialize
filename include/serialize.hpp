#pragma once
/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

The original creator of this software is:

Elnathan Eldredge

It is not required to cite the author in any copy or derivative of this software

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

See also https://unlicense.org/

May 27, 2024
*/

// The c version of certain libraries are used
// because they provide more meticulous RAII control
#include <cstdint>
#include <istream>
#include <unordered_map>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <stack>
#include <string.h>

#define EE_Serialize

// the size of memory address must be 8 bytes (For cross-compatibility. Only this file)
#define un_size_t uint64_t

#define COMPOUND_NODE_BEGIN_FLAG (char)123
#define COMPOUND_NODE_END_FLAG (char)125
#define COMPOUND_NODE_ESCAPE_KEY_FLAG (char)92
#define COMPOUND_NODE_BEGIN_STRING_FLAG (char)44
#define COMPOUND_NODE_BEGIN_ELEMENT_FLAG (char)58
#define COMPOUND_NODE_BEGIN_BLOCK_FLAG (char)45
#define COMPOUND_NODE_BEGIN_LIST_FLAG (char)91
#define COMPOUND_NODE_END_LIST_FLAG (char)93

#define COMPOUND_NODE_BEGIN_FLAG_R *"{"
#define COMPOUND_NODE_BEGIN_STRING_R *"\""
#define COMPOUND_NODE_END_STRING_R *"\""
#define COMPOUND_NODE_ESCAPE_STRING_R *"\\"
#define COMPOUND_NODE_END_R *"}"
#define COMPOUND_NODE_KEY_VALUE_SEPERATOR *":"
#define COMPOUND_NODE_BEGIN_ARRAY_R *"["
#define COMPOUND_NODE_END_ARRAY_R *"]"
#define COMPOUND_NODE_ITEM_SEPERATOR_R *","

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

#define READABLE_COMMENT_CHECK *"/"
#define READABLE_ENDLINE_COMMENT_START *"/"
#define READABLE_COMMENT_VARIABLE_DELIM *"*"
#define READABLE_ENDLINE_DEFINITION 10

#define serialize Serialize
#define compound_node CompoundNode

#define bp(k)   \
  printf("Breakpoint: %s\n",k);

// The programmer has the option to not include the internal
// implementation of the base64 translater
#ifndef SERIALIZE_NO_IMPLEMENT_b64
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

    //this function is used to determine wether a string can be
    // successfully decoded
    un_size_t diagnose(std::string data){
      un_size_t idx;
      for(char c : data){
        ++idx;
        if(!(detail::numtable[(size_t)c] + 1))
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
        unsigned char ans = detail::numtable[(size_t)dat[i]];
        //      printf("I: %d, %c\n", ans, data[i]);
        if(ans == (unsigned char)(SRLS_EQ_ESCAPE_CODE)) break;
        if(ans == (unsigned char)(255)){
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
      for(unsigned char byte : b64raw){
        if(byte == (unsigned char)(SRLS_EQ_ESCAPE_CODE)) break;
        switch(cur64idx % 4){
        case 0:
          output.push_back(0);
          output[nextemptyidx] |= (byte & detail::d1_1_mask) << detail::s1_1_rlshift;
          nextemptyidx += 1;
          break;
        case 1:
          output.push_back(0);
          output[nextemptyidx - 1] |= (byte & detail::d2_1_mask) >> detail::s1_2_lrshift;
          output[nextemptyidx] |= (byte & detail::d2_2_mask) << detail::s2_2_rlshift;
          nextemptyidx += 1;
          break;
        case 2:
          output.push_back(0);
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

  //Inverts the endianness of data in an array
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

  //helper function because according to spec, the [] operator inserts an element
  template<typename T>
  bool exists_key(std::unordered_map<std::string,T>* map, std::string key){
    return map->find(key) != map->end();
  }

  //force type T to be little endian
  template<typename T>
  T little_endian(T d){
    if(!is_big_endian()) return d;
    T d_copp = d;
    T* d_corr = (T*)invert_endian_h(sizeof(T), 1, &d_copp);
    d_copp = *d_corr;
    free(d_corr);
    return d_copp;
  }

  //This class is a fancy wrapper around
  // a basic array.
  class SizedBlock{
  public:
    void* contents_native;
    uint16_t element_span = 0;
    un_size_t span = 0;
    uint8_t meta = 0;
    
    SizedBlock(uint16_t element_size, un_size_t element_count, void* data);

    std::vector<char> lower();

    static constexpr un_size_t header_size_bytes(){
      return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(un_size_t);
    }
    
    SizedBlock(){
      span = 0;
      element_span = 0;
      contents_native = nullptr;
    };

    static un_size_t interpret_size_from_header(std::vector<char>& data){
      un_size_t dsize_u;
      memcpy(&dsize_u,((char*)data.data()) + sizeof(uint8_t) + sizeof(uint16_t),sizeof(un_size_t));
      un_size_t total_span = little_endian<un_size_t>(dsize_u);
      return total_span;
    }

    char* upper(char* data, char* max);

    uint64_t upper(std::vector<char>& data, uint64_t starting_index);
    
    void dump();

    void copy_to(SizedBlock* target);

    void assign_meta(uint8_t meta);

    ~SizedBlock();
  };

  // This data structure represents a serization
  class CompoundNode {
  public:
    std::unordered_map<std::string,SizedBlock*> generic_tags;
    std::unordered_map<std::string,CompoundNode*> child_nodes;
    std::unordered_map<std::string,std::vector<CompoundNode*>> child_node_lists;

    CompoundNode(){};

    bool empty();

    template<typename T> SizedBlock* put(std::string key, T var);

    template<typename T> SizedBlock* put_string(std::string key, un_size_t amount, T* vars);

    template<typename T> SizedBlock* put_string(std::string key, std::vector<T>& vars);

    void put(std::string key, CompoundNode& node);

    void put(std::string key, std::vector<CompoundNode*>& nodes);

    void put_back(std::string key, CompoundNode& node);

    template<typename T> bool has_compat(std::string key);

    template<typename T> bool has_compat_string(std::string key);

    bool has_node(std::string key);

    bool has_node_list(std::string key);

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

    bool deserialize(std::vector<char>& data, un_size_t start_index, un_size_t* end_index);

    std::string serialize_encode();

    bool decode_deserialize(std::string data);

    std::string serialize_readable(bool omit_undefined);

  private:

    bool deserialize_readable(std::vector<char> &data, un_size_t start_index,
                              un_size_t *end_index);

  public:
    
    bool deserialize_readable(std::string data);
    
    ~CompoundNode();

    void destroy_children();
  };

  namespace Binary {
    class PushdownParser;

    enum ParserState {
      AwaitBegin,                //0
      AwaitKeyStart,             //1
      ConstructKey,              //2 
      ConstructKeyEscape,        //3 
      GetIndicator,              //4 
      AwaitNodeArrayNode,        //5
      AquireNodeHeader,          //6
      AquireNodeData,            //7
      Success,                   //8
      Error,                     //9 
      Warning                    //10
    };

    struct ParserData {
      CompoundNode* node = NULL;
      ParserState current_state = AwaitBegin;
      std::string current_string = "";
      std::string current_key = "";
      std::vector<char> node_data;
      uint64_t node_data_counter = 0;
      uint64_t node_data_left = 0;
    };

    class PushdownParser{
    public:
      std::stack<ParserData> state_stack;
      ParserData state;

      PushdownParser();

      ParserState consume(char c);

      void merge_to(CompoundNode* node);

      ~PushdownParser();
    };
    
  }

  namespace Readable {

  class PushdownParser;
    
    enum ParserState {
      AwaitStart,                       // 0
      AwaitKey,                         // 1
      ConstructKey,                     // 2
      ConstructKeyEscape,               // 3
      AwaitKeyValueSeperator,           // 4
      AwaitValueTypeIdentifier,         // 5
      AwaitValue,                       // 6
      ConstructValueStringEscape,       // 7
      ConstructValueString,             // 8
      AwaitValueParsable,               // 9
      ConstructValueParsable,           // 10
      AwaitValueParsableSeperator,      // 11
      ConstructNodeArrayAwaitNode,      // 12
      ConstructNodeArrayAwaitSeperator, // 13
      AwaitItemSeperator,               // 14
      CheckComment,                     // 15
      EndlineComment,                   // 16
      VariableComment,                  // 17
      PossibleCommentEnd,               // 18
      Success,                          // 19
      Error,                            // 20
      Warning                           // 21
    };

    struct ParserData {
      ParserState current_state = AwaitStart;
      ParserState next_state = Error;
      CompoundNode *node = NULL;
      std::string current_construction = "";
      char current_value_type = 0;
      std::string current_key = "";
      std::vector<std::string> value_constructions = std::vector<std::string>();
    };

    class PushdownParser {
    public:
      
      std::stack<ParserData> state_stack;
      ParserData state;

      PushdownParser();

      ParserState consume(char c);

      void merge_to(CompoundNode* node);

      ~PushdownParser();
    };    
  }

  bool CompoundNode::empty() {
    return child_nodes.empty() && generic_tags.empty() && child_node_lists.empty();
  }

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
    std::string new_str = "";
    for(const char c: str){
      if(c == COMPOUND_NODE_END_STRING_R){
        new_str += COMPOUND_NODE_ESCAPE_STRING_R;
      }
      new_str += c;
    }
    return new_str;
  }

  std::vector<char> _add_escapes_to_string_readable(std::vector<char> str){
    std::vector<char> new_string;
    for(const char c : str){
      if(c == 0){
        continue;
      }
      if(c == COMPOUND_NODE_END_STRING_R){
        new_string.push_back(COMPOUND_NODE_ESCAPE_STRING_R);
      }
      new_string.push_back(c);
    }
    return new_string;
  }

  std::string _value_string(SizedBlock *bloc) {
    std::string d = "";
    unsigned char meta = bloc->meta;
    char flag = _get_flag(bloc);
    if (flag == SB_FLAG_STRING) {
      d += flag;
      d += "\"";
      std::vector<char> strval = std::vector<char>(bloc->span);
      memcpy(strval.data(),bloc->contents_native,strval.size());
      std::vector<char> nval = _add_escapes_to_string_readable(strval);
      for(const char nc: nval){
        d += nc;
      }
      d += "\"";
    } else {      
      d += flag;
      d += "[ ";
      switch (meta) {
      case SB_META_INT_STYLE:{
        switch (bloc->element_span) {
        default:
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

  bool CompoundNode::deserialize_readable(std::string data) {
    std::vector<char> vec;
    vec.resize(data.size() + 1);
    memcpy(vec.data(), data.data(), data.size() + 1);
    un_size_t t;
    return deserialize_readable(vec, 0, &t);
  }

  bool CompoundNode::deserialize_readable(std::vector<char> &vdata, un_size_t sidx, un_size_t *endidx) {

    Readable::PushdownParser parser = Readable::PushdownParser();

    std::vector<char>::iterator it;
    Readable::ParserState state = Readable::Warning;
    for (it = vdata.begin() + sidx; it != vdata.end(); ++it) {
      state = parser.consume(*it);
      //      printf("%d \"%c\"\n",state,*it);
      if (state == Readable::Error)
        return false;
      if (state == Readable::Warning)
        parser.state.current_state = parser.state.next_state;
      if (state == Readable::Success)
        break;
    }

    if(state != Readable::Success)
      return false;

    if(endidx != nullptr)
      *endidx =
        it - vdata.begin();

    destroy_children();

    parser.merge_to(this);
    
    return true;
  }

  std::string CompoundNode::serialize_readable(bool omit_undefined) {
    std::string serialization = "{ ";
    un_size_t loop = 0;
    for (std::pair<std::string, SizedBlock*> pair : generic_tags) {
      serialization += "\"";
      serialization += _add_escapes_to_string_readable(pair.first);
      serialization += "\" : ";
      serialization += _value_string(pair.second);
      if (++loop < generic_tags.size() || !child_nodes.empty() || !child_node_lists.empty())
        serialization += ", ";
    }
    loop = 0;
    for (std::pair<std::string, CompoundNode *> pair : child_nodes) {
      serialization += "\"";
      serialization += _add_escapes_to_string_readable(pair.first);
      serialization += "\" : ";
      serialization += pair.second->serialize_readable(omit_undefined);
      if (++loop < child_nodes.size() || !child_node_lists.empty())
        serialization += ", ";
    }
    loop = 0;
    for (std::pair <std::string,std::vector<CompoundNode *>> pair : child_node_lists) {
      serialization += "\"";
      serialization += _add_escapes_to_string_readable(pair.first);
      serialization += "\" : [";
      un_size_t loop2 = 0;
      for (CompoundNode *inode : pair.second) {
        serialization += inode->serialize_readable(omit_undefined);
        if (++loop2 < pair.second.size())
          serialization += ", ";
      }
      serialization += "]";
      if (++loop < child_node_lists.size())
        serialization += ", ";
    }
    serialization += "}";
    return serialization;
  }

  bool CompoundNode::decode_deserialize(std::string data){
    if(base64::diagnose(data) != 0) {
      return false;
    };
    std::vector<char> decoded = base64::decode(data);
    return deserialize(decoded, 0, nullptr);
  }
  
  std::string CompoundNode::serialize_encode(){
    std::vector<char> to_encode = serialize();
    return base64::encode(to_encode);
  }

  bool CompoundNode::operator[](std::string key){
    return (exists_key<SizedBlock*>(&generic_tags, key));
  }
  
  
  bool CompoundNode::deserialize(std::vector<char>& data, un_size_t start_index, un_size_t* end_index){
    Binary::PushdownParser parser = Binary::PushdownParser();
    
    std::vector<char>::iterator it;
    Binary::ParserState state = Binary::Warning;
    for (it = data.begin() + start_index; it != data.end(); ++it) {
      state = parser.consume(*it);
      if (state == Binary::Error)
        return false;
      if (state == Binary::Success)
        break;
    }

    if(state != Binary::Success)
      return false;

    if(end_index != nullptr)
      *end_index = it - data.begin();

    //    printf("parser node: %ld\n",parser.state.node);
    //    printf("parser seri: %s\n",parser.state.node->serialize_readable(false).c_str());
    destroy_children();
    parser.merge_to(this);
    return true;
    /*    if(data->size() == 0 || data->size() <= start_index) return false;
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
          delete new_child_node;
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
          CompoundNode new_child_node = CompoundNode();
          if(!new_child_node.deserialize(data, index, &index)){
            return false;
          }
          new_node.put_back(key, new_child_node);
        }
        break;
      }
      case COMPOUND_NODE_BEGIN_BLOCK_FLAG:{
        ++index;
        SizedBlock* new_block = new SizedBlock();
        index = new_block->upper(*data, index);
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
    return true;*/
  }

  std::string _add_escapes_to_string(std::string str){
    std::string new_string;
    for(int i = 0; i < (ssize_t)str.length(); i ++){
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
      if(++c < (ssize_t)generic_tags.size() || child_nodes.size() || child_node_lists.size())
        json += ",";
    }
    c = 0;
    for(std::pair<std::string, CompoundNode*> pair: child_nodes){
      json += "\"" + pair.first + "\": "+ pair.second->similair_json();
      if(++c < (ssize_t)child_nodes.size() || child_node_lists.size())
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
      if(++c < (ssize_t)child_node_lists.size())
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

  bool CompoundNode::has_node_list(std::string key){
    return exists_key<std::vector<CompoundNode*>>(&child_node_lists, key);
  }

  bool CompoundNode::has_node(std::string key){
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

  void CompoundNode::put(std::string key, std::vector<CompoundNode*>& nodes){
    if(exists_key<std::vector<CompoundNode*>>(&child_node_lists, key)){
      for(CompoundNode* node: child_node_lists[key]){
        delete node;
      }
      child_node_lists[key].clear();
    }
    child_node_lists[key].reserve(nodes.size());
    for(CompoundNode* node : nodes){
      CompoundNode* new_node = new CompoundNode();
      node->copy_to(new_node);
      child_node_lists[key].push_back(new_node);      
    }
  }

  void CompoundNode::put_back(std::string key, CompoundNode& node){
    if(!exists_key<std::vector<CompoundNode*>>(&child_node_lists, key))
      child_node_lists[key] = std::vector<CompoundNode*>();
    CompoundNode* new_node = new CompoundNode();
    node.copy_to(new_node);
    child_node_lists[key].push_back(new_node);
  }

  void CompoundNode::put(std::string key, CompoundNode& node){
    if(exists_key<CompoundNode*>(&child_nodes, key))
      delete child_nodes[key];
    CompoundNode* new_node = new CompoundNode();
    node.copy_to(new_node);
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
    for (std::pair<std::string, SizedBlock *> pair : generic_tags) {
       delete pair.second;
    }
    generic_tags = std::unordered_map<std::string, SizedBlock*>();
  }

  template<typename T>
  SizedBlock* CompoundNode::put_string(std::string key, std::vector<T>& vars){
    T* p_vars = vars.data();
    un_size_t amount = vars.size();
    burninate_generic_if_exists(key);
    SizedBlock* ptr = new SizedBlock(sizeof(T), amount, p_vars);
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
    block->contents_native = nullptr;
    if(contents_native == nullptr) return;
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
    memcpy(contents.data()+sizeof(uint8_t)+sizeof(uint16_t),(char*)&span_d,sizeof(un_size_t)); //bytes 3,4,5,6...
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
    if(contents_native != nullptr) dump();
    char* max = maxaddress + 1;
    if(max < data) return nullptr;
    if((uint64_t)(max - data) < sizeof(uint8_t) + sizeof(uint16_t) + sizeof(un_size_t)) return nullptr;
    meta = data[0];
    //    printf("meta of new uppered node is :%d\n",meta);
    uint16_t element_size;
    memcpy(&element_size, data + sizeof(uint8_t), sizeof(uint16_t));
    element_span = little_endian<uint16_t>(element_size);// also goes from little to native
    un_size_t total_size;
    memcpy(&total_size, data + sizeof(uint8_t) + sizeof(uint16_t), sizeof(un_size_t));
    span = little_endian<un_size_t>(total_size);
    char* data_after_header = data + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(un_size_t);
    if((uint64_t)(max - data) < total_size + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(un_size_t)){
      dump();
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

  uint64_t SizedBlock::upper(std::vector<char>& data, uint64_t starting_index){
    return (upper(data.data() + starting_index, data.data() + data.size() - 1) - data.data());
  }

  void SizedBlock::dump(){
    if(contents_native == nullptr) return;
    free(contents_native);
    contents_native = nullptr;
    span = 0;
    meta = 0;
    element_span = 0;
  }

  void SizedBlock::assign_meta(uint8_t value){
    meta = value;
  }

  SizedBlock::~SizedBlock() { dump(); }

  #define PUSHDOWN_PARSER_TOKEN_WARNING_LENGTH_R 256

  //Pushdown automaton parsers
  namespace Binary{

    ParserData fresh_parser_data_h();
    void clean_parser_data(ParserData *);
    
    PushdownParser::PushdownParser(){state = fresh_parser_data_h();}

    void PushdownParser::merge_to(CompoundNode* node){
      state.node->copy_to(node);
    }
    
    ParserState PushdownParser::consume(char c){
      switch (state.current_state){

      case AwaitNodeArrayNode:{
        if(c == COMPOUND_NODE_END_LIST_FLAG){
          state.current_state = AwaitKeyStart;
          break;
        }
        if(c == COMPOUND_NODE_BEGIN_FLAG){
          state_stack.push(state);
          state = fresh_parser_data_h();
          state.current_state = AwaitKeyStart;
          break;
        }
        break;
      }

      case AquireNodeData:{
        state.node_data.push_back(c);
        state.node_data_left--;
        if(state.node_data_left <= 0){
          if(exists_key<SizedBlock*>(&(state.node->generic_tags),state.current_key))
            break;
          SizedBlock* blk = new SizedBlock();
          //          float f = 0.126;
          //          SizedBlock* blk = new SizedBlock(sizeof(float),1,&f);
          //          std::vector<char> l = blk->lower();
          //          blk->upper(l,0);
          blk->upper(state.node_data,0);
          //          blk->assign_meta(SB_META_FLOAT_STYLE);
          state.node->generic_tags[state.current_key] = blk;
          for(char cd : state.node_data){
            //            printf("%d,",cd);
          }
          //          printf("\nupper'd block: meta:%d element_size:%d span:%d\n",blk->meta,blk->element_span,blk->span);
          //          state.node->put<float>("test",0.125)->assign_meta(SB_META_FLOAT_STYLE);
          state.current_state = AwaitKeyStart;
          break;
        }
        break;
      }

      case AquireNodeHeader:{
        un_size_t headersize = SizedBlock::header_size_bytes();
        state.node_data.push_back(c);
        state.node_data_counter++;
        if(state.node_data_counter < headersize - 1)
          break;
        if(state.node_data_counter == headersize - 1){
          state.node_data_left = SizedBlock::interpret_size_from_header(state.node_data) + 1;
          if(state.node_data_left == 0){
            if(exists_key<SizedBlock*>(&(state.node->generic_tags),state.current_key))
              break;
            SizedBlock* nb = new SizedBlock();
            nb->upper(state.node_data,0);
            state.node->generic_tags[state.current_key] = nb;
            state.current_state = AwaitKeyStart;
            break;
          }
          state.current_state = AquireNodeData;
          break;
        }
        state.current_state = Error;
        break;
      }

      case GetIndicator:{
        if(c == COMPOUND_NODE_BEGIN_BLOCK_FLAG){
          state.current_state = AquireNodeHeader;
          state.node_data.clear();
          state.node_data_counter = 0;
          state.node_data_left = 0;
          break;
        }
        if(c == COMPOUND_NODE_BEGIN_FLAG){
          state.current_state = AwaitKeyStart;
          state_stack.push(state);
          state = fresh_parser_data_h();
          state.current_state = AwaitKeyStart;
          break;
        }
        if(c == COMPOUND_NODE_BEGIN_LIST_FLAG){
          state.current_state = AwaitNodeArrayNode;
          break;
        }
        break;
      }

      case ConstructKeyEscape:{
        if(c == COMPOUND_NODE_BEGIN_ELEMENT_FLAG){
          state.current_string += c;
          state.current_state = ConstructKey;
          break;
        }
        state.current_string += COMPOUND_NODE_BEGIN_ELEMENT_FLAG;
        state.current_string += c; 
        state.current_state = ConstructKey;
        break;
      }

      case ConstructKey:{
        if(c == COMPOUND_NODE_ESCAPE_KEY_FLAG){
          state.current_state = ConstructKeyEscape;
          break;
        }
        if(c == COMPOUND_NODE_BEGIN_ELEMENT_FLAG){
          state.current_key = state.current_string;
          state.current_string = "";
          state.current_state = GetIndicator;
          break;
        }
        state.current_string += c;
        break;
      }

      case AwaitKeyStart:{
        if(c == COMPOUND_NODE_BEGIN_STRING_FLAG){
          state.current_state = ConstructKey;
          state.current_string = "";
          break;
        }
        if(c == COMPOUND_NODE_END_FLAG){
          if(state_stack.size() == 0){
            state.current_state = Success;
            break;
          }
          if(state_stack.size() >= 0){
            ParserState top_state = state_stack.top().current_state;
            std::string top_key = state_stack.top().current_key;
            if(top_state == AwaitKeyStart){
              state_stack.top().node->put(top_key, state.node);
            } else if (top_state == AwaitNodeArrayNode){
              state_stack.top().node->put_back(top_key, *state.node);
            } else {
              state.current_state = Error;
              break;
            }
            //pop from stack, deleting dynamic objects in state
            clean_parser_data(&state);
            state = state_stack.top();
            state_stack.pop();
            break;
          }
          state.current_state = Error;
        }
        break;
      }

      case AwaitBegin:{
        if(c == COMPOUND_NODE_BEGIN_FLAG){
          state.current_state = AwaitKeyStart;
        }
        break;
      }

      default:{
        state.current_state = Error;
      }
      }

      return state.current_state;
    };

    ParserData fresh_parser_data_h() {
      ParserData dat = ParserData{.node = new CompoundNode(), .current_state = AwaitBegin, .current_string = "", .current_key = "", .node_data = std::vector<char>(), .node_data_counter = 0, .node_data_left = 0};
      //      printf("fresh parser data node = %ld\n",dat.node);
      return dat;
    }

    void clean_parser_data(ParserData *data) {
      delete data->node;
      data->node = nullptr;
    }

   PushdownParser::~PushdownParser(){
      clean_parser_data(&state);
      state.node = NULL;
      while (!state_stack.empty()) {
        clean_parser_data(&state_stack.top());
        state_stack.pop();
      }
    };
  } //namespace Binary

  
  namespace Readable {
    
    ParserState get_value_type_state(char c);
    bool is_appropriate_value_start(char c, char ident);
    ParserData fresh_parser_data_h();
    void clean_parser_data(ParserData *);
    ssize_t elem_size(char);
    bool parse_insert_generic(CompoundNode*,std::string key,std::vector<std::string>,char);

    ParserState PushdownParser::consume(char c) {

      switch (state.current_state) {

      case PossibleCommentEnd: {
	if (c == READABLE_COMMENT_CHECK){
	  state.current_state = state.next_state;
	} else {
	  state.current_state = VariableComment;
	}
	break;
      }
	
      case VariableComment: {
	if (c == READABLE_COMMENT_VARIABLE_DELIM)
	  state.current_state = PossibleCommentEnd;
	break;
      }
	
      case EndlineComment: {
	if (c == READABLE_ENDLINE_DEFINITION)
	  state.current_state = state.next_state;
	break;
      }
	
      case CheckComment: {
	if (c == READABLE_ENDLINE_COMMENT_START){
	  state.current_state = EndlineComment;
	  break;
	}
	if (c == READABLE_COMMENT_VARIABLE_DELIM){
	  state.current_state = VariableComment;
	  break;
	}
	state.current_state = Error;
	break;
      }

        // The user's program shoud pause parsing for a warning
        // to prevent possible buffer overflow. The warning state
        // is reached only through attempting to parse unreasonably
        // long tokens. The previous state before the warning is
        // stored in state.next_state

      case Warning: {
        state.current_state = Error;
      }

      case AwaitValueParsableSeperator: {
        if (c == COMPOUND_NODE_ITEM_SEPERATOR_R) {
          state.current_state = AwaitValueParsable;
          break;
        }
        if (c == COMPOUND_NODE_END_ARRAY_R) {
          state.current_construction = "";
          if (!parse_insert_generic(state.node, state.current_key,
                                    state.value_constructions,
                                    state.current_value_type)) {
            state.current_state = Error;
            state.value_constructions.clear();
            break;
          }
          state.current_state = AwaitItemSeperator;
          state.value_constructions.clear();
          break;
        }

        if (c == READABLE_COMMENT_CHECK){
	  state.next_state = state.current_state;
	  state.current_state = CheckComment;
	  break;
	}
        if (!_is_ascii_whitespace(c)) {
          state.current_state = Error;
          break;
        }
        break;
      }

      case ConstructValueParsable: {
        if (c == READABLE_COMMENT_CHECK){
	  state.next_state = state.current_state;
	  state.current_state = CheckComment;
	  break;
	}
        if (c == COMPOUND_NODE_END_ARRAY_R) {
          state.value_constructions.push_back(state.current_construction);
          state.current_construction = "";
          if (!parse_insert_generic(state.node, state.current_key,
                                    state.value_constructions,
                                    state.current_value_type)) {
            state.current_state = Error;
            state.value_constructions.clear();
            break;
          }
          state.current_state = AwaitItemSeperator;
          state.value_constructions.clear();
          break;
        }
        if (_is_ascii_whitespace(c)) {
          state.value_constructions.push_back(state.current_construction);
          state.current_construction = "";
          state.current_state = AwaitValueParsableSeperator;
          break;
        }
        if (c == COMPOUND_NODE_ITEM_SEPERATOR_R) {
          state.value_constructions.push_back(state.current_construction);
          state.current_construction = "";
          state.current_state = AwaitValueParsable;
          break;
        }
        if (state.current_construction.size() >=
            PUSHDOWN_PARSER_TOKEN_WARNING_LENGTH_R) {
          state.next_state = state.current_state;
          state.current_state = Warning;
        }
        state.current_construction += c;
        break;
      }

      case AwaitValueParsable: {
        if (c == COMPOUND_NODE_END_ARRAY_R) {
          if (!state.value_constructions.empty()) {
            state.current_state = Error;
            break;
          }
          if (!parse_insert_generic(state.node, state.current_key,
                                    std::vector<std::string>(),
                                    state.current_value_type)) {
            state.current_state = Error;
          } else {
            state.current_state = AwaitItemSeperator;
          }
          state.current_state = AwaitItemSeperator;
          break;
        }
        if (c == READABLE_COMMENT_CHECK){
	  state.next_state = state.current_state;
	  state.current_state = CheckComment;
	  break;
	}
        if (c == COMPOUND_NODE_ITEM_SEPERATOR_R) {
          state.current_state = Error;
          break;
        }
	if (c == READABLE_COMMENT_CHECK){
	  state.next_state = state.current_state;
	  state.current_state = CheckComment;
	  break;
	}
        if (!_is_ascii_whitespace(c)) {
          state.current_state = ConstructValueParsable;
          state.current_construction += c;
        }
        break;
      }

      case ConstructValueString: {
        if (c == COMPOUND_NODE_ESCAPE_STRING_R) {
          state.current_state = ConstructValueStringEscape;
          break;
        }
        if (c == COMPOUND_NODE_END_STRING_R) {
          state.node->put_string<char>(
                                       state.current_key, state.current_construction.size()+1,
                                       (char *)state.current_construction.c_str())->assign_meta(SB_META_STRING);
          
          state.current_key = "";
          state.current_construction = "";
          state.current_state = AwaitItemSeperator;
          break;
        }
        state.current_construction += c;
        break;
      }
        
      case ConstructValueStringEscape: {
        if (c == COMPOUND_NODE_END_STRING_R) {
          state.current_construction += c;
          break;
        }
        if (c == COMPOUND_NODE_ESCAPE_STRING_R) {
          state.current_construction += c;
          break;
        }
        state.current_construction += COMPOUND_NODE_ESCAPE_STRING_R;
        state.current_construction += c;
        state.current_state = ConstructValueString;
        break;
      }
        
      case AwaitValue: {
        if (c == COMPOUND_NODE_BEGIN_ARRAY_R && state.current_value_type != SB_FLAG_STRING) {
          state.current_state = AwaitValueParsable;
          break;
        }
        if (c == COMPOUND_NODE_BEGIN_STRING_R && state.current_value_type == SB_FLAG_STRING) {
          state.current_construction = "";
          state.current_state = ConstructValueString;
          break;
        }
        if (c == READABLE_COMMENT_CHECK){
	  state.next_state = state.current_state;
	  state.current_state = CheckComment;
	  break;
	}
        if (!_is_ascii_whitespace(c))
          state.current_state = Error;
        break;
      }

      case AwaitItemSeperator: {
        if (c == COMPOUND_NODE_ITEM_SEPERATOR_R) {
          state.current_state = AwaitKey;
          break;
        }
        if (c == COMPOUND_NODE_END_R && state_stack.empty()) {
          state.current_state = Success;
          break;
        }
        if (c == COMPOUND_NODE_END_R && !state_stack.empty()) {
          if (state_stack.top().current_state == AwaitItemSeperator) {
            state_stack.top().node->put(state_stack.top().current_key, *state.node);
          } else if (state_stack.top().current_state == ConstructNodeArrayAwaitSeperator) {
            state_stack.top().node->put_back(state_stack.top().current_key, *state.node);
          }
          clean_parser_data(&state);
          state = state_stack.top();
          state_stack.pop();
          break;
        }
        if (c == READABLE_COMMENT_CHECK){
	  state.next_state = state.current_state;
	  state.current_state = CheckComment;
	  break;
	}
        if (!_is_ascii_whitespace(c))
          state.current_state = Error;
        break;
      }

      case ConstructNodeArrayAwaitSeperator: { 
        if (c == COMPOUND_NODE_ITEM_SEPERATOR_R) {
          state.current_state = ConstructNodeArrayAwaitNode;
          break;
        }
        if (c == COMPOUND_NODE_END_ARRAY_R) {
          state.current_key = "";
          state.current_state = AwaitItemSeperator;
          break;
        }
        if (c == READABLE_COMMENT_CHECK){
	  state.next_state = state.current_state;
	  state.current_state = CheckComment;
	  break;
	}
        if (!_is_ascii_whitespace(c))
          state.current_state = Error;
        break;
      }
        
      case ConstructNodeArrayAwaitNode: {
        if (c == COMPOUND_NODE_END_ARRAY_R && state.node->get_node_list_length(state.current_key) == 0) {
          state.current_key = "";
          state.current_state = AwaitItemSeperator;
          break;
        }
        if (c == READABLE_COMMENT_CHECK){
	  state.next_state = state.current_state;
	  state.current_state = CheckComment;
	  break;
	}
        if (c == COMPOUND_NODE_BEGIN_FLAG_R) {
          state.current_state = ConstructNodeArrayAwaitSeperator;
          state_stack.push(state);
          state = fresh_parser_data_h();
          state.current_state = AwaitKey;
          break;
        }
	if (c == READABLE_COMMENT_CHECK){
	  state.next_state = state.current_state;
	  state.current_state = CheckComment;
	  break;
	}
        if(!_is_ascii_whitespace(c))
          state.current_state = Error;
        break;
      }
      case AwaitValueTypeIdentifier: {
        if (c == READABLE_COMMENT_CHECK){
	  state.next_state = state.current_state;
	  state.current_state = CheckComment;
	  break;
	}

        state.current_state = get_value_type_state(c);
        if (state.current_state == Error)
          break;
        if (state.current_state == AwaitValue)
          state.current_value_type = c;
        if (c == COMPOUND_NODE_BEGIN_FLAG_R) {
          state.current_state = AwaitItemSeperator;
          state_stack.push(state);
          state = fresh_parser_data_h();
          state.current_state = AwaitKey;
          break;
        }
        if (c == COMPOUND_NODE_BEGIN_ARRAY_R) {
          state.current_state = ConstructNodeArrayAwaitNode;
          break;
        }
        break;
      }
       	  
      case AwaitKeyValueSeperator: {
        if (c == COMPOUND_NODE_KEY_VALUE_SEPERATOR) {
          state.current_state = AwaitValueTypeIdentifier;
          break;
        }
        if (c == READABLE_COMMENT_CHECK){
          state.next_state = state.current_state;
          state.current_state = CheckComment;
          break;
        }
        if (!_is_ascii_whitespace(c))
          state.current_state = Error;
        break;
      }
       	  
      case ConstructKeyEscape: {
        if (c == COMPOUND_NODE_END_STRING_R) {
          state.current_construction += c;
          state.current_state = ConstructKey;
          break;
        }
        if (c == COMPOUND_NODE_ESCAPE_STRING_R) {
          state.current_construction += c;
          break;
        }
        state.current_construction += COMPOUND_NODE_ESCAPE_STRING_R;
        state.current_construction += c;
        state.current_state = ConstructKey;
        break;
      }
        
      case ConstructKey: {
        if (c == COMPOUND_NODE_END_STRING_R) {
          state.current_state = AwaitKeyValueSeperator;
          state.current_key = state.current_construction;
          state.current_construction = "";
          break;
        }
        if (c == COMPOUND_NODE_ESCAPE_STRING_R) {
          state.current_state = ConstructKeyEscape;
          break;
        }
        state.current_construction += c;
        break;
      }
        
      case AwaitKey: {
        if (c == COMPOUND_NODE_BEGIN_STRING_R) {
          state.current_state = ConstructKey;
          break;
        }
        if (c == COMPOUND_NODE_END_R && state_stack.empty() && state.node->empty()) {
          state.current_state = Success;
          break;
        }
        if (c == COMPOUND_NODE_END_R && !state_stack.empty() && state.node->empty()) {
          if (state_stack.top().current_state == AwaitItemSeperator){
            state_stack.top().node->put(state_stack.top().current_key, *state.node);
          } else if (state_stack.top().current_state == ConstructNodeArrayAwaitSeperator) {
            state_stack.top().node->put_back(state_stack.top().current_key, *state.node);
          } else {
            state.current_state = Error; //parent node was not in a valid state
          }
          clean_parser_data(&state);
          state = state_stack.top();
          state_stack.pop();
          break;
        }
        if (c == READABLE_COMMENT_CHECK){
	  state.next_state = state.current_state;
	  state.current_state = CheckComment;
	  break;
	}
        if (!_is_ascii_whitespace(c))
          state.current_state = Error;
        break;
      }
          
      case AwaitStart: {
        if (c == COMPOUND_NODE_BEGIN_FLAG_R) {
          state.current_state = AwaitKey;
          break;
        }
        if (c == READABLE_COMMENT_CHECK){
	  state.next_state = state.current_state;
	  state.current_state = CheckComment;
	  break;
	}
        if (!_is_ascii_whitespace(c))
          state.current_state = Error;
        break;
      }
          
      default: {
        state.current_state = Error;
        break;
      }

      } // switch statment
      
      return state.current_state;
    } // consume(char)

    void PushdownParser::merge_to(CompoundNode *node) {
      state.node->copy_to(node);
    }

    PushdownParser::PushdownParser() { state = fresh_parser_data_h(); };

    PushdownParser::~PushdownParser() {
      clean_parser_data(&state);
      while (!state_stack.empty()) {
        clean_parser_data(&state_stack.top());
        state_stack.pop();
      }
    }

    ssize_t elem_size(char flag){
      switch (flag) {
      case SB_FLAG_UNDEFINED:
          return 1;
        break;
      case SB_FLAG_I8:
        return sizeof(int8_t);
        break;
      case SB_FLAG_I16:
        return sizeof(int16_t);
          break;
      case SB_FLAG_I32:
        return sizeof(int32_t);
          break;
      case SB_FLAG_I64:
        return sizeof(int64_t);
        break;
      case SB_FLAG_FLOAT:
        return sizeof(float);
        break;
      case SB_FLAG_DOUBLE:
        return sizeof(double);
        break;
      case SB_FLAG_LONG_DOUBLE:
        return sizeof(long double);
        break;
      case SB_FLAG_BOOLEAN:
        return sizeof(bool);
        break;
      case SB_FLAG_STRING:
        return sizeof(char);
        break;
      }
      return -1;
    }

    long long _safe_iparse(std::string str, bool *success) {
      long long num;
      try {
        num = std::stoll(str);
      } catch (std::exception& e) {
        *success = false;
        return 0;
      }
      *success = true;
      return num;
    }

    long double _safe_fparse(std::string str, bool *success) {
      long double num;
      try {
        num = std::stold(str);
      } catch (std::exception& e) {
        *success = false;
        return 0;
      }
      *success = true;
      return num;
    }

    template <typename T>
    std::vector<T> _parse_ai(std::vector<std::string>& strings, bool *success) {
      std::vector<T> parsed;
      for (std::string parsable : strings) {
        T num = (T)_safe_iparse(parsable, success);
        if (!*success)
          break;
        parsed.push_back(num);
      }
      return parsed;
    }

    template <typename T>
    std::vector<T> _parse_af(std::vector<std::string>& strings, bool *success) {
      std::vector<T> parsed;
      for (std::string parsable : strings) {
        T num = (T)_safe_fparse(parsable, success);
        if (!*success)
          break;
        parsed.push_back(num);
      }
      return parsed;
    }

    bool parse_insert_generic(CompoundNode *node, std::string key,
                              std::vector<std::string> raw, char parse_type) {
      switch (parse_type){
      case SB_FLAG_UNDEFINED: {
        std::vector<uint8_t> vec;
        node->put_string<uint8_t>(key, vec)->assign_meta(SB_META_UNDEFINED);
        return true;
        break;
      }
      case SB_FLAG_I8: {
        std::vector<int8_t> parsed; bool success;
        parsed = _parse_ai<int8_t>(raw, &success);
        if (!success) return false;
        node->put_string<int8_t>(key, parsed)->assign_meta(SB_META_INT_STYLE);
        return true;
        break;
      }
      case SB_FLAG_I16: {
        std::vector<int16_t> parsed; bool success;
        parsed = _parse_ai<int16_t>(raw, &success);
        if (!success) return false;
        node->put_string<int16_t>(key, parsed)->assign_meta(SB_META_INT_STYLE);
        return true;
        break;
      }
      case SB_FLAG_I32: {
        std::vector<int32_t> parsed; bool success;
        parsed = _parse_ai<int32_t>(raw, &success);
        if (!success) return false;
        node->put_string<int32_t>(key, parsed)->assign_meta(SB_META_INT_STYLE);
        return true;
        break;
      }
      case SB_FLAG_I64: {
        std::vector<int64_t> parsed; bool success;
        parsed = _parse_ai<int64_t>(raw, &success);
        if (!success) return false;
        node->put_string<int64_t>(key, parsed)->assign_meta(SB_META_INT_STYLE);
        return true;
        break;
      }
      case SB_FLAG_FLOAT: {
        std::vector<float> parsed; bool success;
        parsed = _parse_af<float>(raw, &success);
        if (!success) return false;
        node->put_string<float>(key, parsed)->assign_meta(SB_META_FLOAT_STYLE);
        return true;
        break;
      }
      case SB_FLAG_DOUBLE: {
        std::vector<double> parsed; bool success;
        parsed = _parse_af<double>(raw, &success);
        if (!success) return false;
        node->put_string<double>(key, parsed)->assign_meta(SB_META_FLOAT_STYLE);
        return true;
        break;
      }
      case SB_FLAG_LONG_DOUBLE: {
        std::vector<long double> parsed; bool success;
        parsed = _parse_af<long double>(raw, &success);
        if (!success) return false;
        node->put_string<long double>(key, parsed)->assign_meta(SB_META_FLOAT_STYLE);
        return true;
        break;
      }
      case SB_FLAG_BOOLEAN: {
        std::vector<uint8_t> parsed;
        for (std::string str : raw) {
          if (str == "true"){
            parsed.push_back(1);
          } else if (str == "false") {
            parsed.push_back(0);
          } else {
            return false;
          }
        }
        node->put_string<uint8_t>(key, parsed)->assign_meta(SB_META_BOOLEAN);
        return true;
        break;
      }
      default:
        break;
      }
      return false;
    }

    ParserData fresh_parser_data_h() {
      return ParserData{.current_state = AwaitStart, .next_state = AwaitKey, .node = new CompoundNode(), .current_construction = "", .current_value_type = 0, .current_key = "", .value_constructions = std::vector<std::string>()};
    }

    void clean_parser_data(ParserData *data) {
      delete data->node;
    }

    ParserState get_value_type_state(char c) {
      ParserState state = Error;
      if (c == SB_FLAG_UNDEFINED || c == SB_FLAG_I8 || c == SB_FLAG_I16 || c == SB_FLAG_I32 ||
          c == SB_FLAG_I64 || c == SB_FLAG_FLOAT || c == SB_FLAG_DOUBLE ||
          c == SB_FLAG_LONG_DOUBLE || c == SB_FLAG_BOOLEAN ||
          c == SB_FLAG_STRING || c == COMPOUND_NODE_BEGIN_FLAG_R || c == COMPOUND_NODE_BEGIN_ARRAY_R)
        state = AwaitValue;
      if (_is_ascii_whitespace(c))
        state = AwaitValueTypeIdentifier;
      return state;
    }

    bool is_appropriate_value_start(char c, char ident) {
      if ((ident == SB_FLAG_UNDEFINED || ident == SB_FLAG_I16 ||
           ident == SB_FLAG_I32 || ident == SB_FLAG_I64 ||
           ident == SB_FLAG_FLOAT || ident == SB_FLAG_DOUBLE ||
           ident == SB_FLAG_LONG_DOUBLE || ident == SB_FLAG_BOOLEAN) &&
          c == COMPOUND_NODE_BEGIN_ARRAY_R)
        return true;
      if ((ident == SB_FLAG_STRING) && c == COMPOUND_NODE_BEGIN_STRING_R)
        return true;
      return false;
    }

  } // namespace Readable

} // namespace Serialize

#undef un_size_t
#undef _return_if_EOF
#undef _rassert_token
#undef bp
