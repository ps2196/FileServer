#ifndef REQENGINE_H
#define REQENGINE_H

#include <fstream>
#include <string>
#include <vector>
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

public:
  RequestEngine(string &data_root, string &auth_root)
  {
    this->data_root = data_root;
    this->auth_root = auth_root;
  }
  RequestEngine(const char *data_root, const char *auth_root) : data_root(data_root), auth_root(auth_root)
  {
  }

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
};

#endif // REQENGINE_H