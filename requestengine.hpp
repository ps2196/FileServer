#ifndef REQENGINE_H
#define REQENGINE_H

#include <fstream>
#include <string>

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
    RequestEngine(string& data_root, string& auth_root, AuthStrategy *auth)
    {
        this->data_root = data_root;
        this->auth_root = auth_root;
        this->auth = auth;
    }
    RequestEngine(const char* data_root, const char* auth_root, AuthStrategy *auth): data_root(data_root), auth_root(auth_root), auth(auth)
    {}

    int createUser(const string &username, const string &password, const string &publicLimit, const string &privateLimit, string &publicUsed, string &privateUsed)
    {
      try
      {
        std::ofstream usersFile;
        usersFile.open(auth_root + "/users.auth", std::ios::app);
        usersFile << username + ":" + password + ":" + publicLimit + ":" + privateLimit + ":" + publicUsed + ":" + privateUsed + "\n";
        usersFile.close();
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
        usersFile.open(auth_root + "/users.auth");
        newUsersFile.open(auth_root + "/usersTemp.auth");

        string line;

        while (std::getline(usersFile, line))
        {
          string usrName = splitWithDelimiter(line, ':')[0];
          if (usrName != username)
          {
            newUsersFile << line << std::endl;
          }
        }

        const string oldFile = auth_root + "/users.auth";
        const string tempFile = auth_root + "/usersTemp.auth";

        remove(oldFile.c_str());
        rename(tempFile.c_str(), oldFile.c_str());

        usersFile.close();
        newUsersFile.close();
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
        string userLine = auth->getUserLine(username);

        if (userLine == "")
          return -1;

        //std::cout << userLine << std::endl;
        std::vector<string> user = splitWithDelimiter(userLine, ':');

        // save used spaces
        string pubUsed = user[4];
        string privUsed = user[5];

        // delete old user
        if (deleteUser(username) != 0)
          return -1;

        // create new
        if (createUser(username, password, pubLimit, privLimit, pubUsed, privUsed) != 0)
          return -1;

        return 0;
      }
      catch (...)
      {
        return -1;
      }
    }

    User* getUser(string &username)
    {
      try
      {
        string userLine = auth->getUserLine(username);

        if (userLine == "")
          return nullptr;

        std::vector<string> user = splitWithDelimiter(userLine, ':');
        string password = user[1];
        string publicLimit = user[2];
        string privateLimit = user[3];
        string publicUsed = user[4];
        string privateUsed = user[5];

        return new User(username, password, std::stoi(publicLimit), std::stoi(privateLimit), std::stof(publicUsed.c_str()), std::stof(privateUsed.c_str()));
      }
      catch (...)
      {
        return nullptr;
      }
    }


};

#endif // REQENGINE_H
