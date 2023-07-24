
#include "connection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>

#include "connection.h"


int tcp_connect( char* hostname, int port )
{
    int fd, wc;
    struct sockaddr_in servaddr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd==-1){
         fprintf(stderr, "Error: socket creation failed: %s\n", strerror(errno));
         return -1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    wc = inet_pton(AF_INET, hostname, &servaddr.sin_addr.s_addr);
    if (wc==-1){
         fprintf(stderr, "Error: inet_pton failed: %s\n", strerror(errno));
         close(fd);
         return -1;
    }
    if (! wc){
        fprintf(stderr,"Error: inet_pton adress format\n");
        close(fd);
        return -1;
    }

    wc= connect(fd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in));
    if (wc==-1){
        fprintf(stderr, "Error: connect failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

int tcp_read( int sock, char* buffer, int n )
{
    int bytes_lest = read(sock, buffer, n);

    if (bytes_lest ==-1) {
        fprintf(stderr, "Error: read failed: %s\n", strerror(errno));
        return bytes_lest;
    } 

    return bytes_lest;

}

int tcp_write( int sock, char* buffer, int bytes )
{
    int bytes_skrvet = write(sock, buffer, bytes);

    if (bytes_skrvet ==-1) {
        fprintf(stderr, "Error: write failed: %s\n", strerror(errno));
        return -1;
    }

    return bytes_skrvet;
}

int tcp_write_loop( int sock, char* buffer, int bytes )
{
    int antall_bytes_skrivet = 0;
    int resterende_bytes;

    while (antall_bytes_skrivet < bytes) {
        resterende_bytes = bytes - antall_bytes_skrivet;
        int bytes_skrvet = tcp_write(sock, buffer + antall_bytes_skrivet, resterende_bytes);

        if (bytes_skrvet < 0) {
            return bytes_skrvet;
        }

        antall_bytes_skrivet += bytes_skrvet;
    }
    return antall_bytes_skrivet;
}

void tcp_close( int sock )
{
    close(sock);
}

int tcp_create_and_listen( int port )
{
    int motefd, rc;
    struct sockaddr_in servaddr;

    motefd = socket(AF_INET, SOCK_STREAM, 0);
    if (motefd==-1){
         fprintf(stderr, "Error: socket creation failed: %s\n", strerror(errno));
         return -1;
    }
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    rc=bind(motefd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in));
    if (rc == -1) {
        fprintf(stderr, "Error: could not bind socket: %s\n", strerror(errno));
        close(motefd);
        return -1;
    }

    rc=listen(motefd, 5);
    if (rc == -1) {
        fprintf(stderr, "Error: could not listen on socket: %s\n", strerror(errno));
        close(motefd);
        return -1;
    }

    return motefd;
}

int tcp_accept( int server_sock )
{
    int msg_fd;
    
    msg_fd = accept(server_sock, NULL, NULL);
    
    if (msg_fd ==-1) {
        fprintf(stderr, "Error: accept failed: %s\n", strerror(errno));
        return -1;
    }

    return msg_fd;
}

int tcp_wait( fd_set* waiting_set, int wait_end )
{
    int rc;
    int max_fd = wait_end + 1;
    rc = select(max_fd, waiting_set, NULL, NULL, NULL);

    if (rc == -1) {
        perror("Error: select failed");
        return -1;
    }

    return rc;
    
}

int tcp_wait_timeout( fd_set* waiting_set, int wait_end, int seconds )
{
    int rc;
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;

    rc = select(wait_end, waiting_set, NULL, NULL, &tv);
    if (rc == -1) {
        perror("Error: select failed");
        return -1;
    }
    return rc;
}

