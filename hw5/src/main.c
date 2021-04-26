#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "debug.h"
#include "server.h"
#include "globals.h"
#include "csapp.h"

static void terminate(int);
void *thread(void *conn);

volatile int exitsign = 0;
void sighubhandler(){
    exitsign = -1;//detect the sign, exit the loop
}

/*
 * "Charla" chat server.
 *
 * Usage: charla <port>
 */
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.

    // Perform required initializations of the client_registry and
    // player_registry.
    user_registry = ureg_init();
    client_registry = creg_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function charla_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.
    char *port;
    if(argc >= 3){
        if(strcmp(*(argv+1),"-p") == 0){//-p exist
            port = *(argv+2);
        }
    }
    int liserver = open_listenfd(port); //return the listenint socket on port.

    struct sigaction sac;
    sac.sa_handler = &sighubhandler;
    if(sigaction(SIGHUP, &sac,NULL) == -1){
        terminate(0);//clean termination
    }

    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    while(exitsign == 0){//while sighup is not received
        clientlen = sizeof(struct sockaddr_storage);
        int *connfd = malloc(sizeof(int));
        *connfd = Accept(liserver, (SA *)&clientaddr,&clientlen);
        if(*connfd >= 0){//on success, return a nonnegative interger
            Pthread_create(&tid,NULL,thread,connfd);
        }
    }
    Pthread_exit(NULL);//wait for all thread to finish and exit

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    // Shut down all existing client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    // Finalize modules.
    creg_fini(client_registry);
    ureg_fini(user_registry);

    debug("%ld: Server terminating", pthread_self());
    exit(status);
}
//thread will run this
void *thread (void *conn){
    int confd = *((int*)conn);
    chla_client_service(conn);
    free(conn);
    Close(confd);
    return NULL;
}