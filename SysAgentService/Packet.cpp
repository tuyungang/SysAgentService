#include "Packet.h"

/*
void purge(void *yv, long long ylen) 
{
    volatile char *y = (volatile char *)yv; 
    while (ylen > 0) { *y++ = 0; --ylen; }
}

void packet_purge(PACKET *pkg)
{
	purge(pkg, sizeof PAKCET);
}

void packet_init(PACKET *pkg)
{
	purge(pkg, sizeof PAKCET);

	pkg->head.command = PACKET_COMMAND_NONE;
	pkg->head.action = PACKET_ACTION_NONE;
	pkg->head.pkgSize = 0;
	pkg->body.data[1] = {0};
	pkg->check = PACKET_CHECK_NONE;
}
*/

BOOL Write_nBytes(int sockfd, const char *buffer, int len)
{
	int bytes_write = 0;
	while (1)
	{
		bytes_write = send(sockfd, buffer, len, 0);
		if (bytes_write == -1) {
			return FALSE;
		}
		else if (bytes_write == 0) {
			return FALSE;
		}
		len -= bytes_write;
		buffer = buffer + bytes_write;
		if (len <= 0) {
			return TRUE;
		}
	}
	return TRUE;
}

int packet_send(int sockfd, const char *buffer, int len)
{
	int bytes_write = 0;
	while (1)
	{
		bytes_write = send(sockfd, buffer, len, 0);
		if (bytes_write == -1) {
			return FALSE;
		}
		else if (bytes_write == 0) {
			return FALSE;
		}
		len -= bytes_write;
		buffer = buffer + bytes_write;
		if (len <= 0) {
			return TRUE;
		}
	}
	return TRUE;
}

BOOL packet_recv(int sockfd, char *buffer, LONG &idx, PPKG_HEAD_STRUCT pHead, char *retBuffer, BOOL &isEnough)
{
	int bytes_read = 0;
	LONG nLength = 0;
	//bytes_read = recv(sockfd, buffer + idx, 1, 0 );
	bytes_read = recv(sockfd, buffer, 1024, 0);
    if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return FALSE;
        } else {
            return FALSE;
        }
    } else if (bytes_read == 0) {
    	return FALSE;
    } else if (bytes_read > 0) {
        idx += bytes_read;
		INT nCommandLen;
		memcpy(&nCommandLen, (INT*)buffer, sizeof(INT));
		INT nActionLen;
		memcpy(&nActionLen, (INT*)buffer + 4, sizeof(INT));
		INT nCheckLen;
		memcpy(&nCheckLen, (INT*)buffer + 8, sizeof(INT));
		INT nPkgSizeLen;
		memcpy(&nPkgSizeLen, (INT*)buffer + 12, sizeof(INT));
		INT nPkgTagLen;
		memcpy(&nPkgTagLen, (INT*)buffer + 16, sizeof(INT));
		memcpy(retBuffer, buffer + sizeof(PKG_HEAD_STRUCT), nPkgSizeLen - sizeof(PKG_HEAD_STRUCT));
		printlog(retBuffer);
		isEnough = TRUE;

   //     if (idx == sizeof(PKG_HEAD_STRUCT)) {
   //     	INT nCommandLen;
   //     	memcpy(&nCommandLen, (INT*)buffer, sizeof(INT));
   //     	INT nActionLen;
   //     	memcpy(&nActionLen, (INT*)buffer + 4, sizeof(INT));
   //     	INT nCheckLen;
   //     	memcpy(&nCheckLen, (INT*)buffer + 8, sizeof(INT));
   //     	INT nPkgSizeLen;
   //     	memcpy(&nPkgSizeLen, (INT*)buffer + 12, sizeof(INT));
   //     	INT nPkgTagLen;
   //     	memcpy(&nPkgTagLen, (INT*)buffer + 16, sizeof(INT));

   //     	// pHead = reinterpret_cast<PKG_HEAD_STRUCT*>(buffer);	
   //     	nLength = nPkgSizeLen - sizeof(PKG_HEAD_STRUCT);
			//if (idx == nPkgSizeLen) {
			//	memcpy(retBuffer, buffer + sizeof(PKG_HEAD_STRUCT), nLength);
			//	printlog(retBuffer);
			//	isEnough = TRUE;
			//}
   //     }
    }
    return TRUE;
}