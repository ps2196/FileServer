#include "connection.h"
#include "auth_strategy/authstrategy.hpp"
#include "requestparser.h"
#include "requestengine.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>

using string = std::string;

#define DEFAULT_PORT 8888
#define BACKLOG_SIZE 5 //maximum number of waiting connections, used in listen


std::unordered_map<std::string, Connection*> activeUploads; // maps path to Connections witch uplad the file

int parseCommandLineArgs(int argc, char **argv, int &port, string &data_root, string& auth_root);

int main(int argc, char **argv)
{
    int sock;
    socklen_t length;
    struct sockaddr_in server;
    fd_set ready, write_ready;
    struct timeval to;
    int msgsock = -1, nfds, nactive;

    string data_root, auth_root;
    int port;
    parseCommandLineArgs(argc, argv,port, data_root, auth_root);
    AuthStrategy auth = AuthStrategy(auth_root+"users.auth");
    RequestEngine engine = RequestEngine(data_root, auth_root);
    RequestParser parser = RequestParser(&engine, &auth);

    std::vector<Connection> connections;

    char buf[1024];
    int rval = 0, i;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("opening stream socket");
        exit(1);
    }
    nfds = sock + 1;

    // zgub denerwuja ̨cy komunikat bł ̨edu "Address already in use"
    int yes = 1;
    if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(1);
      }

    /* dowiaz adres do gniazda */

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    if (bind(sock, (struct sockaddr *)&server, sizeof server) == -1)
    {
        perror("binding stream socket");
        exit(1);
    }
    /* wydrukuj na konsoli przydzielony port */
    length = sizeof(server);
    if (getsockname(sock, (struct sockaddr *)&server, &length) == -1)
    {
        perror("getting socket name");
        exit(1);
    }
    printf("Socket port #%d\n", ntohs(server.sin_port));
    /* zacznij przyjmowaæ polaczenia... */
    listen(sock, BACKLOG_SIZE);

    do
    {
        FD_ZERO(&ready); FD_ZERO(&write_ready);
        FD_SET(sock, &ready);
        for(auto c = connections.begin(); c != connections.end(); ++c)
        {
            if(c->getSocket() == -1)
            { // Erase closed connection from connections vector
                auto next = connections.erase(c);
                if( next == connections.end())
                    break; // erase returns iter to next element after erased
                else
                    c = next;
            }
            c->setReadReady();
            c->setWriteReady();
        }

        to.tv_sec = 5;
        to.tv_usec = 0;
        if ((nactive = select(nfds, &ready, &write_ready, (fd_set *)0, &to)) == -1)
        {
            perror("select");
            continue;
        }
        if (nactive == 0)
        {
            printf("Timeout, restarting select...\n");
            continue;
        }
        if (FD_ISSET(sock, &ready))
        {
            msgsock = accept(sock, (struct sockaddr *)0, (socklen_t *)0);
            if (msgsock == -1)
                perror("accept");
            nfds = std::max(nfds, msgsock + 1); /* brak sprawdzenia czy msgsock>MAX_FDS */

            // set SO_KEEPALIVE opt
            /* Set the option active */
            int optval = 1;
            int optlen = sizeof(optval);
            if(setsockopt(msgsock, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
              perror("setsockopt()");
              close(msgsock);
              exit(EXIT_FAILURE);
            }
            printf("SO_KEEPALIVE set on socket\n");

            socklen_t optlen_t = sizeof(optval);

            if(getsockopt(msgsock, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen_t) < 0) {
              perror("getsockopt()");
              close(msgsock);
              exit(EXIT_FAILURE);
            }
            printf("SO_KEEPALIVE is %s\n", (optval ? "ON" : "OFF"));

            connections.push_back( Connection(msgsock, nullptr, &ready, &write_ready) );
            printf("accepted...(active connections = %d)\n", (int)connections.size());
        }
        for (i = 0; i < connections.size(); i++)
        {
          //std::cout << "NOW CONN = " << connections.size() << std::endl;
            if (connections[i].isReadReady())
            {
                rval = connections[i].reciveMsg();
                if (rval == 0)
                {
                    printf("Ending connection\n");
                    connections[i].closeConnection();
                }
            }

            if(connections[i].isRequsetComplete())
            {
                parser.parseRequest(&connections[i]);
            }

            if(connections[i].isWriteReady())
            {
                if (connections[i].responsesPending())
                {
                  connections[i].sendResponse();
                }

            }
        }
        //sleep(1);

    } while (true);
    /*
     * gniazdo sock nie zostanie nigdy zamkniete jawnie,
     * jednak wszystkie deskryptory zostana zamkniete gdy proces
     * zostanie zakonczony (np w wyniku wystapienia sygnalu)
     */

    exit(0);
}

//
// Parse command line arguments and set port and path to data and auth root
// Return 0 on success
int parseCommandLineArgs(int argc, char **argv, int &port, string &data_root, string &auth_root)
{
    port = DEFAULT_PORT;
    data_root = "data/";
    auth_root = "auth/";

    if (argc < 3)
        return -1; // setting any parameter requires at least 3 arguments
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i],"-p")==0)
        {
            if (i + 1 == argc)
            {
                perror("Too few arguments");
                exit(-1);
            }
            port = atoi(argv[i + 1]);
            if (port < 0 || port > 65536)
            {
                perror("Incorrect port numer");
                exit(-1);
            }
            i++;
            continue;
        }
        else if (strcmp(argv[i],"-d")==0 || strcmp(argv[i],"-data")==0)
        {
            if (i + 1 == argc)
            {
                perror("Too few arguments");
                exit(-1);
            }
            data_root = argv[i+1];
            i++;
            continue;
        }
        else if (strcmp(argv[i], "-a")==0 || strcmp(argv[i],"-auth")==0)
        {
            if (i + 1 == argc)
            {
                perror("Too few arguments");
                exit(-1);
            }
            auth_root = argv[i+1];
            i++;
            continue;
        }
        else { printf("Unrecognized option: %s\n",argv[i]);}
    }
    return 0;
}
