#include "../include/serialize.hpp"
#include <iostream>
using namespace Serialize;
//interface
class archivable{
    public:
    virtual void archive(CompoundNode* node){}
    virtual void unpack(CompoundNode& node){}
};

//example use case
class transaction : public archivable {
    public:
    std::string party_a;
    std::string party_b;
    long id;
    
    transaction(std::string pa, std::string pb){
        static long sid = 0;
        party_a = pa;
        party_b = pb;
        id = sid;
        sid++;
    }
    
    virtual void archive(CompoundNode* node){
        archivable::archive(node);
        node->put_string<char>("party_a", party_a.size(), (char*)party_a.c_str())->assign_meta(SB_META_STRING);
        node->put_string<char>("party_b", party_b.size(), (char*)party_b.c_str())->assign_meta(SB_META_STRING);
        node->put<uint64_t>("id", id)->assign_meta(SB_META_INT_STYLE);
    }
    
    virtual void unpack(CompoundNode& node){
        archivable::unpack(node);
        if(!node.has_compat_string<char>("party_a") ||
           !node.has_compat_string<char>("party_a") ||
           !node.has_compat<uint64_t>("id"))
           throw std::invalid_argument("transaction::unpack requires tags \"party_a\", \"party_b\", and \"id\".");
      // It may be acceptable to instead include the null character when
      // there is no possibility of using the readable serialization 
      // (while assigning a stringlike meta), but  it is desirable to 
      // instead construct std::strings with size and data.
       party_a = std::string(node.get_ref<char>("party_a"), node.get_string_length("party_a"));
       party_b = std::string(node.get_ref<char>("party_b"), node.get_string_length("party_b"));
       id = node.get<uint64_t>("id");
    }
};

int main(){
    transaction t("foo","bar");
    CompoundNode node;
    t.archive(&node);
    std::string data = node.serialize_readable(false);
    std::cout << "node as serailized: " << data << "\n";
    node.deserialize_readable(data);
    t.unpack(node);
    t.archive(&node);
    std::cout << "node after serialized: " << node.serialize_readable(false) << "\n";
  return 0;
}
