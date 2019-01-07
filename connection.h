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
#include "auth_strategy/user.hpp"

//
// Connection provides intrface for user connection handling
//
class Connection
{
  public:
    static const int READ_SIZE = 256;
    using string = std:: string;
  private:
    int socket; // socket used for communication with user
    User *user; // nullptr when user is not authorized
    fd_set* read_fdset;
    fd_set* write_fdset;
    char buf[512];
    string request;
    bool req_complete; // flag idicates wether whole request has been recived
    string response;
    bool response_pending; // flag indicates wether this connection has a response that needs to be sent

  public:
    Connection()
    {
        this->socket = -1;
        this->user = nullptr;
        this->read_fdset = nullptr;
        this->write_fdset = nullptr;
        response = "";
        request = "";
        req_complete = false;
        response_pending=false;
    }
    Connection(int socket, User *user = nullptr, fd_set* rfdset=nullptr, fd_set* wfdset=nullptr)
    {
        this->socket = socket;
        this->user = user;
        this->read_fdset = rfdset;
        this->write_fdset = wfdset;
        response = "";
        request = "";
        req_complete = false;
        response_pending=false;
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
    string getRequest() const { return request;}
    void resetRequest() { request = "";}
    string getResponse() const { return response;}
    void setResponse(string res)
    {
        this->response = res;
        if( response != "")
            response_pending = true;
    }
    bool isRequsetComplete() const {return req_complete;}
    bool responsePending() const {return response_pending;}


    // Read data from socket and return read result
    int reciveMsg()
    {   int rval=0;
        memset(buf, 0, sizeof buf);
        req_complete = false;
        if ((rval = read(socket, buf, READ_SIZE)) == -1)
            perror("reading stream message");
        else if (rval > 0)
        {
            request += buf;
            if(buf[rval-1] == '\0')
                req_complete = true;
        }
        return rval;
    }

    void sendResponse()
    {
        int bytes_sent = send(socket,response.c_str(), response.size(),MSG_DONTWAIT);
        response_pending=false;
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
        request = "";
        response = "";
        req_complete = false;
        response_pending = false;
    }
};

#endif //CONNECTION_H
