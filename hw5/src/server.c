


#include "protocol.h"
#include "globals.h"
#include "server.h"



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
			client_login(cl,handle);
			client_send_ack(cl,msgid,payload,payloadlen);
		}
		else if(type == CHLA_LOGOUT_PKT){//logout request

		}
		else if(type == CHLA_USERS_PKT){//user request

		}
		else if(type == CHLA_SEND_PKT){//send request

		}
	}
	return 0;
}
