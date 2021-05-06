#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "protocol.h"
#include "csapp.h"

/*
 * Send a packet with a specified header and payload.
 *   fd - file descriptor on which packet is to be sent
 *   hdr - the packet header, with multi-byte fields in network byte order
 *   payload - pointer to packet payload, or NULL, if none.
 *
 * Multi-byte fields in the header are assumed to be in network byte order.
 *
 * On success, 0 is returned.
 * On error, -1 is returned and errno is set.
 */
int proto_send_packet(int fd, CHLA_PACKET_HEADER *hdr, void *payload){
	//convert to network byte order

	int ret = rio_writen(fd,hdr,sizeof(CHLA_PACKET_HEADER));
	if(ret == -1){//error in writing
		errno = EIO;
		return -1;
	}
	uint32_t sz = ntohl(hdr->payload_length);
	if(sz > 0){
		ret = rio_writen(fd,payload,sz);
	}
	if(ret == -1){//error in writing
		errno = EIO;
		return -1;
	}
	return 0;//success writing
}
/*
 * Receive a packet, blocking until one is available.
 *  fd - file descriptor from which packet is to be received
 *  hdr - pointer to caller-supplied storage for packet header
 *  payload - variable into which to store payload pointer
 *
 * The returned header has its multi-byte fields in network byte order.
 *
 * If the returned payload pointer is non-NULL, then the caller
 * is responsible for freeing the storage.
 *
 * On success, 0 is returned.
 * On error, -1 is returned, payload and length are left unchanged,
 * and errno is set.
 */
int proto_recv_packet(int fd, CHLA_PACKET_HEADER *hdr, void **payload){
	//header is in network byte order, caller should call ntohl to convert to host byte order
	printf("%s\n", "waiting for packet");
	int ret=rio_readn(fd,hdr,sizeof(CHLA_PACKET_HEADER));//read a packet header store into hdr.
	printf("%s\n", "received packet");
	if(ret == -1){//error in reading
		errno = EIO;
		return -1;
	}
	if(ret == 0){//EOF
		return -1;
	}
	uint32_t sz = ntohl(hdr->payload_length);
	if(sz > 0){//non zero payload length
		*payload = malloc(sz);
		ret = rio_readn(fd,*payload,sz);
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