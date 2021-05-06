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
    printf("%s\n", "enter handler");
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
    int tem = argc;
    while(tem >= 3){
        tem--;
        int ct = 1;
        if(strcmp(*(argv+ct),"-p") == 0){//-p exist
            ct = ct+1;
            port = *(argv+ct);
            tem =0;
        }
        else{
            ct = ct+1;
            tem--;
        }
    }
    int liserver = open_listenfd(port); //return the listenint socket on port.

    struct sigaction sac = {0};
    sac.sa_handler = &sighubhandler;
    if(sigaction(SIGHUP, &sac,NULL) == -1){
        terminate(0);//clean termination
    }

    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid[100];
    int c = 0;
    int *connfd[100];
    while(exitsign == 0){//while sighup is not received
        clientlen = sizeof(struct sockaddr_storage);
        connfd[c] = malloc(sizeof(int));
        *(connfd[c]) = accept(liserver, (SA *)&clientaddr,&clientlen);
        if(*(connfd[c]) >= 0){//on success, return a nonnegative interger
            Pthread_create(&(tid[c]),NULL,chla_client_service,connfd[c]);
            printf("%s%i\n", "thread started at fd",*(connfd[c]));
            c++;
        }
    }
    printf("%s\n", "exit while");
    for(int i=0;i<c;i++){
        Pthread_join(tid[i],NULL);
        free(connfd[i]);
    }
    free(connfd[c]);
    terminate(EXIT_SUCCESS);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    // Shut down all existing client connections.
    // This will trigger the eventual termination of service threads.
    printf("%s\n", "Enter terminate");
    creg_shutdown_all(client_registry);

    // Finalize modules.
    creg_fini(client_registry);
    ureg_fini(user_registry);

    debug("%ld: Server terminating", pthread_self());
    exit(status);
}