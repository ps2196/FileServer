#ifndef REQENGINE_H
#define REQENGINE_H

#include <fstream>
#include <string>
#include <vector>
#include "utils/base644.h"
#include "auth_strategy/authstrategy.hpp"
#include <boost/filesystem.hpp>

namespace FS = boost::filesystem;
//
// Request engine provides interface for executing operations on the server
//
class RequestEngine
{
private:
  using string = std::string;

  string data_root; // path to directory with users catalogues
  string auth_root; // path to directory with auth files
  AuthStrategy *auth;

public:
  RequestEngine(string &data_root, string &auth_root)
  {
    this->data_root = data_root;
    this->auth_root = auth_root;
  }
  RequestEngine(const char *data_root, const char *auth_root) : data_root(data_root), auth_root(auth_root)
  {
  }

  string getDataRoot() const {return data_root;}

    RequestEngine(string& data_root, string& auth_root, AuthStrategy *auth)
    {
        this->data_root = data_root;
        this->auth_root = auth_root;
        this->auth = auth;
    }
    RequestEngine(const char* data_root, const char* auth_root, AuthStrategy *auth): data_root(data_root), auth_root(auth_root), auth(auth)
    {}

    int createUser(const string &username, const string &password, const string &publicLimit, const string &privateLimit, const string& pubUsed = "0", const string privUsed = "0")
    {
      try
      {
        std::ofstream usersFile;
        usersFile.open(auth_root + "users.auth", std::ios::app);
        usersFile << username + ":" + password + ":" + publicLimit + ":" + privateLimit + ":"+pubUsed+":"+privUsed+"\n";
        usersFile.close();

        // Make directory
        string err;
        if (createDirectory("", username, err) == -1)
        {
          std::cout << err << std::endl;
          return -1;
        }
        if (createDirectory(username, "public", err) == -1)
        {
          std::cout << err << std::endl;
          return -1;
        }
        if (createDirectory(username, "private", err) == -1)
        {
          std::cout << err << std::endl;
          return -1;
        }

        return 0;
      }
      catch (...)
      {
        return -1;
      }
    }

    int deleteUser(const string &username)
    {
      try
      {
        std::ifstream usersFile;
        std::ofstream newUsersFile;
        usersFile.open(auth_root + "users.auth");
        newUsersFile.open(auth_root + "usersTemp.auth");

        string line;
        bool userDeleted = false;

        while (std::getline(usersFile, line))
        {
          string usrName = splitWithDelimiter(line, ':')[0];
          if (usrName != username)
            newUsersFile << line << std::endl;
          else
            userDeleted = true;
        }

        if (!userDeleted)
          return -1;

        const string oldFile = auth_root + "users.auth";
        const string tempFile = auth_root + "usersTemp.auth";

        remove(oldFile.c_str());
        rename(tempFile.c_str(), oldFile.c_str());

        usersFile.close();
        newUsersFile.close();

        // delete directory
        string err;
        if (deleteFile(username, err) == -1)
        {
          std::cout << err << std::endl;
          return -1;
        }
        return 0;
      }
      catch (...)
      {
        return -1;
      }
    }

    int alterUser(const string &username, const string &password, const string &pubLimit, const string &privLimit)
    {
      try
      {
        // find user
        User *user = findUser(username);
        if (user == nullptr)
          return -1;

        // save used spaces
        string pubUsed = std::to_string(user->publicUsed);
        string privUsed = std::to_string(user->privateUsed);

        // delete old user
        if (deleteUser(username) != 0)
        {
          delete user;
          return -1;
        }

        // create new
        if (createUser(username, password, pubLimit, privLimit, pubUsed, privUsed) != 0)
        {
          delete user;
          return -1;
        }

        delete user;
        return 0;
      }
      catch (...)
      {
        return -1;
      }
    }

    string getUser(const string &username)
    {
      try
      {
        string userLine = auth->getUserLine(username);

        if (userLine == "")
          return "";

        std::vector<string> user = splitWithDelimiter(userLine, ':');
        string password = user[1];
        string publicLimit = user[2];
        string privateLimit = user[3];
        string publicUsed = user[4];
        string privateUsed = user[5];
        User u(username, password, std::stoi(publicLimit), std::stoi(privateLimit), std::stof(publicUsed.c_str()), std::stof(privateUsed.c_str()));
        return u.toJson();
      }
      catch (...)
      {
        return nullptr;
      }
    }

    bool uploadFile(string &name, string &path, string &dataEncoded)
    {
      try
      {
        // Decode and save
        string decoded = base64_decode(dataEncoded);
        std::ofstream file(path + "temp_" + name, std::ios::binary | std::ios::app);
        file << decoded;
        file.close();
        return true;
      }
      catch (...)
      {
        return false;
      }

    }

    bool finishUpload(string &path, string &name)
    {
      // rename file
      try
      {
        FS::rename(path + "temp_" + name, path + name);
        return true;
      }
      catch (...)
      {
        return false;
      }
    }
/*
    int sendFile(const string &path, string &fileChunk)
    {
      const int chunkSize = 30;  // TODO: zmienic to na jakis const
      const int offset = 0;

      // read chunk of file considering offset
      std::ifstream file(path, std::ios::binary);
      file.seekg(offset);
      char *buffer = new char[chunkSize];
      file.read(buffer, chunkSize);

      // Encode buffer
      unsigned char *bufferToEncode = reinterpret_cast<unsigned char*>(buffer);
      fileChunk = base64_encode(bufferToEncode, chunkSize);

      // 7. clean up
      file.close();
      delete[] buffer;

      return 0;
    }
*/
  int createFile(const string &path, const string &name, string &err_msg)
  {
    string p = data_root+path;
    try
    {
      if (!FS::exists(p))
      {
        err_msg = path + " does not exist";
        return -1;
      }
      std::ofstream f(p+"/"+name);
      if(f.is_open())
        f.close();
    }
    catch (const FS::filesystem_error &ex)
    {
      err_msg = ex.what();
      return -1;
    }
    return 0;
  }

  int createDirectory(const string &path, const string &name, string &err_msg)
  {
    try
    {
      FS::create_directory(data_root + path + "/" + name);
    }
    catch (const FS::filesystem_error &ex)
    {
      err_msg = ex.what();
      return -1;
    }
    return 0;
  }

  //
  // Go through given path put all file names in FILES and all directories names in DIRS
  //
  int listDirectory(const string &path, std::vector<string> &files, std::vector<string> &dirs, string &err_msg)
  {
    string p = data_root+path;
    try
    {
      if (FS::exists(p))
      {
        if (FS::is_directory(p))
        {
          for (FS::directory_entry &i : FS::directory_iterator(p))
          {
            if (FS::is_directory(i.path()))
              dirs.push_back(i.path().filename().string());
            else
              files.push_back(i.path().filename().string());
          }
        }
        return 0;
      }
      else
      {
        err_msg = path + " does not exist.";
      }
    }
    catch (const FS::filesystem_error &ex)
    {
      err_msg = ex.what();
      return -1;
    }
  }

  int deleteFile(const string &path, string& err_msg)
  {
    try
    {
      int files_removed = FS::remove_all(data_root + path);
      return files_removed;
    }
    catch (const FS::filesystem_error &ex)
    {
      err_msg = ex.what();
      return -1;
    }
  }

  User* findUser(const string &username)
  {
    std::ifstream usersFile;
    usersFile.open(auth_root + "users.auth");

    string line;
    while (std::getline(usersFile, line))
    {
      std::vector<string> userString = splitWithDelimiter(line, ':');
      string usrName = userString[0];
      string pass = userString[1];
      string pubLimit = userString[2];
      string privLimit = userString[3];
      string pubUsed = userString[4];
      string privUsed = userString[5];

      if (usrName == username)
      {
        usersFile.close();
        return new User(username, pass, std::stoi(pubLimit), std::stoi(privLimit), std::stof(pubUsed.c_str()), std::stof(privUsed.c_str()));
      }
    }
  }
};
#endif // REQENGINE_H
