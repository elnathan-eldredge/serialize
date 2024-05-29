#include "../serialize/serialize.hpp"
#include <iostream>


int main(){
  CompoundNode top_node = CompoundNode();
  
  top_node.put<int,false>(12,"twelve");
  
  std::cout << "compat tag twelve: "
            << top_node.compat_tag<int>("twelve") << "\n";
  std::cout << "compat tag nonex: "
            << top_node.compat_tag<int>("nonex") << "\n";
  std::cout << "retrieve tag twelve as int: "
            << top_node.retrieve<int,false>("twelve") << "\n\n";
  
  CompoundNode letter_node = CompoundNode();
  
  letter_node.put_string<char,false>("abcd",5,"letters");
  
  std::cout << "compat string letters: "
    
            << letter_node.compat_string<char>("letters") <<"\n";
  std::cout << "compat string noex: "
            << letter_node.compat_string<char>("nonex") << "\n";
  std::cout << "retrieve string letters: "
            << letter_node.retrieve_string<char,false>("letters") << "\n";

  top_node.put_node(&letter_node, "letter_node");
  
  std::cout << "exists node letter_node: "
            << top_node.exists_node("letter_node") << "\n";
  std::cout << "exists node noex: "
            << top_node.exists_node("nonex") << "\n";
  std::cout << "retrive node retrieve string letters: "
            << top_node.retrieve_node("letter_node")
    ->retrieve_string<char,false>("letters") << "\n";
  
  top_node.retrieve_node("letter_node")->put_string<char,false>("efgh",5,"letters_extended");
  
  std::cout << "retrieve node retrieve string letters_extended: "
            << top_node.retrieve_node("letter_node")
    ->retrieve_string<char,false>("letters_extended") << "\n";
  
  std::cout << "compat string letters_extended: "
            << letter_node.compat_string<char>("letters_extended") << "\n";
  
}
