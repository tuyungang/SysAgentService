#include "ManageIOCPModel.h"
#include "ManageThreadPool.h"
#include "ManageConnector.h"
#include "Util.h"
#include "ManageRules.h"

#define WORKER_THREADS_PER_PROCESSOR 2
#define MAX_POST_ACCEPT              10
#define EXIT_CODE                    NULL



#define RELEASE(x)                      {if(x != NULL ){delete x;x=NULL;}}
#define RELEASE_HANDLE(x)               {if(x != NULL && x!=INVALID_HANDLE_VALUE){ CloseHandle(x);x = NULL;}}
#define RELEASE_SOCKET(x)               {if(x !=INVALID_SOCKET) { closesocket(x);x=INVALID_SOCKET;}}



CIOCPModel::CIOCPModel(void) :
m_nThreads(0),
m_hShutdownEvent(NULL),
m_hIOCompletionPort(NULL),
m_phWorkerThreads(NULL),
//m_strIP(DEFAULT_IP),
m_nPort(DEFAULT_PORT),
//m_pMain(NULL),
m_lpfnAcceptEx(NULL),
m_pListenContext(NULL),
m_pConnector(NULL),
m_pManage(NULL),
m_pThreadPool(NULL),
m_pWorkThread(NULL)
{

}

CIOCPModel::CIOCPModel(void *arg) :
m_nThreads(0),
m_hShutdownEvent(NULL),
m_hIOCompletionPort(NULL),
m_phWorkerThreads(NULL),
//m_strIP(DEFAULT_IP),
m_nPort(DEFAULT_PORT),
//m_pMain(NULL),
m_lpfnAcceptEx(NULL),
m_pListenContext(NULL),
m_pConnector(NULL),
m_pManage(NULL),
m_pThreadPool(NULL),
m_pWorkThread(NULL)
{
	m_pManage = (CManageThreadPool*)arg;	
	InitCS(&m_csContextList);
}

CIOCPModel::~CIOCPModel(void)
{
	this->Stop();
}

unsigned int WINAPI CIOCPModel::_WorkerThread(LPVOID lpParam)
{
	CIOCPModel *pIOCPModel = (CIOCPModel*)lpParam;
	/*__tag1 *t = (__tag1*)lpParam;
	CIOCPModel *pIOCPModel = (CIOCPModel*)t->arg1;
	pIOCPModel->m_pManage = (CManageThreadPool*)t->arg;
	pIOCPModel->m_pManage->FtpUploadVideoDataProc(NULL);*/
	/*__tag1 *t1 = (__tag1*)lpParam;
	__tag2 *t2 = (__tag2*)t1->arg1;
	__tag2 *t3 = (__tag2*)t2->arg1;
	CIOCPModel *pIOCPModel = (CIOCPModel*)t3->arg1;
	pIOCPModel->m_pManage = (CManageThreadPool*)t3->arg;
	pIOCPModel->m_pWorkThread = (CWorkThread*)t1->arg;
	pIOCPModel->m_pThreadPool = (CThreadPool*)t2->arg;*/
	/*free(t3);
	free(t2);
	free(t1);*/

	// THREADPARAMS_WORKER* pParam = (THREADPARAMS_WORKER*)lpParam;
	// CIOCPModel* pIOCPModel = (CIOCPModel*)pParam->pIOCPModel;
	// int nThreadNo = (int)pParam->nThreadNo;

	OVERLAPPED           *pOverlapped = NULL;
	PER_SOCKET_CONTEXT   *pSocketContext = NULL;
	DWORD                dwBytesTransfered = 0;
	printlog("server thread %u", GetCurrentThreadId());
	while (WAIT_OBJECT_0 != WaitForSingleObject(pIOCPModel->m_hShutdownEvent, 0))
	{
		recordlog("_WorkerThread GetQueuedCompletionStatus");
		BOOL bReturn = GetQueuedCompletionStatus(
			pIOCPModel->m_hIOCompletionPort,
			&dwBytesTransfered,
			(PULONG_PTR)&pSocketContext,
			&pOverlapped,
			INFINITE);
		if (EXIT_CODE == (DWORD)pSocketContext)
		{
			break;
		}

		if (!bReturn)
		{
			DWORD dwErr = GetLastError();
			
			if (!pIOCPModel->HandleError(pSocketContext, dwErr))
			{
				break;
			}

			continue;
		}
		else
		{
			PER_IO_CONTEXT* pIoContext = CONTAINING_RECORD(pOverlapped, PER_IO_CONTEXT, m_Overlapped);

			if ((0 == dwBytesTransfered) && (RECV_POSTED == pIoContext->m_OpType || SEND_POSTED == pIoContext->m_OpType))
			{
				pIOCPModel->_RemoveContext(pSocketContext);

				continue;
			}
			else
			{
				switch (pIoContext->m_OpType)
				{
					// Accept  
				case ACCEPT_POSTED:
				{
					pIOCPModel->_DoAccpet(pSocketContext, pIoContext);
				}
				break;

				// RECV
				case RECV_POSTED:
				{
					pIOCPModel->_DoRecv(pSocketContext, pIoContext, dwBytesTransfered);
				}
				break;

				// SEND
				case SEND_POSTED:
				{
					// pIOCPModel->_DoSend(pSocketContext, pIoContext);
					//pIOCPModel->_PostRecv(pIoContext);

				}
				break;
				default:
					break;
				} //switch
			}//if
		}//if

	}//while
	RELEASE(lpParam);
	return 0;
}

bool CIOCPModel::LoadSocketLib()
{
	m_pConnector = new CConnector(this, DEFAULT_IP, DEFAULT_PORT, CConnector::STATE_SERVER);
	if (NULL == m_pConnector) {
		return false;
	}
	return true;
}


bool CIOCPModel::Start()
{
	::InitializeCriticalSection(&m_csContextList);

	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (false == _InitializeIOCP())
	{
		recordlog("_InitializeIOCP failre");
		return false;
	}


	if (false == _InitializeListenSocket())
	{
		recordlog("_InitializeListenSocket  failre");
		this->_DeInitialize();
		return false;
	}



	return true;
}

void CIOCPModel::Stop()
{
	if (m_pListenContext != NULL && m_pListenContext->m_Socket != INVALID_SOCKET)
	{
		SetEvent(m_hShutdownEvent);

		for (int i = 0; i < m_nThreads; i++)
		{
			PostQueuedCompletionStatus(m_hIOCompletionPort, 0, (DWORD)EXIT_CODE, NULL);
		}

		WaitForMultipleObjects(m_nThreads, m_phWorkerThreads, TRUE, INFINITE);

		this->_ClearContextList();

		this->_DeInitialize();
	}
}


bool CIOCPModel::_InitializeIOCP()
{
	m_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == m_hIOCompletionPort)
	{
		return false;
	}
	m_pManage->CreateWorkerThreadPool(this);

	/*
	m_nThreads = WORKER_THREADS_PER_PROCESSOR * _GetNoOfProcessors();

	m_phWorkerThreads = new HANDLE[m_nThreads];

	DWORD nThreadID;
	for (int i = 0; i < m_nThreads; i++)
	{
		THREADPARAMS_WORKER* pThreadParams = new THREADPARAMS_WORKER;
		pThreadParams->pIOCPModel = this;
		pThreadParams->nThreadNo = i + 1;
		m_phWorkerThreads[i] = ::CreateThread(0, 0, _WorkerThread, (void *)pThreadParams, 0, &nThreadID);
	}
	*/	
	return true;
}

bool CIOCPModel::_InitializeListenSocket()
{
	recordlog("_InitializeListenSocket");
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;

	struct sockaddr_in ServerAddress;

	m_pListenContext = new PER_SOCKET_CONTEXT;
	m_pListenContext->m_Socket = m_pConnector->CreateSocket();

	if (NULL == CreateIoCompletionPort((HANDLE)m_pListenContext->m_Socket, m_hIOCompletionPort, (DWORD)m_pListenContext, 0))
	{
		 RELEASE_SOCKET(m_pListenContext->m_Socket);
		return false;
	}
	else
	{
	}
	m_pConnector->CreateBind();
	m_pConnector->CreateListen();

	/*	
	m_pListenContext->m_Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_pListenContext->m_Socket)
	{
		return false;
	}
	else
	{
	}

	if (NULL == CreateIoCompletionPort((HANDLE)m_pListenContext->m_Socket, m_hIOCompletionPort, (DWORD)m_pListenContext, 0))
	{
		RELEASE_SOCKET(m_pListenContext->m_Socket);
		return false;
	}
	else
	{
	}

	ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	//ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);                      
	ServerAddress.sin_addr.s_addr = inet_addr(m_strIP.GetString());
	ServerAddress.sin_port = htons(m_nPort);

	if (SOCKET_ERROR == bind(m_pListenContext->m_Socket, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)))
	{
		return false;
	}
	else
	{
	}

	if (SOCKET_ERROR == listen(m_pListenContext->m_Socket, SOMAXCONN))
	{
		return false;
	}
	else
	{
	}
*/
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(
		m_pListenContext->m_Socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx,
		sizeof(GuidAcceptEx),
		&m_lpfnAcceptEx,
		sizeof(m_lpfnAcceptEx),
		&dwBytes,
		NULL,
		NULL))
	{
		//this->_DeInitialize();
		switch(WSAGetLastError())  
        {  
        case WSAENETDOWN:  
            cout<<""<<endl;  
            break;  
        case WSAEFAULT:  
            cout<<"WSAEFAULT"<<endl;  
            break;  
        case WSAEINVAL:  
            cout<<"WSAEINVAL"<<endl;  
            break;  
        case WSAEINPROGRESS:  
            cout<<"WSAEINPROGRESS"<<endl;  
            break;  
        case WSAENOTSOCK:  
            cout<<"WSAENOTSOCK"<<endl;  
            break;  
        case WSAEOPNOTSUPP:  
            cout<<"WSAEOPNOTSUPP"<<endl;  
            break;  
        case WSA_IO_PENDING:  
            cout<<"WSA_IO_PENDING"<<endl;  
            break;  
        case WSAEWOULDBLOCK:  
            cout<<"WSAEWOULDBLOCK"<<endl;  
            break;  
        case WSAENOPROTOOPT:  
            cout<<"WSAENOPROTOOPT"<<endl;  
            break;  
        }
		return false;
	}

	if (SOCKET_ERROR == WSAIoctl(
		m_pListenContext->m_Socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs),
		&m_lpfnGetAcceptExSockAddrs,
		sizeof(m_lpfnGetAcceptExSockAddrs),
		&dwBytes,
		NULL,
		NULL))
	{
		this->_DeInitialize();
		switch(WSAGetLastError())  
        {  
        case WSAENETDOWN:  
            cout<<""<<endl;  
            break;  
        case WSAEFAULT:  
            cout<<"WSAEFAULT"<<endl;  
            break;  
        case WSAEINVAL:  
            cout<<"WSAEINVAL"<<endl;  
            break;  
        case WSAEINPROGRESS:  
            cout<<"WSAEINPROGRESS"<<endl;  
            break;  
        case WSAENOTSOCK:  
            cout<<"WSAENOTSOCK"<<endl;  
            break;  
        case WSAEOPNOTSUPP:  
            cout<<"WSAEOPNOTSUPP"<<endl;  
            break;  
        case WSA_IO_PENDING:  
            cout<<"WSA_IO_PENDING"<<endl;  
            break;  
        case WSAEWOULDBLOCK:  
            cout<<"WSAEWOULDBLOCK"<<endl;  
            break;  
        case WSAENOPROTOOPT:  
            cout<<"WSAENOPROTOOPT"<<endl;  
            break;  
        }
		return false;
	}

	for (int i = 0; i < MAX_POST_ACCEPT; i++)
	{
		PER_IO_CONTEXT* pAcceptIoContext = m_pListenContext->GetNewIoContext();
		if (false == this->_PostAccept(pAcceptIoContext))
		{
			m_pListenContext->RemoveContext(pAcceptIoContext);
			return false;
		}
	}
	return true;
}

void CIOCPModel::_DeInitialize()
{
	DestCS(&m_csContextList);
	RELEASE_HANDLE(m_hShutdownEvent);
	RELEASE(m_pConnector);
	RELEASE(m_pManage);
	RELEASE(m_pWorkThread);
	RELEASE(m_pThreadPool);
	RELEASE_HANDLE(m_hIOCompletionPort);
	RELEASE_HANDLE(m_hIOCompletionPort);
	RELEASE(m_pListenContext);
	/*DeleteCriticalSection(&m_csContextList);

	RELEASE_HANDLE(m_hShutdownEvent);

	for (int i = 0; i<m_nThreads; i++)
	{
		RELEASE_HANDLE(m_phWorkerThreads[i]);
	}

	RELEASE(m_phWorkerThreads);

	RELEASE_HANDLE(m_hIOCompletionPort);

	RELEASE(m_pListenContext);*/
}

bool CIOCPModel::_PostAccept(PER_IO_CONTEXT* pAcceptIoContext)
{
	assert(INVALID_SOCKET != m_pListenContext->m_Socket);

	DWORD dwBytes = 0;
	pAcceptIoContext->m_OpType = ACCEPT_POSTED;
	WSABUF *p_wbuf = &pAcceptIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pAcceptIoContext->m_Overlapped;

	pAcceptIoContext->m_sockAccept = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == pAcceptIoContext->m_sockAccept)
	{
		return false;
	}

	if (FALSE == m_lpfnAcceptEx(m_pListenContext->m_Socket, pAcceptIoContext->m_sockAccept, p_wbuf->buf, p_wbuf->len - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, p_ol))
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			return false;
		}
	}

	return true;
}

bool CIOCPModel::_DoAccpet(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext)
{
	SOCKADDR_IN* ClientAddr = NULL;
	SOCKADDR_IN* LocalAddr = NULL;
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);

	this->m_lpfnGetAcceptExSockAddrs(pIoContext->m_wsaBuf.buf, pIoContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&ClientAddr, &remoteLen);

	printlog("new client");
	INT objFlag = -1;
	if (strlen(pIoContext->m_wsaBuf.buf) > 0) {
		m_pManage->ReceiveDataFromClient(pIoContext->m_wsaBuf.buf, objFlag);
	}

	PER_SOCKET_CONTEXT* pNewSocketContext = new PER_SOCKET_CONTEXT;
	pNewSocketContext->m_Socket = pIoContext->m_sockAccept;
	memcpy(&(pNewSocketContext->m_ClientAddr), ClientAddr, sizeof(SOCKADDR_IN));
	pNewSocketContext->dwObjFlag = objFlag;
	
	if (false == this->_AssociateWithIOCP(pNewSocketContext))
	{
		RELEASE(pNewSocketContext);
		return false;
	}

	PER_IO_CONTEXT* pNewIoContext = pNewSocketContext->GetNewIoContext();
	//pNewIoContext->m_OpType = SEND_POSTED;
	 pNewIoContext->m_OpType = RECV_POSTED;
	pNewIoContext->m_sockAccept = pNewSocketContext->m_Socket;
	//memcpy( pNewIoContext->m_szBuffer,pIoContext->m_szBuffer,MAX_BUFFER_LEN );

	if (false == this->_PostRecv(pNewIoContext))
	{
		pNewSocketContext->RemoveContext(pNewIoContext);
		return false;
	}
	
	this->_AddToContextList(pNewSocketContext);

	int nBytesSend;
	DWORD dwBytes = 0;
	DWORD dwFlags = 0;
	OVERLAPPED *p_ol = &pNewIoContext->m_Overlapped;
	vector<CObject*> objList = m_pManage->GetMonitorObjectList();
	recordlog("new client");
	char sBuffer[1024];
	WSABUF wbuf;
	wbuf.buf = sBuffer;
	ZeroMemory(sBuffer, 1024);
	int index = 0;
	switch (objFlag) {
		case 1:{
			vector<CObject*>::iterator it = objList.begin();	
			(*it)->m_ObjectName.c_str();
			while (it != objList.end()) {
				if (strcmp((*it)->m_ObjectName.c_str(), "QQ.exe") == 0) {
					map<CRule, CPolicy>::iterator pos = (*it)->m_RulePolicyMap.begin();
					while (pos != (*it)->m_RulePolicyMap.end()) {
						memcpy(sBuffer + index, &(*pos).second.m_StartTime, sizeof(LONG));
						index += sizeof(LONG);
						memcpy(sBuffer + index, &(*pos).second.m_EndTime, sizeof(LONG));
						index += sizeof(LONG);
						INT len = (*pos).first.m_ActionName.size();
						memcpy(sBuffer + index, &len, sizeof(INT));
						index += sizeof(INT);
						memcpy(sBuffer + index, (*pos).first.m_ActionName.c_str(), (*pos).first.m_ActionName.size());
						index += (*pos).first.m_ActionName.size();
						len = (*pos).first.m_TriggleExpr.size();
						memcpy(sBuffer + index, &len, sizeof(INT));
						index += sizeof(INT);
						memcpy(sBuffer + index, (*pos).first.m_TriggleExpr.c_str(), (*pos).first.m_TriggleExpr.size());
						index += (*pos).first.m_TriggleExpr.size();
						len = (*it)->m_ObjectInstallPath.size();
						memcpy(sBuffer + index, &len, sizeof(INT));
						index += sizeof(INT);
						memcpy(sBuffer + index, (*it)->m_ObjectInstallPath.c_str(), len);
						index += (*it)->m_ObjectInstallPath.size();
						pos++;
					}
					/*wbuf.len = index;
					nBytesSend = WSASend(pNewIoContext->m_sockAccept, &wbuf, 1, &dwBytes, dwFlags, p_ol, NULL);*/
					break;
				}
				it++;
			}
			break;	
		}
		case 2:{
			vector<CObject*>::iterator it = objList.begin();	
			while (it != objList.end()) {
				if (strcpy(const_cast<char*>((*it)->m_ObjectName.c_str()), "WeChat.exe") == 0) {
					map<CRule, CPolicy>::iterator pos = (*it)->m_RulePolicyMap.begin();
					while (pos != (*it)->m_RulePolicyMap.end()) {
						memcpy(sBuffer + index, (char*)(*pos).second.m_StartTime, sizeof(LONG));
						index += sizeof(LONG);
						memcpy(sBuffer + index, (char*)(*pos).second.m_EndTime, sizeof(LONG));
						index += sizeof(LONG);
						memcpy(sBuffer + index, (char*)(*pos).first.m_ActionName.size(), sizeof(INT));
						index += sizeof(INT);
						memcpy(sBuffer + index, (*pos).first.m_ActionName.c_str(), (*pos).first.m_ActionName.size());
						index += (*pos).first.m_ActionName.size();
						memcpy(sBuffer + index, (char*)(*pos).first.m_TriggleExpr.size(), sizeof(INT));
						index += sizeof(INT);
						memcpy(sBuffer + index, (*pos).first.m_TriggleExpr.c_str(), (*pos).first.m_TriggleExpr.size());
						index += (*pos).first.m_TriggleExpr.size();
						pos++;
					}
					/*wbuf.len = index;
					nBytesSend = WSASend(pNewIoContext->m_sockAccept, &wbuf, 1, &dwBytes, dwFlags, p_ol, NULL);*/
					break;
				}
				it++;
			}
			break;	
		}
		case 3:{
			vector<CObject*>::iterator it = objList.begin();	
			while (it != objList.end()) {
				if (strcpy(const_cast<char*>((*it)->m_ObjectName.c_str()), "Fetion.exe") == 0) {
					map<CRule, CPolicy>::iterator pos = (*it)->m_RulePolicyMap.begin();
					while (pos != (*it)->m_RulePolicyMap.end()) {
						memcpy(sBuffer + index, (char*)(*pos).second.m_StartTime, sizeof(LONG));
						index += sizeof(LONG);
						memcpy(sBuffer + index, (char*)(*pos).second.m_EndTime, sizeof(LONG));
						index += sizeof(LONG);
						memcpy(sBuffer + index, (char*)(*pos).first.m_ActionName.size(), sizeof(INT));
						index += sizeof(INT);
						memcpy(sBuffer + index, (*pos).first.m_ActionName.c_str(), (*pos).first.m_ActionName.size());
						index += (*pos).first.m_ActionName.size();
						memcpy(sBuffer + index, (char*)(*pos).first.m_TriggleExpr.size(), sizeof(INT));
						index += sizeof(INT);
						memcpy(sBuffer + index, (*pos).first.m_TriggleExpr.c_str(), (*pos).first.m_TriggleExpr.size());
						index += (*pos).first.m_TriggleExpr.size();
						pos++;
					}
					/*wbuf.len = index;
					nBytesSend = WSASend(pNewIoContext->m_sockAccept, &wbuf, 1, &dwBytes, dwFlags, p_ol, NULL);*/
					break;
				}
				it++;
			}
			break;	
		}
		default:
			break;
	}
	int bytes_write = 0;
	int idx = 0;
	while (1)
	{
		bytes_write = send(pNewIoContext->m_sockAccept, sBuffer + idx, index, 0);
		if (bytes_write == -1) {
			return false;
		}
		else if (bytes_write == 0) {
			return false;
		}
		index -= bytes_write;
		idx = idx + bytes_write;
		if (index <= 0) {
			break;
		}
	}
	/*if ((SOCKET_ERROR == nBytesSend) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		return false;
	}*/

	pIoContext->ResetBuffer();
	return this->_PostAccept(pIoContext);
}

bool CIOCPModel::_PostSend(PER_IO_CONTEXT* pIoContext)
{
	DWORD dwBytes = 0;
	DWORD dwFlags = 0;
	WSABUF *p_wbuf = &pIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pIoContext->m_Overlapped;
	pIoContext->ResetBuffer();
	pIoContext->m_OpType = SEND_POSTED;
	//ZeroMemory(&(pIoContext->m_Overlapped), sizeof(OVERLAPPED));
	int nBytesSend = WSASend(pIoContext->m_sockAccept, p_wbuf, 1, &dwBytes, dwFlags, p_ol, NULL);
	if ((SOCKET_ERROR == nBytesSend) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		return false;
	}
	return true;
}

bool CIOCPModel::_PostRecv(PER_IO_CONTEXT* pIoContext)
{
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	WSABUF *p_wbuf = &pIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pIoContext->m_Overlapped;

	pIoContext->ResetBuffer();
	pIoContext->m_OpType = RECV_POSTED;

	int nBytesRecv = WSARecv(pIoContext->m_sockAccept, p_wbuf, 1, &dwBytes, &dwFlags, p_ol, NULL);

	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		return false;
	}
	return true;
}

bool CIOCPModel::_DoSend(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext)
{
	return _PostSend(pIoContext);
}

bool CIOCPModel::_DoRecv(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext, DWORD nLen)
{
	SOCKADDR_IN* ClientAddr = &pSocketContext->m_ClientAddr;
	
	INT dwFlag = -1;
	if (nLen > 0 && strlen(pIoContext->m_wsaBuf.buf) > 0) {
		m_pManage->ReceiveDataFromClient(pIoContext->m_wsaBuf.buf, dwFlag, nLen);
	}

	return _PostRecv(pIoContext);
}

bool CIOCPModel::_AssociateWithIOCP(PER_SOCKET_CONTEXT *pContext)
{
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)pContext->m_Socket, m_hIOCompletionPort, (DWORD)pContext, 0);

	if (NULL == hTemp)
	{
		return false;
	}

	return true;
}

void CIOCPModel::_AddToContextList(PER_SOCKET_CONTEXT *pHandleData)
{
	EnterCriticalSection(&m_csContextList);

	m_arrayClientContext.push_back(pHandleData);
	// m_arrayClientContext.Add(pHandleData);

	LeaveCriticalSection(&m_csContextList);
}

void CIOCPModel::_RemoveContext(PER_SOCKET_CONTEXT *pSocketContext)
{
	EnterCriticalSection(&m_csContextList);
	vector<PER_SOCKET_CONTEXT*>::iterator pos;
	pos = find(m_arrayClientContext.begin(), m_arrayClientContext.end(), pSocketContext);
	if (pos != m_arrayClientContext.end()) {
		RELEASE(pSocketContext);
		m_arrayClientContext.erase(pos);
	}
	/*for (int i = 0; i<m_arrayClientContext.GetCount(); i++)
	{
		if (pSocketContext == m_arrayClientContext.GetAt(i))
		{
			RELEASE(pSocketContext);
			m_arrayClientContext.RemoveAt(i);
			break;
		}
	}
*/
	LeaveCriticalSection(&m_csContextList);
}

void CIOCPModel::_ClearContextList()
{
	EnterCriticalSection(&m_csContextList);

	for (int i = 0; i < m_arrayClientContext.size(); i++) {
		delete m_arrayClientContext[i];
	}
	m_arrayClientContext.clear();

	/*for (int i = 0; i<m_arrayClientContext.GetCount(); i++)
	{
		delete m_arrayClientContext.GetAt(i);
	}

	m_arrayClientContext.RemoveAll();*/

	LeaveCriticalSection(&m_csContextList);
}

//CString CIOCPModel::GetLocalIP()
//{
//	char hostname[MAX_PATH] = { 0 };
//	gethostname(hostname, MAX_PATH);
//	struct hostent FAR* lpHostEnt = gethostbyname(hostname);
//	if (lpHostEnt == NULL)
//	{
//		return DEFAULT_IP;
//	}
//
//	LPSTR lpAddr = lpHostEnt->h_addr_list[0];
//
//	struct in_addr inAddr;
//	memmove(&inAddr, lpAddr, 4);
//	m_strIP = CString(inet_ntoa(inAddr));
//
//	return m_strIP;
//}

int CIOCPModel::_GetNoOfProcessors()
{
	SYSTEM_INFO si;

	GetSystemInfo(&si);

	return si.dwNumberOfProcessors;
}

//void CIOCPModel::_ShowMessage(const CString szFormat, ...) const
//{
//	CString   strMessage;
//	va_list   arglist;
//
//	va_start(arglist, szFormat);
//	strMessage.FormatV(szFormat, arglist);
//	va_end(arglist);
//
//	CMainDlg* pMain = (CMainDlg*)m_pMain;
//	if (m_pMain != NULL)
//	{
//		pMain->AddInformation(strMessage);
//	}
//}

bool CIOCPModel::_IsSocketAlive(SOCKET s)
{
	int nByteSent = send(s, "", 0, 0);
	if (-1 == nByteSent) return false;
	return true;
}

bool CIOCPModel::HandleError(PER_SOCKET_CONTEXT *pContext, const DWORD& dwErr)
{
	if (WAIT_TIMEOUT == dwErr)
	{
		if (!_IsSocketAlive(pContext->m_Socket))
		{
			this->_RemoveContext(pContext);
			return true;
		}
		else
		{
			return true;
		}
	}
	else if (ERROR_NETNAME_DELETED == dwErr)
	{
		this->_RemoveContext(pContext);
		return true;
	}
	else
	{
		return false;
	}
}

