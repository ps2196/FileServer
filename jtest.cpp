//#include "utils/json.hpp"
#include <iostream>
#include <string>
#include <string.h>
//#include "auth_strategy/authstrategy.hpp"
//#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>
#include <list>
#include "utils/base64.h"
#include "utils/base644.h"

//using json = nlohmann::json;
using string = std::string;
//namespace FS = boost::filesystem;
using namespace std;
int main()
{

    std::ifstream file("outbin", std::ios::binary);
    std::ofstream fileOutput("output.html", std::ios::app | std::ios::binary);
    string line;
    string encoded = "";
    string decoded = "";

    Base64 *base = Base64::getInstance();
    //char *lineChar;

    //char *encodedChar;
    //char *decodedChar;

    while (std::getline(file, line))
    {
      decoded = base64_decode(line);
      std::cout << decoded;
    }


/*
    int a;
    while (std::getline(file, line))
    {
      /*
      encoded = base64_encode(reinterpret_cast<const unsigned char*>(line.c_str()), line.length());
      decoded = base64_decode(encoded);
      fileOutput << decoded << std::endl;

      encodedChar = new char[base->getEncodeLength(line.length())];
      base->Encode(reinterpret_cast<const char*>(line.c_str()), line.length(), encodedChar);
      decodedChar = new char[line.length()];
      base->Decode(encodedChar, sizeof(encodedChar), decodedChar);

      fileOutput << decodedChar << std::endl;
      delete[] decodedChar;
      delete[] encodedChar;

    }

*/




/*
    list<string> l;
    string s1 = "Piotrek";
    l.push_back(s1);
    cout<<"S1.1 = "<< l.front() <<endl;
    l.front().erase(0,3);
    cout<<"S1.2 = "<< l.front() <<endl;
    return 0;
*/
    // std::ofstream f("/TINRepo/server/Hello.txt");
    // if(f.good())
    //     std::cout<< "OK\n";
    // else
    //     std::cout << "ups...\n";
    // return 0;

    //FS::create_directories("./data/root");
    // try{
    // FS::create_directory("./data/scott/tmp");

    // std::ofstream f("./data/scott/Hello.txt");
    // if(f.good())
    //     std::cout<< "OK\n";
    // else
    //     std::cout << "ups...\n";
    // }
    // catch(FS::filesystem_error e)
    // {
    //     std::cout<<e.what()<<std::endl;
    // }

//    using namespace std;
//    vector<string> v;
//    v.push_back("vect1");
//    v.push_back("vect2");
//    v.push_back("vect3");
//     json b;
//     b["BBB"] ="tu B";
//     b["cos"] ="tam";
//     json a;
//     a["AAA"] = "abc";
//     a["vector"] =  v;

//     cout<<a.dump(4)<<endl;

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
