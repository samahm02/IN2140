

#include "xmlfile.h"
#include "connection.h"
#include "record.h"
#include "recordToFormat.h"
#include "recordFromFormat.h"
 
#include <arpa/inet.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
 
#define MAKS_CLIENTER 26


struct Client
{
int sock_fd;
char* buffer;
size_t bufferPlass;
size_t bufferBrukt;
int mldType;
int mldDest;
int mldPlass;
bool aktiv; 
};
 
typedef struct Client Client;


void usage( char* cmd )
{
    fprintf( stderr, "Usage: %s <port>\n"
                     "       This is the proxy server. It takes as imput the port where it accepts connections\n"
                     "       from \"xmlSender\", \"binSender\" and \"anyReceiver\" applications.\n"
                     "       <port> - a 16-bit integer in host byte order identifying the proxy server's port\n"
                     "\n",
                     cmd );
    exit( -1 );
}

int aktiveClienter(Client clients[], int clienter) {
    int aktive = 0;
    for(int i = 0; i < clienter; i++) {
        if(clients[i].aktiv) {
            aktive++;
        }
    }
    return aktive;
}

 
void handleNewClient(int server_sock, fd_set* venteSet, Client clients[]) 
{
    int nyKoblig = tcp_accept(server_sock);

    if(nyKoblig == -1) {
        perror("HandleNewClient");
        return;
    }

    FD_SET(nyKoblig, venteSet);
    bool lagtTil = false;
    int aktive = aktiveClienter(clients, MAKS_CLIENTER);
    if(aktive >= MAKS_CLIENTER) {
        fprintf(stderr, "Kan ikke godta ny klient. Maksimalt antall nådd.\n");
        tcp_close(nyKoblig);
        return;
    }

    for (int i = 0; i < MAKS_CLIENTER && !lagtTil; i++) {
        if (clients[i].sock_fd == -1) {
            clients[i].sock_fd = nyKoblig;
            clients[i].bufferPlass = 5000;
            clients[i].bufferBrukt = 0;
            clients[i].buffer = (char*) malloc(clients[i].bufferPlass);

            if (clients[i].buffer == NULL) {
                fprintf(stderr, "Minnetildeling mislyktes for ny klient buffer\n");
                tcp_close(nyKoblig);
                return;
            }

            clients[i].aktiv = true;
            lagtTil = true;
        }
    }
}


void removeClient( Client* client, fd_set *venteSet) {
    FD_CLR(client->sock_fd, venteSet);
    tcp_close(client->sock_fd);
    client->sock_fd = -1;
    
    if (client->buffer) {
        free(client->buffer);
        client->buffer = NULL;
    }

    client->bufferPlass = 0;
    client->bufferBrukt = 0;
    client->mldType = 0;
    client->mldDest = 0;
    client->mldPlass = 0;
    client->aktiv = false;
}
 

void forwardMessage(Record* msg, Client clients[]) 
{
    Client *client = NULL;
    if (msg->has_dest) 
    {
        int dest = msg->dest;
        for (int i = 0; i < MAKS_CLIENTER; i++) 
        {
            if (clients[i].sock_fd != -1 && clients[i].mldDest == dest) 
            {
                client = &clients[i];
                break;
            }
        }
    }

    if (!client) 
    {
        deleteRecord(msg);
        return;
    }
 
    int bufStor = 5000;
    char* formatertMdl = (client->mldType == 88) ? recordToXML(msg, &bufStor) : recordToBinary(msg, &bufStor);
    client->mldPlass = bufStor;
    tcp_write(client->sock_fd, formatertMdl, bufStor);


    deleteRecord(msg);
    free(formatertMdl);
}
 

void handleClient(Client* client, fd_set *venteSet, Client clients[]) 
{
    puts("Motokk en melding");

    int restBuffer = client->bufferPlass - client->bufferBrukt;

    if (restBuffer <= 0) {
        fputs("Error: Utilstrekkelig klient buffer størrelse \n", stderr);
        return;
    }

    int lesteBytes = tcp_read(client->sock_fd, client->buffer + client->bufferBrukt, restBuffer);

    if (lesteBytes <= 0) {
        removeClient(client, venteSet);
        fputs("Error: Feil ved lesing av melding \n", stderr);
        return;
    }

    client->bufferBrukt += lesteBytes;

    for(int i=0; i<2; i++) {
        if(client->bufferBrukt > 0 && (i==0 ? client->mldType == 0 : client->mldDest == 0)){
            if(i==0)
                client->mldType = client->buffer[0];
            else
                client->mldDest = client->buffer[0];

            memmove(client->buffer, client->buffer+1, client->bufferBrukt-1);
            client->bufferBrukt--;
        }
    }

    if (client->bufferBrukt > 2) {
        Record* record = NULL;
        int lest = 0;
        do {
            record = (client->mldType == 88 ? XMLtoRecord : BinaryToRecord)
                    (client->buffer, client->bufferBrukt, &lesteBytes);
            if (record) {
                memmove(client->buffer, client->buffer+lesteBytes, client->bufferPlass-lesteBytes);
                client->bufferBrukt -= lesteBytes;
                forwardMessage(record, clients);
                lest = 1;
            } else {
                lest = 0;
            }
        } while (lest);
    }
}

 
 
int main( int argc, char* argv[] )
{
    int port;
    int server_sock;
 
    if( argc != 2 )
    {
        usage( argv[0] );
    }
 
    port = atoi( argv[1] );
 
    server_sock = tcp_create_and_listen( port );
    if( server_sock < 0 ) exit( -1 );
 
   
    fd_set leseSet;
    FD_ZERO(&leseSet);
    FD_SET(server_sock, &leseSet);
 
    fd_set venteSet;
    FD_ZERO(&venteSet);
 
    Client clients[MAKS_CLIENTER];
    for (int i = 0; i < MAKS_CLIENTER; i++) {
        clients[i].sock_fd = -1;
        clients[i].buffer = NULL;
        clients[i].bufferPlass = 0;
        clients[i].bufferBrukt = 0;
        clients[i].mldType = 0;
        clients[i].mldDest = 0;
        clients[i].mldPlass = 0;
        clients[i].aktiv = false;
    }
 
    bool start = true;
 
    do
    {
        fd_set klareSet = leseSet;
        fd_set aktiveSet = venteSet;
        int antKlare = start ? tcp_wait(&klareSet, FD_SETSIZE) : tcp_wait_timeout(&klareSet, FD_SETSIZE, 1);
        start = false;
 
        int antAktive = tcp_wait_timeout(&aktiveSet, FD_SETSIZE, 1);
 
        printf("Klienter som prøver å koble til: %d\n", antKlare);
        printf("Antall active klienter: %d\n", antAktive);
 
        if(antKlare == -1 || antAktive == -1){
            perror("tcp_wait_timeout");
        }
 
        if (antKlare > 0 && FD_ISSET(server_sock, &klareSet)){
            printf("Ny koblig!");
            handleNewClient(server_sock, &venteSet, clients);
        }
 
        if(antAktive > 0) {
            for (int client_nr = 0; client_nr <= MAKS_CLIENTER; client_nr++) {
                if (FD_ISSET(clients[client_nr].sock_fd, &aktiveSet)) {
                    handleClient(&clients[client_nr], &venteSet, clients);
                 }
            }
        }
 
    } while(aktiveClienter(clients, MAKS_CLIENTER) > 0);
 
    tcp_close( server_sock );
 
    return 0;
}

 