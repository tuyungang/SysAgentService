#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <winsock2.h>
#include <MSWSock.h>
#include <windows.h>
#include <process.h>
#include <assert.h>
#include <vector>
#include "ProcDataDef.h"

using namespace std;
//#include "Util.h"
//#include "ManageThreadPool.h"

#pragma comment(lib,"ws2_32.lib")
// ���������� (1024*8)
// ֮����Ϊʲô����8K��Ҳ��һ�������ϵľ���ֵ
// ���ȷʵ�ͻ��˷�����ÿ�����ݶ��Ƚ��٣���ô�����õ�СһЩ��ʡ�ڴ�
// #define MAX_BUFFER_LEN        8192  
// Ĭ�϶˿�
// #define DEFAULT_PORT          12345    
// Ĭ��IP��ַ
// #define DEFAULT_IP            "127.0.0.1"


////////////////////////////////////////////////////////////////////
//// ����ɶ˿���Ͷ�ݵ�I/O����������
typedef enum _OPERATION_TYPE
{
	ACCEPT_POSTED,                     // ��־Ͷ�ݵ�Accept����
	SEND_POSTED,                       // ��־Ͷ�ݵ��Ƿ��Ͳ���
	RECV_POSTED,                       // ��־Ͷ�ݵ��ǽ��ղ���
	NULL_POSTED                        // ���ڳ�ʼ����������

}OPERATION_TYPE;

//====================================================================================
//
//				��IO���ݽṹ�嶨��(����ÿһ���ص������Ĳ���)
//
//====================================================================================

typedef struct _PER_IO_CONTEXT
{
	OVERLAPPED     m_Overlapped;                               // ÿһ���ص�����������ص��ṹ(���ÿһ��Socket��ÿһ����������Ҫ��һ��)              
	SOCKET         m_sockAccept;                               // ������������ʹ�õ�Socket
	WSABUF         m_wsaBuf;                                   // WSA���͵Ļ����������ڸ��ص�������������
	char           m_szBuffer[MAX_BUFFER_LEN];                 // �����WSABUF�������ַ��Ļ�����
	OPERATION_TYPE m_OpType;                                   // ��ʶ�������������(��Ӧ�����ö��)

	// ��ʼ��
	_PER_IO_CONTEXT()
	{
		ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));
		ZeroMemory(m_szBuffer, MAX_BUFFER_LEN);
		m_sockAccept = INVALID_SOCKET;
		m_wsaBuf.buf = m_szBuffer;
		m_wsaBuf.len = MAX_BUFFER_LEN;
		m_OpType = NULL_POSTED;
	}
	// �ͷŵ�Socket
	~_PER_IO_CONTEXT()
	{
		if (m_sockAccept != INVALID_SOCKET)
		{
			closesocket(m_sockAccept);
			m_sockAccept = INVALID_SOCKET;
		}
	}
	// ���û���������
	void ResetBuffer()
	{
		ZeroMemory(m_szBuffer, MAX_BUFFER_LEN);
	}

} PER_IO_CONTEXT, *PPER_IO_CONTEXT;

typedef struct _PER_SOCKET_CONTEXT
{
	SOCKET      m_Socket;                                  // ÿһ���ͻ������ӵ�Socket
	SOCKADDR_IN m_ClientAddr;                              // �ͻ��˵ĵ�ַ
	vector<_PER_IO_CONTEXT*> m_arrayIoContext;             // �ͻ���������������������ݣ�
	INT dwObjFlag;

	// Ҳ����˵����ÿһ���ͻ���Socket���ǿ���������ͬʱͶ�ݶ��IO�����

	// ��ʼ��
	_PER_SOCKET_CONTEXT()
	{
		m_Socket = INVALID_SOCKET;
		memset(&m_ClientAddr, 0, sizeof(m_ClientAddr));
		dwObjFlag = -1;
	}

	// �ͷ���Դ
	~_PER_SOCKET_CONTEXT()
	{
		if (m_Socket != INVALID_SOCKET)
		{
			closesocket(m_Socket);
			m_Socket = INVALID_SOCKET;
		}
		// �ͷŵ����е�IO����������
		for (int i = 0; i < m_arrayIoContext.size(); i++) {
			delete m_arrayIoContext[i];
		}
		m_arrayIoContext.clear();
		/*for (int i = 0; i<m_arrayIoContext.GetCount(); i++)
		{
			delete m_arrayIoContext.GetAt(i);
		}
		m_arrayIoContext.RemoveAll();*/
	}

	// ��ȡһ���µ�IoContext
	_PER_IO_CONTEXT* GetNewIoContext()
	{
		_PER_IO_CONTEXT* p = new _PER_IO_CONTEXT;

		//m_arrayIoContext.Add(p);
		m_arrayIoContext.push_back(p);

		return p;
	}

	// ���������Ƴ�һ��ָ����IoContext
	void RemoveContext(_PER_IO_CONTEXT* pContext)
	{
		assert(pContext != NULL);

		vector<_PER_IO_CONTEXT*>::iterator pos;
		pos = find(m_arrayIoContext.begin(), m_arrayIoContext.end(), pContext);
		if (pos != m_arrayIoContext.end()) {
			delete pContext;
			pContext = NULL;
			m_arrayIoContext.erase(pos);
		}
		/*for (int i = 0; i<m_arrayIoContext.GetCount(); i++)
		{
			if (pContext == m_arrayIoContext.GetAt(i))
			{
				delete pContext;
				pContext = NULL;
				m_arrayIoContext.RemoveAt(i);
				break;
			}
		}*/
	}

} PER_SOCKET_CONTEXT, *PPER_SOCKET_CONTEXT;

class CConnector;
class CManageThreadPool;
class CIOCPModel;
typedef struct _tagThreadParams_WORKER
{
	CIOCPModel* pIOCPModel;  
	int         nThreadNo;

} THREADPARAMS_WORKER, *PTHREADPARAM_WORKER;

class CThreadPool;
class CWorkThread;
class CIOCPModel
{
public:
	CIOCPModel(void);
	CIOCPModel(void *arg);
	~CIOCPModel(void);

public:

	// ����������
	bool Start();

	//	ֹͣ������
	void Stop();

	// ����Socket��
	bool LoadSocketLib();

	// ж��Socket�⣬��������
	void UnloadSocketLib() { WSACleanup(); }

	// ��ñ�����IP��ַ
	//CString GetLocalIP();

	// ���ü����˿�
	void SetPort(const int& nPort) { m_nPort = nPort; }

	// �����������ָ�룬���ڵ�����ʾ��Ϣ��������
	//void SetMainDlg(void* p) { m_pMain = p; }

//protected:
public:

	// ��ʼ��IOCP
	bool _InitializeIOCP();

	// ��ʼ��Socket
	bool _InitializeListenSocket();

	// ����ͷ���Դ
	void _DeInitialize();

	// Ͷ��Accept����
	bool _PostAccept(PER_IO_CONTEXT* pAcceptIoContext);

	// Ͷ�ݽ�����������
	bool _PostRecv(PER_IO_CONTEXT* pIoContext);

	bool _PostSend(PER_IO_CONTEXT* pIoContext);

	// ���пͻ��������ʱ�򣬽��д���
	bool _DoAccpet(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext);

	// ���н��յ����ݵ����ʱ�򣬽��д���
	bool _DoRecv(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext, DWORD nLen);

	bool _DoSend(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext);

	// ���ͻ��˵������Ϣ�洢��������
	void _AddToContextList(PER_SOCKET_CONTEXT *pSocketContext);

	// ���ͻ��˵���Ϣ���������Ƴ�
	void _RemoveContext(PER_SOCKET_CONTEXT *pSocketContext);

	// ��տͻ�����Ϣ
	void _ClearContextList();

	// ������󶨵���ɶ˿���
	bool _AssociateWithIOCP(PER_SOCKET_CONTEXT *pContext);

	// ������ɶ˿��ϵĴ���
	bool HandleError(PER_SOCKET_CONTEXT *pContext, const DWORD& dwErr);

	// �̺߳�����ΪIOCP�������Ĺ������߳�
	static unsigned int WINAPI CIOCPModel::_WorkerThread(LPVOID lpParam);

	// ��ñ����Ĵ���������
	int _GetNoOfProcessors();

	// �жϿͻ���Socket�Ƿ��Ѿ��Ͽ�
	bool _IsSocketAlive(SOCKET s);

	// ������������ʾ��Ϣ
	//void _ShowMessage(const CString szFormat, ...) const;

private:
	CConnector*                  m_pConnector;
	CManageThreadPool*			 m_pManage;
	CThreadPool*                 m_pThreadPool;
	CWorkThread*                 m_pWorkThread;
	HANDLE                       m_hShutdownEvent;              // ����֪ͨ�߳�ϵͳ�˳����¼���Ϊ���ܹ����õ��˳��߳�

	HANDLE                       m_hIOCompletionPort;           // ��ɶ˿ڵľ��

	HANDLE*                      m_phWorkerThreads;             // �������̵߳ľ��ָ��

	int		                     m_nThreads;                    // ���ɵ��߳�����

	//CString                      m_strIP;                       // �������˵�IP��ַ
	int                          m_nPort;                       // �������˵ļ����˿�

	//CManageThreadPool*           m_pMain;                       // ������Ľ���ָ�룬����������������ʾ��Ϣ

	CRITICAL_SECTION             m_csContextList;               // ����Worker�߳�ͬ���Ļ�����

	// CArray<PER_SOCKET_CONTEXT*>  m_arrayClientContext;          // �ͻ���Socket��Context��Ϣ        
	vector<PER_SOCKET_CONTEXT*>  m_arrayClientContext;

	PER_SOCKET_CONTEXT*          m_pListenContext;              // ���ڼ�����Socket��Context��Ϣ

	LPFN_ACCEPTEX                m_lpfnAcceptEx;                // AcceptEx �� GetAcceptExSockaddrs �ĺ���ָ�룬���ڵ�����������չ����
	LPFN_GETACCEPTEXSOCKADDRS    m_lpfnGetAcceptExSockAddrs;

};