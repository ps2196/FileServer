#include "connection.h"

#define TRUE 1
#define READ_SIZE 20
#define MAX_FDS 20
#define max(a, b) (((a) > (b)) ? (a) : (b))

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
    int socktab[MAX_FDS]; // oddzielne gniazdo dla kazdego polaczenia

    Connection connections[MAX_FDS];
    
    char buf[1024];
    int rval = 0, i;

    for (i = 0; i < MAX_FDS; i++)
        socktab[i] = 0;

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
        for (i = 0; i < MAX_FDS; i++) /* dodaj aktywne do zbioru */
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

        if (FD_ISSET(sock, &ready))
        {
            msgsock = accept(sock, (struct sockaddr *)0, (socklen_t *)0);
            if (msgsock == -1)
                perror("accept");
            nfds = max(nfds, msgsock + 1); /* brak sprawdzenia czy msgsock>MAX_FDS */
            connections[msgsock] = Connection(msgsock, nullptr, &ready, &write_ready);
            printf("accepted...(MAX_FDS = %d)\n", MAX_FDS);
        }
        for (i = 0; i < MAX_FDS; i++)
        {
            if (connections[i].isReadReady())
            {
                rval = connections[i].reciveMsg();
                if (rval == 0)
                {
                    printf("Ending connection\n");
                    connections[i].closeConnection();
                    socktab[msgsock] = -1;
                }
                else
                {
                    //printf("- %2d ->%s\n", msgsock, connections[i].getRequest().c_str());
                    if(connections[i].isRequsetComplete())
                    {
                        connections[i].parseRequest();
                    }
                    //sleep(1);
                }
            }
            if(connections[i].isWriteReady() && connections[i].responsePending())
            {
                connections[i].sendResponse();
            }
        }
        if (nactive == 0)
            printf("Timeout, restarting select...\n");
    } while (TRUE);
    /*
     * gniazdo sock nie zostanie nigdy zamkniete jawnie,
     * jednak wszystkie deskryptory zostana zamkniete gdy proces 
     * zostanie zakonczony (np w wyniku wystapienia sygnalu) 
     */

    exit(0);
}