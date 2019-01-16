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
#include <unordered_map>
#include <utility>

#define RESPONSE_BAD_REQUEST "{ \"type\":\"RESPONSE\", \"code\":400, \"data\":\"Bad request\"}"
#define RESPONSE_SERVER_ERROR "{ \"type\":\"RESPONSE\", \"code\":500, \"data\":\"Internal server error\"}"
#define RESPONSE_UNAUTHORIZED "{ \"type\":\"RESPONSE\", \"command\":\"AUTH\", \"code\":401, \"data\":\"Unauthorized\"}"

extern std::unordered_map<std::string, Connection*> activeUploads;//maps path to Connections witch uplad the file

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
      if (res != "")
        conn->setResponse(res + "\n");
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
                    return RESPONSE_UNAUTHORIZED;

                else
                { //Authorized
                    conn->setUser(user);
                    return generateResponse(200,cmd, "Welcome "+user->username);
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
                string path = req["path"];
                string name = req["name"];
                int result  = engine->createFile(path, name, err_msg);
                if(result < 0)
                    generateResponse(409,cmd, err_msg);

                return generateResponse(200,cmd, "File created");
            }
            else if (cmd == "MKDIR")
            {
                string path_access = checkPathAuth(conn, req);
                if (path_access != "")
                    return path_access;
                if(req["name"] == nullptr)
                    return RESPONSE_BAD_REQUEST;
                string err_msg;
                string path = req["path"];
                string name = req["name"];
                int result = engine->createDirectory(path, name, err_msg);
                if(result < 0)
                    return generateResponse(409, cmd, err_msg);
                //OK
                return generateResponse(200,cmd,"Direcory created");
            }
            else if( cmd == "LS")
            {
                string path_access = checkPathAuth(conn, req);
                if (path_access != "")
                    return path_access;
                std::vector<string> files, dirs; // containers for ls reult
                string err_msg;

                string path = req["path"];
                int result = engine->listDirectory(path, files, dirs, err_msg);
                if( result < 0)
                    return generateResponse(409,cmd,err_msg);

                return generateLSResponse(path, files, dirs);
            }
            else if( cmd == "RM")
            {
                string path_access = checkPathAuth(conn, req);
                if (path_access != "")
                    return path_access;
                string err_msg;

                string path = req["path"];
                int result  = engine->deleteFile(path, err_msg);
                if(result < 0)
                    return  generateResponse(409, cmd, err_msg);
                if(result == 0)
                    return generateResponse(409, cmd, "Path not found");
                return generateResponse(200, cmd, path + " deleted");
            }
            else if (cmd == "CREATEUSER")
            {
                if (!checkAuth(conn, true)) // Check if connection has admin privilages
                    return RESPONSE_UNAUTHORIZED;

                string username = req["username"];
                string password = req["password"];
                string publicLimit = req["public"];
                string privateLimit = req["private"];

                if (auth->getUserLine(username) != "")
                    return generateResponse(406, cmd, "Username is already used: " + username);

                if (engine->createUser(username, password, publicLimit, privateLimit) == 0)
                    return generateResponse(200, cmd, "User created: " + username);
                else
                    return generateResponse(409, cmd, "Something went wrong.");

            }
            else if (cmd == "DELETEUSER")
            {
                if (!checkAuth(conn, true)) // Check if connection has admin privilages
                    return RESPONSE_UNAUTHORIZED;
                // TODO: wylogowac go najpierw
                string username = req["username"];
                if (engine->deleteUser(username) == 0)
                    return generateResponse(200, cmd, "User has been deleted: " + username);
                return generateResponse(409, cmd, "User has NOT been deleted: " + username);
            }
            else if (cmd == "CHUSER")
            {
                if (!checkAuth(conn, true)) // Check if connection has admin privilages
                    return RESPONSE_UNAUTHORIZED;

                string username = req["username"];
                string password = req["password"];
                string publicLimit = req["public"];
                string privateLimit = req["private"];

                if (engine->alterUser(username, password, publicLimit, privateLimit) == 0)
                    return generateResponse(200,cmd, "User altered: " + username);

                return generateResponse(409, cmd,"User not altered: " + username);
            }
            else if (cmd == "USER")
            {
              User *user = conn->getUser();
              if (user != nullptr  && (user->username == "root" || user->username == req["username"]))
              {
                string username = req["username"];
                string userJSON = engine->getUser(username);
                if(userJSON == "")
                    return generateResponse(404, cmd, "User not found.");
                return generateResponse(200, cmd, userJSON);
              }
              else
                return RESPONSE_UNAUTHORIZED;
            }
            else if (cmd == "DWL")
            {
              // 1. Check if authorized
              if (!checkAuth(conn, false)) // Check if connection has admin privilages
                  return RESPONSE_UNAUTHORIZED;

              // 2. Get details form request
              string path = req["path"];
              string priority = req["priority"];
              int priorityInt = std::atoi(priority.c_str());

              if (priorityInt > 10 || priorityInt < 1)
                return RESPONSE_BAD_REQUEST;

              // 3. check if user has access to the requested file
              // todo

              // Create download process and push to the conn.activeDownloads list
              DownloadProcess *dwlProc = new DownloadProcess(path, conn, priorityInt);
              conn->pushDownloadProcess(dwlProc);

              // Push first package of data
              dwlProc->putNextPackage(5);

              std::cout << "DWL [" << path << "] RESPONSE\n";
              return ""; // empty string because there were repsponses pushed already
            }
            else if (cmd == "DWLABORT")
            {
              // 1. Check if authorized
              if (!checkAuth(conn, false)) // Check if connection has admin privilages
                  return RESPONSE_UNAUTHORIZED;

              // 2. Get details form request
              string path = req["path"];

              // 3. Delete downloadProcess
              if (conn->abortDownloadProcess(path))
                return generateResponse(200, cmd, path);
              else
                return generateResponse(409, cmd, path);
            }
            else if (cmd == "DWLPRI")
            {
              // 1. Check if authorized
              if (!checkAuth(conn, false)) // Check if connection has admin privilages
                  return RESPONSE_UNAUTHORIZED;

              // 2. Get details form request
              string path = req["path"];
              string priority = req["priority"];
              int priorityInt = std::atoi(priority.c_str());

              if (priorityInt > 10 || priorityInt < 1)
                return RESPONSE_BAD_REQUEST;

              // 3. Alter downloadProcess priority
              if (conn->changeDownloadPriority(path, priorityInt))
                return generateResponse(200, cmd, path);
              else
                return generateResponse(409, cmd, path);
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
    string generateResponse(int code, const string& cmd="", const string& text="")
    {
        json res_json;
        res_json["type"] = "RESPONSE";
        res_json["command"] = cmd;
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
        res_json["command"] = "LS";
        res_json["path"] = path;
        res_json["files"] = files;
        res_json["dirs"] = dirs;
        return res_json.dump();
    }
};

#endif //REQPARER_H
