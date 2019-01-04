#include <string>
#include <vector>
#include "utils/json.hpp"
#include "auth_strategy/authstrategy.hpp" // tmp

class RequestParser
{
  public:
    using string = std::string;
    using json = nlohmann::json;

    static string parseRequest(string &request)
    {
        try
        {
            json req = json::parse(request);
            if(req["type"] != nullptr && req["type"] == "REQUEST")
            {
               auto cmd = req["command"];
               if( cmd == "AUTH")
               {
                   AuthStrategy authStrategy = AuthStrategy("auth/users.auth");
                   string username = req["username"];
                   string pass = req["password"];
                   User* user = authStrategy.auth(username, pass);
                   if(user == nullptr)
                   {
                       json res_json;
                       res_json["type"] = "RESPONSE";
                       res_json["code"] = 401;
                       res_json["data"] = "Username or password incorrect";
                       return res_json.dump();
                   }
                   else
                   { //Authorized
                       json res_json;
                       res_json["type"] = "RESPONSE";
                       res_json["code"] = 200;
                       res_json["data"] = "Welcome "+user->username;
                       return res_json.dump();
                   }
               }
            } 
            else // Bad request
            {
                json res_json;
                res_json["type"] = "RESPONSE";
                res_json["code"] = 400;
                res_json["data"] = "Error while parsing request";
            }  
        }
        catch (json::parse_error)
        {
            json res_json;
            res_json["type"] = "RESPONSE";
            res_json["code"] = 400;
            res_json["data"] = "Error while parsing request";
            return res_json.dump();
        }
        catch(...)
        {
            json res_json;
            res_json["type"] = "RESPONSE";
            res_json["code"] = 500;
            res_json["code"] = "Server error";
            return res_json.dump();
        }
    }
};
