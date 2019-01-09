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
    ~Connection()
    {
        if (user)
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
    string getResponse() const { return responses.front();}
    void setResponse(string res) 
    {
        responses.push_back(res);
    }
    bool isRequsetComplete() const {return (requests.size() > 0);}
    bool responsePending() const {return (responses.size() > 0);}
        

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

    void sendResponse()
    {   
        string& res = responses.front();
        std::cout<<"["<<socket<<"]Sending response: "<<responses.front()<<std::endl<<std::flush;
        int bytes_sent = send(socket,res.c_str(), res.size(),MSG_DONTWAIT);
        
        if(bytes_sent < res.size() && bytes_sent > 0) // erase sent fragment from response and keep it in the Q
            res.erase(0,bytes_sent);
        else // Whole response was sent - pop it from Q
            responses.pop_front();
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
        user = nullptr;
        read_fdset = nullptr;
        write_fdset = nullptr;
        requests.clear();
        responses.clear(); 
    }
};

#endif //CONNECTION_H
