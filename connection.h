#ifndef CONNECTION_H
#define CONNECTION_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <list>
#include <vector>
#include "auth_strategy/user.hpp"
#include "utils/base644.h"
#include "utils/json.hpp"
#include <iostream>
#include <fstream>
//#include "downloadProcess.h"

unsigned long long bytes = 0;

class Connection;

class downloadProcess
{
private:
  using string = std::string;
  string path; // file path
  unsigned long long offset; // current file offset
  std::ifstream file; // file to be sent
  const int CHUNK_SIZE = 1024; // size of data chunk of file
  Connection *connection; // connection which triggered download process
  using json = nlohmann::json;

public:
  downloadProcess(string &path, Connection *conn);

  ~downloadProcess();

  int putNextPackage(int packageSize);

  int putOneChunk();

  char* getDataChunk(int &);

  string getPath() const {return path;}

};

//
// Connection provides intrface for user connection handling
//
class Connection
{
  public:
    static const int READ_SIZE = 256;
    static const int REQUEST_SIZE_LIMIT = 10000; // maximum length of a single request
    using string = std::string;
  private:
    int socket; // socket used for communication with user
    User *user; // nullptr when user is not authorized
    fd_set* read_fdset;
    fd_set* write_fdset;
    char buf[READ_SIZE+1]; // buffer for reading from socket
    std::vector<char> recived_chars; // container for storing text read from socket until whole request is recived
    std::list<string> requests; // incoming reques are queued in connection
    std::list<string> responses; // responses are queued waiting to be sent
    std::vector<downloadProcess*> downloadProcesses;

  public:
    Connection(): requests(),responses(), recived_chars()
    {
        this->socket = -1;
        this->user = nullptr;
        this->read_fdset = nullptr;
        this->write_fdset = nullptr;
    }
    Connection(int socket, User *user = nullptr, fd_set* rfdset=nullptr, fd_set* wfdset=nullptr):
    requests(),responses(),recived_chars()
    {
        this->socket = socket;
        this->user = user;
        this->read_fdset = rfdset;
        this->write_fdset = wfdset;
    }

    Connection(const Connection &other)
    {
        user = nullptr;
        *this = other;
    }

    Connection& operator=(const Connection& other)
    {
        if(this != &other)
        {
            requests = other.requests;
            responses = other.responses;
            socket = other.socket;

            if(other.user != nullptr)
            {
                if(user != nullptr)
                    delete user;
                user = new User(*(other.user));
            }
            else
                user = nullptr;

            read_fdset = other.read_fdset;
            write_fdset = other.write_fdset;
            recived_chars = other.recived_chars;
        }
        return *this;
    }


    ~Connection()
    {
        std::cout<<"Connections destructor: "<<user<<std::endl<<std::flush;
        if (user!=nullptr)
            delete user;
    }

    User* getUser() const {return user;}
    void setUser(User* user ){this->user = user;}
    int getSocket() const{return socket;}
    void setSocket(int socket){this->socket = socket;}
    //return request from Q fron without popping it
    string getRequest() const {return requests.front();}
    // Pops request from the queue and reurns it
    string popRequest()
    {
        string req = requests.front();
        requests.pop_front();
        return req;
    }
    void setResponse(string res)
    {
        //std::cout << "ADDING RESPONSE: " + res << std::endl;
        responses.push_back(res);
    }
    bool isRequsetComplete() const {return (requests.size() > 0);}
    int responsesPending() const {return responses.size();}


    // Read data from socket and return read result
    int reciveMsg()
    {   int rval=0;
       // memset(buf, 0, sizeof buf);
       // req_complete = false;
        if ((rval = read(socket, buf, READ_SIZE)) == -1)
            perror("reading stream message");
        else if (rval > 0)
        {
            buf[rval] = '\0';
            for(int i =0; i < rval; i++)
            {
                if(buf[i] != '\0')
                {
                    recived_chars.push_back(buf[i]);
                    if(recived_chars.size() > REQUEST_SIZE_LIMIT)
                        closeConnection(); // Close connection when request size limi is exceeded
                }
                else
                {//'\0' means that wohle request has been recived - push it in the Q and reset recived_chars
                    requests.push_back(string(recived_chars.begin(), recived_chars.end()));
                    std::cout<<"["<<socket<<"]NEW Request:"<<requests.back()<<std::endl<<std::flush;
                    recived_chars.clear();
                }
            }
        }
        return rval;
    }

    int sendResponse()
    {
        string& res = responses.front();
        //std::cout<<"["<<socket<<"]Sending response: "<<std::endl<<std::flush;//<<responses.front()<<std::endl<<std::flush;
/*
        int optval = 0;
        socklen_t optlen_t = sizeof(optval);
        if(getsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen_t) < 0) {
          perror("getsockopt()");
          close(socket);
          exit(EXIT_FAILURE);
        }
        printf("SO_KEEPALIVE is %s\n", (optval ? "ON" : "OFF"));
*/
        int bytes_sent = send(socket, res.c_str(), res.size(),MSG_DONTWAIT);
        std::cout<<"["<<socket<<"]Bytes sent: " << bytes_sent<<std::endl<<std::flush;
        bytes += bytes_sent;

        //sleep(1);
        if(bytes_sent < res.size() && bytes_sent > 0) // erase sent fragment from response and keep it in the Q
            res.erase(0,bytes_sent);
        else // Whole response was sent - pop it from Q
        {
            responses.pop_front();
            handleDownloads();
        }

        return bytes_sent;
    }

    void handleDownloads()
    {
      for (int i = 0; i < downloadProcesses.size(); i++)
      {
        if (downloadProcesses[i]->putNextPackage(1) == 0) // it was unable to put next package into the responses queue. Whole file was queued.
        {
          //std::cout<<"DWL [" << downloadProcesses[i]->getPath() << "] ENDED\n" << "PENDING DOWNLOADS: " << downloadProcesses.size() << std::endl;
          delete downloadProcesses[i];
          downloadProcesses.erase(downloadProcesses.begin() + i);
          //std::cout<<"PENDING DOWNLOADS AFTER POP: " << downloadProcesses.size() << std::endl;
        }
      }
    }

    void pushDownloadProcess(downloadProcess *actvDwnl)
    {
      downloadProcesses.push_back(actvDwnl);
    }

    // Return true if read from socket won't block
    // Server needs to call select() for this to work
    bool isReadReady()
    {
        return (socket > 0 && read_fdset != nullptr && FD_ISSET(socket, read_fdset));
    }

    bool isWriteReady()
    {
        return (socket > 0 && write_fdset != nullptr && FD_ISSET(socket, write_fdset));
    }
    void setReadReady()
    {
        if(read_fdset && socket > 0)
            FD_SET(socket, read_fdset);
    }
    void setWriteReady()
    {
        if(write_fdset && socket > 0)
            FD_SET(socket, write_fdset);
    }

    void closeConnection()
    {
        if(socket > 0)
            close(socket);
        socket = -1 ;
        if(user != nullptr){
            user = nullptr;
        }
        read_fdset = nullptr;
        write_fdset = nullptr;
        requests.clear();
        responses.clear();

        std::cout <<bytes<<std::endl;
    }
};


downloadProcess::downloadProcess(string &path, Connection *conn)
{
  this->path = path;
  this->connection = conn;
  offset = 0; // initial offset is 0, start reading at beginning
  file.open(path, std::ios::binary);
}

downloadProcess::~downloadProcess()
{
  file.close();
  connection = nullptr;
}

/**
* Appends next package of data chunks to the connection.responses.
* packageSize parameter determines number of chunks in one package.
* Returns 0 when whole file was sent and there is no need to put next packages. Object can be destroyed.
* Returns 1 when there are still chunks to be sent.
*/
int downloadProcess::putNextPackage(int packageSize)
{
  for (int i = 0; i < packageSize; i++)
  {
    if (putOneChunk() == -1) // last chunk of file appneded
      return 0;
  }
  return 1;
}

int downloadProcess::putOneChunk()
{
  // get chunk of file
  int sizeOfChunk;
  char *binaryChunk = getDataChunk(sizeOfChunk);
  offset += CHUNK_SIZE;

  if (binaryChunk == nullptr) // end of file
    return -1;

  // encode with base64
  unsigned char *chunkToEncode = reinterpret_cast<unsigned char*>(binaryChunk);
  string encodedChunk = base64_encode(chunkToEncode, sizeOfChunk);

  // prepare json wrapper for chunk = response
  json response;
  response["type"] = "RESPONSE";
  response["command"] = "DWL";
  response["code"] = 200; // ok
  response["path"] = path;
  response["data"] = encodedChunk;

  // appned json wrapper
  string responseString = response.dump()+"\n";
  connection->setResponse(responseString);

  // clean up
  delete[] binaryChunk;
  //delete encodedChunk;
  return 0;
}

char* downloadProcess::getDataChunk(int &sizeOfChunk)
{
  file.seekg(offset);
  char *buffer = new char[CHUNK_SIZE];
  file.read(buffer, CHUNK_SIZE);
  sizeOfChunk = file.gcount();

  if (file.gcount() == 0) // end of file
  {
    file.close();
    delete[] buffer;
    return nullptr;
  }
  return buffer;
}

#endif //CONNECTION_H
