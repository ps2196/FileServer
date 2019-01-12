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

    User(const string &name, string &pass, int pubLimit, int privLimit, float pubUsed, float privUsed) :
      username(name),
      password(pass),
      publicLimit(pubLimit),
      privateLimit(privLimit),
      publicUsed(pubUsed),
      privateUsed(privUsed) {}

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
