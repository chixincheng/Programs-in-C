



#include "user_registry.h"





/*
 * Initialize a new user registry.
 *
 * @return the newly initialized USER_REGISTRY, or NULL if initialization
 * fails.
 */
USER_REGISTRY *ureg_init(void){

}

/*
 * Finalize a user registry, freeing all associated resources.
 *
 * @param cr  The USER_REGISTRY to be finalized, which must not
 * be referenced again.
 */
void ureg_fini(USER_REGISTRY *ureg){

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

}