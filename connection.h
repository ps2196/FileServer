#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "auth_strategy/user.hpp"

//
// Connection provides intrface for user connection handling
//

class Connection
{
    int socket; // socket used for communication with user
    User *user;

  public:
    Connection()
    {
        this->socket = 0;
        this->user = nullptr;
    }
    Connection(int socket, User *user = nullptr)
    {
        this->socket = socket;
        this->user = user;
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
};