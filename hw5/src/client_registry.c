#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#include "client_registry.h"
#include "client.h"
#include "csapp.h"


typedef struct client_registry{
	int count;
	sem_t mutex;
	CLIENT *clientlist[MAX_CLIENTS];//64
}CLIENT_REGISTRY;


/*
 * Initialize a new client registry.
 *
 * @return  the newly initialized client registry, or NULL if initialization
 * fails.
 */
CLIENT_REGISTRY *creg_init(){
	CLIENT_REGISTRY *newcr = (CLIENT_REGISTRY*)malloc(sizeof(CLIENT_REGISTRY));
	newcr->count = 0;
	sem_init(&(newcr->mutex),0,1);//init mutex to be 1
	return newcr;
}

/*
 * Finalize a client registry, freeing all associated resources.
 *
 * @param cr  The client registry to be finalized, which must not
 * be referenced again.
 */
void creg_fini(CLIENT_REGISTRY *cr){
	P(&((*cr).mutex));
	for(int i=0;i<(*cr).count;i++){
		if((*cr).clientlist[i] != NULL){

			free((*cr).clientlist[i]);//free all client
		}
	}
	free(cr);//free the client registry
}

/*
 * Register a client connection.
 * If successful, returns a reference to the the newly registered CLIENT,
 * otherwise NULL.  The returned CLIENT has a reference count of two:
 * one for the pointer retained by the registry and one for the pointer
 * returned to the caller.
 *
 * @param cr  The client registry.
 * @param fd  The file descriptor of the connection to the client.
 * @return a reference to the newly registered CLIENT, if registration
 * is successful, otherwise NULL.
 */
CLIENT *creg_register(CLIENT_REGISTRY *cr, int fd){
	CLIENT *newcl = client_create(cr,fd);//refc =1
	//P(&((*cr).mutex));
	if((*cr).count < 64){
		for(int i=0;i<MAX_CLIENTS;i++){
			if((*cr).clientlist[i] == NULL){//search for first open place
				(*cr).clientlist[i] = newcl;
				(*cr).count++;
				i = MAX_CLIENTS;//exit loop
			}
		}
	}
	else{
		return NULL;//max client
	}
	client_ref(newcl,"one pointer retain by registry, one is returned");//increase count, refc=2
	//V(&((*cr).mutex));//unlock
	return newcl;
}

/*
 * Unregister a CLIENT, removing it from the registry.
 * The file descriptor associated with the client connection is *not*
 * closed by this function; that remains the responsibility of whomever
 * originally obtained it.
 * It is an error if the CLIENT is not among the currently registered
 * clients at the time this function is called.
 * The reference count of the CLIENT is decreased by one to account
 * for the pointer being discarded.  If this results in the number of
 * connected clients reaching zero, then any threads waiting in
 * creg_shutdown_all() are allowed to proceed.
 *
 * @param cr  The client registry.
 * @param client  The CLIENT to be unregistered.
 * @return 0  if unregistration succeeds, otherwise -1.
 */
int creg_unregister(CLIENT_REGISTRY *cr, CLIENT *client){
	P(&((*cr).mutex));
	int ret=-1;
	for(int i=0;i<(*cr).count;i++){
		if((*cr).clientlist[i] == client){//client found on given client registry
			(*cr).clientlist[i] = NULL;//remove pointer
			client_unref(client,"pointer from client_registry is deleted");
			ret =0;
			(*cr).count--;
		}
	}
	V(&((*cr).mutex));
	return ret;
	//fd is not close here
}

/*
 * Return a list of all currently connected clients.  The result is
 * returned as a malloc'ed array of CLIENT pointers, with a NULL
 * pointer marking the end of the array.  Each CLIENT in the array
 * has had its reference count incremented by one, to account for
 * the pointer stored in the array.  It is the caller's responsibility
 * to decrement the reference count of each of the entries and to
 * free the array when it is no longer needed.
 *
 * @param cr  The registry from which the set of clients is to be
 * obtained.
 * @return the list of clients as a NULL-terminated array.
 */
CLIENT **creg_all_clients(CLIENT_REGISTRY *cr){
	P(&((*cr).mutex));
	CLIENT **retlist = malloc(sizeof(CLIENT*)*(((*cr).count)+1));//malloc space for array
	retlist[(*cr).count] = NULL;//null pointer
	for(int i=0;i<(*cr).count;i++){
		retlist[i] = (*cr).clientlist[i];
		client_ref((*cr).clientlist[i],"pointer added to the returned array");
	}
	V(&((*cr).mutex));
	return retlist;
	//caller have to free the array, and decrease the reference count
}

/*
 * Shut down (using shutdown(2)) all the sockets for connections
 * to currently registered clients.  The calling thread will block
 * in this function until all the server threads have recognized
 * the EOF on their connections caused by the socket shutdown and
 * have unregistered the corresponding clients.  This function
 * returns only when the number of registered clients has reached zero.
 * This function may be called more than once, but not concurrently.
 * Calling this function does not finalize the client registry.
 *
 * @param cr  The client registry.
 */
void creg_shutdown_all(CLIENT_REGISTRY *cr){
	P(&((*cr).mutex));
	for(int i=0;i<(*cr).count;i++){
		if((*cr).clientlist[i] != NULL){
			int fd = client_get_fd((*cr).clientlist[i]);
			shutdown(fd,SHUT_RDWR);//shut down,disable reception and transmission
			(*cr).count--;
		}
	}
	V(&((*cr).mutex));
}