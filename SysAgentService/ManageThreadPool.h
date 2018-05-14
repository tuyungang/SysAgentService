#pragma once

//#include <dbt.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <conio.h>
#include <list>
#include <vector>
#include <map>
/*#include <winsock2.h>
#include <MSWSock.h>
#include <windows.h>
#include <process.h>
*/
#include <assert.h>
#include <time.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include "Util.h"
//#include "Rules.h"
//#include "ManageConnector.h"
//#include "ManageIOCPModel.h"
#include "Packet.h"
#include "ManageRapidJson.h"

using namespace std;

class CThreadPool;

typedef struct event_tag
{
	event_tag(){
		memset(name , '\0', 10);
		tm = (time_t)0;
		type = -1;
	}
	char name[10];
	time_t tm;
	int type;
}event_tag;

typedef struct __tag1
{
	void * arg;
	void * arg1;
	HANDLE hdThread;
}__tag1;

typedef struct __tag2
{
	void * arg;
	void * arg1;
}__tag2;

class CWorkThread
{
public:
	CWorkThread();
	CWorkThread(void *arg, volatile LONG &num, void *tag);
	CWorkThread(void *arg, volatile LONG &num);
	~CWorkThread();

	/*typedef struct __tag1
	{
		void * tag;
		HANDLE hdThread;
	}__tag1;*/

	BOOL IsBusy() { return m_Busy; }													// 是否有任务在执行
	//void ExecuteTask(TaskFun task, PVOID param, TaskCallbackFun taskCallback);	// 执行任务
	void Start(void *proc, void *arg = NULL);

private:
	//static unsigned CALLBACK WorkThreadCallBack();
	BOOL IsExit();
	void Run(HANDLE hThread);

private:
	CThreadPool *m_ThreadPool;											// 所属线程池
	BOOL	m_Busy;													// 是否有任务在执行
	BOOL    m_Exit;													// 是否退出
	HANDLE  m_HandleThr;													// 线程句柄
	//TaskFun	TaskFunCb;													// 要执行的任务
	PVOID	m_Param;													// 任务参数
	//TaskCallbackFun TaskCallFunCb;											// 回调的任务
	static unsigned int __stdcall ThreadProc(PVOID arg);				// 线程函数
};

//template <typename C>
//class CThreadSafeQueue : protected list<C>
//{
//public:
//	CThreadSafeQueue(HANDLE evEmpty = NULL, stdCallback cbOnEmpty = NULL, void * cbParam = NULL)
//	{
//		//手动触发，必须一个个来激活
//		m_ev = ::CreateEvent(NULL, FALSE, FALSE, NULL);
//		InitCS(&m_Crit);
//
//		m_evEmpty = evEmpty;
//		m_onEmpty = cbOnEmpty;
//		m_cbParam = cbParam;
//	}
//	~CThreadSafeQueue()
//	{
//		DestCS(&m_Crit);
//		::CloseHandle(m_ev);
//		m_ev = NULL;
//	}
//
//	void push_front(C& c)
//	{
//		CSLock lock(m_Crit);
//		__super::push_front(c);
//		::SetEvent(m_ev);
//	}
//
//	void push(C& c, bool high_priv = false)
//	{
//		CSLock lock(m_Crit);
//		if (!high_priv)
//			push_back(c);
//		else
//			__super::push_front(c);
//		::SetEvent(m_ev);
//	}
//
//	bool pop(C& c)
//	{
//		CSLock lock(m_Crit);
//		if (__super::empty())
//			return false;
//		c = front();
//		pop_front();
//		if (!_empty())
//			::SetEvent(m_ev);	//发信号，激活下一个
//		return true;
//	}
//
//	void push_n(list<C>& li)
//	{
//		CSLock lock(m_Crit);
//		splice(end(), li);
//		::SetEvent(m_ev);
//	}
//
//	void pop_all(list<C>& li)
//	{
//		CSLock lock(m_Crit);
//		li.clear();
//		swap(li);
//		::ResetEvent(m_ev);
//	}
//
//	//最后清理，不需要唤醒等待的线程
//	void clear()
//	{
//		CSLock lock(m_Crit);
//		::ResetEvent(m_ev);
//		__super::clear();
//	}
//	bool empty()
//	{
//		CSLock lock(m_Crit);
//		return __super::empty();
//	}
//	bool _empty()
//	{
//		bool empty = __super::empty();
//		if (empty)
//		{
//			if (m_evEmpty)
//				::SetEvent(m_evEmpty);
//			if (m_onEmpty)
//				m_onEmpty(m_cbParam);
//		}
//		return __super::empty();
//	}
//
//	template<typename T>
//	void clear_invalid(const T & arg, bool(*isvalid)(const C & item, const T &))
//	{
//		CSLock lock(m_Crit);
//		list<C>::iterator it = begin();
//		for (; it != end();)
//			it = isvalid(*it, arg) ? ++it : erase(it);
//	}
//
//	//获得句柄
//	HANDLE GetWaitHandle() { return m_ev; }
//protected:
//	HANDLE m_ev;
//	HANDLE m_evEmpty;
//	CRITICAL_SECTION m_Crit;
//	stdCallback	m_onEmpty;
//	void * m_cbParam;
//};

class CManageThreadPool;
class CSLock;
class CThreadPool
{
	friend class CWorkThreak;
public:
	static const int MAX_OBJECTS = 2;
	typedef enum PoolState { pool_closed, pool_closing, pool_opening, pool_running } POOLSTATE;
	typedef enum ThreadState { thread_running, thread_sleep, thread_blocking } THREADSTATE;

private:
	CThreadPool(void *arg = NULL);
	CThreadPool(int num, void *arg = NULL);
/*
public:
	CThreadPool(void *arg = NULL);
	CThreadPool(int num, void *arg = NULL);
	~CThreadPool();

*/
protected:

public:
	~CThreadPool();
	static CThreadPool *GetInstance(int threadNum, void *arg, void *arg1)
	{
		/*CManageThreadPool *t = (CManageThreadPool*)arg;
		t->FtpUploadVideoDataProc(NULL);*/
		InitCS(&m_LockCS);
		::InterlockedIncrement(&m_NumOfObjects);
		if (::InterlockedDecrement(&m_NumOfObjects) < MAX_OBJECTS) {
			CSLock lock(m_LockCS);
			if (threadNum == 0) {
				pInstance = new CThreadPool(arg1);
				::InterlockedIncrement(&m_NumOfObjects);
			}
			else {
				pInstance = new CThreadPool(threadNum, arg);
				::InterlockedIncrement(&m_NumOfObjects);
			}
			return pInstance;
		}
		else {
			throw MyException();
			//return NULL;
		}
	}

	class MyException : public exception
	{
	public:
		MyException() : exception(){}
		const char *what()const throw() { return "ERROR!, only one instance is allowed!"; }
	};

	class _FreeInstancee
		{
		public:
			~_FreeInstancee()
			{
				if (CThreadPool::pInstance) {
					delete CThreadPool::pInstance;
				}
			}
		};
		static _FreeInstancee _free;
		static CRITICAL_SECTION  m_LockCS;
private:
	static CThreadPool *pInstance;
	CManageThreadPool *m_Manage;
	//CIOCPModel *m_iocp;


	static volatile unsigned int m_NumOfObjects;
	//static CRITICAL_SECTION  m_LockCS;

	list<CWorkThread*> m_IdleThreadList;
	/*list<CWorkThread*> m_BusyThreadListLock;
	list<CWaitTask*> m_WaitTaskList;
	list<CTimeTask*> m_TimeTaskList;
	list<CTask*> m_TaskList;*/
	//CThreadSafeQueue<CTask*> m_TaskQueue;

	CRITICAL_SECTION m_WaitTaskListLock;
	CRITICAL_SECTION m_IdleThreadListLock;
	CRITICAL_SECTION m_BusyThreadListLock;
	CRITICAL_SECTION m_TaskListLock;
	CRITICAL_SECTION m_TimeTaskListLock;

	HANDLE					m_StopEvent;
	volatile POOLSTATE      m_PoolState;
	volatile LONG           m_ThreadNum;
	volatile LONG           m_nServerThreadNum;
	volatile LONG           m_nClientThreadNum;
	int m_MinThreadNums;
	int m_MaxThreadNums;

public:
	void PutTask();
	void WaitOnQuit()
	{
		if (Closed())
		{
			//log("WaitOnQuit::closed, return\n");
			return;
		}
		bool need_wait = m_ThreadNum > 0 ? true : false;
		Close();
		if (need_wait)
		{
			//log("WaitOnQuit::need wait threads quits\n");
			//WaitForSingleObject(m_evQuit, INFINITE);
			//log("WaitOnQuit::all threads quits\n");
		}
	}
	
	void CThreadPool::RunBeforeStart()
	{
		//InitializeCriticalSection(&g_cs_log);
	}
	void CThreadPool::RunAfterEnd()
	{
		//DeleteCriticalSection(&g_cs_log);
	}

	void CreateIdleThread(void *proc);

private:
	BOOL Init(void *arg, int num = 0, void *arg1 = NULL);
	bool Closed() const { return m_PoolState == pool_closed; }
	bool Running() const { return m_PoolState == pool_running || m_PoolState == pool_opening; }
	void Close();
	bool Empty();
	// void CreateIdleThread(void *proc);

	void MoveIdleThreadToBusyThreadList();
	void MoveBusyThreadToIdleThreadList();
	void PushTaskList();
	void PopTaskList();
	void PushIdleThreadList();
	void PopIdleThreadList();
	void PushBusyThreadList();
	void PopBusyThreadList();
	void PushTimeTaskList();
	void PopTimeTaskList();
	void PushWaitTaskList();
	void PopWaitTaskList();

};
class CCRapidJson;
class CRuleGenerator
{
public:
	CRuleGenerator() :js(){}
	CRuleGenerator(void *arg);
	~CRuleGenerator();
private:
	//CManageThreadPool *pManage;
	CCRapidJson js;
public:
	BOOL GenerateObject(char *data, vector<CObject*> &objList);
	//BOOL GenerateObject(char *data);
	BOOL GenerateDataToServer(PCHAR type, void *in_data, string &out_data, void *buf = NULL, UINT bufLen = 0);
};

class CObject;

//class CRapidJson;
class CConnector;
class CIOCPModel;
//class CRuleGenerator;
class CManageThreadPool
{
private:
	CManageThreadPool();
	CManageThreadPool(int num, void *arg = NULL);
	// CManageThreadPool(const CManageThreadPool &);
	// CManageThreadPool & operator = (const CManageThreadPool&)const;

public:
	~CManageThreadPool()
	{
		if (NULL != m_ThreadPool_clientPtr) {
			delete m_ThreadPool_clientPtr;
		}
		if (NULL != m_ThreadPool_serverPtr) {
			delete m_ThreadPool_serverPtr;
		}
		DestGlobalCS();
		DestCS(&m_LockCS);
	}

	static CManageThreadPool *CreateInstance()
	{
		InitGlobalCS();
		InitCS(&m_LockCS);
		if (pInstance == NULL) {
			CSLock lock(m_LockCS);
			if (pInstance == NULL) {
				pInstance = new CManageThreadPool();
			}
		}
		return pInstance;
	}

	class _FreeInstancee
	{
	public:
		~_FreeInstancee()
		{
			if (CManageThreadPool::pInstance) {
				delete CManageThreadPool::pInstance;
			}
		}
	};
	static _FreeInstancee _free;
	static CRITICAL_SECTION  m_LockCS;
	HANDLE  m_EndRecordDesktopEvent;
	//CRuleGenerator m_pRuelGenerator;
	vector<CObject*>  m_MonitorObjectList;
private:
	static CManageThreadPool *pInstance;
	//static CRITICAL_SECTION  m_LockCS;
	static volatile unsigned int m_NumOfObjects;

	CThreadPool    *m_ThreadPool_clientPtr;
	CThreadPool    *m_ThreadPool_serverPtr;
	CSysConfig     *m_SysConfigPtr;
	CIOCPModel     *m_IOCPModelPtr;
	CRuleGenerator m_pRuelGenerator;
	//CRapidJson  m_pRapidJson;

	//list<CConnector*> m_ConnServerList;
	list<CConnector*> m_ConnClientList;
	vector<string>    m_MonitorDataList;
	vector<char *>    m_RuleDataStringList;
	vector<char *>    m_SaveLogInfoList;
	vector<string>    m_ReadyUploadVideoList;
	//vector<CObject*>  m_MonitorObjectList;
	//map<CRules*, string> m_pCaptureRuleInfoMap;
	//map<event_tag*, time_t> m_IdenticalDataReadyMap;

	CRITICAL_SECTION m_ConnClientListLock;
	CRITICAL_SECTION m_MonitorDataListLock;
	CRITICAL_SECTION m_RuleDataStringListLock;
	CRITICAL_SECTION m_SaveLogInfoListLock;
	CRITICAL_SECTION m_ReadyUploadVideoListLock;
	// CRITICAL_SECTION m_CaptureRulesInfoMapLock;
	// CRITICAL_SECTION m_IdenticalDataReadyMapLock;

	static volatile LONG  m_IsRecordDesktoping;
	static volatile LONG  m_nOnlyOneRequestVar;
	static volatile LONG  m_nGetRuleFromLocalVar;
	HANDLE  m_ExitEvent;
	HANDLE  m_ConnEvent;
	HANDLE  m_MonitorDataListEvent;
	// HANDLE  m_IdenticalDataReadyEvent;
	HANDLE  m_GetRuleFromLocalEvent;
	HANDLE  m_SaveLogInfoListEvent;
	HANDLE  m_LoadRuleReadyEvent;
	HANDLE  m_StartRecordDesktopEvent;
	//HANDLE  m_EndRecordDesktopEvent;
	HANDLE  m_StartLiveVideoEvent;
	HANDLE  m_EndLiveVideoEvent;
	HANDLE  m_StartUploadVideoEvent;
	HANDLE  m_SuccessConnectServerEvent;

	HANDLE hTimerQueue;
	HANDLE hTimer;

public:
	char g_sWebMainServerIP[50];
	char g_sWebStandbyServerIP[50];
	char g_sWebServerPort[20];
	char g_sFtpServerIP[50];
	char g_sFtpUserNamePsw[50];
	char g_sGuard_qq_dll[256];
	//WCHAR g_wGuard_qq_dll[20];
	char g_sGuard_wechat_dll[256];
	char g_sGuard_fetion_dll[256];
	char g_sVideo_save_path[256];

public:
	BOOL RunServer(void *arg = NULL);
	void ExitServer();
	void StartServerThreadPool();
	void StartClientThreadPool(int num);
	volatile LONG & GetRuleVar() { return m_nGetRuleFromLocalVar; }
	unsigned GetRuleVarVaule() {
		::InterlockedIncrement(&m_nGetRuleFromLocalVar);
		return ::InterlockedDecrement(&m_nGetRuleFromLocalVar); 
	}
	HANDLE GetRuleFromLocalEventHandle() { return m_GetRuleFromLocalEvent; }
	LONG IncrementVar(volatile LONG &var)
	{
		::InterlockedIncrement(&var);
		::InterlockedIncrement(&var);
		return ::InterlockedDecrement(&var);
	}
	LONG DecrementVar(volatile LONG &var)
	{
		::InterlockedDecrement(&var);
		::InterlockedIncrement(&var);
		return ::InterlockedDecrement(&var);
	}
	void CreateWorkerThreadPool(PVOID arg);

	static unsigned int __stdcall CommunicateWithServerProc(PVOID arg);
	// static unsigned int __stdcall CommunicateWithClientProc(PVOID arg);
	static unsigned int __stdcall MonitorProcessProc(PVOID arg);
	static unsigned int __stdcall FtpUploadVideoDataProc(PVOID arg);
	static unsigned int __stdcall RecordDesktopProc(PVOID arg);
	static unsigned int __stdcall RecordLogProc(PVOID arg);
	static unsigned int __stdcall ReadCameraLiveVideoProc(PVOID arg);
	static unsigned int __stdcall MonitorUsbDriveProc(PVOID arg);
	//VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired);

	//LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL ReceiveDataFromClient(PCHAR data, INT &dwFlag, ULONG nlen = 0);
	vector<CObject*> & GetMonitorObjectList() { return m_MonitorObjectList; }
	bool DispatchDataToHandleProc(INT command, char *data);
	BOOL DispatchObjProcessToHandleProc(void *arg, INT &n);

	void MapKeyToHandle(); 	
	void MapKeyToHandleInject();
	void Log(char *pszFormat, ...);

private:
	void GetSysConfigInfo();
	void GetRulesInfoFromLocal();
	void SaveCurrentRulesToLocal();
	// void DispatchDataToHandleProc(PPKG_HEAD_STRUCT pHead, char *data);
	BOOL SendRuleRequestToServer(SOCKET sockfd);
	BOOL SendRuleUpdateRequestToServer(SOCKET sockfd);
	BOOL SendMonitorDataToServer(SOCKET sockfd, string &data);
	BOOL ReceiveDataFromServer(SOCKET sockfd, char *buffer, LONG &idx, PPKG_HEAD_STRUCT pHead, char *retBuffer, BOOL &isEnough);
	void ParseJsonDataFromServer(char *data);
	BOOL WaitForSendMonitorDataToServer();
	//LRESULT OnDeviceChange(HWND hwnd, WPARAM wParam, LPARAM lParam);

//private:
public:
	//bool DispatchDataToHandleProc(PPKG_HEAD_STRUCT pHead, char *data);
	//bool DispatchDataToHandleProc(INT command , char *data);
	BOOL HandleProc(char *data) { return TRUE; }
	BOOL HandleRuleProc(char *data);
	BOOL HandleConfigFileProc(char *data);	
	BOOL HandleExpiredTimeProc(char *data);
	BOOL HandleUpdateRuleProc(char *data);
	BOOL DispatchInjectHandleProc(void *arg);
	BOOL HandleInjectViaEasyHookProc(void *data);
	BOOL HandleInjectViaWindowsProc(void *data);
	BOOL HandleInjectViaManualProc(void *data);
	/*int DoInject(DWORD aPid, const WCHAR *aFullpath);
	BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
	BOOL InjectDll(DWORD dwPID, const wchar_t *szDllName);
	BOOL MyCreateRemoteThread(HANDLE hProcess, LPTHREAD_START_ROUTINE pThreadProc, LPVOID pRemoteBuf);*/

	BOOL HandleQQProcessProc(void *data, INT &n);
	BOOL HandleWeChatProcessProc(void *data, INT &n);
	BOOL HandleFetionProcessProc(void *data, INT &n);
public:
	typedef BOOL (CManageThreadPool::*pHandleProc)(char *data);  
	map<int, pHandleProc> m_KeyToHandleProcMap;
	typedef BOOL (CManageThreadPool::*pHandleInjectProc)(void *data);  
	map<int, pHandleInjectProc> m_KeyToHandleInjectProcMap;
	typedef BOOL(CManageThreadPool::*pHandleObjProc)(void *data, INT &n);
	map<int, pHandleObjProc> m_KeyToHandleObjProcMap;

};