#ifndef REQPARER_H
#define REQPARER_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "utils/json.hpp"
#include "auth_strategy/authstrategy.hpp"
#include "requestengine.hpp"
#include "connection.h"
#include <exception>
#include <stdio.h>

#define RESPONSE_BAD_REQUEST "{ \"type\":\"RESPONSE\", \"code\":400, \"data\":\"Bad request\"}"
#define RESPONSE_SERVER_ERROR "{ \"type\":\"RESPONSE\", \"code\":500, \"data\":\"Internal server error\"}"
#define RESPONSE_UNAUTHORIZED "{ \"type\":\"RESPONSE\", \"command\":\"AUTH\", \"code\":401, \"data\":\"Unauthorized\"}"

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
        string res = intParseRequest(conn)+'\n';
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
            json req = json::parse(conn->popRequest());
            string command = req["command"];
            string type = req["type"];

            printf("%s %s\n", type.c_str(), command.c_str());

            if(req["type"] != nullptr && req["type"] != "REQUEST") //Bad request
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
                    res_json["command"] = cmd;
                    res_json["code"] = 200;
                    res_json["data"] = user->toJson();
                    return res_json.dump();
                }
            }
            else if (cmd == "TOUCH")
            {
                string path_access = checkPathAuth(conn, req);
                if (path_access != "")
                    return path_access;
                if(req["name"] == nullptr)
                    return RESPONSE_BAD_REQUEST;
                string err_msg;
                int result  = engine->createFile(static_cast<string>(req["path"]), static_cast<string>(req["name"]), err_msg);
                if(result < 0)
                    generateResponse(409, err_msg);

                return generateResponse(200, "File created");
            }
            else if (cmd == "MKDIR")
            {
                string path_access = checkPathAuth(conn, req);
                if (path_access != "")
                    return path_access;
                if(req["name"] == nullptr)
                    return RESPONSE_BAD_REQUEST;
                string err_msg;
                int result = engine->createDirectory(static_cast<string>(req["path"]), static_cast<string>(req["name"]),err_msg);
                if(result < 0)
                    return generateResponse(409, err_msg);
                //OK
                return generateResponse(200,"Direcory created");
            }
            else if( cmd == "LS")
            {
                string path_access = checkPathAuth(conn, req);
                if (path_access != "")
                    return path_access;
                std::vector<string> files, dirs; // containers for ls reult
                string err_msg;
                int result = engine->listDirectory(static_cast<string>(req["path"]), files, dirs, err_msg);
                if( result < 0)
                    return generateResponse(409, err_msg);

                return generateLSResponse(static_cast<string>(req["path"]), files, dirs);
            }
            else if( cmd == "RM")
            {
                string path_access = checkPathAuth(conn, req);
                if (path_access != "")
                    return path_access;
                string err_msg;
                int result  = engine->deleteFile(static_cast<string>(req["path"]),err_msg);
                if(result < 0)
                    return  generateResponse(409, err_msg);
                if(result == 0)
                    return generateResponse(409, "Path not found");
                return generateResponse(200,static_cast<string>(req["path"])+ " deleted");
            }
            else if (cmd == "CREATEUSER")
            {
              User *user = conn->getUser();
              if (user != nullptr && user->username == "root")
              {
                // TODO: sprawdzac czy username zajęty. po stronie serwera
                string username = req["username"];
                string password = req["password"];
                string publicLimit = req["public"];
                string privateLimit = req["private"];
                string privateUsed = "0";
                string publicUsed = "0";

                json response;
                response["type"] = "RESPONSE";
                response["command"] = cmd;

                if(auth->getUserLine(username) != "")
                {
                  response["code"] = 406; // not acceptable
                  response["data"] = "Username is already used: " + username;
                }
                else if (engine->createUser(username, password, publicLimit, privateLimit, publicUsed, privateUsed) == 0)
                {
                  response["code"] = 200; // ok
                  response["data"] = "User created: " + username;
                }
                return response.dump();
              }
              else
                return RESPONSE_UNAUTHORIZED;
            }
            else if (cmd == "DELETEUSER")
            {
              //std::cout << "zara cie usune!";

              User *user = conn->getUser();
              if (user != nullptr && user->username == "root")
              {
                json response;
                response["type"] = "RESPONSE";
                response["command"] = cmd;

                // TODO: wylogowac go najpierw
                string username = req["username"];
                if (engine->deleteUser(username) == 0)
                {
                  response["code"] = 200; // ok
                  response["data"] = "User has been deleted: " + username;
                }
                else
                {
                  response["code"] = 409; // conflict
                  response["data"] = "User has NOT been deleted: " + username;
                }
                return response.dump();
              }
              else
                return RESPONSE_UNAUTHORIZED;
            }
            else if (cmd == "CHUSER")
            {
              User *user = conn->getUser();
              if (user != nullptr && user->username == "root")
              {
                json response;
                response["type"] = "RESPONSE";
                response["command"] = cmd;

                string username = req["username"];
                string password = req["password"];
                string publicLimit = req["public"];
                string privateLimit = req["private"];

                if (engine->alterUser(username, password, publicLimit, privateLimit) == 0)
                {
                  response["code"] = 200; // ok
                  response["data"] = "User altered: " + username;
                }
                else
                {
                  response["code"] = 409; // conflict
                  response["data"] = "User not altered: " + username;
                }

                return response.dump();
              }
              else
                return RESPONSE_UNAUTHORIZED;
            }
            else if (cmd == "USER")
            {
              User *user = conn->getUser();
              if (user != nullptr  && (user->username == "root" || user->username == req["username"]))
              {
                json response;
                response["type"] = "RESPONSE";
                response["command"] = cmd;

                string username = req["username"];
                User *userToReturn = engine->getUser(username);

                if (userToReturn != nullptr)
                {
                  response["code"] = 200; //ok
                  response["data"] = userToReturn->toJson();
                  delete user;
                }
                else
                {
                  response["code"] = 404; // not found
                  response["data"] = "User not found.";
                }
                return response.dump();
              }
              else
                return RESPONSE_UNAUTHORIZED;
            }
            else if (cmd == "DWL")
            {
              // 1. Check if authorized
              User *user = conn->getUser();
              if (true)//(user != nullptr  && (user->username == "root" || user->username == req["username"]))
              {
                // 2. Get details form request
                string path = req["path"];

                // 3. check if user has access to the requested file
                string fileChunk;
                engine->sendFile(path, fileChunk);

                // 6. Create response
                json response;
                response["type"] = "RESPONSE";
                response["command"] = "DWL";
                response["code"] = 200; // ok
                response["path"] = path;
                response["data"] = fileChunk;

                return response.dump();
              }
              else
                return RESPONSE_UNAUTHORIZED;
            }
            else if (cmd == "GETFILE")
            {
              std::ifstream file("fotka.jpg", std::ios::binary);
              file.seekg(0, file.end);
              int length = file.tellg();
              file.seekg(0, file.beg);

              char *buffer = new char[length];
              file.read(buffer, length);


              //std::cout << buffer << std::endl;
              unsigned char *toEncode = reinterpret_cast<unsigned char*>(buffer);
              //std::cout << toEncode << std::endl;


              string encoded = base64_encode(toEncode, length);

              std::cout << "Size toEncode: " << length << " After encoding: " << encoded.size() << std::endl;

              //std::cout << encoded << std::endl;

              string decoded = base64_decode(encoded);
              //std::cout << decoded << std::endl;

              std::ofstream output("output", std::ios::binary);
              output << decoded;

              json fileJson;
              fileJson["name"] = "fotka.jpg";
              fileJson["path"] = "/server";
              //fileJson["data"] = encoded;

              json response;
              response["type"] = "RESPONSE";
              response["command"] = cmd;
              response["code"] = 200; //ok
              //response["data"] = fileJson;

                return response.dump();

              //return RESPONSE_BAD_REQUEST;

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


    // Generate response with given code and text - text is put in "data" field
    string generateResponse(int code, const string& text)
    {
        json res_json;
        res_json["type"] = "RESPONSE";
        res_json["code"] = code;
        res_json["data"] = text;
        return res_json.dump();
    }

    // Generate response for LS request in JSON format
    string generateLSResponse(const string& path, std::vector<string> files,std::vector<string> dirs)
    {
        json res_json;
        res_json["type"] = "RESPONSE";
        res_json["code"] = 200;
        res_json["path"] = path;
        res_json["files"] = files;
        res_json["dirs"] = dirs;
        return res_json.dump();
    }
};

#endif //REQPARER_H
