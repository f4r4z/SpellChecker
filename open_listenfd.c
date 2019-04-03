/*
 * Faraz Heravi
 * open_listenfd.c
 */

#include "server.h"
// Used from the Computer Systems textbook

int open_listenfd(int port)
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        return -1;
    }

    /* Eliminates "Address already in use" error from bind */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval , sizeof(int)) < 0){
        return -1;
    }

    // Reset the serveraddr struct, setting all of it's bytes to zero.
    // Some properties are then set for the struct, you don't
    // need to worry about these.
    // bind() is then called, associating the port number with the
    // socket descriptor.
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0){
        return -1;
    }

    // Prepare the socket to allow accept() calls. The value 20 is
    // the backlog, this is the maximum number of connections that will be placed
    // on queue until accept() is called again.
    if (listen(listenfd, 20) < 0){
        return -1;
    }

    return listenfd;
}
 