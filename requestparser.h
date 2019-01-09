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

  public:
    RequestParser(RequestEngine *engine, AuthStrategy *auth_strategy)
    {
        this->engine = engine;
        this->auth = auth_strategy;
    }

    //
    // Parse request from given connection and set connection properties
    //
    void parseRequest(Connection *conn)
    {
        string res = intParseRequest(conn);
        conn->setResponse(res);
    }

    //
    //Check if given connection is authorized to perform request
    //
    bool checkAuth(Connection *conn, bool admin_required = false)
    {
        User *user = conn->getUser();
        if (user == nullptr)
            return false;
        else if (!admin_required)
            return true;
        else
        { // Check admin privilages
            return (user->username == "root");
        }
    }


    //
    // Check if given permission is authorized to access path in given req
    // When "" is retuned auth is success and req conatins "path" field
    //
    static const int PATH_AUTH_OK = 1;
    static const int PATH_AUTH_NO_PATH = 2;
    static const int PATH_AUTH_NOAUTH = 3;
    string checkPathAuth(Connection *conn, const json &req)
    {
        int path_access = intCheckPathAuth(conn, req);
        if (path_access == PATH_AUTH_NOAUTH)
            return RESPONSE_UNAUTHORIZED;
        else if (path_access == PATH_AUTH_NO_PATH)
            return RESPONSE_BAD_REQUEST;
        return "";
    }
    
  private:
    //
    // Check if given permission is authorized to access path in given req
    // Success mean that req conatins "path" field
    //
    int intCheckPathAuth(Connection *conn, const json &req)
    {   
        auto path = req["path"];  
        if(path == nullptr)
            return PATH_AUTH_NO_PATH;
        User* user = conn->getUser();
        if(user == nullptr)
            return PATH_AUTH_NOAUTH;
        if(user->username == "root")
            return PATH_AUTH_OK;
        
        string spath = req["path"];
        if(spath == "")
            return PATH_AUTH_NO_PATH;
        std::vector<char> cv;
        char cc=spath[0];
        while(cc != '/' && cc != '\0' && cv.size() < 256) // Linux path lenght limit is 255
        {
            cv.push_back(cc);
            cc = spath[cv.size()];
        }
        string path_user(cv.begin(), cv.end());
        if(path_user == user->username)
            return PATH_AUTH_OK;

        return PATH_AUTH_NOAUTH;
    }

    //
    // Parse request held by given connection and generate response
    //
    string intParseRequest(Connection *conn)
    {
        try
        {
            json req = json::parse(conn->getRequest());
            if (req["type"] != nullptr && req["type"] != "REQUEST") //Bad request
                return RESPONSE_BAD_REQUEST;

            auto cmd = req["command"];
            if (cmd == nullptr)
                return RESPONSE_BAD_REQUEST;
            if (cmd == "AUTH")
            {
                if (req["username"] == nullptr || req["password"] == nullptr)
                    return RESPONSE_BAD_REQUEST;

                string username = req["username"];
                string pass = req["password"];
                User *user = auth->auth(username, pass);
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
            else if (cmd == "TOUCH")
            { 
                string path_access = checkPathAuth(conn, req);
                if (path_access != "")
                    return path_access;
                int result  = engine->createFile(static_cast<string>(req["path"]), static_cast<string>(req["name"]));
                return responseForReturnCode(result);
            }
            else if( cmd == "LS")
            {
                string path_access = checkPathAuth(conn, req);
                if (path_access != "")
                    return path_access;
                std::vector<string> files; // container for ls reult
                int result = engine->listDirectory(static_cast<string>(req["path"]), files);
                if( result < 0)
                    return responseForReturnCode(result);
                
                return "TO DO"; // zamien files na JSON
            }
            else if( cmd == "RM")
            {
                string path_access = checkPathAuth(conn, req);
                if (path_access != "")
                    return path_access;
                int result  = engine->deleteFile(static_cast<string>(req["path"]));
                return responseForReturnCode(result);
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

    //
    // Generates response based on return code of methods returned by engine methods
    //
    string responseForReturnCode(int code)
    {
        return "TO DO";
    }
};

#endif //REQPARER_H