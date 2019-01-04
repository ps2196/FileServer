#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

int main()
{   
    json j;
    j["dupa"] = "gruba";
    j["twoj_stary"] = "pijany";
    std::cout<<"JSON:\n"<<j<<std::endl;

    return 0;
}