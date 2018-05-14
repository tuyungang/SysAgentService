#pragma once
// #pragma warning (disable:4793)

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
//#include <conio.h>
#include <tchar.h>
#include <errno.h>
#include <assert.h>
#include <vector>
#include <memory>
#include <list>
#include <map>

#include <winsock2.h>
#include <MSWSock.h>
#include <windows.h>

#include <process.h>
#include <strsafe.h>
#include <Shlwapi.h>
#include <tlhelp32.h>  
#include <atlstr.h>    
#include <locale.h>
#include <winuser.h>
#include <Psapi.h>
#include <ntstatus.h> 
//#include "cryptdatafunc.h"
//#include "ManageSqlite.h"
#include "NetFX4.0/easyhook.h"
#include "ProcDataDef.h"

using namespace std;

#pragma comment(lib,"shlwapi.lib")
#pragma comment ( lib,"user32.lib" )  
#pragma comment(lib,"psapi.lib")
#pragma warning (disable:4793)
#pragma warning(disable: 4996) 

#if _WIN64
#pragma comment(lib, "NetFX4.0/EasyHook64.lib")
#pragma comment(lib, "curl_release/64/libcurl.lib")
#else
#pragma comment(lib, "NetFX4.0/EasyHook32.lib")
#pragma comment(lib, "curl_release/32/libcurl.lib")
#endif

char *dup_wchar_to_utf8(const wchar_t *w);
void show_dshow_device();
void show_dshow_device_option();
void show_vfw_device();
void show_avfoundation_device();
//void Filesearch(string Path, int Layer);
//void CopyPPT(vector<FileInfo> FileList);
//void CopyDOC(vector<FileInfo> FileList);
void GetProcessNum(int & logicNum, int & procCore);
// BOOL CheckFolderExist();

//sqlite3
int OpenDB(const char *path);
void GetSysConfigInfoFromDB();
void InsertSysConfigInfoIntoDB();
void UpdateSysConfigInfoFromDB();
void GetRulesInfoFromDB();
void InsertRulesInfoIntoDB();
void UpdateRulesInfoFromDB();
void GetAgentExpiredDateFromDB(time_t &tm);
void InsertAgentExpiredDateIntoDB();
void UpdateAgentExpiredDateFromDB();
void GetSensitiveInfoFromDB();
void InsertSensitiveInfoIntoDB();
void DeleteSensitiveInfoFromDB();
void CloseDB();
/*
int opendatabase(const char *path);
char** getinfofromblacklist(const char *sql, int &num);
void getnumbersofrowsfromblacklist(int &sum);
void addnewintoblacklist(const char *ip, const char *reason);
void deleteenpiredinblacklist(const char *ip);
int checkenpiredinblacklist();
void closedatabase();
*/
void recordlog(char *arg);
typedef void(*SimpleCallback)(void* user_data);
typedef void (__stdcall * stdCallback)(void* data);

//ÈÎÎñ±»Çå³ýµÄÊ±ºòµÄ»Øµ÷
//@param not_running: true, ±íÊ¾Î´ÔËÐÐÒÑ±»³·Ïú£» false±íÊ¾ÈÎÎñÒÑ¾­ÔÚÔËÐÐ£¬ÕâÊ±ºòÐèÒª×ö³·Ïú£¨¸Ä±äuser_dataµÄÏà¹Ø±äÁ¿£©
typedef void (* ClearCallback)(bool not_running, void* user_data);
typedef int(*TaskFun)(PVOID param);				// ÈÎÎñº¯Êý
typedef void(*TaskCallbackFun)(int result);		// »Øµ÷º¯Êý

extern CRITICAL_SECTION g_cs_log;

//#ifdef _UNICODE
//#define MyOutputDebugString MyOutputDebugStringW
//#else
//#define MyOutputDebugString MyOutputDebugStringA
//#endif

#define USE_COUT  1

//#ifdef USE_COUT
//#include<iostream>
//#endif

//ÉèÖÃ¿ØÖÆÌ¨Êä³öÑÕÉ«
inline BOOL SetConsoleColor(WORD wAttributes)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE)
		return FALSE;
	return SetConsoleTextAttribute(hConsole, wAttributes);
}

inline size_t now_s(char * buf, size_t len)
{
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	return sprintf_s(buf, len, "%d-%d-%d %d:%d:%d.%d\t",
		sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
}

inline char * now()
{
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	static char TS[255] = "";
	sprintf_s(TS, 255, "%d-%d-%d %d:%d:%d.%d\t",
		sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
	return &TS[0];
}

#ifdef _DEBUG
inline void printlogc(WORD color, char *pszFormat, ...)
{
	va_list   pArgList;
	va_start(pArgList, pszFormat);
	EnterCriticalSection(&g_cs_log);
	if (color != 0 && color != COLOR_WHITE)
		SetConsoleColor(color);
	char buf[1024];
	size_t n = now_s(buf, 1024);
	vsnprintf_s(buf + n, 1024 - n, _TRUNCATE, pszFormat, pArgList);
//#ifdef USE_COUT
//	std::cout << buf;
//#else
//	fprintf(stdout, buf);
//#endif
#if USE_COUT
	//std::cout << buf;

#else
	fprintf(stdout, buf);
#endif
	if (color != 0 && color != COLOR_WHITE)
		SetConsoleColor(COLOR_WHITE);
	LeaveCriticalSection(&g_cs_log);
	va_end(pArgList);
}

inline void printlog(char *pszFormat, ...)
{
	va_list   pArgList;
	va_start(pArgList, pszFormat);
	EnterCriticalSection(&g_cs_log);
	char buf[1024];
	size_t n = now_s(buf, 1024);
	vsnprintf_s(buf + n, 1024 - n, _TRUNCATE, pszFormat, pArgList);
	va_end(pArgList);
	/*wchar_t szPrint[4096 + 100] = { 0 };
	wchar_t *szFilter = L"[sunflover] ";
	wcscpy(szPrint, szFilter);
	wcscat(szPrint, szBuffer);*/
	int iLen = 0;
	iLen = MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
	TCHAR * pBuf = new TCHAR[iLen + 1];
	MultiByteToWideChar(CP_ACP, 0, buf, -1, pBuf, iLen);
	wcscat(pBuf, L"\n");
	/*int iLen = WideCharToMultiByte(CP_ACP, 0, psz, -1, NULL, 0, NULL, NULL);
	char *tu = new char[iLen*sizeof(char)];
	WideCharToMultiByte(CP_ACP, 0, psz, -1, tu, iLen, NULL, NULL);*/
	OutputDebugString(pBuf);
//#ifdef USE_COUT
//	std::cout << buf;
//#else
//	fprintf(stdout, buf);
//#endif
#if USE_COUT
	//std::cout << buf;

#else
	// fprintf(stdout, buf);
#endif
	LeaveCriticalSection(&g_cs_log);
	//va_end(pArgList);
}
inline void printlogA(const char * szOutputString, ...)
{
#if USE_COUT
	char szBuffer[4096] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, szOutputString);
	_vsnprintf(szBuffer, sizeof(szBuffer) - 1, szOutputString, vlArgs);
	//vsprintf(szBuffer,szOutputString,vlArgs);
	va_end(vlArgs);
	char szPrint[4096 + 100] = { 0 };
	char *szFilter = "[error] ";
	strcpy(szPrint, szFilter);
	strcat(szPrint, szBuffer);
	OutputDebugStringA(szPrint);
#endif
}

inline void printlogW(const wchar_t * szOutputString, ...)
{
#if USE_COUT
	wchar_t szBuffer[4096] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, szOutputString);
	_vsnwprintf(szBuffer, sizeof(szBuffer) - 2, szOutputString, vlArgs);
	//vswprintf(szBuffer,szOutputString,vlArgs);
	va_end(vlArgs);
	wchar_t szPrint[4096 + 100] = { 0 };
	wchar_t *szFilter = L"[error] ";
	wcscpy(szPrint, szFilter);
	wcscat(szPrint, szBuffer);
	OutputDebugStringW(szPrint);
#endif
}
#else

#define printlogW(...)
#define printlogA(...)
#define printlog(...)
#define printlogc(...)

#endif

////临界区同步类
//class CThreadLock
//{
//	//变量定义
//private:
//	CRITICAL_SECTION                m_csLock;            //临界区变量
//
//	//函数定义
//public:
//	//构造函数
//	inline CThreadLock() { ::InitializeCriticalSection(&m_csLock); };
//	//析构函数
//	inline ~CThreadLock() { ::DeleteCriticalSection(&m_csLock); };
//
//	//功能函数
//public:
//	//锁定函数
//	inline void Lock() { ::EnterCriticalSection(&m_csLock); };
//	//解锁函数
//	inline void UnLock() { ::LeaveCriticalSection(&m_csLock); };
//};
//
//////////////////////////////////////////////////////////////////////////////
//
////安全同步锁定句柄
//class CThreadLockHandle
//{
//	//变量定义
//private:
//	int                                m_nLockCount;        //锁定计数
//	CThreadLock                        *m_pThreadLock;        //锁定对象
//
//	//函数定义
//public:
//	//构造函数
//	CThreadLockHandle(CThreadLock *pThreadLock, bool bAutoLock = true);
//	//析构函数
//	virtual ~CThreadLockHandle();
//
//	//功能函数
//public:
//	//锁定函数
//	void Lock();
//	//解锁函数
//	void UnLock();
//	//火枪锁定次数
//	int inline GetLockCount() { return m_nLockCount; };
//};
//
////安全同步锁定句柄
////构造函数
//CThreadLockHandle::CThreadLockHandle(CThreadLock *pThreadLock, bool bAutoLock)
//{
//	assert(pThreadLock != NULL);
//	m_nLockCount = 0;
//	m_pThreadLock = pThreadLock;
//	if (bAutoLock)    Lock();
//	return;
//}
////析构函数
//CThreadLockHandle::~CThreadLockHandle()
//{
//	while (m_nLockCount>0) UnLock();                                //生存周期结束自动解锁
//}
//
////锁定函数
//void CThreadLockHandle::Lock()
//{
//	//校验状态
//	assert(m_nLockCount >= 0);
//	assert(m_pThreadLock != NULL);
//
//	//锁定对象
//	m_nLockCount++;
//	m_pThreadLock->Lock();
//}
//
////解锁函数
//void CThreadLockHandle::UnLock()
//{
//	//校验状态
//	assert(m_nLockCount>0);
//	assert(m_pThreadLock != NULL);
//
//	//解除状态
//	m_nLockCount--;
//	m_pThreadLock->UnLock();
//}

inline void InitCS(CRITICAL_SECTION * cs){
	if (!::InitializeCriticalSectionAndSpinCount(cs, 4000))
		::InitializeCriticalSection(cs);
}

inline void DestCS(CRITICAL_SECTION * cs){
	::DeleteCriticalSection(cs);
}
inline void InitGlobalCS()
{
	InitCS(&g_cs_log);
}
inline void DestGlobalCS()
{
	DestCS(&g_cs_log);	
}

class CSLock
{
public:
	CSLock(
		_Inout_ CRITICAL_SECTION& cs,
		_In_ bool bInitialLock = true,
		_In_ bool bInitialcs = false);
	~CSLock() throw();
	void Lock();
	void Unlock() throw();
private:
	CRITICAL_SECTION& m_cs;
	bool m_bLocked;
	// Private to avoid accidental use
	CSLock(_In_ const CSLock&) throw();
	CSLock& operator=(_In_ const CSLock&) throw();
};

inline CSLock::CSLock(
	_Inout_ CRITICAL_SECTION& cs,
	_In_ bool bInitialLock,
	_In_ bool bInitialcs) :
	m_cs(cs), m_bLocked(false)
{
	if (bInitialcs)
		InitCS(&m_cs);
	if (bInitialLock)
		Lock();
}
inline CSLock::~CSLock() throw()
{
	if (m_bLocked)
		Unlock();
}
inline void CSLock::Lock()
{
	::EnterCriticalSection(&m_cs);
	m_bLocked = true;
}
inline void CSLock::Unlock() throw()
{
	::LeaveCriticalSection(&m_cs);
	m_bLocked = false;
}

class CLog
{
public:
	typedef enum {ERR_NONE = 0, ERR_MINOR = -1001, ERR_MAJOR = -1002, ERR_CRITICAL = 1003, ERR_BLOCKER = -1004 } LOG_ERROR_LEVEL;
public:
	CLog();
	~CLog();
	
private:
	CLog(_In_ const CLog&) throw();
	CLog& operator=(_In_ const CLog&) throw();

	vector<string> *m_LogCacheList;
	CRITICAL_SECTION m_LogCS;

protected:

public:
	void Log(char *logInfo, LOG_ERROR_LEVEL err);
	void PushLogInQueue(string logInfo);
	void PopLogOutQueue();
};

class CSysConfig
{
public:
	typedef enum { FOR_ITEM1 = 1, FOR_ITEM2, FOR_ITEM3 }UserType;
public:
	CSysConfig(){}
	~CSysConfig(){}

private:
	char m_Main_ServerIP[20];
	char m_Standby_ServerIP[20];
	int m_Port;
	volatile UserType m_UserType;
	volatile long m_ExpireDate;
	volatile BOOL m_RuleFileExistState;
	volatile BOOL m_LisenceFileExistState;
public:
	BOOL GetSysConfigInfoFromLocal();
	BOOL GetRulesInfoFromLocal();
	BOOL SaveCurrentRulesInfoToLocal();
	BOOL CheckFolderExist();
	BOOL GetRuleFileExistState() { return m_RuleFileExistState; }
	BOOL GetLisenceFileExistState() { return m_LisenceFileExistState; }
};
