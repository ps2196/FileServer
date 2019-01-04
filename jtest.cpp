#include "utils/json.hpp"
#include <iostream>
#include <string>
#include <string.h>
#include "auth_strategy/authstrategy.hpp"

using json = nlohmann::json;
using string = std::string;

int main()
{   
     AuthStrategy auth = AuthStrategy("auth/users.auth");
     string u = "root";
     string p = "root";
     User* user = auth.auth(u, p);
     if(user != nullptr)
         std::cout << "OK\n";
     else 
          std::cout<<"Auth fail!\n";
    // json j;
    // j["dupa"] = "gruba";
    // j["twoj_stary"] = "pijany";
    // std::cout<<"JSON:\n"<<j["Aaa"]<<std::endl;
    
    // string s = "{\"dupa\" : \"dupa\"}";
    // try{
    // json j = json::parse(s);
    // std::cout<<"JSON:\n"<<j<<std::endl;
    // }
    // catch(json::parse_error)
    // {
    //    std::cout<<"JSON parse error\n";
    //    exit(-1);     
    // }
    
    // string s = "";
    // char buf[100];
    // strcpy(buf, "siurek");

    // char buf1[100];
    // strcpy(buf1, "twoj stary");

    // s+=buf;
    // s+=buf1;
    // std::cout<<"buf: "<<sizeof(buf)<<std::endl;
    // std::cout<<"S = "<<s<<"\nsize: "<<s.size()<<std::endl;

    return 0;
}