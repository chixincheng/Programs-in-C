#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#include "client_registry.h"
#include "client.h"
#include "csapp.h"
#include "globals.h"
#include "protocol.h"

typedef struct client{
	int fd;
	int refc;
	int log;//0 means logged in, -1 means logged out, start from -1
	USER *user;
	MAILBOX *mail;
	sem_t mutex;
}CLIENT;


/*
 * Create a new CLIENT object with a specified file descriptor with which
 * to communicate with the client.  The returned CLIENT has a reference
 * count of one and is in the logged-out state.
 *
 * @param creg  The client registry in which to create the client.
 * @param fd  File descriptor of a socket to be used for communicating
 * with the client.
 * @return  The newly created CLIENT object, if creation is successful,
 * otherwise NULL.
 */
CLIENT *client_create(CLIENT_REGISTRY *creg, int fd){
	CLIENT *newcl = malloc(sizeof(CLIENT));
	CLIENT temcl = {.fd = fd,.refc = 1,.log=-1};
	sem_init(&temcl.mutex,0,1);//init mutex to be 1
	*newcl = temcl;
	return newcl;
}

/*
 * Increase the reference count on a CLIENT by one.
 *
 * @param client  The CLIENT whose reference count is to be increased.
 * @param why  A string describing the reason why the reference count is
 * being increased.  This is used for debugging printout, to help trace
 * the reference counting.
 * @return  The same CLIENT that was passed as a parameter.
 */
CLIENT *client_ref(CLIENT *client, char *why){
	P(&((*client).mutex));
	(*client).refc++;//increment ref count
	V(&((*client).mutex));
	return client;
}

/*
 * Decrease the reference count on a CLIENT by one.  If after
 * decrementing, the reference count has reached zero, then the CLIENT
 * and its contents are freed.
 *
 * @param client  The CLIENT whose reference count is to be decreased.
 * @param why  A string describing the reason why the reference count is
 * being decreased.  This is used for debugging printout, to help trace
 * the reference counting.
 */
void client_unref(CLIENT *client, char *why){
	P(&((*client).mutex));
	(*client).refc--;//decrement ref count
	V(&((*client).mutex));
	if((*client).refc == 0){//ref count reach 0
		free(client);//free the client
	}
}

/*
 * Log this CLIENT in under a specified handle.
 * The handle is registered with the user registry, creating a new
 * USER object corresponding to the handle if one did not already
 * exist.  A MAILBOX is also created and retained by the CLIENT.
 * The login fails if the CLIENT is already logged in or there is already
 * some other CLIENT that is logged under the specified handle.
 * Otherwise, the login is successful and the CLIENT is marked as "logged in".
 *
 * @param CLIENT  The CLIENT that is to be logged in.
 * @param handle  The handle under which the CLIENT is to be logged in.
 * @return 0 if the login operation is successful, otherwise -1.
 */
int client_login(CLIENT *client, char *handle){
	if((*client).log == 0){
		return -1;//already logged in
	}
	CLIENT ** connlist = creg_all_clients(client_registry);//return all connected client
	int cont =0;
	while(connlist[cont] != NULL){
		USER *u = (*connlist[cont]).user;//get user from client
		char *h = user_get_handle(u);//get handle from user
		if(strcmp(handle,h) == 0){//if handles exist in connected client
			return -1;
		}
		client_unref(connlist[cont],"pointer from allclient is deleted");
		connlist[cont] = NULL;//delete pointer
		cont++;
	}
	free(connlist);//free the malloc array
	USER *newu = ureg_register(user_registry,handle);
	(*client).user = newu;//add to client
	(*client).mail = mb_init(handle);//create the mailbox
	return 0;//success
}

/*
 * Log out this CLIENT.  If the client was not logged in, then it is
 * an error.  The references to the USER and MAILBOX that were saved
 * when the CLIENT was logged in are discarded.
 *
 * @param client  The CLIENT that is to be logged out.
 * @return 0 if the client was logged in and has been successfully
 * logged out, otherwise -1.
 */
int client_logout(CLIENT *client){
	if((*client).log == -1){//not logged in
		return -1;
	}
	user_unref((*client).user,"discard user from client");

	//do we need to check for refc = 0 and free?

	(*client).user = NULL;//discard user
	mb_unref((*client).mail,"discard mailbox from client");
	mb_shutdown((*client).mail);


	(*client).mail = NULL;//discard mailbox
	(*client).log = -1;//logged out
	return 0;
}

/*
 * Get the USER object for the specified logged-in CLIENT.
 *
 * @param client  The CLIENT from which to get the USER.
 * @param no_ref  If nonzero, then the reference count on the returned
 * USER is *not* incremented.  This is a convenience feature that
 * should only be used if the caller knows that the CLIENT cannot
 * be logged out until the USER object is no longer needed.
 * Otherwise, the caller should pass a zero value for no_ref and
 * accept the responsibility for decrementing the reference count
 * when finished with the USER object.
 */
USER *client_get_user(CLIENT *client, int no_ref){
	if(no_ref != 0){//ref count do not increase
		return (*client).user;
	}
	else{//ref count increase, caller need to decrease
		user_ref((*client).user,"increase count for returning pointer to user");
		return (*client).user;
	}
}

/*
 * Get the MAILBOX for the specified logged-in CLIENT.
 *
 * @param client  The CLIENT from which to get the MAILBOX.
 * @param no_ref  If nonzero, then the reference count on the returned
 * MAILBOX is *not* incremented.  This is a convenience feature that
 * should only be used if the caller knows that the CLIENT cannot
 * be logged out until the MAILBOX object is no longer needed.
 * Otherwise, the caller should pass a zero value for no_ref and
 * accept the responsibility for decrementing the reference count
 * when finished with the MAILBOX object.
 * @return  The MAILBOX that the CLIENT is currently using,
 * otherwise NULL if the client is not currently logged in.
 */
MAILBOX *client_get_mailbox(CLIENT *client, int no_ref){
	if(no_ref != 0){//ref count do not increase
		return (*client).mail;
	}
	else{//ref count increase, caller need to decrease
		mb_ref((*client).mail,"increase count for returning pointer to mailbox");
		return (*client).mail;
	}
}

/*
 * Get the file descriptor for the network connection associated with
 * this CLIENT.  The file descriptor is set at the time the CLIENT
 * is created and does not change.
 *
 * @param client  The CLIENT for which the file descriptor is to be
 * obtained.
 * @return the file descriptor.
 */
int client_get_fd(CLIENT *client){
	return (*client).fd;
}

/*
 * Send a packet to a client.  Exclusive access to the network connection
 * is obtained for the duration of this operation, to prevent concurrent
 * invocations from corrupting each other's transmissions.  To prevent
 * such interference, only this function should be used to send packets to
 * the client, rather than the lower-level proto_send_packet() function.
 *
 * @param client  The CLIENT who should be sent the packet.
 * @param pkt  The header of the packet to be sent.
 * @param data  Data payload to be sent, or NULL if none.
 * @return 0 if transmission succeeds, -1 otherwise.
 */
int client_send_packet(CLIENT *client, CHLA_PACKET_HEADER *pkt, void *data){
	int fd =(*client).fd;
	int ret = proto_send_packet(fd,pkt,data);
	return ret;
}

/*
 * Send an ACK packet to a client.  This is a convenience function that
 * streamlines a common case.
 *
 * @param client  The CLIENT who should be sent the packet.
 * @param msgid  Message ID (in host byte order) to which the ACK pertains.
 * @param data  Pointer to the optional data payload for this packet,
 * or NULL if there is to be no payload.
 * @param datalen  Length of the data payload, or 0 if there is none.
 * @return 0 if transmission succeeds, -1 otherwise.
 */
int client_send_ack(CLIENT *client, uint32_t msgid, void *data, size_t datalen){
	int fd =(*client).fd;
	CHLA_PACKET_TYPE pkty= CHLA_ACK_PKT;
	CHLA_PACKET_HEADER *head = malloc(sizeof(CHLA_PACKET_HEADER));
	CHLA_PACKET_HEADER temp ={.type=pkty,.payload_length=datalen,.msgid=msgid};
	*head = temp;
	int ret = proto_send_packet(fd,head,data);
	free(head);
	return ret;
}

/*
 * Send an NACK packet to a client.  This is a convenience function that
 * streamlines a common case.
 *
 * @param client  The CLIENT to be sent the packet.
 * @param msgid  Message ID to which the ACK pertains.
 * @return 0 if transmission succeeds, -1 otherwise.
 */
int client_send_nack(CLIENT *client, uint32_t msgid){
	int fd =(*client).fd;
	CHLA_PACKET_TYPE pkty= CHLA_NACK_PKT;
	CHLA_PACKET_HEADER *head = malloc(sizeof(CHLA_PACKET_HEADER));
	CHLA_PACKET_HEADER temp ={.type=pkty,.payload_length=0,.msgid=msgid};
	*head = temp;
	int ret = proto_send_packet(fd,head,NULL);
	free(head);
	return ret;
}