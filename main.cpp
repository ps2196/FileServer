#include "connection.h"
#include "auth_strategy/authstrategy.hpp"
#include "requestparser.h"
#include "requestengine.hpp"
#include <vector>

#define TRUE 1

#define DEFAULT_PORT 3333
#define BACKLOG_SIZE 5 //maximum nuber of waiting connections, used in listen

int main(int argc, char **argv)
{
    int sock;
    socklen_t length;
    struct sockaddr_in server;
    fd_set ready, write_ready;
    struct timeval to;
    int msgsock = -1, nfds, nactive;

    AuthStrategy auth = AuthStrategy("auth/users.auth");
    RequestEngine engine = RequestEngine("data/", "auth/");
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

    /* dowiaz adres do gniazda */

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);
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
        for (i = 0; i < connections.size(); i++) /* dodaj aktywne do zbioru */
        {
            connections[i].setReadReady();
            connections[i].setWriteReady();
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
            connections.push_back( Connection(msgsock, nullptr, &ready, &write_ready) );
            printf("accepted...(active connections = %d)\n", (int)connections.size());
        }
        for (i = 0; i < connections.size(); i++)
        {
            if (connections[i].isReadReady())
            {
                rval = connections[i].reciveMsg();
                if (rval == 0)
                {
                    printf("Ending connection\n");
                    connections[i].closeConnection();
                }
                else
                {
                    //printf("- %2d ->%s\n", msgsock, connections[i].getRequest().c_str());
                    if(connections[i].isRequsetComplete())
                    {
                        parser.parseRequest(&connections[i]);
                    }
                    //sleep(1);
                }
            }
            if(connections[i].isWriteReady() && connections[i].responsePending())
            {
                connections[i].sendResponse();
            }
        }

    } while (TRUE);
    /*
     * gniazdo sock nie zostanie nigdy zamkniete jawnie,
     * jednak wszystkie deskryptory zostana zamkniete gdy proces
     * zostanie zakonczony (np w wyniku wystapienia sygnalu)
     */

    exit(0);
}
