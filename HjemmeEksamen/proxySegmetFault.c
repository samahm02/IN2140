/*
 * This is the main program for the proxy, which receives connections for sending and receiving clients
 * both in binary and XML format. Many clients can be connected at the same time. The proxy implements
 * an event loop.
 *
 * *** YOU MUST IMPLEMENT THESE FUNCTIONS ***
 *
 * The parameters and return values of the existing functions must not be changed.
 * You can add function, definition etc. as required.
 */
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

#define MAX_CLIENTS 15

/* This struct should contain the information that you want
 * keep for one connected client.
 */
struct Client
{
int sock_fd; // socket file descriptor for this client
char* buffer; // buffer for receiving partial messages
size_t buffer_size; // size of the buffer
size_t buffer_used; // number of bytes used in the buffer
char client_format; // type of message being received (binary or XML)
char client_id; // destination client for this message
};

typedef struct Client Client;
/* 
int check_error(int res, char *msg){
    if (res == -1) {
        // perror bruker stderr for å skrive ut enkle feilmeldinger ved bruk av errno
        perror(msg);
    
        return -1;
    }
    return EXIT_SUCCESS;
}
 */
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

/*
 * This function is called when a new connection is noticed on the server
 * socket.
 * The proxy accepts a new connection and creates the relevant data structures.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
void handleNewClient( int server_sock, fd_set* waiting_fds, Client clients[])
{
    int new_conenction;
    new_conenction = tcp_accept(server_sock);
    check_error(new_conenction, "handleNewClientAccept");
    FD_SET(new_conenction, waiting_fds);
    bool added = false;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if ( (clients[i].sock_fd == -1) & (!added)){
            clients[i].sock_fd = new_conenction;
            clients[i].buffer_size = 1024;
            clients[i].buffer_used = 0;
            clients[i].buffer = (char*) malloc(clients[i].buffer_size);
            added = true;
        }
    }

}

/*
 * This function is called when a connection is broken by one of the connecting
 * clients. Data structures are clean up and resources that are no longer needed
 * are released.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
void removeClient( Client* client, fd_set *waiting_fds)
{
    FD_CLR(client->sock_fd, waiting_fds);
    tcp_close(client->sock_fd);
    client->sock_fd = -1; // Initialize the socket file descriptor to -1
    client->buffer = NULL; // Initialize the buffer pointer to NULL
    client->buffer_size = 0; // Initialize the buffer size to 0
    client->buffer_used = 0; // Initialize the buffer used to 0
    client->client_format = 0; // Initialize the message type to 0
    client->client_id = 0; // Initialize the message destination to 0
}

/*
 * This function is called when the proxy received enough data from a sending
 * client to create a Record. The 'dest' field of the Record determines the
 * client to which the proxy should send this Record.
 *
 * If no such client is connected to the proxy, the Record is discarded without
 * error. Resources are released as appropriate.
 *
 * If such a client is connected, this functions find the correct socket for
 * sending to that client, and determines if the Record must be converted to
 * XML format or to binary format for sendig to that client.
 *
 * It does then send the converted messages.
 * Finally, this function deletes the Record before returning.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
void forwardMessage( Record* msg )
{
    /*
    void forwardMessage( Record* msg ) 
{
    // Find the client with the correct destination
    int dest = msg->dest;
    Client* client = NULL;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sock_fd != -1 && clients[i].msg_dest == dest) {
            client = &clients[i];
            break;
        }
    }
    if (!client) {
        // No client with the given destination found, discard the message
        free(msg);
        return;
    }
    // Convert the Record to the format required by the destination client
    void* data;
    int data_size;
    if (client->msg_type == XML) {
        data = recordToXml(msg, &data_size);
    } else {
        data = recordToBinary(msg, &data_size);
    }
    // Send the converted message to the destination client
    int bytes_written = tcp_write(client->sock_fd, data, data_size);
    if (bytes_written == -1) {
        // Error occurred, remove the client
        removeClient(client);
    }
    free(data);
    free(msg);

    
}
*/
}

/*
 * This function is called whenever activity is noticed on a connected socket,
 * and that socket is associated with a client. This can be sending client
 * or a receiving client.
 *
 * The calling function finds the Client structure for the socket where acticity
 * has occurred and calls this function.
 *
 * If this function receives data that completes a record, it creates an internal
 * Record data structure on the heap and calls forwardMessage() with this Record.
 *
 * If this function notices that a client has disconnected, it calls removeClient()
 * to release the resources associated with it.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
void handleClient( Client* client, fd_set *waiting_fds )
{
    printf("Activity on FD registered\n");
    
    //makes a buffer for incoming string
    int buffer_len = 1024;
    char buffer[buffer_len];


    //uses tcp_read to read up to 1024 bytes into buffer
    int num_bytes = tcp_read(client->sock_fd, buffer, sizeof(buffer));
    printf("read bytes: %d\n", num_bytes);

    
    //checking to se if client has disconnected 
    if (num_bytes == 0) {
            removeClient(client, waiting_fds);
            return;
    }

    memcpy(client->buffer, buffer, num_bytes);
    client->buffer_used = num_bytes; 

    client->client_format = buffer[0];
    client->client_id = buffer[1];

    memmove(buffer, buffer+2, buffer_len);

   

    //checks to see if it is a reciver
    if (num_bytes == 2){
        //code for reciver
        
    } else {
         //printf("first byte in buffer %c\n", buffer[0]);
        //checks for X in start of buffer indicating XML sender
        if (client->client_format == 88) {
            int bytes_read = 0;
            bool read = true;
            while (read){
                Record* record = XMLtoRecord(buffer, buffer_len, &bytes_read);
                read = bytes_read;
            }
            printf("end of while loop\n");
        }
        
        else {
            //code for binary sender
        }


    }

   


    //client->buffer_size = buffer_len;
    //client->buffer_used = num_bytes; 
    //client->buffer = buffer;

    /*
    //adds null byte (Probably not necessary )
    buffer[num_bytes-1] = 0;

    //adds read string to client buffer
    strcat(client->buffer, buffer);
    
    //checks for endtag in xml to see if there is a whole record to prosses
    char* end_of_msg = strstr(client->buffer, "/record");

    //loops buffer until no more /record are present
    while (end_of_msg != NULL) {
        int bytes_read = 0;

        //uses help method to make record
        Record* record = XMLtoRecord(buffer, num_bytes, &bytes_read);
        if (record != NULL) {
            forwardMessage(record);

            //removes the prosessed record from buffer 
            memmove(buffer, buffer+bytes_read, buffer_len-bytes_read+1);
            end_of_msg = strstr(client->buffer, "/record");
        }
        end_of_msg = NULL;
        //end_of_msg = strstr(client->buffer, "/record");
    }
  
  //  
  */

}


//To test
// ./proxy 2021
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


    //Creates set for server_sock
    fd_set read_fds;
    //init empty fd_set
    FD_ZERO(&read_fds);
    //add element to list
    FD_SET(server_sock, &read_fds);


    //Creates set for waiting_fds
    fd_set waiting_fds;
    FD_ZERO(&waiting_fds);

    //declare an array of clients
    Client clients[MAX_CLIENTS];

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].sock_fd = -1; // Initialize the socket file descriptor to -1
        clients[i].buffer = NULL; // Initialize the buffer pointer to NULL
        clients[i].buffer_size = 0; // Initialize the buffer size to 0
        clients[i].buffer_used = 0; // Initialize the buffer used to 0
        clients[i].client_format = 0; // Initialize the message type to 0
        clients[i].client_id = 0; // Initialize the message destination to 0
    }

    /* add your initialization code */
    
    /*
     * The following part is the event loop of the proxy. It waits for new connections,
     * new data arriving on existing connection, and events that indicate that a client
     * has disconnected.
     *
     * This function uses handleNewClient() when activity is seen on the server socket
     * and handleClient() when activity is seen on the socket of an existing connection.
     *
     * The loops ends when no clients are connected any more.
     */
    do
    {
        //makes a set for ready connectios
        fd_set ready_fds = read_fds;

        //makes a set for active fds
        fd_set active_fds = waiting_fds;

        //checks for ready fds
        int num_ready = tcp_wait_timeout(&ready_fds, FD_SETSIZE, 1);

        //checks for active fds
        int num_active = tcp_wait_timeout(&active_fds, FD_SETSIZE, 1);

        printf("Number of clients trying to connect: %d\n", num_ready);
        printf("Number of active clients: %d\n", num_active);

        //error checking tcp_wait_timeout
        check_error( num_ready, "tcp_wait_timeout");
        check_error( num_active, "tcp_wait_timeout");

        //check if there is active clients 
        if (num_ready > 0){
            printf("New Connection Found!");
            if (FD_ISSET(server_sock, &ready_fds)){
                handleNewClient(server_sock, &waiting_fds, clients);
        }
        }

        if(num_active > 0) {
            for (int client_nr = 0; client_nr <= MAX_CLIENTS; client_nr++) {
                if (FD_ISSET(clients[client_nr].sock_fd, &active_fds)) {
                    // Handle activity on fd
                    handleClient(&clients[client_nr], &waiting_fds);
                 }
            }
            printf("End of for loop");
        }

       

       
        /* fill in your code */
    }
    while( 1 /* fill in your termination condition */ );

    /* add your cleanup code */

    tcp_close( server_sock );

    return 0;
}

