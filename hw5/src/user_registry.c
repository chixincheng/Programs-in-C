#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>


#include "globals.h"
#include "user.h"
#include "csapp.h"


/*
 * The USER_REGISTRY type is a structure type that defines the state
 * of a user registry.  You will have to give a complete structure
 * definition in user_registry.c. The precise contents are up to
 * you.  Be sure that all the operations that might be called
 * concurrently are thread-safe.
 */
typedef struct user_registry{
	int count;
	sem_t mutex;
	USER *userlist[100];
}USER_REGISTRY;


/*
 * Initialize a new user registry.
 *
 * @return the newly initialized USER_REGISTRY, or NULL if initialization
 * fails.
 */
USER_REGISTRY *ureg_init(void){
	USER_REGISTRY *newur = malloc(sizeof(USER_REGISTRY));
	USER_REGISTRY nur = {.count=0};
	sem_init(&nur.mutex,0,1);//init mutex to be 1
	*newur = nur;
	return newur;
}

/*
 * Finalize a user registry, freeing all associated resources.
 *
 * @param cr  The USER_REGISTRY to be finalized, which must not
 * be referenced again.
 */
void ureg_fini(USER_REGISTRY *ureg){
	P(&((*ureg).mutex));
	for(int i=0;i<100;i++){
		if((*ureg).userlist[i] != NULL){
			free((*ureg).userlist[i]);//free all user
		}
	}
	free(ureg);//free the user registry
	//V(&((*ureg).mutex));
}

/*
 * Register a user with a specified handle.  If there is already
 * a user registered under that handle, then the existing registered
 * user is returned, otherwise a new user is created.
 * If an existing user is returned, then its reference count is incremented
 * to reflect the new reference that is being exported from the registry.
 * If a new user is created, then the returned user has reference count
 * equal to two; one corresponding to the reference that is held by
 * the user registry and the other to the pointer that is returned.
 *
 * @param ureg  The user registry into which to register the user.
 * @param handle  The user's handle, which is copied by this function.
 * @return A pointer to a USER object, in case of success, otherwise NULL.
 *
 */
USER *ureg_register(USER_REGISTRY *ureg, char *handle){
	P(&((*ureg).mutex));//lock the array while searching
	for(int i=0;i<100;i++){
		if((*ureg).userlist[i] != NULL){//if a user exist
			if(strcmp(user_get_handle((*ureg).userlist[i]),handle) == 0){//handle exist
				user_ref((*ureg).userlist[i],"user registry held one pointer");//increase ref count;
				return (*ureg).userlist[i];
			}
		}
	}
	int ct = (*ureg).count;
	USER *ret;
	if(ct < 100){
		for(int i=0;i<100;i++){
			if((*ureg).userlist[i] == NULL){
				ret = user_create(handle);//this use malloc, free in ureg_fini
				(*ureg).userlist[i] = ret;
				(*ureg).count++;
				user_ref(ret,"user registry held one pointer");//increase ref count;
			}
		}
	}
	else{
		return NULL;//full register, cap of 100
	}
	V(&((*ureg).mutex));//unlock mutex
	return ret;
}

/*
 * Unregister a handle, removing it and the associated USER object from
 * the registry.  The reference count on the USER object is decreased by one.
 * If the handle was not previously registered, nothing is done by this
 * function.
 *
 * @param ureg  The user registry from which to unregister the user.
 * @param handle  The user's handle.
 */
void ureg_unregister(USER_REGISTRY *ureg, char *handle){
	P(&((*ureg).mutex));//lock the array while searching
	for(int i=0;i<100;i++){
		if((*ureg).userlist[i] != NULL){//if a user exist
			if(strcmp(user_get_handle((*ureg).userlist[i]),handle) == 0){//handle exist
				user_unref((*ureg).userlist[i],"user registry's pointer is deleted");//increase ref count;
				(*ureg).userlist[i] = NULL;//remove the user with the associate handle
				(*ureg).count--;
			}
		}
	}
	V(&((*ureg).mutex));//unlock mutex
}