//#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "b64.hpp"
#include <stdlib.h>
#include <unordered_map>
#include <vector>
#include <string>

//NOTE: This server is for testing the javascript implementation
// ONLY! There are serious vulneribilities if this code is used
// for an actual server!


std::unordered_map<std::string,std::string> extention2mime = {
  {".html","text/html"},
  {".js","text/javascript"},
  {".wasm","application/wasm"}
};

std::string guessmime(std::string path){
  size_t index = path.find_last_of(".");
  std::string extention = path.substr(index,path.size()-index);
  if(extention2mime.find(extention) != extention2mime.end()){
    return extention2mime[extention];
  }
  return "application/octet-stream";
} 

std::string grabfile(std::string path){
  FILE * f = fopen(path.c_str(),"r");
  if(f == NULL)
    return "Error: cannot fetch file";
  std::vector<char> contents;
  char c = fgetc(f);
  while(c != EOF){
    contents.push_back(c);
    c = fgetc(f);
  }
  std::string str;
  contents.push_back(*"\0");
  str = std::string(contents.data());
  printf("%s\n",str.c_str());
  return str;
}

std::vector<char> grabfileb(std::string path){
  FILE * f = fopen(path.c_str(),"r");
  if(f == NULL)
    return std::vector<char>();
  std::vector<char> contents;
  char c = fgetc(f);
  while(c != EOF){
    contents.push_back(c);
    c = fgetc(f);
  }
  return contents;
}

int main(){
// HTTP
  httplib::Server svr;

  svr.Get("/", [](const httplib::Request &, httplib::Response &res) {
    res.set_content(grabfile("../example.html"), "text/html");
  });
  
  svr.Get("/:file", [](const httplib::Request & req, httplib::Response &res) {
    /*    if(guessmime(req.path) == "application/wasm"){*/
      std::vector<char> data = grabfileb("../" + req.path);
      res.set_content(data.data(), data.size(), guessmime(req.path));
      /*    } else {
      res.set_content(grabfile("../" + req.path), guessmime(req.path));
      }*/
  });

  svr.listen("0.0.0.0", 8080);
}
