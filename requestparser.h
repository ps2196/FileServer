#ifndef REQPARER_H
#define REQPARER_H

#include <string>
#include <vector>
#include "utils/json.hpp"
#include "auth_strategy/authstrategy.hpp"
#include "requestengine.hpp"
#include "connection.h"

#define RESPONSE_BAD_REQUEST "{ \"type\":\"RESPONSE\", \"code\":400, \"data\":\"Bad request\"}"
#define RESPONSE_SERVER_ERROR "{ \"type\":\"RESPONSE\", \"code\":500, \"data\":\"Internal server error\"}"
#define RESPONSE_UNAUTHORIZED "{ \"type\":\"RESPONSE\", \"code\":401, \"data\":\"Unauthorized\"}"

class RequestParser
{
  public:
    using string = std::string;
    using json = nlohmann::json;

  private:
    AuthStrategy *auth;
    RequestEngine *engine;
    
    //
    // Parse request held by given connection and generate response
    //
    string intParseRequest(Connection* conn)
    {
        try
        {
            json req = json::parse(conn->getRequest());
            if(req["type"] != nullptr && req["type"] != "REQUEST") //Bad request
                return RESPONSE_BAD_REQUEST;

            auto cmd = req["command"];
            if(cmd == nullptr)
                return RESPONSE_BAD_REQUEST;
            if (cmd == "AUTH")
            {
                if(req["username"] == nullptr || req["password"] == nullptr)
                    return RESPONSE_BAD_REQUEST;
                
                string username = req["username"];
                string pass = req["password"];
                User *user = auth->auth( username, pass );
                if (user == nullptr)
                {
                    return RESPONSE_UNAUTHORIZED;
                }
                else
                { //Authorized
                    conn->setUser(user);
                    json res_json;
                    res_json["type"] = "RESPONSE";
                    res_json["code"] = 200;
                    res_json["data"] = "Welcome " + user->username;
                    return res_json.dump();
                }
            }
            else if( cmd == "TOUCH")
            { // chec if authorized and call engine method

            }
        }
        catch (json::parse_error)
        {
            return RESPONSE_BAD_REQUEST;
        }
        catch (...)
        {
            return RESPONSE_SERVER_ERROR;
        }
    }

  public:
    RequestParser(RequestEngine* engine, AuthStrategy* auth_strategy)
    {
        this->engine =  engine;
        this->auth = auth_strategy;
    }

    //
    // Parse request from given connection and set connection properties
    // 
    void parseRequest(Connection* conn)
    {
        string res = intParseRequest(conn);
        conn->setResponse(res);
    }
};

#endif //REQPARER_H