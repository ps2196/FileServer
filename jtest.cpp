#include "utils/json.hpp"
#include <iostream>
#include <string>
#include <string.h>
#include "auth_strategy/authstrategy.hpp"
#include <boost/filesystem.hpp>
#include <fstream>

using json = nlohmann::json;
using string = std::string;
namespace FS = boost::filesystem;

int main()
{   
    // std::ofstream f("/TINRepo/server/Hello.txt");
    // if(f.good())
    //     std::cout<< "OK\n";
    // else
    //     std::cout << "ups...\n";
    // return 0;
    
    //FS::create_directories("./data/root");
    try{
    FS::create_directory("./data/scott/tmp");
    
    std::ofstream f("./data/scott/Hello.txt");
    if(f.good())
        std::cout<< "OK\n";
    else
        std::cout << "ups...\n";
    }
    catch(FS::filesystem_error e)
    {
        std::cout<<e.what()<<std::endl;
    }
    
    return 0;
    

    //  AuthStrategy auth = AuthStrategy("auth/users.auth");
    //  string u = "root";
    //  string p = "root";
    //  User* user = auth.auth(u, p);
    //  if(user != nullptr)
    //      std::cout << "OK\n";
    //  else 
    //       std::cout<<"Auth fail!\n";
    
    // json j;
    // j["dupa"] = "gruba";
    // j["twoj_stary"] = "pijany";
    // std::cout<<"JSON:\n"<<j["Aaa"]<<std::endl;

    // auto ja = j["dupa"];
    // string a;
    // if(ja== nullptr)
    //     std::cout<<"ja\n";
    // else 
    //      a = static_cast<string>(ja);
    // std::cout<<"xx:\n"<<a<<std::endl;

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