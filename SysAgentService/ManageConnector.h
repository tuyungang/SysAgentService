#pragma once

//#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <vector>
#include <list>
#include <winsock2.h>
#include <MSWSock.h>
#include <algorithm>
#include <ws2tcpip.h>
#include <assert.h>
#include "Packet.h"
//#include "Util.h"


//#pragma comment (lib, "Ws2_32.lib")
//#pragma comment (lib, "Mswsock.lib")
//#pragma comment (lib, "AdvApi32.lib")

//#define WIN32_LEAN_AND_MEAN
class CManageThreadPool;
class CIOCPModel;
class CConnector
{
public:
	typedef enum { STATE_NONE, STATE_SERVER, STATE_CLIENT } STATE;
	typedef enum { ACTIVE_NONE, ACTIVE_ONLINE, ACTIVE_OUTLINE } ACTIVE;
public:
	CConnector(void *arg, char *IP, int port, STATE st);
	CConnector(void *manage, char *serverIP, char *standbyIP, int port, STATE st);
	virtual ~CConnector();
	/*CConnector(const CConnector&);
	CConnector & operator = (const CConnector&)const;*/

private:
	CManageThreadPool *m_Manage;
	CIOCPModel        *m_IOCPModel;
	char m_ServerIP[20];
	char m_StandbyIP[20];
	char m_LocalIP[20];
	int m_Port;
	int m_LocalPort;
	SOCKET m_Socket;
	SOCKADDR_IN m_Address;
	SOCKADDR_IN service;
	WSADATA wsaData;

	STATE m_State;
	ACTIVE m_Active;
	std::vector<char*> m_ReadySendInfoQueue;

public:
	BOOL InitConnector();
	BOOL ConnectServer();
	BOOL ReceiveClient();
	SOCKET CreateSocket();
	void CreateListen();
	void CreateBind();
	BOOL CloseConnector();
	void SendToServer();
	void SendToClient();
	void ReceiveFromServer();
	void ReceiveFromClient();
	SOCKET GetSocketHandle();

private:
	BOOL _init();
	SOCKET _socket(int type);
	BOOL _bind();
	BOOL _listen();
	BOOL _accept();
	BOOL _connect();
	void _close();
	void _setnonblocking(u_long iMode);
	BOOL SelectListen();

};