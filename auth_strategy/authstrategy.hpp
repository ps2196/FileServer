#ifndef AUTHSTRATEGY_H
#define AUTHSTRATEGY_H

#include "../utils/StringSplitter.h"
#include <fstream>
#include "user.hpp"
#include <iostream>
//
// AuthStrategy provides interface for user authorisation
// Credentials are saved in a text file formatted in the following way:
// username : password : total_disk_space : public_catalog_size  
//
class AuthStrategy
{
  public:
    using string = std::string;
    using descVect = std::vector<string>;

  private:
    string user_file;
  
  public:
  
    AuthStrategy(string& user_file): user_file(user_file)
    {}
    AuthStrategy(const char* user_file): user_file(user_file)
    {}

    // Returns line from user file describing given user
    const string getUserLine(string& username)
    {
      std::ifstream uf(user_file.c_str());
      string line;
      while(std::getline(uf, line))
      {
        std::cout << "FIND line: " << line << std::endl; //Debug
        descVect userDesc = splitWithDelimiter(line, ':');
        if(userDesc.size() < 4)
          continue;
        if(userDesc[0] == username)
          return line;
      }
      uf.close();
      return "";
    }

    // Check given credentials against credentials stored in user file
    User* auth(string& username, string& password)
    {
       string line = getUserLine(username);
       std::cout << "AUTH line: "<<line<<std::endl; //Debug
       if(line == "")
         return nullptr;
       descVect userDesc = splitWithDelimiter(line, ':');
       // line returned by getUserLine is at least size 4 after split
       if(userDesc[1] != password)
         return nullptr;
       
       return new User(userDesc[0], userDesc[1], stoi(userDesc[2]), stoi(userDesc[3]));
    }
};

#endif //AUTHSTRATEGY_H