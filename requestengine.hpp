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

  public:
    RequestEngine(string& data_root, string& auth_root)
    {
        this->data_root = data_root;
        this->auth_root = auth_root;
    }
    RequestEngine(const char* data_root, const char* auth_root): data_root(data_root), auth_root(auth_root)
    {}

    int createUser(string &username, string &password, string &publicLimit, string &privateLimit)
    {
      try
      {
        std::ofstream usersFile;
        usersFile.open(auth_root + "/users.auth", std::ios::app);
        usersFile << username + ":" + password + ":" + publicLimit + ":" + privateLimit + ":0:0\n";
        usersFile.close();
        return 0;
      }
      catch (...)
      {
        return -1;
      }
    }


};

#endif // REQENGINE_H
