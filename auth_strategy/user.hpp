#ifndef USER_H
#define USER_H

#include <string>
#include "../utils/json.hpp"

struct User
{
    using string = std::string;
    using json = nlohmann::json;

    string username;
    string password;
    int publicLimit;
    int privateLimit;
    float publicUsed;
    float privateUsed;

    User(string &name, string &pass, int pubLimit, int privLimit, float pubUsed, float privUsed) :
      username(name),
      password(pass),
      publicLimit(pubLimit),
      privateLimit(privLimit),
      publicUsed(pubUsed),
      privateUsed(privUsed) {}
    
    User(const User& other)
    {
      *this = other;
    }
    const User& operator=(const User& other)
    {
      if(this != &other)
      {
        username = other.username;
        password = other.password;
        publicLimit = other.publicLimit;
        privateLimit = other.privateLimit;
        publicUsed = other.publicUsed;
        privateUsed = other.privateUsed;
      }
      return *this;
    }

    json toJson()
    {
      json user;
      user["username"] = username;
      user["password"] = password;
      user["publicLimit"] = publicLimit;
      user["privateLimit"] = privateLimit;
      user["publicUsed"] = publicUsed;
      user["privateUsed"] = privateUsed;

      return user;
    }

};
#endif //USER_H
