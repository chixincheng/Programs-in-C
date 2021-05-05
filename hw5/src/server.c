#include <string.h>


#include "protocol.h"
#include "globals.h"
#include "server.h"
#include "client_registry.h"


void *chla_client_service(void *arg){
	int fd = *((int*)arg);
	CHLA_PACKET_HEADER *hdr = (CHLA_PACKET_HEADER*)malloc(sizeof(CHLA_PACKET_HEADER));
	void *payload = NULL;
	CLIENT *cl = creg_register(client_registry,fd);
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
				client_send_ack(cl,msgid,payload,payloadlen);//login success
			}
			else{
				client_send_nack(cl,msgid);//login fail
			}
		}
		else if(type == CHLA_LOGOUT_PKT){//logout request
			int log = client_logout(cl);
			if(log != -1){
				client_send_nack(cl,msgid);//logout fail
			}
			else{
				client_send_ack(cl,msgid,payload,payloadlen);//logout success
			}
		}
		else if(type == CHLA_USERS_PKT){//user request
			CLIENT ** connlist = creg_all_clients(client_registry);//return all connected client
			int cont =0;
			char *payl;
			int len =0;
			while(connlist[cont] != NULL){
				USER *u = client_get_user(connlist[cont],1);//get user from client
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
				client_unref(connlist[cont],"pointer from allclient is deleted");
				connlist[cont] = NULL;//delete pointer
				cont++;
			}
			client_send_ack(cl,msgid,payl,len);
		}
		else if(type == CHLA_SEND_PKT){//send request

		}
	}
	return 0;
}