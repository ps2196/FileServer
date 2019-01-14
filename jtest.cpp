#include "utils/json.hpp"
#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>
#include "auth_strategy/authstrategy.hpp"
//#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>
#include <list>
#include "utils/cppcodec/base64_rfc4648.hpp"

using json = nlohmann::json;
using string = std::string;
//namespace FS = boost::filesystem;
using namespace std;

void fencode(const string& sfile, const string& encfile)
{
    using base64 = cppcodec::base64_rfc4648;
    ifstream f(sfile, ifstream::binary);
    ofstream of(encfile, ios::out|ios::binary);
    char buf[900];
    char encbuf[2000];
    
    cout<<"Encoding ..."<<endl;
    while( true )
    {
        f.read(buf, 900);
        int rval = f.gcount();
        cout<<"Bytes read = "<< rval <<endl;    
        int enclen = base64::encode(encbuf, 2000, buf, rval);
        cout<<"Encoded len = "<< enclen<<endl;
        of.write(encbuf, enclen);
        if(!f)
            break;
    }
    f.close();
    of.close();
}



void fdecode(const string& encfile, const string& rfile)
{
    using base64 = cppcodec::base64_rfc4648;
    ifstream f(encfile, ifstream::binary);
    ofstream of(rfile, ios::out|ios::binary);
    char buf[1200];
    char decbuf[900];
    
    cout<<"Encoding ..."<<endl;
    while( true )
    {
        f.read(buf, 1200);
        int rval = f.gcount();
        cout<<"Bytes read = "<< rval <<endl;    
        int declen = base64::decode(decbuf, 900, buf, rval);
        cout<<"Encoded len = "<< declen<<endl;
        of.write(decbuf, declen);
        if(!f)
            break;
    }
    f.close();
    of.close();
}

int main()
{   
    using base64 = cppcodec::base64_rfc4648;
    // int enclen = base64::encode(encbuf, 3000, buf.c_str(), buf.size());
    // encbuf[enclen] = '\0';
    // printf("%s", encbuf);
    fencode("onet.html", "onetb64");
    fdecode("onetb64","onet_decoded.html");

    //std::vector<uint8_t> encoded = base64::encode(char* encoded_result, size_t encoded_buffer_size, const [uint8_t|char]* binary, size_t binary_size);



    // list<string> l;
    // string s1 = "Piotrek";
    // l.push_back(s1);
    // cout<<"S1.1 = "<< l.front() <<endl;
    // l.front().erase(0,3);
    // cout<<"S1.2 = "<< l.front() <<endl;
    // return 0;

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