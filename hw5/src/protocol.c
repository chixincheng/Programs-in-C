#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "protocol.h"
#include "csapp.h"


int proto_send_packet(int fd, CHLA_PACKET_HEADER *hdr, void *payload){
	//convert to network byte order
	(*hdr).payload_length = htonl((*hdr).payload_length);
	(*hdr).msgid = htonl((*hdr).msgid);
	(*hdr).timestamp_sec = htonl((*hdr).timestamp_sec);
	(*hdr).timestamp_nsec = htonl((*hdr).timestamp_nsec);
	int ret = rio_writen(fd,hdr,sizeof(hdr));
	if(ret == -1){//error in writing
		errno = EIO;
		return -1;
	}
	if((*hdr).payload_length > 0){
		ret = rio_writen(fd,payload,(*hdr).payload_length);
	}
	if(ret == -1){//error in writing
		errno = EIO;
		return -1;
	}
	return 0;//success writing
}

int proto_recv_packet(int fd, CHLA_PACKET_HEADER *hdr, void **payload){
	//header is in network byte order, caller should call ntohl to convert to host byte order
	int ret = rio_readn(fd,hdr,sizeof(hdr));//read a packet header store into hdr.
	if(ret == -1){//error in reading
		errno = EIO;
		return -1;
	}
	if(ret == 0){//EOF
		return -1;
	}
	if(ntohl((*hdr).payload_length) > 0){//non zero payload length
		ret = rio_readn(fd,payload,ntohl((*hdr).payload_length));
	}
	if(ret == -1){//error in reading
		errno = EIO;
		return -1;
	}
	if(ret == 0){//EOF
		return -1;
	}
	return 0;//success reading

	//rio_readn might encounter short count on EOF
}