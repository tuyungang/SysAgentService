#pragma once

#include "Util.h"

enum PacketCommand /*: unsigned char*/
{
	PACKET_COMMAND_NONE = -1,
    PACKET_COMMAND_REQUEST_RULE = 0,
    PACKET_COMMAND_REQUEST_CONFIG_FILE = 1, 
    PACKET_COMMAND_POST_MONITOR_INFO = 2,
    PACKET_COMMAND_REQUEST_RECONNECT = 3,
    PACKET_COMMAND_REQUEST_EXPIRED_TIME = 4,
    PACKET_COMMAND_REQUEST_LOGIN = 5,
    PACKET_COMMAND_REQUEST_RELOGIN = 6,
    PACKET_COMMAND_REQUEST_UPDATE_RULE = 7,
    PACKET_CLIENT_COMMAND_REQUEST_QQ_RULE = 8,
    PACKET_CLIENT_COMMAND_REQUEST_WECHAT_RULE = 9,
    PACKET_CLIENT_COMMAND_REQUEST_FETION_RULE = 10,
    PACKET_CLIENT_COMMAND_REQUEST_USB_RULE = 11,
    PACKET_CLIENT_COMMAND_REQUEST_WEB_RULE = 12,
    PACKET_CLIENT_COMMAND_REQUEST_NETDISK_RULE = 13,
    PACKET_CLIENT_COMMAND_REQUEST_FOXMAIL_RULE = 14,
    PACKET_CLIENT_COMMAND_REQUEST_UPDATE_RULE = 15, 
    PACKET_CLIENT_COMMAND_REQUEST_RECONNECT = 16,
    PACKET_CLIENT_COMMAND_REQUEST_LOGIN = 17,
    PACKET_CLIENT_COMMAND_REQUEST_RELOGIN = 18,
    PACKET_CLIENT_COMMAND_POST_QQ_INFO = 19,
    PACKET_CLIENT_COMMAND_POST_WECHAT_INFO = 20,
    PACKET_CLIENT_COMMAND_POST_FETION_INFO = 21,
    PACKET_CLIENT_COMMAND_POST_USB_INFO = 22,
    PACKET_CLIENT_COMMAND_POST_WEB_INFO = 23,
    PACKET_CLIENT_COMMAND_POST_NETDISK_INFO = 24,
    PACKET_CLIENT_COMMAND_POST_FOXMAIL_INFO = 25,
    PACKET_CLIENT_COMMAND_REQUEST_START_RECORD_DESKTOP = 26
};

enum PacketAction /*: unsigned char*/
{
	PACKET_ACTION_NONE = 0, 
	PACKET_ACTION_GET = 1,
    PACKET_ACTION_POST = 2,
    PACKET_ACTION_REPLY = 3,
    PACKET_ACTION_UPDATE = 4,
    PACKET_ACTION_ACCEPT = 5,
    PACKET_ACTION_REMOVE = 6,
    PACKET_ACTION_AGREE = 7,
    PACKET_ACTION_CREATE = 8,
    PACKET_ACTION_ADD = 9
};

enum PacketEncryption /* : unsigned char*/
{
	PACKET_ENCRYPTION_NONE = 0, 
	PACKET_ENCRYPTION_MD5 = 1,
	PACKET_ENCRYPTION_RSA = 2,
	PACKET_ENCRYPTION_DES = 3,
	PACKET_ENCRYPTION_SHA = 5,
	PACKET_ENCRYPTION_AES = 4
};

enum PacketCheck/* : unsigned char*/
{
	PACKET_CHECK_NONE = 0, 
	PACKET_CHECK_MD5 = 1,
	PACKET_CHECK_CRC = 2,
};

typedef struct qq_tag
{
	UINT nLocalIPLength;
	UINT nSenderQQLength;
	UINT nReceiverIPLength;
	UINT nSendFilePathLength;
	INT  nTriggleTime;
	UINT nHostName;
	// CHAR sLocalIP[50];
	// CHAR sSenderQQ[50];
	// CHAR sReceiverIP[50];
	// CHAR sSendFilePath[50];
}qq;

typedef struct wechat_tag
{
	UINT nLocalIPLength;
	UINT nReceiverIPLength;
	UINT nSendFilePathLength;
	INT  nTriggleTime;
	UINT nHostName;
	// CHAR sLocalIP[50];
	// CHAR sReceiverIP[50];
	// CHAR sSendFilePath[50];
}wechat;

typedef struct fetion_tag
{
	UINT nLocalIPLength;
	UINT nReceiverIPLength;
	UINT nSendFilePathLength;
	INT  nTriggleTime;
	UINT nHostName;
	// CHAR sLocalIP[50];
	// CHAR sRemoteIP[50];
	// CHAR sSendFilePath[50];
}fetion;

typedef struct usb_tag
{
	bool bUsbInsert;
	UINT nCopyFileLength;
	INT  nTriggleTime;
	// CHAR sCopyFile[50];
}usb;

typedef struct foxmail_tag
{
	UINT nUserMailAddressLength;
	UINT nReceiverMailAddressLength;
	UINT nAttachFilePathLenth;
	INT  nTriggleTime;
	// CHAR sUserMailAddress[50];
	// CHAR sReceiverMailAddress[50];
	// CHAR sAttachFilePath[50];
}foxmail;

typedef struct web_tag
{
	UINT nAccessURLLength;
	INT  nTriggleTime;
	// CHAR sAccessURL[50];
}web;

typedef struct netdisk_tag
{
	UINT nNetDiskNameLenth;
	UINT nNetDiskNameLength;
	INT  nTriggleTime;
	// CHAR sUploadFilePath[50];
	// CHAR sUploadFilePath[50];
}netdisk;

//client
typedef struct packet_client_header
{
	INT command;
	INT action;
	INT check;
	INT pkgSize;
	INT pkgTag;
	union pkgInfoTag
	{
		qq pkg_qq;
		wechat pkg_wechat;
		fetion pkg_fetion;
		usb pkg_usb;
		foxmail pkg_foxmail;
		web pkg_web;
		netdisk pkg_netdisk;
	}pkgInfo;
}PKG_CLIENT_HEAD_STRUCT, *PPKG_CLIENT_HEAD_STRUCT;

//server
typedef struct packet_header
{
	INT command;
	INT action;
	INT check;
	/*PacketCommand command;
	PacketAction action;
	PacketCheck check;*/
	INT pkgSize;
	INT pkgTag;
}PKG_HEAD_STRUCT, *PPKG_HEAD_STRUCT;

typedef struct packet_body
{
	CHAR data[1];
}PKG_BODY_STRUCT, *PPKG_BODY_STRUCT;

typedef struct packet
{
	PKG_HEAD_STRUCT head;
	PKG_BODY_STRUCT body;
}PACKET, *PPACKET;

// extern void purge(void *yv, long long ylen);
// extern void packet_purge(PACKET *pkg);
// extern void packet_init(PACKET *pkg);

extern BOOL packet_send(int sockfd, const char *buffer, int len);
extern BOOL packet_recv(int sockfd, char *buffer, LONG &idx, PPKG_HEAD_STRUCT pHead, char *retBuffer, BOOL &isEnough);


