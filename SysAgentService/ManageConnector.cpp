#include "ManageConnector.h"
#include "ManageThreadPool.h"
#include "ManageIOCPModel.h"

CConnector::CConnector(void *arg, char *IP, int port, STATE st)
{
	m_IOCPModel = (CIOCPModel*)arg;
	// m_Manage = (CManageThreadPool*)manage;
	memset(m_ServerIP, '\0', 20);
	memset(m_StandbyIP, '\0', 20);
	memset(m_LocalIP, '\0', 20);

	m_Port = -1;
	m_LocalPort = port;
	memcpy(m_LocalIP, IP, strlen(IP));
    m_Socket = INVALID_SOCKET;
    m_State = st;
	InitConnector();
}

CConnector::CConnector(void *manage, char *serverIP, char *standbyIP, int port, STATE st)
{
	m_Manage = (CManageThreadPool*)manage;
	memset(m_ServerIP, '\0', 20);
	memset(m_StandbyIP, '\0', 20);
	memset(m_LocalIP, '\0', 20);
	assert(serverIP != NULL && standbyIP != NULL);
	m_Port = port;
	m_LocalPort = -1;
	memcpy(m_ServerIP, serverIP, strlen(serverIP));
	memcpy(m_StandbyIP, standbyIP, strlen(standbyIP));
    m_Socket = INVALID_SOCKET;
    m_State = st;
	InitConnector();
}

CConnector::~CConnector()
{
	closesocket(m_Socket);
	WSACleanup();
}

BOOL CConnector::ReceiveClient()
{
    return TRUE;
}

BOOL CConnector::ConnectServer()
{

	BOOL bRet;

RETRY_MAIN:
	bRet = _socket(0);
	
	if (!bRet) {
		recordlog("CConnector::ConnectServer()-->_socket(0)  failure");
		goto RETRY_MAIN;
	}
	bRet = _connect();
	if (!bRet) {
	//	recordlog("CConnector::ConnectServer()-->_connect()0  failure");
		// goto RETRY_MAIN;
	}
	else
		goto SUCCESSFULLY;
	closesocket(m_Socket);
	m_Socket = INVALID_SOCKET;
RETRY_STANDBY:
	bRet = _socket(1);
	if (!bRet) {
		recordlog("CConnector::ConnectServer()-->_socket(1)  failure");
		goto RETRY_STANDBY;
	}
	bRet = _connect();
	if (!bRet) {
		//recordlog("CConnector::ConnectServer()-->_connect()1  failure");
		// goto RETRY_MAIN;
	}
	else
		goto SUCCESSFULLY;

	unsigned long nLong;
	if ((nLong = m_Manage->GetRuleVarVaule()) <= 3) {
		if (nLong = 3) {
			/*HANDLE hr = m_Manage->GetRuleFromLocalEventHandle();
			SetEvent(&hr);*/
		}
		m_Manage->IncrementVar(m_Manage->GetRuleVar());
	}
	goto RETRY_MAIN;
	// return FALSE;

SUCCESSFULLY:
	//HANDLE hr = m_Manage->GetRuleFromLocalEventHandle();
	//SetEvent(&hr);
	return TRUE;
}

BOOL CConnector::InitConnector()
{
	BOOL bRet;
REINIT:
	bRet = _init();
	if (!bRet) {
		goto REINIT;
	}

	if (m_State == STATE_CLIENT) {
RELOGIN:
		bRet = ConnectServer();
		if (!bRet) {
			goto RELOGIN;
		}
	}
	else if (m_State == STATE_SERVER) {
		bRet = ReceiveClient();
		if (!bRet) {
		}
	}

	m_Active = ACTIVE_ONLINE;
	return TRUE;
}

BOOL CConnector::_init()
{
	int iresult =WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iresult != 0) {	
		recordlog(" CConnector::_init() failed");
		return FALSE;
	}
	recordlog("CConnector::_init() successed");
	return TRUE;
}

SOCKET CConnector::_socket(int type)
{

	ZeroMemory(&m_Address, sizeof(m_Address));
	m_Address.sin_family = AF_INET;
	switch (type) {
		case 0: {
			inet_pton(AF_INET, m_ServerIP, &m_Address.sin_addr);
			recordlog(m_ServerIP);
			m_Address.sin_port = htons(m_Port);
			goto CLIENT;
			break;
		}
		case 1: {
			inet_pton(AF_INET, m_StandbyIP, &m_Address.sin_addr);
			m_Address.sin_port = htons(m_Port);
			goto CLIENT;
			break;
		}
		case 2: {
			inet_pton(AF_INET, "127.0.0.1"/*m_LocalIP*/, &m_Address.sin_addr);
			m_Address.sin_port = htons(m_LocalPort);
			goto SERVER;
			break;
		}
		default:
			break;
	}

SERVER:
	m_Socket = INVALID_SOCKET;
	m_Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_Socket < 0)
	{
		//log("main socket() error");
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return INVALID_SOCKET;
	}
	return m_Socket;
CLIENT:
	m_Socket = INVALID_SOCKET;
	m_Socket = socket(PF_INET, SOCK_STREAM, 0);
	if (m_Socket < 0)
	{
		//log("main socket() error");
		closesocket(m_Socket);
		return INVALID_SOCKET;
	}
	return m_Socket;
}

SOCKET CConnector::CreateSocket()
{
	return _socket(2);
}

void CConnector::CreateListen()
{
	_listen();
}

void CConnector::CreateBind()
{
	_bind();	
}

SOCKET CConnector::GetSocketHandle()
{
	return m_Socket;
}

BOOL CConnector::_bind()
{
	int iResult = bind(m_Socket, (SOCKADDR *)&m_Address, sizeof (m_Address));
    if (iResult == SOCKET_ERROR) {
        wprintf(L"bind failed with error %u\n", WSAGetLastError());
        closesocket(m_Socket);
        WSACleanup();
        return FALSE;
    }
    else
        wprintf(L"bind returned success\n");
    return TRUE;
}

BOOL CConnector::_listen()
{
	int iResult = listen(m_Socket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(m_Socket);
        WSACleanup();
        return FALSE;
    }
    return TRUE;
}

BOOL CConnector::_accept()
{
	/*SOCKET ClientSocket = accept(m_Socket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(m_Socket);
        WSACleanup();
        return FALSE;
    }*/
    return TRUE; 
}

BOOL CConnector::_connect()
{
	int reuse = 1, on = 1, sndbuf = 0;
	int iResult;
	BOOL bRet;
	u_long iMode = 1;
    setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
    setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&on, sizeof(on));
    //setsockopt(m_sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));
    /*struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    socklen_t len = sizeof(timeout);
    setsockopt(m_sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, len);
   */ 
    _setnonblocking(iMode);
    if ((iResult = connect(m_Socket, (SOCKADDR* )&m_Address, sizeof(m_Address))) == 0)
    {
        recordlog( "build main connection successfully\n");
        goto SUCCESS;
    }
    else if (iResult < 0)
    {			
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
			bRet = SelectListen();
			if (bRet)
				goto SUCCESS;
		}
        /*if (errno == EINPROGRESS) {
            bRet = SelectListen();
            if (bRet)
                goto SUCCESS;
        }*/
    }
	closesocket(m_Socket);
	m_Socket = INVALID_SOCKET;
    return FALSE;
SUCCESS:
	recordlog("connect server sucess");
	iMode = 0;
	_setnonblocking(iMode);
	return TRUE;
}

void CConnector::_close()
{
	closesocket(m_Socket);
	WSACleanup();
}

void CConnector::_setnonblocking(u_long iMode)
{
	int iResult;
	iResult = ioctlsocket(m_Socket, FIONBIO, &iMode);
	if (iResult != NO_ERROR)
		printlog("ioctlsocket failed with error: %ld\n", iResult);
	  //printf("ioctlsocket failed with error: %ld\n", iResult);	
}

BOOL CConnector::SelectListen()
{
    struct timeval tm;
    int len, err = -1;
    tm.tv_sec = 3;  
    tm.tv_usec = 0;  
    fd_set wset;
    FD_ZERO(&wset);  
    FD_SET(m_Socket, &wset);  
	char temp[10] = {0};
    int retval = select(m_Socket + 1, NULL, &wset, NULL, &tm);  
	sprintf(temp, "%d", retval);
	recordlog("SelectListen select return err:");
	recordlog(temp);
    switch(retval)  
    {  
        case -1:  
        {

			recordlog("SelectListen select error");
            return FALSE;  
        }
        case 0:  
        {
			recordlog("SelectListen connect timeout\n");
            return FALSE;  
        }
        case 1:
        {
			recordlog("SelectListen 1");
            if(FD_ISSET(m_Socket, &wset))  
            {  
				recordlog("SelectListen build connect successfully!\n");
            }
            break;
        }
        default:  
        {
			recordlog("SelectListen default");
            if(FD_ISSET(m_Socket, &wset))  
            {  
                if(getsockopt(m_Socket,SOL_SOCKET,SO_ERROR, (char*)&err, (socklen_t *)&len) < 0)  
                {  
					recordlog("SelectListen getsockopt error");
                    return FALSE;  
                }  
                if(err != 0)  
                {  
					recordlog("SelectListen getsockopt error != 0");
                    return FALSE;
                }  
            }
            break;  
        }
    }
    return TRUE;
}
void CConnector::SendToServer()
{
	//if (m_DataHtmlList.empty()) {
	//	MessageBox(NULL, L"is empty", L"BHO", MB_OK);
	//	return FALSE;
	//}
	//string html = m_DataHtmlList.front();
	//int aLen = sizeof(PKG_HEAD_STRUCT) + sizeof(BODYInfo) + html.length() + 1;
	//char *data = (char*)malloc(aLen * sizeof(char));
	////char data[sizeof(PKG_HEAD_STRUCT) + sizeof(BODYInfo)] = { 0 };
	//BODYInfo *pReport = reinterpret_cast<BODYInfo*>(data + sizeof(PKG_HEAD_STRUCT));

	//memcpy(pReport->buffer, html.c_str(), html.length());

	//PPKG_HEAD_STRUCT pHead = reinterpret_cast<PKG_HEAD_STRUCT*>(data);
	//pHead->type = 1;
	//pHead->length = sizeof(PKG_HEAD_STRUCT) + html.length();
	//int nLen = pHead->length;
	//m_DataHtmlList.pop_back();
	//while (1) {
	//	BOOL ret = Write_nBytes(sockfd, (char*)pHead, pHead->length);
	//	if (ret == FALSE) {
	//		closesocket(sockfd);
	//		return FALSE;
	//	}
	//	break;
	//	/*int nSize = send(sockfd, (char*)pHead, nLen, 0);
	//	if (nSize <= 0) {
	//	closesocket(sockfd);
	//	return FALSE;
	//	}
	//	nLen -= nSize;*/
	//}

	//return TRUE;
}

void CConnector::SendToClient()
{

}

void CConnector::ReceiveFromServer()
{

}

void CConnector::ReceiveFromClient()
{

}

