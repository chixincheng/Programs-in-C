#include <string.h>


#include "protocol.h"
#include "globals.h"
#include "server.h"
#include "client_registry.h"


void *chla_client_service(void *arg){
	int fd = *((int*)arg);
	CHLA_PACKET_HEADER *hdr = (CHLA_PACKET_HEADER*)malloc(sizeof(CHLA_PACKET_HEADER));
	hdr->type = -1;
	void *payload = NULL;
	CLIENT *cl = creg_register(client_registry,fd);
	MAILBOX *from =NULL;
	int run = 0;
	while(run != -1){
		run = proto_recv_packet(fd,hdr,&payload);
		CHLA_PACKET_TYPE type = hdr->type;
		char *handle = payload;
		uint32_t msgid = hdr->msgid;
		uint32_t payloadlen = hdr->payload_length;
		if(type == CHLA_LOGIN_PKT){//login request
			int log = client_login(cl,handle);
			if(log != -1){
				payloadlen = htonl(payloadlen);
				msgid = htonl(msgid);
				client_send_ack(cl,msgid,payload,payloadlen);//login success
			}
			else{
				msgid = htonl(msgid);
				client_send_nack(cl,msgid);//login fail
			}
		}
		else if(type == CHLA_LOGOUT_PKT){//logout request
			int log = client_logout(cl);
			if(log != 0){
				msgid = htonl(msgid);
				client_send_nack(cl,msgid);//logout fail
			}
			else{
				payloadlen = htonl(payloadlen);
				msgid = htonl(msgid);
				client_send_ack(cl,msgid,payload,payloadlen);//logout success
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
				}
				user_unref(u,"pointer deleted");
				client_unref(connlist[cont],"pointer from allclient is deleted");
				connlist[cont] = NULL;//delete pointer
				cont++;
			}
			len = htonl(len);
			msgid = htonl(msgid);
			client_send_ack(cl,msgid,payl,len);
		}
		else if(type == CHLA_SEND_PKT){//send request
			client_send_ack(cl,htonl(msgid),payload,0);//send ack

			char *temp = (char*)malloc(htonl(payloadlen));
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
						CHLA_PACKET_HEADER *head = (CHLA_PACKET_HEADER*)malloc(sizeof(CHLA_PACKET_HEADER));
						head->type = CHLA_MESG_PKT;
						head->payload_length = htonl(payloadlen);
						head->msgid = htonl(msgid);
						//set from mailbox
						from = client_get_mailbox(cl,0);
						//send msg packet to other client
						client_send_packet(connlist[cont],head,payload);
						free(head);
					}
				}
				user_unref(u,"pointer deleted");
				client_unref(connlist[cont],"pointer from allclient is deleted");
				connlist[cont] = NULL;//delete pointer
				cont++;
			}
			free(connlist);//free the malloc array
			free(temp);
		}
		else if(type == CHLA_MESG_PKT){//receive send request
			//add message to mailbox
			if(from != NULL){
				mb_add_message(client_get_mailbox(cl,1),htonl(msgid),from,payload,htonl(payloadlen));
				mb_unref(from,"delete pointer from the mailbox to add mesg");
				from = NULL;//reset 'from' mailbox
			}



			char *temp = (char*)malloc(htonl(payloadlen));
			int ct = 0;
			while(*(handle+ct) != '\n'){
				*(temp+ct) = *(handle+ct);
				ct++;
			}
			*(temp+ct) = '\n';
			ct++;
			printf("%s%s%s%s\n", "Message from",handle,": ",handle+ct);

			CLIENT ** connlist = creg_all_clients(client_registry);//return all connected client
			int cont =0;
			while(connlist[cont] != NULL){
				USER *u = client_get_user(connlist[cont],0);//get user from client
				if(u != NULL){
					char *h = user_get_handle(u);//get handle from user
					if(strcmp(h,temp) == 0){
						CHLA_PACKET_HEADER *head = (CHLA_PACKET_HEADER*)malloc(sizeof(CHLA_PACKET_HEADER));
						head->type = CHLA_RCVD_PKT;
						head->payload_length = 0;
						head->msgid = htonl(msgid);
						//send msg packet to other client
						client_send_packet(connlist[cont],head,payload);
						free(head);
					}
				}
				user_unref(u,"pointer deleted");
				client_unref(connlist[cont],"pointer from allclient is deleted");
				connlist[cont] = NULL;//delete pointer
				cont++;
			}
			free(connlist);//free the malloc array
			free(temp);
		}
	}
	free(hdr);
	return 0;
}