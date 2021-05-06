#include <semaphore.h>
#include <string.h>

#include "mailbox.h"
#include "csapp.h"

//linked list node for mailbox queue
typedef struct entrynode ENTRYNODE;
typedef void (MAILBOX_DISCARD_HOOK)(MAILBOX_ENTRY *);
typedef struct entrynode{
	MAILBOX_ENTRY *en;
	ENTRYNODE *next;
}ENTRYNODE;


typedef struct mailbox{
	int refc;
	char *handle;
	volatile int func;//0 = functioning, -1 = defunct
	MAILBOX_DISCARD_HOOK * discardhook;
	sem_t mutex;
	ENTRYNODE *front;
	ENTRYNODE *rear;
}MAILBOX;

//create new entry node for linked list
ENTRYNODE* newentry(MAILBOX_ENTRY *ent){
	ENTRYNODE *newe = (ENTRYNODE *)malloc(sizeof(ENTRYNODE));
	newe->en = ent;
	newe->next = NULL;
	return newe;
}

/*
 * Create a new mailbox for a given handle.  A private copy of the
 * handle is made.  The mailbox is returned with a reference count of 1.
 */
MAILBOX *mb_init(char *handle){
	MAILBOX *newmail = (MAILBOX*)malloc(sizeof(MAILBOX));
	newmail->refc = 1;
	newmail->handle = handle;
	newmail->func = 0;
	newmail->discardhook = NULL;
	sem_init(&(newmail->mutex),0,1);//init mutex to be 1
	newmail->front = newmail->rear =NULL;
	return newmail;
}

/*
 * Increase the reference count on a mailbox.
 * This must be called whenever a pointer to a mailbox is copied,
 * so that the reference count always matches the number of pointers
 * that exist to the mailbox.
 */
void mb_ref(MAILBOX *mb, char *why){
	P(&((*mb).mutex));
	mb->refc++;//increment ref count
	V(&((*mb).mutex));
}

/*
 * Decrease the reference count on a mailbox.
 * This must be called whenever a pointer to a mailbox is discarded,
 * so that the reference count always matches the number of pointers
 * that exist to the mailbox.  When the reference count reaches zero,
 * the mailbox will be finalized.
 */
void mb_unref(MAILBOX *mb, char *why){
	P(&((*mb).mutex));
	mb->refc--;
	V(&((*mb).mutex));
	if(mb->refc == 0){//free all entries and the mailbox
		while(mb->front != NULL){
			ENTRYNODE *temp = mb->front;
			mb->front = mb->front->next;
			free(temp);
		}
		free(mb);
	}
}

/*
 * Shut down this mailbox.
 * The mailbox is set to the "defunct" state.  A defunct mailbox should
 * not be used to send any more messages or notices, but it continues
 * to exist until the last outstanding reference to it has been
 * discarded.  At that point, the mailbox will be finalized, and any
 * entries that remain in it will be discarded.
 */
void mb_shutdown(MAILBOX *mb){
	mb->func = -1;
}
/*
 * Get the handle of the user associated with a mailbox.
 * The handle is set when the mailbox is created and it does not change.
 */
char *mb_get_handle(MAILBOX *mb){
	return mb->handle;
}

/*
 * Add a message to the end of the mailbox queue.
 *   msgid - the message ID
 *   from - the sender's mailbox
 *   body - the body of the message, which can be arbitrary data, or NULL
 *   length - number of bytes of data in the body
 *
 * The message body must have been allocated on the heap,
 * but the caller is relieved of the responsibility of ultimately
 * freeing this storage, as it will become the responsibility of
 * whomever removes this message from the mailbox.
 *
 * Unless the recipient's mailbox ("mb') is the same as the sender's
 * mailbox ("from"), this function must increment the reference count of the
 * senders mailbox, to ensure that this mailbox persists so that it can receive
 * a notification in case the message bounces.
 */
void mb_add_message(MAILBOX *mb, int msgid, MAILBOX *from, void *body, int length){
	P(&((*mb).mutex));//lock reciver mailbox
	P(&((*from).mutex));//lock sender mailbox
	if(from->func == 0){//sender not in defunct state
		MESSAGE mesg = {.msgid = msgid, .from = from, .length = length};
		mesg.body = malloc(sizeof(body));
		if(body != NULL){
			strcpy((char*)mesg.body,(char*)body);
		}
		MAILBOX_ENTRY *entry = (MAILBOX_ENTRY *)malloc(sizeof(MAILBOX_ENTRY));
		entry->type = MESSAGE_ENTRY_TYPE;
		entry->content.message = mesg;
		//create entrynode for the queue
		ENTRYNODE *node = newentry(entry);

		//if queue is empty
		if(mb->rear == NULL){
			mb->front = mb->rear = node;
		}
		//add new entry to the end and change the end
		mb->rear->next = node;
		mb->rear = node;

		if(mb != from){//compare address of mailbox
			mb_ref(from,"increase refcount of sender by mb_add_message");
		}
	}
	else{
		printf("%s\n", "sender mailbox is in defunct state, can not send message");
	}
	V(&((*from).mutex));//unlock sender mailbox
	V(&((*mb).mutex));//unlock receiver mailbox
}

/*
 * Add a notice to the end of the mailbox queue.
 *   ntype - the notice type
 *   msgid - the ID of the message to which the notice pertains
 */
void mb_add_notice(MAILBOX *mb, NOTICE_TYPE ntype, int msgid){
	P(&((*mb).mutex));//lock mailbox
	NOTICE not = {.type = ntype, .msgid = msgid};
	MAILBOX_ENTRY *entry = (MAILBOX_ENTRY *)malloc(sizeof(MAILBOX_ENTRY));
	entry->type = NOTICE_ENTRY_TYPE;
	entry->content.notice = not;
	//create entrynode for the queue
	ENTRYNODE *node = newentry(entry);

	//if queue is empty
	if(mb->rear == NULL){
		mb->front = mb->rear = node;
	}
	//add new entry to the end and change the end
	mb->rear->next = node;
	mb->rear = node;
	V(&((*mb).mutex));//unlock mailbox
}

/*
 * Remove the first entry from the mailbox, blocking until there is
 * one.  The caller assumes the responsibility of freeing the entry
 * and its body, if present.  In addition, if it is a message entry,
 * then the caller must decrease the reference count on the sender's
 * mailbox to account for the destruction of the pointer to it.
 *
 * This function will return NULL in case the mailbox is defunct.
 * The thread servicing the mailbox should use this as an indication
 * that service should be terminated.
 */
MAILBOX_ENTRY *mb_next_entry(MAILBOX *mb){
	if(mb->func == -1){//mailbox is defunct
		return NULL;
	}
	else{
		int change = -1;
		while(change == -1){//while change did not happen, blocking
			if(mb->front != NULL){//entry exist
				MAILBOX_ENTRY *ret = (mb->front)->en;

				(mb->front) = (mb->front)->next;//move front one node up
				if(mb->front == NULL){//if front is null, rear = null
					mb->rear = NULL;
				}
				change = 0;
				return ret;
				//caller free msg body, caller decrement count on
				//sender's mailbox
			}
		}
	}
	return NULL;
}


//Set the discard hook for a mailbox.
void mb_set_discard_hook(MAILBOX *mb, MAILBOX_DISCARD_HOOK *hook){
	mb->discardhook = hook;
}