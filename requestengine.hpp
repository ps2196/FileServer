#ifndef REQENGINE_H
#define REQENGINE_H

#include <fstream>
#include <string>
#include <vector>
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
    
    int createFile(const string& path, const string& name)
    {
      //TO DO
    }
    
    int listDirectory(const string& path, std::vector<string>& files)
    {
      //TO DO
    }

    int deleteFile(const string& path)
    {
      //TO DO
    }

};

#endif // REQENGINE_H