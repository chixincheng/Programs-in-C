#include <string.h>


#include "protocol.h"
#include "globals.h"
#include "server.h"
#include "client_registry.h"
#include "csapp.h"
#include "helper.h"


void *mailboxservice(void *arg);

void *chla_client_service(void *arg){
	int fd = *((int*)arg);
	CHLA_PACKET_HEADER *hdr = (CHLA_PACKET_HEADER*)malloc(sizeof(CHLA_PACKET_HEADER));
	hdr->type = -1;
	void *payload = NULL;
	CLIENT *cl = creg_register(client_registry,fd);//refc=2
	MAILBOX *from =NULL;
	int run = 0;
	int mailstart = -1;
	//
	pthread_t tid;
	//
	while(run != -1){
		run = proto_recv_packet(fd,hdr,&payload);
		CHLA_PACKET_TYPE type = hdr->type;
		char *handle = payload;
		uint32_t msgid = hdr->msgid;//network byte order
		uint32_t payloadlen = hdr->payload_length;//network byte order
		if(type == CHLA_LOGIN_PKT){//login request
			int log = client_login(cl,handle);//mb refc=1
			if(log != -1){
				client_send_ack(cl,ntohl(msgid),payload,ntohl(payloadlen));//login success
				mb_add_notice(client_get_mailbox(cl,1),NO_NOTICE_TYPE,ntohl(msgid));
				//new mailbox service thread start here after client login
				Pthread_create(&tid,NULL,mailboxservice,client_get_mailbox(cl,1));
				mailstart = 0;
			}
			else{
				client_send_nack(cl,ntohl(msgid));//login fail
			}
		}
		else if(type == CHLA_LOGOUT_PKT){//logout request
			int log = client_logout(cl);
			if(log != 0){
				client_send_nack(cl,ntohl(msgid));//logout fail
			}
			else{
				client_send_ack(cl,ntohl(msgid),payload,ntohl(payloadlen));//logout success
			}
		}
		else if(type == CHLA_USERS_PKT){//user request
			CLIENT ** connlist = creg_all_clients(client_registry);//return all connected client
			int cont =0;
			char *payl;
			int len =0;
			while(connlist[cont] != NULL){
				USER *u = client_get_user(connlist[cont],0);//get user from client
				if(u != NULL){
					char *h = user_get_handle(u);//get handle from user
					int ct=0;
					while(*(h+ct) != '\0'){
						len++;//get length of payload
						ct++;
					}
					payl = realloc(payl,len);//realloc payload size
					strcat(payl,h);//append user into payload
					printf("%s\n", h);
					user_unref(u,"pointer deleted");
				}
				client_unref(connlist[cont],"pointer from allclient is deleted");
				connlist[cont] = NULL;//delete pointer
				cont++;
			}
			free(connlist);
			client_send_ack(cl,ntohl(msgid),payl,len);
		}
		else if(type == CHLA_SEND_PKT){//send request
			int fail = -5;
			char *temp = (char*)malloc(ntohl(payloadlen));
			int ct = 0;
			while(*(handle+ct) != '\n'){
				*(temp+ct) = *(handle+ct);
				ct++;
			}
			*(temp+ct) = '\n';

			CLIENT ** connlist = creg_all_clients(client_registry);//return all connected client
			int cont =0;
			while(connlist[cont] != NULL){
				USER *u = client_get_user(connlist[cont],0);//get user from client
				if(u != NULL){
					char *h = user_get_handle(u);//get handle from user
					if(strcmp(h,temp) == 0){
						fail = 0;
						CHLA_PACKET_HEADER *head = (CHLA_PACKET_HEADER*)malloc(sizeof(CHLA_PACKET_HEADER));
						head->type = CHLA_MESG_PKT;
						head->payload_length = payloadlen;//already in network byte order
						head->msgid = msgid;//already in network byte order
						//set from mailbox
						from = client_get_mailbox(cl,0);
						//send msg packet to other client
						client_send_ack(cl,ntohl(msgid),payload,0);//send ack
						mb_add_notice(client_get_mailbox(cl,1),NO_NOTICE_TYPE,ntohl(msgid));
						USER *usen = client_get_user(cl,1);//ref will not increase
						char *sender = user_get_handle(usen);
						int c = 0;
						while(*((char*)payload+c) != '\n'){//skip handle
							*(temp+c) = *(sender+c);
							c++;
						}
						*(temp+c) = '\n';
						c++;
						while(*((char*)payload+c) != '\0'){//copy the msg payload over
							*((char*)temp+c) = *((char*)payload+c);
							c++;
						}

						int ret =client_send_packet(connlist[cont],head,temp);
						if(ret == 0){//send successful
							mb_add_message(client_get_mailbox(connlist[cont],1),ntohl(msgid),from,temp,ntohl(payloadlen));
						}
						else{//send fail
							mb_add_notice(client_get_mailbox(cl,1),BOUNCE_NOTICE_TYPE,ntohl(msgid));
						}
						free(head);
					}
					user_unref(u,"pointer deleted");
				}
				client_unref(connlist[cont],"pointer from allclient is deleted");
				connlist[cont] = NULL;//delete pointer
				cont++;
			}
			if(fail == -5){
				mb_add_notice(client_get_mailbox(cl,1),BOUNCE_NOTICE_TYPE,ntohl(msgid));
			}
			free(connlist);//free the malloc array
		}
	}
	if(client_get_log(cl) == 0){
		client_logout(cl);
		sleep(1);
	}
	free(payload);
	free(hdr);
	if(mailstart == 0){
		Pthread_join(tid,NULL);
	}
	Pthread_exit(arg);
	return 0;
}

void *mailboxservice(void *arg){
	MAILBOX *mb = (MAILBOX*)arg;
	MAILBOX_ENTRY *ent;

	while((ent = mb_next_entry(mb)) != NULL){//while mailbox is not defunct or still have notice/message
		if(ent->type == MESSAGE_ENTRY_TYPE){
			MESSAGE msg = (ent->content).message;
			void *payload = msg.body;
			CHLA_PACKET_HEADER *head = (CHLA_PACKET_HEADER*)malloc(sizeof(CHLA_PACKET_HEADER));
			head->type = CHLA_RCVD_PKT;
			head->msgid = htonl(msg.msgid);
			head->payload_length = 0;
			char *handle = mb_get_handle(msg.from);
			CLIENT ** connlist = creg_all_clients(client_registry);//return all connected client
			int cont =0;
			while(connlist[cont] != NULL){
				USER *u = client_get_user(connlist[cont],0);//get user from client
				if(u != NULL){
					char *h = user_get_handle(u);//get handle from user
					if(strcmp(h,handle) == 0){
						client_send_packet(connlist[cont],head,payload);
					}
					user_unref(u,"pointer deleted");
				}
				client_unref(connlist[cont],"pointer from allclient is deleted");
				connlist[cont] = NULL;//delete pointer
				cont++;
			}
			mb_unref(msg.from,"msg has been processed, pointer from message is discarded");
			free(connlist);//free the malloc array
			free(head);
			free(payload);
		}
		else if(ent->type == NOTICE_ENTRY_TYPE){
			NOTICE ntc = (ent->content).notice;
			if(ntc.type == NO_NOTICE_TYPE){//ack or nack and stuff
				;
			}
			else if(ntc.type == BOUNCE_NOTICE_TYPE){
				MAILBOX_DISCARD_HOOK *hook = (void*)ent;
				mb_set_discard_hook(mb,hook);//setting discard hook
				CHLA_PACKET_HEADER *head = (CHLA_PACKET_HEADER*)malloc(sizeof(CHLA_PACKET_HEADER));
				head->type = CHLA_BOUNCE_PKT;
				head->msgid = htonl(ntc.msgid);
				head->payload_length = 0;
				char *handle = mb_get_handle(mb);
				CLIENT ** connlist = creg_all_clients(client_registry);//return all connected client
				int cont =0;
				while(connlist[cont] != NULL){
					USER *u = client_get_user(connlist[cont],0);//get user from client
					if(u != NULL){
						char *h = user_get_handle(u);//get handle from user
						if(strcmp(h,handle) == 0){
							client_send_packet(connlist[cont],head,NULL);
						}
						user_unref(u,"pointer deleted");
					}
					client_unref(connlist[cont],"pointer from allclient is deleted");
					connlist[cont] = NULL;//delete pointer
					cont++;
				}
				free(connlist);//free the malloc array
				free(head);
			}
			else if(ntc.type == RRCPT_NOTICE_TYPE){
				;
			}
		}
		free(ent);
	}
	printf("%s\n", "mailbox shutdown complete");
	Pthread_exit(arg);
	return 0;
}