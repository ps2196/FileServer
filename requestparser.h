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

    //
    // Parse request held by given connection and generate response
    //
    string intParseRequest(Connection* conn)
    {
        try
        {
            json req = json::parse(conn->getRequest());
            string command = req["command"];
            string type = req["type"];

            printf("%s %s\n", type.c_str(), command.c_str());

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
                    res_json["command"] = cmd;
                    res_json["code"] = 200;
                    res_json["data"] = user->toJson();
                    return res_json.dump();
                }
            }
            else if( cmd == "TOUCH")
            { // chec if authorized and call engine method

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
