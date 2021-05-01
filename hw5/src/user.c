#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#include "user.h"
#include "csapp.h"


typedef struct user{
	char *handle;
	int refc;
	sem_t mutex;
}USER;

/*
 * Create a new USER with a specified handle.  A private copy is
 * made of the handle that is passed.  The newly created USER has
 * a reference count of one, corresponding to the reference that is
 * returned from this function.
 *
 * @param handle  The handle of the USER.
 * @return  A reference to the newly created USER, if initialization
 * was successful, otherwise NULL.
 */
USER *user_create(char *handle){
	int hsz = sizeof(handle);
	if(hsz == 0 ){
		return NULL;
	}
	USER *ret = (USER*)malloc(hsz+sizeof(int)+sizeof(sem_t));//allocate space for new user
	ret->handle = handle;
	ret->refc = 1;
	sem_init(&(ret->mutex),0,1);//init mutex to be 1
	return ret;
}

/*
 * Increase the reference count on a user by one.
 *
 * @param user  The USER whose reference count is to be increased.
 * @param why  A string describing the reason why the reference count is
 * being increased.  This is used for debugging printout, to help trace
 * the reference counting.
 * @return  The same USER object that was passed as a parameter.
 */
USER *user_ref(USER *user, char *why){
	P(&((*user).mutex));
	(*user).refc++;//increment ref count
	V(&((*user).mutex));
	return user;
}
/*
 * Decrease the reference count on a USER by one.
 * If after decrementing, the reference count has reached zero, then the
 * USER and its contents are freed.
 *
 * @param user  The USER whose reference count is to be decreased.
 * @param why  A string describing the reason why the reference count is
 * being decreased.  This is used for debugging printout, to help trace
 * the reference counting.
 *
 */
void user_unref(USER *user, char *why){
	P(&((*user).mutex));
	(*user).refc--;//decrement ref count
	V(&((*user).mutex));
	if((*user).refc == 0){//ref count reach 0
		free(user);//free the user
	}
}
/*
 * Get the handle of a user.
 *
 * @param user  The USER that is to be queried.
 * @return the handle of the user.
 */
char *user_get_handle(USER *user){
	return (*user).handle;
}