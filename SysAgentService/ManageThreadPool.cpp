#include "ManageThreadPool.h"
#include "ManageIOCPModel.h"
#include "Util.h"
#include "ManageConnector.h"
#include "Packet.h"
#include "ManageRapidJson.h"
#include "ManageRules.h"
#include <dbt.h>
#include <windows.h>
#include <curl/curl.h>
#include "ini_config.h"

CManageThreadPool * CManageThreadPool::pInstance = NULL;
volatile unsigned int CManageThreadPool::m_NumOfObjects = 0;
volatile LONG  CManageThreadPool::m_nOnlyOneRequestVar = 0;
CThreadPool * CThreadPool::pInstance = NULL;
volatile unsigned int CThreadPool::m_NumOfObjects = 0;
CRITICAL_SECTION  CManageThreadPool::m_LockCS = {0};
CRITICAL_SECTION  CThreadPool::m_LockCS = {0};
volatile LONG  CManageThreadPool::m_nGetRuleFromLocalVar = 0;
volatile LONG CManageThreadPool::m_IsRecordDesktoping = 0;
CHAR U[2];//保存U盘的盘符
static BOOL tu = FALSE;
CHAR g_sTimeBuf[50];

CRuleGenerator::CRuleGenerator(void *arg)
{
	//pManage = (CManageThreadPool*)arg;
	js = new CCRapidJson(this);
}

CRuleGenerator::~CRuleGenerator()
{

}

BOOL CRuleGenerator::GenerateObject(char *data, vector<CObject*> &obj)
//BOOL CRuleGenerator::GenerateObject(char *data)
{
	assert(data != NULL);
	js.ParseJson(data, obj);

	return TRUE;
}

BOOL CRuleGenerator::GenerateDataToServer(PCHAR type, void *in_data, string &out_data, void *buf, UINT bufLen)
{
	assert(in_data != NULL);
	js.GenerateJson(type, in_data, out_data, buf, bufLen);

	return TRUE;
}

CManageThreadPool::CManageThreadPool()
{
	InitCS(&m_ConnClientListLock);
	//InitCS(&m_MonitorDataListLock);
	::InitializeCriticalSection(&m_MonitorDataListLock);
	::InitializeCriticalSection(&m_ReadyUploadVideoListLock);
	InitCS(&m_SaveLogInfoListLock);
	m_ThreadPool_clientPtr = NULL;
	m_ThreadPool_serverPtr = NULL;
	m_SysConfigPtr = NULL;
	m_IOCPModelPtr = NULL;

	m_ExitEvent = NULL;
	m_ConnEvent = NULL;
	m_MonitorDataListEvent = NULL;
	m_GetRuleFromLocalEvent = NULL;
	m_SaveLogInfoListEvent = NULL;
	m_LoadRuleReadyEvent = NULL;
	m_StartRecordDesktopEvent = NULL;
	m_StartLiveVideoEvent = NULL;
	m_EndLiveVideoEvent = NULL;
	m_StartUploadVideoEvent = NULL;

	hTimerQueue = NULL;
	hTimer = NULL;

	/*memset(g_sWebMainServerIP, 0, 50);
	memset(g_sWebStandbyServerIP, 0, 50);
	memset(g_sWebServerPort, 0, 50);
	memset(g_sFtpServerIP, 0, 50);
	memset(g_sFtpUserNamePsw, 0, 50);
	memset(g_sGuard_qq_dll, 0, 50);
	memset(g_sGuard_wechat_dll, 0, 50);
	memset(g_sGuard_fetion_dll, 0, 50);*/

	m_SysConfigPtr = new CSysConfig();
	m_pRuelGenerator = new CRuleGenerator(this);
	MapKeyToHandle();
	MapKeyToHandleInject();
	//m_ConnEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	//m_MonitorDataListEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	//// m_IdenticalDataReadyEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	//m_GetRuleFromLocalEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	//m_ExitEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	//m_SaveLogInfoListEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	//m_LoadRuleReadyEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	//m_StartRecordDesktopEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	//m_EndRecordDesktopEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	//m_StartLiveVideoEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	//m_EndLiveVideoEvent = CreateEvent(NULL, FALSE, TRUE, NULL);

	//hTimer = NULL;
	//hTimerQueue = NULL;
	//hTimerQueue = CreateTimerQueue();
	//if (NULL == hTimerQueue)
	//{
	//	printlog("CreateTimerQueue failed (%d)\n", GetLastError());
	//	//return FALSE;
	//}

	//m_ConnClientList.clear();
	//m_MonitorDataList.clear();
	//m_RuleDataStringList.clear();
	//m_SaveLogInfoList.clear();
	//m_MonitorObjectList.clear();
}

CManageThreadPool::CManageThreadPool(int num, void *arg)
{
	/*InitCS(&m_ConnClientListLock);
	InitCS(&m_MonitorDataListLock);
	InitCS(&m_SaveLogInfoListLock);
	m_SysConfigPtr = new CSysConfig();
	m_pRuelGenerator = new CRuleGenerator(this);
	MapKeyToHandle();
	MapKeyToHandleInject();*/
}

void CManageThreadPool::MapKeyToHandle()
{
	m_KeyToHandleProcMap[PACKET_COMMAND_REQUEST_RULE] = &CManageThreadPool::HandleRuleProc;  
	m_KeyToHandleProcMap[PACKET_COMMAND_REQUEST_CONFIG_FILE] = &CManageThreadPool::HandleConfigFileProc;  
	m_KeyToHandleProcMap[PACKET_COMMAND_POST_MONITOR_INFO] = &CManageThreadPool::HandleProc;  
	m_KeyToHandleProcMap[PACKET_COMMAND_REQUEST_RECONNECT] = &CManageThreadPool::HandleProc;  
	m_KeyToHandleProcMap[PACKET_COMMAND_REQUEST_EXPIRED_TIME] = &CManageThreadPool::HandleExpiredTimeProc;  
	m_KeyToHandleProcMap[PACKET_COMMAND_REQUEST_LOGIN] = &CManageThreadPool::HandleProc;  
	m_KeyToHandleProcMap[PACKET_COMMAND_REQUEST_RELOGIN] = &CManageThreadPool::HandleProc;  
	m_KeyToHandleProcMap[PACKET_COMMAND_REQUEST_UPDATE_RULE] = &CManageThreadPool::HandleUpdateRuleProc;
	
	m_KeyToHandleObjProcMap[0] = &CManageThreadPool::HandleQQProcessProc;
	m_KeyToHandleObjProcMap[1] = &CManageThreadPool::HandleWeChatProcessProc;
	m_KeyToHandleObjProcMap[2] = &CManageThreadPool::HandleFetionProcessProc;
} 	

void CManageThreadPool::MapKeyToHandleInject()
{
	m_KeyToHandleInjectProcMap[0] = &CManageThreadPool::HandleInjectViaEasyHookProc;
	m_KeyToHandleInjectProcMap[1] = &CManageThreadPool::HandleInjectViaWindowsProc;
	m_KeyToHandleInjectProcMap[2] = &CManageThreadPool::HandleInjectViaManualProc;
}

BOOL CManageThreadPool::RunServer(void *arg)
{
	recordlog("RunServer");

	m_ConnEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_MonitorDataListEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	// m_IdenticalDataReadyEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	m_GetRuleFromLocalEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_ExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_SaveLogInfoListEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_LoadRuleReadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_StartRecordDesktopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_EndRecordDesktopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_StartLiveVideoEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_EndLiveVideoEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_StartUploadVideoEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	hTimer = NULL;
	hTimerQueue = NULL;
	hTimerQueue = CreateTimerQueue();
    if (NULL == hTimerQueue)
    {
        printlog("CreateTimerQueue failed (%d)\n", GetLastError());
        return FALSE;
    }

	m_ConnClientList.clear();
	m_MonitorDataList.clear();
	m_RuleDataStringList.clear();
	m_SaveLogInfoList.clear();
	m_MonitorObjectList.clear();
	m_ReadyUploadVideoList.clear();
	
	/*BOOL bRet = m_SysConfigPtr->CheckFolderExist();
	if (!bRet) {
		printlog("");
	}
	*/
	
	BOOL bRet = InitIniConfig();
	if (!bRet) {
		printlog("");
	}
	GetSysConfigInfo();
	recordlog("GetSysConfigInfo");
	/*OpenDB("C:\\agent\\info.db");
	time_t tm;
	time_t tm_now;
	GetAgentExpiredDateFromDB(tm);
	if (tm == (time_t)0) {
		if (!(m_SysConfigPtr->GetLisenceFileExistState())) {
			printlog("");
			goto FAIL;
		} else {
			tm = (time_t)0;
			m_SysConfigPtr->GetExpiredDateFromLisenceFile();
			InsertAgentExpiredDateIntoDB();
			m_SysConfigPtr->GetSysConfigInfoFromLocal();
			InsertSysConfigInfoIntoDB();
		}
	}
	tm_now = time(0);
	if (tm > tm_now) {
		printlog("");
		goto FAIL;
	}
	if (m_SysConfigPtr->GetLisenceFileExistState()) {
		bRet = m_SysConfigPtr->GetSysConfigInfoFromLocal();
		if (!bRet) {
			printlog("");
			goto FAIL;
		}
	}*/
		
	StartServerThreadPool();
	StartClientThreadPool(1);
	/*if (WaitForSingleObject(m_GetRuleFromLocalEvent, INFINITE) == WAIT_OBJECT_0) {
		::InterlockedIncrement(&m_nGetRuleFromLocalVar);
		if (::InterlockedDecrement(&m_nGetRuleFromLocalVar) == 3 ||
				::InterlockedDecrement(&m_nGetRuleFromLocalVar) == 4) {
			if (m_SysConfigPtr->GetRuleFileExistState()) {
				bRet = m_SysConfigPtr->GetRulesInfoFromLocal();
				if (!bRet) {
					printlog("");
					goto FAIL;
				}
			} else {
				printlog("");
				goto FAIL;
			}
		}
		SetEventm_LoadRuleReadyEvent);
	}
	return TRUE;

*/
//FAIL:
	//printlog("init fail");
	return FALSE;
}

void CManageThreadPool::ExitServer()
{
	
}

void CManageThreadPool::CreateWorkerThreadPool(PVOID arg)
{
	m_IOCPModelPtr = (CIOCPModel*)arg;
	try {
		m_ThreadPool_serverPtr = CThreadPool::GetInstance(0, this, arg);
		if (m_ThreadPool_serverPtr == NULL) {

		}
	}
	catch (CThreadPool::MyException &e) {
		cout << e.what() << endl;
	}
}

void CManageThreadPool::StartServerThreadPool()
{
	m_IOCPModelPtr = new CIOCPModel(this);
	bool bRet = m_IOCPModelPtr->LoadSocketLib();
	if (!bRet) {
		printlog("");
	}

	bRet = m_IOCPModelPtr->Start();
	if (!bRet) {
		printlog("");
	}
	recordlog("StartServerThreadPool cross");
}

void CManageThreadPool::StartClientThreadPool(int num)
{
	try {
		m_ThreadPool_clientPtr = CThreadPool::GetInstance(num, this, NULL);
		if (m_ThreadPool_clientPtr == NULL) { 

		}
	}
	catch (CThreadPool::MyException &e) {
		cout << e.what() << endl;
	}
}

void CManageThreadPool::GetSysConfigInfo()
{
	ZeroMemory(g_sWebMainServerIP, sizeof(g_sWebMainServerIP));
	ZeroMemory(g_sWebStandbyServerIP, sizeof(g_sWebStandbyServerIP));
	ZeroMemory(g_sWebServerPort, sizeof(g_sWebServerPort));
	ZeroMemory(g_sFtpServerIP, sizeof(g_sFtpServerIP));
	ZeroMemory(g_sFtpUserNamePsw, sizeof(g_sFtpUserNamePsw));
	ZeroMemory(g_sGuard_qq_dll, sizeof(g_sGuard_qq_dll));
	ZeroMemory(g_sGuard_wechat_dll, sizeof(g_sGuard_wechat_dll));
	ZeroMemory(g_sGuard_fetion_dll, sizeof(g_sGuard_fetion_dll));
	ZeroMemory(g_sVideo_save_path, sizeof(g_sVideo_save_path));

	/*GetPrivateProfileStringA("server", "main_server_ip", "test", g_sWebMainServerIP, sizeof(g_sWebMainServerIP), "C:\\hondavda\\agent.ini");
	GetPrivateProfileStringA("server", "standby_server_ip", "test", g_sWebStandbyServerIP, sizeof(g_sWebStandbyServerIP), "C:\\hondavda\\agent.ini");
	GetPrivateProfileStringA("server", "server_port", "test", g_sWebServerPort, sizeof(g_sWebServerPort), "C:\\hondavda\\agent.ini");
	GetPrivateProfileStringA("server", "ftp_server_ip", "test", g_sFtpServerIP, sizeof(g_sFtpServerIP), "C:\\hondavda\\agent.ini");
	GetPrivateProfileStringA("server", "ftp_username_psw", "test", g_sFtpUserNamePsw, sizeof(g_sFtpUserNamePsw), "C:\\hondavda\\agent.ini");
	GetPrivateProfileStringA("client", "qq_dll", "test", g_sGuard_qq_dll, sizeof(g_sGuard_qq_dll), "C:\\hondavda\\agent.ini");
	GetPrivateProfileStringA("client", "qq_dll", "test", g_sGuard_wechat_dll, sizeof(g_sGuard_wechat_dll), "C:\\hondavda\\agent.ini");
	GetPrivateProfileStringA("client", "qq_dll", "test", g_sGuard_fetion_dll, sizeof(g_sGuard_fetion_dll), "C:\\hondavda\\agent.ini");
	GetPrivateProfileStringA("client", "qq_dll", "test", g_sVideo_save_path, sizeof(g_sVideo_save_path), "C:\\hondavda\\agent.ini");*/

	GetIniKeyString("[server]", "main_server_ip", g_sWebMainServerIP);
	GetIniKeyString("[server]", "standby_server_ip", g_sWebStandbyServerIP);
	GetIniKeyString("[server]", "server_port", g_sWebServerPort);
	GetIniKeyString("[server]", "ftp_server_ip", g_sFtpServerIP);
	GetIniKeyString("[server]", "ftp_username_psw", g_sFtpUserNamePsw);
	GetIniKeyString("[client]", "qq_dll", g_sGuard_qq_dll);
	GetIniKeyString("[client]", "wechat_dll", g_sGuard_wechat_dll);
	GetIniKeyString("[client]", "fetion_dll", g_sGuard_fetion_dll);
	GetIniKeyString("[client]", "vedio_path", g_sVideo_save_path);

	
}

void CManageThreadPool::GetRulesInfoFromLocal()
{

}

void CManageThreadPool::SaveCurrentRulesToLocal()
{

}

BOOL CManageThreadPool::SendRuleRequestToServer(SOCKET sockfd)
{
	int Len = sizeof(PKG_HEAD_STRUCT) + sizeof(PKG_BODY_STRUCT) + strlen("123") + 1;
	char *data = (char*)malloc(Len * sizeof(char));
	//char data[sizeof(PKG_HEAD_STRUCT) + sizeof(BODYInfo)] = { 0 };
	PKG_BODY_STRUCT *pReport = reinterpret_cast<PKG_BODY_STRUCT*>(data + sizeof(PKG_HEAD_STRUCT));
	
	memcpy(pReport->data, "123", strlen("123"));
	
	PPKG_HEAD_STRUCT pHead = reinterpret_cast<PKG_HEAD_STRUCT*>(data);
	pHead->command = PACKET_COMMAND_REQUEST_RULE;
	pHead->action = PACKET_ACTION_GET;
	pHead->check = PACKET_CHECK_NONE;
	pHead->pkgSize = sizeof(PKG_HEAD_STRUCT) + strlen("123");
	pHead->pkgTag = time(0);
	int nLen = pHead->pkgSize;
	while (1) {
		BOOL ret = packet_send(sockfd, (char*)pHead, nLen);
		if (ret == FALSE) {
			closesocket(sockfd);
			return FALSE;
		}
		break;
		/*int nSize = send(sockfd, (char*)pHead, nLen, 0);
		if (nSize <= 0) {
			closesocket(sockfd);
			return FALSE;
		}
		nLen -= nSize;*/
	}
	return TRUE;
}

BOOL CManageThreadPool::SendRuleUpdateRequestToServer(SOCKET sockfd)
{
	int Len = sizeof(PKG_HEAD_STRUCT) + sizeof(PKG_BODY_STRUCT) + strlen("123") + 1;
	char *data = (char*)malloc(Len * sizeof(char));
	//char data[sizeof(PKG_HEAD_STRUCT) + sizeof(BODYInfo)] = { 0 };
	PKG_BODY_STRUCT *pReport = reinterpret_cast<PKG_BODY_STRUCT*>(data + sizeof(PKG_HEAD_STRUCT));
	
	memcpy(pReport->data, "123", strlen("123"));
	
	PPKG_HEAD_STRUCT pHead = reinterpret_cast<PKG_HEAD_STRUCT*>(data);
	pHead->command = PACKET_COMMAND_REQUEST_RULE;
	pHead->action = PACKET_ACTION_UPDATE;
	pHead->check = PACKET_CHECK_NONE;
	pHead->pkgSize = sizeof(PKG_HEAD_STRUCT) + strlen("123");
	pHead->pkgTag = time(0);
	LONG nLen = pHead->pkgSize;
	while (1) {
		BOOL ret = packet_send(sockfd, (char*)pHead, nLen);
		if (ret == FALSE) {
			closesocket(sockfd);
			return FALSE;
		}
		break;
		/*int nSize = send(sockfd, (char*)pHead, nLen, 0);
		if (nSize <= 0) {
			closesocket(sockfd);
			return FALSE;
		}
		nLen -= nSize;*/
	}
	return TRUE;
}

BOOL CManageThreadPool::SendMonitorDataToServer(SOCKET sockfd, string &data)
{
	int Len = sizeof(PKG_HEAD_STRUCT) + sizeof(PKG_BODY_STRUCT) + data.size() + 1;
	char *buf = (char*)malloc(Len * sizeof(char));
	//char buf[sizeof(PKG_HEAD_STRUCT) + sizeof(BODYInfo)] = { 0 };
	PKG_BODY_STRUCT *pReport = reinterpret_cast<PKG_BODY_STRUCT*>(buf + sizeof(PKG_HEAD_STRUCT));

	memcpy(pReport->data, data.c_str(), data.size());

	PPKG_HEAD_STRUCT pHead = reinterpret_cast<PKG_HEAD_STRUCT*>(buf);
	pHead->command = PACKET_COMMAND_POST_MONITOR_INFO;
	pHead->action = PACKET_ACTION_POST;
	pHead->check = PACKET_CHECK_NONE;
	pHead->pkgSize = sizeof(PKG_HEAD_STRUCT) + data.size();
	pHead->pkgTag = time(0);
	LONG nLen = pHead->pkgSize;
	while (1) {
		BOOL ret = packet_send(sockfd, (char*)pHead, nLen);
		if (ret == FALSE) {
			closesocket(sockfd);
			return FALSE;
		}
		break;
		/*int nSize = send(sockfd, (char*)pHead, nLen, 0);
		if (nSize <= 0) {
		closesocket(sockfd);
		return FALSE;
		}
		nLen -= nSize;*/
	}
	return TRUE;
}

//bool CManageThreadPool::DispatchDataToHandleProc(PPKG_HEAD_STRUCT pHead, char *data)
bool CManageThreadPool::DispatchDataToHandleProc(INT command, char *data)
{	
	typedef map<int, pHandleProc>::const_iterator CI;  
	CI iter = m_KeyToHandleProcMap.find(command);
  
    if (m_KeyToHandleProcMap.end() == iter)  
    {  
        printlog("no match handler function");  
        return false;  
    }  
  
    pHandleProc pFunction = iter->second;  
    return (this->*pFunction)(data);

    return true;
}

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	CManageThreadPool *pM = (CManageThreadPool*)lpParam;

	SetEvent(pM->m_EndRecordDesktopEvent);
}

BOOL CManageThreadPool::ReceiveDataFromClient(PCHAR data, INT &dwFlag, ULONG nLen)
{
	assert(data != NULL);
	PKG_CLIENT_HEAD_STRUCT* pHeadClient = reinterpret_cast<PKG_CLIENT_HEAD_STRUCT*>(data);
	switch (pHeadClient->command)
	{
		case PACKET_CLIENT_COMMAND_REQUEST_START_RECORD_DESKTOP: {
			::InterlockedIncrement(&m_IsRecordDesktoping);
			if (::InterlockedDecrement(&m_IsRecordDesktoping) == 0) {
				SetEvent(m_StartRecordDesktopEvent);
			}
			recordlog("ReceiveDataFromClient SetEvent");
			/*if (hTimer != NULL) {
				DeleteTimerQueueTimer(hTimerQueue, hTimer, INVALID_HANDLE_VALUE);
			}*/
			if (!CreateTimerQueueTimer(&hTimer, hTimerQueue, (WAITORTIMERCALLBACK)TimerRoutine, this, 10000, 0, 0)) {
		        printlog("CreateTimerQueueTimer failed (%d)\n", GetLastError());
		        return FALSE;
		    }
			/*string js_out;
	    	m_pRuelGenerator.GenerateDataToServer("record_time", data, js_out);
	    	//CSLock lock(m_MonitorDataListLock);
			::EnterCriticalSection(&m_MonitorDataListLock);
	    	m_MonitorDataList.push_back(js_out);
			::LeaveCriticalSection(&m_MonitorDataListLock);
			SetEvent(m_MonitorDataListEvent);
			*/			
		    break;
		}
	    case PACKET_CLIENT_COMMAND_REQUEST_QQ_RULE: {
	    	dwFlag = 1;	
		    break;
	    }
	    case PACKET_CLIENT_COMMAND_REQUEST_WECHAT_RULE: {
	    	dwFlag = 2;
		    break;
	    }
	    case PACKET_CLIENT_COMMAND_REQUEST_FETION_RULE: {
	    	dwFlag = 3;
		    break;
	    }
	    case PACKET_CLIENT_COMMAND_REQUEST_USB_RULE: {
	    	dwFlag = 4;
		    break;
	    }
	    case PACKET_CLIENT_COMMAND_REQUEST_WEB_RULE: {
	    	dwFlag = 5;
		    break;
	    }
	    case PACKET_CLIENT_COMMAND_REQUEST_NETDISK_RULE: {
	    	dwFlag = 6;
		    break;
	    }
	    case PACKET_CLIENT_COMMAND_REQUEST_FOXMAIL_RULE: {
	    	dwFlag = 7;
	    	break;
	    }
	    case PACKET_CLIENT_COMMAND_POST_QQ_INFO: {
	    	string js_out;
	    	m_pRuelGenerator.GenerateDataToServer("QQ.exe", data, js_out);
	    	//CSLock lock(m_MonitorDataListLock);
			::EnterCriticalSection(&m_MonitorDataListLock);
	    	m_MonitorDataList.push_back(js_out);
			::LeaveCriticalSection(&m_MonitorDataListLock);
			SetEvent(m_MonitorDataListEvent);
			printlogA(js_out.c_str());
	    	break;
	    }
	    case PACKET_CLIENT_COMMAND_POST_WECHAT_INFO: {
			string js_out;
			m_pRuelGenerator.GenerateDataToServer("WeChat.exe", data, js_out);
			CSLock lock(m_MonitorDataListLock);
			m_MonitorDataList.push_back(js_out);
			SetEvent(m_MonitorDataListEvent);
	    	break;
	    }
	    case PACKET_CLIENT_COMMAND_POST_FETION_INFO: {
			string js_out;
			m_pRuelGenerator.GenerateDataToServer("Fetion.exe", data, js_out);
			CSLock lock(m_MonitorDataListLock);
			m_MonitorDataList.push_back(js_out);
			SetEvent(m_MonitorDataListEvent);
	    	break;
	    }
	    case PACKET_CLIENT_COMMAND_POST_USB_INFO: {
	    	/*usb readyData;
	    	pHeadClient->pkgInfo.pkg_usb.bUsbInsert = pHeadClient->pkgInfo.pkg_usb.bUsbInsert;
			memcpy(readyData.sCopyFile, pHeadClient->pkgInfo.pkg_usb.sCopyFile, strlen(pHeadClient->pkgInfo.pkg_usb.sCopyFile));
	    	string js_out;
	    	m_pRuelGenerator->GenerateDataToServer("USB", &readyData, js_out);
	    	CSLock lock(m_MonitorDataListLock);
	    	m_MonitorDataList.push_back(js_out);
			SetEvent(m_MonitorDataListEvent);*/
	    	break;
	    }
	    case PACKET_CLIENT_COMMAND_POST_WEB_INFO: {
	    	/*web readyData;
	    	memcpy(readyData.sAccessURL, pHeadClient->pkgInfo.pkg_web.sAccessURL, strlen(pHeadClient->pkgInfo.pkg_web.sAccessURL));
	    	int length = pHeadClient->pkgSize - sizeof(PKG_HEAD_STRUCT);
	    	PKG_BODY_STRUCT *pBody = reinterpret_cast<PKG_BODY_STRUCT*>(data + sizeof(PKG_HEAD_STRUCT));
	    	string js_out;
	    	m_pRuelGenerator->GenerateDataToServer("Web", &readyData, js_out, pBody->data, length);
	    	CSLock lock(m_MonitorDataListLock);
	    	m_MonitorDataList.push_back(js_out);
			SetEvent(m_MonitorDataListEvent);*/
	    	break;
	    }
	    case PACKET_CLIENT_COMMAND_POST_NETDISK_INFO: {
	    	/*netdisk readyData;
	    	memcpy(readyData.sNetDiskName, pHeadClient->pkgInfo.pkg_netdisk.sNetDiskName, strlen(pHeadClient->pkgInfo.pkg_netdisk.sNetDiskName));
			memcpy(readyData.sUploadFilePath, pHeadClient->pkgInfo.pkg_netdisk.sUploadFilePath, strlen(pHeadClient->pkgInfo.pkg_netdisk.sUploadFilePath));
	    	string js_out;
	    	m_pRuelGenerator->GenerateDataToServer("NetDisk", &readyData, js_out);
	    	CSLock lock(m_MonitorDataListLock);
	    	m_MonitorDataList.push_back(js_out);
			SetEvent(m_MonitorDataListEvent);*/
	    	break;
	    }
		case PACKET_CLIENT_COMMAND_POST_FOXMAIL_INFO: {
	    	/*foxmail readyData;
	    	memcpy(readyData.sUserMailAddress, pHeadClient->pkgInfo.pkg_foxmail.sUserMailAddress, strlen(pHeadClient->pkgInfo.pkg_foxmail.sUserMailAddress));
			memcpy(readyData.sReceiverMailAddress, pHeadClient->pkgInfo.pkg_foxmail.sReceiverMailAddress, strlen(pHeadClient->pkgInfo.pkg_foxmail.sReceiverMailAddress));
			memcpy(readyData.sAttachFilePath, pHeadClient->pkgInfo.pkg_foxmail.sAttachFilePath, strlen(pHeadClient->pkgInfo.pkg_foxmail.sAttachFilePath));
	    	string js_out;
	    	m_pRuelGenerator->GenerateDataToServer("foxmail.exe", &readyData, js_out);
	    	CSLock lock(m_MonitorDataListLock);
	    	m_MonitorDataList.push_back(js_out);
			SetEvent(m_MonitorDataListEvent);*/
			break;
		}
	    default:
		    break;
	}
	return TRUE;
}

BOOL CManageThreadPool::ReceiveDataFromServer(SOCKET sockfd, char *buffer, LONG &idx, PPKG_HEAD_STRUCT pHead, char *retBuffer, BOOL &isEnough)
{
	BOOL bRet = FALSE;
	bRet = packet_recv(sockfd, buffer, idx, pHead, retBuffer, isEnough);
	if (!bRet) {
		return FALSE;
	}
	return TRUE;
}

void CManageThreadPool::ParseJsonDataFromServer(char *data)
{

}

BOOL CManageThreadPool::HandleRuleProc(char *data)
{
	BOOL bRet = m_pRuelGenerator.GenerateObject(data, m_MonitorObjectList);
	//BOOL bRet = m_pRuelGenerator.GenerateObject(data);
	if (!bRet) {
		return FALSE;
	}
	SetEvent(m_LoadRuleReadyEvent);
	return TRUE;
}

BOOL CManageThreadPool::HandleConfigFileProc(char *data)
{
	return TRUE;
}

BOOL CManageThreadPool::HandleExpiredTimeProc(char *data)
{
	return TRUE;
}

BOOL CManageThreadPool::HandleUpdateRuleProc(char *data)
{
	return TRUE;
}

BOOL CManageThreadPool::DispatchInjectHandleProc(void *arg)
{
	CObject *obj = (CObject*)arg;
	typedef map<int, pHandleInjectProc>::const_iterator CI;  
    CI iter = m_KeyToHandleInjectProcMap.find(obj->m_InjectModeIndex);  
  
    if (m_KeyToHandleInjectProcMap.end() == iter)  
    {  
        printlog("no match handler function");  
        return FALSE;  
    }  
  
    pHandleInjectProc pFunction = iter->second;  
    return (this->*pFunction)(arg);
}

BOOL CManageThreadPool::DispatchObjProcessToHandleProc(void *arg, INT &n)
{
	CObject *obj = (CObject*)arg;
	typedef map<int, pHandleObjProc>::const_iterator CI;  
    CI iter = m_KeyToHandleObjProcMap.find(obj->m_InjectModeIndex);  
  
    if (m_KeyToHandleObjProcMap.end() == iter)  
    {  
        printlog("no match handler function");  
        return FALSE;  
    }  
  
    pHandleObjProc pFunction = iter->second;  
    return (this->*pFunction)(arg, n);
}

BOOL CManageThreadPool::HandleQQProcessProc(void *data, INT &n)
{
	CQqObject *obj = (CQqObject*)data;
	if (!obj->m_IsExist) {
		switch (obj->m_Count) {
			case 0: {
				obj->m_Count += 1;
				obj->m_FirstObjectPid = obj->m_ObjectPid;
				break;
			}
			case 1: {
				obj->m_Count += 1;
				obj->m_SecondObjectPid = obj->m_ObjectPid;
				break;
			}
			case 2: {
				if (obj->m_FirstObjectPid != obj->m_SecondObjectPid) {
					obj->m_Count = 0;
				} else {
					obj->m_FirstObjectPid = -1;
					obj->m_SecondObjectPid = -1;
					obj->m_Count = 0;
					obj->m_IsExist = TRUE;
					BOOL bErr = DispatchInjectHandleProc(obj);
					if (!bErr) {
						obj->m_InjectModeIndex = 2;
						DispatchInjectHandleProc(obj);
					}
				}
				break;
			}
			default:
				break;
		}
	}
	return TRUE;
}

BOOL CManageThreadPool::HandleWeChatProcessProc(void *data, INT &n)
{
	Sleep(10000);
	CObject *obj = (CObject*)data;
	BOOL bErr = DispatchInjectHandleProc(obj);
	if (!bErr) {
		obj->m_InjectModeIndex = 2;
		DispatchInjectHandleProc(obj);
	}
	n++;
	return TRUE;
}

BOOL CManageThreadPool::HandleFetionProcessProc(void *data, INT &n)
{
	Sleep(10000);
	CObject *obj = (CObject*)data;
	BOOL bErr = DispatchInjectHandleProc(obj);
	if (!bErr) {
		obj->m_InjectModeIndex = 2;
		DispatchInjectHandleProc(obj);
	}
	n++;
	return TRUE;
}

//BOOL CManageThreadPool::MyCreateRemoteThread(HANDLE hProcess, LPTHREAD_START_ROUTINE pThreadProc, LPVOID pRemoteBuf)
//{
//	HANDLE      hThread = NULL;
//	FARPROC     pFunc = NULL;
//	BOOL bHook;
//
//	// 判断系统版本  
//	OSVERSIONINFO osvi;
//	//BOOL bIsWindowsXPorLater;  
//
//	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
//	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
//	////操作系统版本判断
//	//BOOL Cx64Inject::IsVistaOrLater()
//	//{
//	//	OSVERSIONINFO osvi;
//	//	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
//	//	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
//	//	GetVersionEx(&osvi);
//	//	if (osvi.dwMajorVersion >= 6)
//	//		return TRUE;
//	//	return FALSE;
//	//}
//	GetVersionEx(&osvi);
//
//	if (osvi.dwMajorVersion >= 6)
//	{
//		bHook = TRUE;
//	}
//	else
//	{
//		bHook = FALSE;
//	}
//
//	if (bHook)    // Vista, 7, Server2008  
//	{
//		cout << "NtCreateThreadEx" << endl;
//		pFunc = GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtCreateThreadEx");
//		if (pFunc == NULL)
//		{
//			//GetLastError());  
//			return FALSE;
//		}
//		((_NtCreateThreadEx64)pFunc)(&hThread, 0x1FFFFF, NULL, hProcess, pThreadProc, pRemoteBuf, FALSE, NULL, NULL, NULL, NULL);
//		if (hThread == NULL)
//		{
//			DWORD iErr = GetLastError();
//			printf("NtCreateThreadEx error:%ld\n", iErr);
//			return FALSE;
//		}
//
//		//pFunc = GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtCreateThreadEx");
//		//if (pFunc == NULL)
//		//{
//		//	//GetLastError());  
//		//	return FALSE;
//		//}
//		//cout << "MyCreateRemoteThread" << endl;
//		//OutputDebugString(L"MyCreateRemoteThread");
//		//((PFNTCREATETHREADEX)pFunc)(&hThread,
//		//	0x1FFFFF,
//		//	NULL,
//		//	hProcess,
//		//	pThreadProc,
//		//	pRemoteBuf,
//		//	FALSE,
//		//	NULL,
//		//	NULL,
//		//	NULL,
//		//	NULL);
//		//if (hThread == NULL)
//		//{
//		//	cout << "MyCreateRemoteThread error" << endl;
//		//	DWORD iErr = GetLastError();
//		//	printf("CreateRemoteThread error:%ld\n", iErr);
//		//	return FALSE;
//		//}
//		/*hThread = CreateRemoteThread(hProcess,
//			NULL,
//			0,
//			pThreadProc,
//			pRemoteBuf,
//			0,
//			NULL);
//		if (hThread == NULL)
//		{
//			cout << "CreateRemoteThread error" << endl;
//
//			DWORD iErr = GetLastError();
//			printf("CreateRemoteThread error:%ld\n", iErr);
//			return FALSE;
//		}*/
//		//HANDLE ThreadHandle = NULL;
//		//pFunc = GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlCreateUserThread");
//		//if (pFunc == NULL)
//		//{
//		//	//GetLastError());  
//		//	return FALSE;
//		//}
//		//HANDLE ThreadHandle = NULL;
//		//NTSTATUS Status = ((pfnRtlCreateUserThread)pFunc)(hProcess, NULL, FALSE, 0, 0, 0, pThreadProc, pRemoteBuf, &ThreadHandle, NULL);
//	}
//	else                    // 2000, XP, Server2003  
//	{
//		cout << "CreateRemoteThread" << endl;
//
//		/*HANDLE
//			WINAPI
//			CreateRemoteThreadEx(
//			_In_ HANDLE hProcess,
//			_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
//			_In_ SIZE_T dwStackSize,
//			_In_ LPTHREAD_START_ROUTINE lpStartAddress,
//			_In_opt_ LPVOID lpParameter,
//			_In_ DWORD dwCreationFlags,
//			_In_opt_ LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
//			_Out_opt_ LPDWORD lpThreadId
//			);*/
//
//		/*hThread = CreateRemoteThreadEx(
//			);*/
//		hThread = CreateRemoteThread(hProcess,
//			NULL,
//			0,
//			pThreadProc,
//			pRemoteBuf,
//			0,
//			NULL);
//		if (hThread == NULL)
//		{
//			//cout << "CreateRemoteThread error" << endl;
//			
//			DWORD iErr = GetLastError();
//			printf("CreateRemoteThread error:%ld\n", iErr);
//			return FALSE;
//		}
//	}
//
//	cout << "wait hook" << endl;
//	if (WAIT_FAILED == WaitForSingleObject(hThread, INFINITE))
//	{
//		return FALSE;
//	}
//
//	return TRUE;
//}
//
//BOOL CManageThreadPool::InjectDll(DWORD dwPID, const wchar_t *szDllName)
//{
//	HANDLE hProcess = NULL;
//	LPVOID pRemoteBuf = NULL;
//	LPVOID pThreadData = NULL;
//	FARPROC pThreadProc = NULL;
//	LPVOID pCode = NULL;
//	DWORD dwBufSize = wcslen(szDllName)*sizeof(wchar_t) + 2;
//
//	wprintf(L"PID: %d\n", dwPID);
//	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
//	//if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, dwPID)))
//	{
//		int num = GetLastError();
//		char warn[50];
//		sprintf(warn, "%s,%d", "OpenProcess", num);
//		//MessageBoxA(NULL, warn, "warning ", 0);
//		cout << warn  << endl;
//		return FALSE;
//	}
//
//	THREAD_DATA data;
//	HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
//	data.fnRtlInitUnicodeString = (pRtlInitUnicodeString)GetProcAddress(hNtdll, "RtlInitUnicodeString");
//	data.fnLdrLoadDll = (pLdrLoadDll)GetProcAddress(hNtdll, "LdrLoadDll");
//	memcpy(data.DllName, szDllName, (wcslen(szDllName) + 1)*sizeof(WCHAR));
//	data.DllPath = NULL;
//	data.Flags = 0;
//	data.ModuleHandle = INVALID_HANDLE_VALUE;
//	pThreadData = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
//	if (pThreadData == NULL) {
//		int num = GetLastError();
//		char warn[50];
//		sprintf(warn, "%s,%d", "VirtualAlloc 出错", num);
//		//MessageBoxA(NULL, warn, "warning ", 0);
//		cout << warn << endl;
//		return FALSE;
//	}
//	BOOL bWriteOK = WriteProcessMemory(hProcess, pThreadData, &data, sizeof(data), NULL);
//	if (!bWriteOK)
//		return FALSE;
//
//	//pRemoteBuf = VirtualAllocEx(hProcess, NULL, dwBufSize,
//	//	MEM_COMMIT, PAGE_READWRITE);
//	//if (pRemoteBuf == NULL) {
//	//	int num = GetLastError();
//	//	char warn[50];
//	//	sprintf(warn, "%s,%d", "VirtualAlloc 出错", num);
//	//	//MessageBoxA(NULL, warn, "warning ", 0);
//	//	cout << warn << endl;
//	//	return FALSE;
//	//}
//
//	DWORD SizeOfCode = (DWORD)ThreadProcEnd - (DWORD)ThreadProc;
//	pCode = VirtualAllocEx(hProcess, NULL, SizeOfCode, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
//	if (pCode == NULL)
//	{
//		
//	}
//	bWriteOK = WriteProcessMemory(hProcess, pCode, (PVOID)ThreadProc, SizeOfCode, NULL);
//	if (!bWriteOK)
//		return FALSE;
//
//	//BOOL err = WriteProcessMemory(hProcess, pRemoteBuf, (LPVOID)szDllName,
//	//	dwBufSize, NULL);
//	//if (!err) {
//	//	int num = GetLastError();
//	//	char warn[50];
//	//	sprintf(warn, "%s,%d", "WriteProcessMemory出错", num);
//	//	//MessageBoxA(NULL, warn, "warning ", 0);
//	//	cout << warn << endl;
//	//	return FALSE;
//	//}
//
//#if _WIN64
//#ifdef UNICODE
//	pThreadProc = GetProcAddress(GetModuleHandle(L"kernel32.dll"),
//		"LoadLibraryW");
//#else
//	pThreadProc = GetProcAddress(GetModuleHandle(L"kernel32.dll"),
//		"LoadLibraryA");
//#endif // !UNICODE
//#else
////#ifdef UNICODE
////	pThreadProc = GetProcAddress(GetModuleHandle(L"kernel32.dll"),
////		"LoadLibraryW");
////#else
////	pThreadProc = GetProcAddress(GetModuleHandle(L"kernel32.dll"),
////		"LoadLibraryA");
////#endif // !UNICODE
//#endif
////#ifdef UNICODE
////	/*pThreadProc = GetProcAddress(GetModuleHandle(L"kernel32.dll"),
////		"LoadLibraryW");*/
////	//pThreadProc = GetProcAddress(GetModuleHandle(L"ntdll.dll"), "LdrLoadDll");
////#else
////	pThreadProc = GetProcAddress(GetModuleHandle(L"kernel32.dll"),
////		"LoadLibraryA");
////#endif // !UNICODE
//
//	//if (pThreadProc == NULL) {
//	//	int num = GetLastError();
//	//	char warn[50];
//	//	sprintf(warn, "%s,%d", "GetProcAddress出错", num);
//	//	//MessageBoxA(NULL, warn, "warning ", 0);
//	//	cout << warn << endl;
//	//	return FALSE;
//	//}
//	
//	cout << "start inject hook" << endl;
//	//if (!MyCreateRemoteThread(hProcess, (LPTHREAD_START_ROUTINE)pThreadProc, pRemoteBuf))
//	if (!MyCreateRemoteThread(hProcess, (LPTHREAD_START_ROUTINE)pCode, pThreadData))
//	{
//		return FALSE;
//	}
//
//	//VirtualFreeEx(hProcess, pRemoteBuf, dwBufSize, MEM_RELEASE);
//	VirtualFreeEx(hProcess, pThreadData, 0, MEM_RELEASE);
//	CloseHandle(hProcess);
//	return TRUE;
//}
//
//BOOL CManageThreadPool::SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
//{
//	TOKEN_PRIVILEGES tp;
//	HANDLE hToken;
//	LUID luid;
//	DWORD dwSize = sizeof(TOKEN_PRIVILEGES);
//	TOKEN_PRIVILEGES oldtp;
//
//	if (!OpenProcessToken(GetCurrentProcess(),
//		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
//		&hToken))
//	{
//		if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
//			return true;
//		else {
//			cout << "SetPrivilege 1" << endl;
//			return FALSE;
//		}
//	}
//
//	if (!LookupPrivilegeValue(NULL,             // lookup privilege on local system  
//		lpszPrivilege,    // privilege to lookup   
//		&luid))          // receives LUID of privilege  
//	{
//		cout << "SetPrivilege 2" << endl;
//		CloseHandle(hToken);
//		return FALSE;
//	}
//	ZeroMemory(&tp, sizeof(tp));
//	tp.PrivilegeCount = 1;
//	tp.Privileges[0].Luid = luid;
//	if (bEnablePrivilege) {
//		cout << "bEnablePrivilege 0" << endl;
//		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
//
//	}
//	else {
//		cout << "bEnablePrivilege 1" << endl;
//		tp.Privileges[0].Attributes = 0;
//	}
//
//	//// Enable the privilege or disable all privileges.  
//	//if (!AdjustTokenPrivileges(hToken,
//	//	FALSE,
//	//	&tp,
//	//	sizeof(TOKEN_PRIVILEGES),
//	//	(PTOKEN_PRIVILEGES)NULL,
//	//	(PDWORD)NULL))
//	//{
//	//	cout << "SetPrivilege 3" << endl;
//	//	return FALSE;
//	//}
//
//	//if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
//	//{
//	//	cout << "SetPrivilege 4" << endl;
//	//	//The token does not have the specified privilege.  
//	//	return FALSE;
//	//}
//
//	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &oldtp, &dwSize)) {
//		cout << "SetPrivilege 4" << endl;
//		CloseHandle(hToken);
//		return false;
//	}
//	// close handles
//	CloseHandle(hToken);
//
//	return TRUE;
//}
//int CManageThreadPool::DoInject(DWORD aPid, const WCHAR *aFullpath)
//{
//	if (wcslen(aFullpath) <= 0)
//	{
//		return -1;
//	}
//
//	//判断dll是否存在  
//	HANDLE hFile = CreateFile(aFullpath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
//	if (hFile != INVALID_HANDLE_VALUE)
//	{
//		DWORD dwsize = GetFileSize(hFile, NULL);
//		CloseHandle(hFile);
//		if (dwsize < 10)
//		{
//			return -2;
//		}
//		cout << "dll file is ready" << endl;
//	}
//	else
//	{
//		return -3;
//	}
//
//	BOOL bSuc = SetPrivilege(SE_DEBUG_NAME, TRUE);
//	if (bSuc == FALSE)
//	{
//		cout << "SetPrivilege error" << endl;
//		return -4;
//	}
//	bSuc = InjectDll((DWORD)aPid, aFullpath);
//	if (bSuc == FALSE)
//	{
//		cout << "InjectDll error" << endl;
//		return -5;
//	}
//
//	return 0;
//}

BOOL CManageThreadPool::HandleInjectViaEasyHookProc(void *data)
{
	recordlog("HandleInjectViaEasyHookProc");
	CObject *obj = (CObject*)data;
	DWORD pid = obj->m_ObjectPid;
	NTSTATUS nt;
	static int count = 0;
	int iLen = 0;
	iLen = MultiByteToWideChar(CP_ACP, 0, g_sGuard_qq_dll, -1, NULL, 0);
	TCHAR * pBuf = new TCHAR[iLen + 1];
	MultiByteToWideChar(CP_ACP, 0, g_sGuard_qq_dll, -1, pBuf, iLen);
	recordlog(g_sGuard_qq_dll);
RETRY:
	nt = RhInjectLibrary(
		pid,   // The process to inject into
		0,           // ThreadId to wake up upon injection
		EASYHOOK_INJECT_DEFAULT,
		pBuf/*aFullpath32*/, // 32-bit
		pBuf/*aFullpath64*/,		 // 64-bit not provided
		&pid,
		//data, // data to send to injected DLL entry point
		sizeof(DWORD)
		//sizeof(CObject)// size of data to send
		);
	if (nt != 0)
	{
		char tm[10] = {0};
		sprintf(tm, "%d", nt);
		recordlog("RhInjectLibrary failed with error code");
		recordlog(tm);
		PWCHAR err = RtlGetLastErrorString();
		printlogW(err);
		/*if (wcscmp(err, L"Unable to allocate memory in target process") == 0){
			Sleep(20000);
			goto RETRY;
		}*/
		/*if (count >= 3) {
			return FALSE;
		}
		count++;*/
		/*Sleep(1000);
		goto RETRY;*/
		// std::wcout << err << "\n";
		// return FALSE;
	}
	else
	{
		// std::wcout << L"Library injected successfully.\n";
		recordlog("Library injected successfully.");
	}
	return TRUE;
}

BOOL CManageThreadPool::HandleInjectViaWindowsProc(void *data)
{
	return TRUE;
}

BOOL CManageThreadPool::HandleInjectViaManualProc(void *data)
{
	// DoInject(DWORD aPid, const WCHAR *aFullpath);
	return TRUE;
}

unsigned int __stdcall CManageThreadPool::MonitorProcessProc(PVOID arg)
{
	recordlog("MonitorProcessProc");
	BOOL bExit = FALSE;
	CManageThreadPool *pManage = (CManageThreadPool*)arg;

	while (WAIT_OBJECT_0 != WaitForSingleObject(pManage->m_LoadRuleReadyEvent, 0)) {
		Sleep(50);
		continue;
	}
	//WaitForSingleObject(pManage->m_LoadRuleReadyEvent, INFINITE);

	INT i = 0;
	while (!bExit) 
	{
		if(WAIT_OBJECT_0 == WaitForSingleObject(pManage->m_ExitEvent, 0))
			bExit = TRUE;

		DWORD pID, cbNeeded;
		// HMODULE hMods[1024];
		HANDLE procSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (procSnap == INVALID_HANDLE_VALUE)
		{
			printlog("CreateToolhelp32Snapshot failed, %d \n", GetLastError());
			//return -1;
		}

		PROCESSENTRY32 procEntry = { 0 };
		procEntry.dwSize = sizeof(PROCESSENTRY32);
		BOOL bRet = Process32First(procSnap, &procEntry);
		BOOL bErr;
		BOOL bIsExist = FALSE;
		BOOL bQQState;
		if (i >= pManage->m_MonitorObjectList.size())
			i = 0;
		while (bRet)
		{
			/*if (i >= pManage->m_MonitorObjectList.size())
				i = 0;*/
			int iLen = 0;
			
			iLen = MultiByteToWideChar(CP_ACP, 0, ((pManage->m_MonitorObjectList)[i])->m_ObjectName.c_str(), -1, NULL, 0);
			TCHAR * pBuf = new TCHAR[iLen + 1];
			MultiByteToWideChar(CP_ACP, 0, ((pManage->m_MonitorObjectList)[i])->m_ObjectName.c_str(), -1, pBuf, iLen);
			if (wcscmp(procEntry.szExeFile, pBuf) == 0) {
				recordlog("MonitorProcessProc qq");
				(pManage->m_MonitorObjectList)[i]->m_ObjectPid = procEntry.th32ProcessID;
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procEntry.th32ProcessID);
				if (NULL == hProcess) {
					bIsExist = FALSE;
					((CQqObject*)(pManage->m_MonitorObjectList)[i])->m_IsExist = TRUE;
					delete[]pBuf;
					break;
				}

				char strFilePath[256];
				GetModuleFileNameExA(hProcess, NULL, strFilePath, 256);
				(pManage->m_MonitorObjectList)[i]->m_ObjectInstallPath = strFilePath;
				pManage->DispatchObjProcessToHandleProc((pManage->m_MonitorObjectList)[i], i);
				bIsExist = TRUE;
				delete []pBuf;
			}
			// i += 1;
			bRet = Process32Next(procSnap, &procEntry);
		}
		if (!bIsExist){
			if ((pManage->m_MonitorObjectList)[i]->m_IsExist) {
				CQqObject *qqObj = (CQqObject*)(pManage->m_MonitorObjectList)[i];
				if (strcmp(qqObj->m_ObjectName.c_str(), "QQ.exe") == 0) {
					qqObj->m_IsExist = FALSE;
					qqObj->m_FirstObjectPid = FALSE;
					qqObj->m_SecondObjectPid = FALSE;
					qqObj->m_Count = FALSE;
					qqObj->m_ObjectPid = FALSE;
					printlog("\n%s exit", (pManage->m_MonitorObjectList)[i]->m_ObjectName.c_str());
				}
			}
		}
		i += 1;
		CloseHandle(procSnap);
		Sleep(3000);
	}
	return 0;
}

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	curl_off_t nread;
	/* in real-world cases, this would probably get this data differently
	as this fread() stuff is exactly what the library already would do
	by default internally */
	size_t retcode = fread(ptr, size, nmemb, (FILE*)stream);

	nread = (curl_off_t)retcode;

	/*fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
		" bytes from file\n", nread);*/
	//printlog("*** We read %d bytes from file", nread);
	return retcode;
}

unsigned int __stdcall CManageThreadPool::FtpUploadVideoDataProc(PVOID arg)
{
	BOOL bExit = FALSE, bOffline = FALSE;
	vector<string> readyUploadCacheList;
	CManageThreadPool *pManage = (CManageThreadPool*)arg;
	while (!bExit) 
	{
		if(WAIT_OBJECT_0 == WaitForSingleObject(pManage->m_ExitEvent, 0))
			bExit = TRUE;

		if(WAIT_OBJECT_0 != WaitForSingleObject(pManage->m_StartUploadVideoEvent, 0)) {
			Sleep(50);
			continue;
		}
		recordlog("FtpUploadVideoDataProc  StartUploadVideoEvent");
		CURL *curl = NULL;
		CURLcode res = CURLE_GOT_NOTHING;
		FILE *hd_src = NULL;
		struct stat file_info;
		curl_off_t fsize;
		readyUploadCacheList.clear();
		curl_global_init(CURL_GLOBAL_ALL);

		/*struct curl_slist *headerlist = NULL;
		static const char buf_1 [] = "RNFR " UPLOAD_FILE_AS;
		static const char buf_2 [] = "RNTO " RENAME_FILE_TO;
		*/		

		::EnterCriticalSection(&pManage->m_ReadyUploadVideoListLock);
		if (pManage->m_ReadyUploadVideoList.empty()) {
			::LeaveCriticalSection(&pManage->m_ReadyUploadVideoListLock);
			continue;
		}
		for (int i = 0; i < pManage->m_ReadyUploadVideoList.size(); i++) {
			readyUploadCacheList.push_back((pManage->m_ReadyUploadVideoList)[i]);
		}
		pManage->m_ReadyUploadVideoList.clear();
		::LeaveCriticalSection(&pManage->m_ReadyUploadVideoListLock);

		for (int i = 0; i < readyUploadCacheList.size(); i++)
		{
			hd_src = fopen(readyUploadCacheList[i].c_str(), "r");
			if (hd_src == NULL) {
				printlog("fopen failed");
			}
			fclose(hd_src);
		RESTAT:
			if (stat(readyUploadCacheList[i].c_str(), &file_info)) {
				printlog("Couldn't open '%s': %s\n", readyUploadCacheList[i].c_str(), strerror(errno));
				//return 1;
			}
			fsize = (curl_off_t)file_info.st_size;
			if (fsize <= 0) {
				goto RESTAT;
			}
		REUPLOAD:
			hd_src = fopen(readyUploadCacheList[i].c_str(), "rb");
			if (hd_src == NULL) {
				printlog("fopen failed");
			}
			string remote_url = REMOTE_URL;
			remote_url += readyUploadCacheList[i].c_str();
			//curl_global_init(CURL_GLOBAL_ALL);
			curl = curl_easy_init();

			if(curl) {
				/* build a list of commands to pass to libcurl */ 
				// headerlist = curl_slist_append(headerlist, buf_1);
				// headerlist = curl_slist_append(headerlist, buf_2);

				/* we want to use our own read function */ 
				curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

				/* enable uploading */ 
				curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
				
				/* specify target */ 
				curl_easy_setopt(curl, CURLOPT_URL, remote_url.c_str());
				//curl_easy_setopt(curl, CURLOPT_PORT, 21);
				// curl_easy_setopt(curl, CURLOPT_USERPWD, USRPASSWD);	
				curl_easy_setopt(curl, CURLOPT_USERPWD, pManage->g_sFtpUserNamePsw);	

				/* pass in that last of FTP commands to run after the transfer */ 
				// curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);

				/* now specify which file to upload */ 
				curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);

				/* Set the size of the file to upload (optional).  If you give a *_LARGE
				option you MUST make sure that the type of the passed-in argument is a
				curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
				make sure that to pass in a type 'long' argument. */ 
				curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize);
				//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

				/* Now run off and do what you've been told! */ 
				res = curl_easy_perform(curl);
				/* Check for errors */ 
				if (res != CURLE_OK) {
					// fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
					printlog("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
					switch (res) {
						case CURLE_FAILED_INIT: {
							break;
						}
						case CURLE_URL_MALFORMAT: {
							break;
						}
						case CURLE_COULDNT_CONNECT: {
							bOffline = TRUE;
							break;
						}
						case CURLE_FTP_ACCESS_DENIED: {
							break;
						}
						case CURLE_FTP_USER_PASSWORD_INCORRECT: {
							break;
						}
						case CURLE_UPLOAD_FAILED: {
							break;
						}
						default:
							break;
					}
				}

				/* clean up the FTP commands list */ 
				// curl_slist_free_all(headerlist);

				/* always cleanup */ 
				curl_easy_cleanup(curl);
			}
			fclose(hd_src); /* close the local file */ 
			hd_src = NULL;
			remote_url.clear();
			/*if (bOffline) {
				if(WAIT_OBJECT_0 == WaitForSingleObject(pManage->m_SuccessConnectServerEvent, INFINITE)) {
					bOffline = FALSE;
					goto REUPLOAD;
				}
			}*/
		}
		 curl_global_cleanup();
	}
	return 0;
}

unsigned int __stdcall CManageThreadPool::RecordDesktopProc(PVOID arg)
{
	printlog("RecordDesktopProc");
	BOOL bExit = FALSE;
	CManageThreadPool *pManage = (CManageThreadPool*)arg;
	while (!bExit) 
	{
		if(WAIT_OBJECT_0 == WaitForSingleObject(pManage->m_ExitEvent, 0))
			bExit = TRUE;

		if (WAIT_OBJECT_0 != WaitForSingleObject(pManage->m_StartRecordDesktopEvent, 0)) {
			Sleep(50);
			continue;
		}
		recordlog("m_StartRecordDesktopEvent");
		::InterlockedIncrement(&m_IsRecordDesktoping);

		AVFormatContext* pInputFormatContext = NULL;
		AVCodec* pInputCodec = NULL;
		AVCodecContext* pInputCodecContex = NULL;

		AVFormatContext *pOutputFormatContext = NULL;
		AVCodecContext* pOutCodecContext = NULL;
		AVCodec* pOutCodec = NULL;
		AVStream* pOutStream = NULL;

		AVFilterContext *buffersink_ctx;
		AVFilterContext *buffersrc_ctx;
		AVFilterGraph *filter_graph;
		const char *filter_descr = "movie=my_logo.png[wm];[in][wm]overlay=5:5[out]";

		av_register_all();

		avformat_network_init();

		avdevice_register_all();
		avfilter_register_all();

		char time_buf[50] = { 0 };
		/*SYSTEMTIME sys;
		GetLocalTime(&sys);
		sprintf(time_buf, "%d-%d-%d-%d:%d:%d.%d.flv", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);*/
		sprintf(time_buf, "%ld.flv", time(0));
		string tm;
		tm = pManage->g_sVideo_save_path;
		tm += time_buf;

		ZeroMemory(g_sTimeBuf, 50);
		memcpy(g_sTimeBuf, time_buf, strlen(time_buf));
		recordlog("record 1");
		 int ret, i;
		 int videoindex = -1;
		 //输入（Input）  
		 pInputFormatContext = avformat_alloc_context();
		 AVDictionary* options = NULL;
		 //AVInputFormat *ifmt = av_find_input_format("x11grab");
		 AVInputFormat *ifmt = av_find_input_format("gdigrab");
		 av_dict_set(&options, "framerate", "5", 0);
		 //av_dict_set(&options, "framerate", "5", 0);
		 /*1080P = 1920 * 1080
		 720p = 1280×720
		 480p = 720X480
		 360p = 480×360
		 240p = 320X240*/
		 //av_dict_set(&options,"video_size","1440x900",0); 
		 av_dict_set(&options, "video_size", "1366x768", 0);
		 //av_dict_set(&options, "video_size", "1280x720", 0);
		 //av_dict_set(&options, "video_size", "720x480", 0);
		 //av_dict_set(&options, "video_size", "640x480", 0);
		 //av_dict_set(&options, "video_size", "320x240", 0);
		 //if (avformat_open_input(&pInputFormatContext, ":0.0", ifmt, &options) != 0)
		 if (avformat_open_input(&pInputFormatContext, "desktop", ifmt, &options) != 0)
		 { //Grab at position 10,20 真正的打开文件,这个函数读取文件的头部并且把信息保存到我们给的AVFormatContext结构体中  
			 recordlog("Couldn't open input stream.");
			 return -1;
		 }
		 recordlog("record 2");
		 //AVFormatContext *pAudioFormatCtx = avformat_alloc_context();
		 //AVDictionary* audo_options = NULL;
		 //av_dict_set(&audo_options, "list_options", "true", 0);
		 //AVInputFormat *audio_iformat = av_find_input_format("dshow");
		 ////avformat_open_input(&pAudioFormatCtx, "video=Integrated Camera", iformat, &audo_options);
		 //avformat_open_input(&pAudioFormatCtx, "audio=Internal Microphone (Conexant 20751 SmartAudio HD)", audio_iformat, &audo_options);

		 if ((ret = avformat_find_stream_info(pInputFormatContext, 0)) < 0) {
			 recordlog("Failed to retrieve input stream information");
			 return -1;
		 }

		 for (i = 0; pInputFormatContext->nb_streams; i++)
			 if (pInputFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
				 videoindex = i;
				 break;
			 }
		 recordlog("record 3");
		 pInputCodecContex = pInputFormatContext->streams[videoindex]->codec;// 相当于虚基类,具体参数流中关于编解码器的信息就是被我们叫做"codec context"（编解码器上下文）的东西。  
		 pInputCodec = avcodec_find_decoder(pInputCodecContex->codec_id);//这里面包含了流中所使用的关于编解码器的所有信息，现在我们有了一个指向他的指针。但是我们必需要找到真正的编解码器并且打开  
		 if (pInputCodec == NULL)
		 {
			 recordlog("Codec not found.\n");
			 return -1;
		 }
		 //打开解码器  
		 if (avcodec_open2(pInputCodecContex, pInputCodec, NULL)<0)
		 {
			 recordlog("Could not open codec.\n");
			 return -1;
		 }
		 recordlog("record 4");
		 char args[512];
		 AVFilter *buffersrc = avfilter_get_by_name("buffer");
		 AVFilter *buffersink = avfilter_get_by_name("ffbuffersink");
		 AVFilterInOut *outputs = avfilter_inout_alloc();
		 AVFilterInOut *inputs = avfilter_inout_alloc();
		 enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
		 AVBufferSinkParams *buffersink_params;
		 filter_graph = avfilter_graph_alloc();

		 /* buffer video source: the decoded frames from the decoder will be inserted here. */
		 snprintf(args, sizeof(args),
			 "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			 pInputCodecContex->width, pInputCodecContex->height, pInputCodecContex->pix_fmt,
			 pInputCodecContex->time_base.num, pInputCodecContex->time_base.den,
			 pInputCodecContex->sample_aspect_ratio.num, pInputCodecContex->sample_aspect_ratio.den);

		 ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
			 args, NULL, filter_graph);
		 if (ret < 0) {
			 recordlog("Cannot create buffer source\n");
			 return ret;
		 }
		 recordlog("record 5");
		 /* buffer video sink: to terminate the filter chain. */
		 buffersink_params = av_buffersink_params_alloc();
		 buffersink_params->pixel_fmts = pix_fmts;
		 ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
			 NULL, buffersink_params, filter_graph);
		 av_free(buffersink_params);
		 if (ret < 0) {
			 recordlog("Cannot create buffer sink\n");
			 return ret;
		 }
		 recordlog("record 6");
		 /* Endpoints for the filter graph. */
		 outputs->name = av_strdup("in");
		 outputs->filter_ctx = buffersrc_ctx;
		 outputs->pad_idx = 0;
		 outputs->next = NULL;

		 inputs->name = av_strdup("out");
		 inputs->filter_ctx = buffersink_ctx;
		 inputs->pad_idx = 0;
		 inputs->next = NULL;

		 if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_descr, &inputs, &outputs, NULL)) < 0) {
			 recordlog("avfilter_graph_parse_ptr fail");
			 return ret;
		 }

		if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0) {
			recordlog("recordlog 7");
			return ret;
		}
		recordlog("recordlog 8");
		 //为一帧图像分配内存  
		 AVFrame *pFrame;
		 AVFrame *pFrameYUV, *pFrameOut;
		 pFrame = av_frame_alloc();
		 pFrameYUV = av_frame_alloc();//为转换来申请一帧的内存(把原始帧->YUV)  
		 pFrameOut = av_frame_alloc();

		 pFrameYUV->format = AV_PIX_FMT_YUV420P;
		 pFrameYUV->width = pInputCodecContex->width;
		 pFrameYUV->height = pInputCodecContex->height;
		 unsigned char *out_buffer = (unsigned char *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pInputCodecContex->width, pInputCodecContex->height));
		 //现在我们使用avpicture_fill来把帧和我们新申请的内存来结合  
		 avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pInputCodecContex->width, pInputCodecContex->height);

		 struct SwsContext *img_convert_ctx;
		 img_convert_ctx = sws_getContext(pInputCodecContex->width, pInputCodecContex->height, pInputCodecContex->pix_fmt, pInputCodecContex->width, pInputCodecContex->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
		 //=============================================================================================  
		 avformat_alloc_output_context2(&pOutputFormatContext, NULL, "flv", tm.c_str()); //RTMP  
		 if (!pOutputFormatContext) {
			 recordlog("Could not create output context\n");
			 ret = AVERROR_UNKNOWN;
			 return -1;
		 }
		 recordlog("recordlog 9");
		 //Open output URL 打开输入文件 返回AVIOContext(pFormatCtx->pb)  
		 if (avio_open(&pOutputFormatContext->pb, tm.c_str(), AVIO_FLAG_READ_WRITE) < 0){
			 recordlog("Failed to open output file! \n");
			 return -1;
		 }

		 //创建输出流,AVStream  与  AVCodecContext一一对应  
		 pOutStream = avformat_new_stream(pOutputFormatContext, 0);
		 if (pOutStream == NULL)
		 {
			 recordlog("Failed create pOutStream!\n");
			 return -1;
		 }
		 recordlog("recordlog 10");
		 pOutStream->codec->codec_id = AV_CODEC_ID_H264;
		 pOutStream->codec->pix_fmt = AV_PIX_FMT_YUV420P;
		 pOutStream->codec->width = pInputCodecContex->width;
		 pOutStream->codec->height = pInputCodecContex->height;
		 //pOutStream->codec->bit_rate = 400000;
		 pOutStream->codec->bit_rate = 100000;
		 //pOutStream->codec->bit_rate = 1000 * iQuality;
		 pOutStream->codec->gop_size = 75;
		 pOutStream->time_base.num = 1;
		 pOutStream->time_base.den = 25;

		 //av_dump_format(pOutputFormatContext, 0, out_file, 1);
		 HWND hDesktop = ::GetDesktopWindow();
		 //ASSERT(hDesktop);

		 RECT rect;
		 ::GetWindowRect(hDesktop, &rect);
		 int nWidth = rect.right - rect.left;
		 int nHeight = rect.bottom - rect.top;

		 //相当于虚基类,具体参数  
		 pOutCodecContext = pOutStream->codec;

		 pOutCodecContext->codec_id = AV_CODEC_ID_H264;
		 //type  
		 pOutCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
		 //像素格式,  
		 pOutCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
		 //size  
		 pOutCodecContext->width = pInputCodecContex->width;
		 //pOutCodecContext->width = nWidth;
		 pOutCodecContext->height = pInputCodecContex->height;
		 //pOutCodecContext->height = nHeight;
		 //目标码率  
		 //pOutCodecContext->bit_rate = 400000;
		 pOutCodecContext->bit_rate = 100000;
		 //每250帧插入一个I帧,I帧越小视频越小  
		 //pOutCodecContext->gop_size = 100;
		 //pOutCodecContext->gop_size = 75;
		 pOutCodecContext->gop_size = 15;
		 //Optional Param B帧  
		 //pOutCodecContext->max_b_frames = 3;
		 pOutCodecContext->max_b_frames = 0;

		 pOutCodecContext->time_base.num = 1;
		 pOutCodecContext->time_base.den = 25;
		 //pOutCodecContext->time_base.den = 2;

		 pOutCodecContext->lmin = 1;
		 pOutCodecContext->lmax = 50;
		 //最大和最小量化系数  
		 pOutCodecContext->qmin = 10;
		 pOutCodecContext->qmax = 51;
		 pOutCodecContext->qblur = 0.0;
		 pOutCodecContext->delay = 0;

		 av_opt_set(pOutCodecContext->priv_data, "preset", "superfast", 0);
		 //av_opt_set(pOutCodecContext->priv_data, "preset", "slow", 0);
		 av_opt_set(pOutCodecContext->priv_data, "tune", "zerolatency", 0);

		 // av_dump_format(pOutputFormatContext, 0, out_file, 1);

		 AVDictionary *param = 0;
		 //    //H.264  
		 if (pOutCodecContext->codec_id == AV_CODEC_ID_H264) {
			 //av_dict_set(&param, "preset", "superfast", 0);
			 //av_dict_set(&param, "preset", "slow", 0);
			 //av_dict_set(&param, "tune", "zerolatency", 0);
			 //av_dict_set(&param, "profile", "baseline", 0);
		 }
		 recordlog("recordlog 11");
		 //  
		 // av_dump_format(pOutputFormatContext, 0, out_file, 1);

		 pOutCodec = avcodec_find_encoder(pOutCodecContext->codec_id);
		 if (!pOutCodec){
			 recordlog("Can not find encoder! \n");
			 return -1;
		 }

		 if (avcodec_open2(pOutCodecContext, pOutCodec, &param) < 0)
			 //if (avcodec_open2(pOutCodecContext, pOutCodec, NULL) < 0)
		 {
			 recordlog("Failed to open encoder! \n");
			 return -1;
		 }
		 recordlog("recordlog 12");
		 //Write File Header  
		 int r = avformat_write_header(pOutputFormatContext, NULL);
		 if (r<0)
		 {
			 recordlog("Failed write header!\n");
			 return -1;
		 }

		 AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
		 int got_picture;

		 AVPacket pkt;
		 int picture_size = avpicture_get_size(pOutCodecContext->pix_fmt, pOutCodecContext->width, pOutCodecContext->height);
		 av_new_packet(&pkt, picture_size);
		 recordlog("recordlog 13");
		 int64_t start_time = 0;
		 int frame_index = 0;
		 start_time = av_gettime();
		 recordlog("av_read_frame");
		 while ((av_read_frame(pInputFormatContext, packet)) >= 0)
		 {
			 AVStream *in_stream, *out_stream;
			 if (packet->stream_index == videoindex)
			 {
				 //真正解码,packet to pFrame  
				 avcodec_decode_video2(pInputCodecContex, pFrame, &got_picture, packet);

				 if (got_picture)
				 {
					 pFrame->pts = av_frame_get_best_effort_timestamp(pFrame);
					 if (av_buffersrc_add_frame(buffersrc_ctx, pFrame) < 0) {
						 printf("Error while feeding the filtergraph\n");
						 break;
					 }
					 while (1) {
						 ret = av_buffersink_get_frame(buffersink_ctx, pFrameOut);
						 if (ret < 0)
							 break;
						 //if (pFrameOut->format == AV_PIX_FMT_YUV420P) {
						 /*sws_scale(img_convert_ctx, (const unsigned char* const*)pFrameOut->data, pFrameOut->linesize, 0, pInputCodecContex->height, pFrameYUV->data, pFrameYUV->linesize);
						 pFrameYUV->pts = frame_index;
						 frame_index++;*/

						 //pFrameOut->pts = av_frame_get_best_effort_timestamp(pFrameOut);
						 /*pFrameOut->pts = frame_index;
						 frame_index++;*/
						 int picture;
						 //真正编码 
						 //int ret = avcodec_encode_video2(pOutCodecContext, &pkt, pFrameYUV, &picture);
						 int ret = avcodec_encode_video2(pOutCodecContext, &pkt, pFrameOut, &picture);
						 if (ret < 0){
							 printf("Failed to encode! \n");
							 return -1;
						 }

						 if (pkt.pts == AV_NOPTS_VALUE){
							 //Write PTS
							 AVRational time_base1 = pInputFormatContext->streams[videoindex]->time_base;
							 //Duration between 2 frames (us)
							 int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(pInputFormatContext->streams[videoindex]->r_frame_rate);
							 //Parameters
							 pkt.pts = (double)(frame_index*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
							 pkt.dts = pkt.pts;
							 pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
						 }
						 //Important:Delay
						 if (pkt.stream_index == videoindex){
							 AVRational time_base = pInputFormatContext->streams[videoindex]->time_base;
							 AVRational time_base_q = { 1, AV_TIME_BASE };
							 int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
							 int64_t now_time = av_gettime() - start_time;
							 if (pts_time > now_time)
								 av_usleep(pts_time - now_time);

						 }

						 in_stream = pInputFormatContext->streams[pkt.stream_index];
						 out_stream = pOutputFormatContext->streams[pkt.stream_index];
						 /* copy packet */
						 //Convert PTS/DTS
						 pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
						 pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
						 pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
						 pkt.pos = -1;
						 //Print to Screen
						 if (pkt.stream_index == videoindex){
							 printf("Send %8d video frames to output URL\n", frame_index);
							 frame_index++;
						 }

						 if (picture == 1)
						 {
							 ret = av_interleaved_write_frame(pOutputFormatContext, &pkt);
							 //ret = av_write_frame(pOutputFormatContext, &pkt);
							 if (ret < 0) {
								 printf("Error muxing packet\n");
								 break;
							 }
							 
							 /*if (tu) {
								 bIs = TRUE;
								 break;
							 }
							*/
							 av_free_packet(&pkt);
						 }
						 //}
						 av_frame_unref(pFrameOut);
					 }

					 //sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pInputCodecContex->height, pFrameYUV->data, pFrameYUV->linesize);
					 //pFrameYUV->pts = frame_index;
					 //frame_index++;
					 //int picture;
					 ////真正编码  
					 //int ret = avcodec_encode_video2(pOutCodecContext, &pkt, pFrameYUV, &picture);
					 //if (ret < 0){
					 //	printf("Failed to encode! \n");
					 //	return -1;
					 //}
					 //if (picture == 1)
					 //{
					 //	ret = av_interleaved_write_frame(pOutputFormatContext, &pkt);

					 //	if (ret < 0) {
					 //		printf("Error muxing packet\n");
					 //		break;
					 //	}

					 //	av_free_packet(&pkt);
					 //}
				 }
				 av_frame_unref(pFrame);
			 }
			 av_free_packet(packet);

			 if (WAIT_OBJECT_0 == WaitForSingleObject(pManage->m_EndRecordDesktopEvent, 0)) {
				 recordlog("stop record");
				 ::InterlockedDecrement(&m_IsRecordDesktoping);
				 if (pManage->hTimer != NULL) {
					 DeleteTimerQueueTimer(pManage->hTimerQueue, pManage->hTimer, INVALID_HANDLE_VALUE);
					 pManage->hTimer != NULL;
				 }
				 break;
			 }
		}

		//Write file trailer  
		av_write_trailer(pOutputFormatContext);

		avfilter_graph_free(&filter_graph);
		sws_freeContext(img_convert_ctx);
		//fclose(fp_yuv);  
		av_free(out_buffer);
		av_free(pFrameYUV);
		av_free(pFrame);
		avcodec_close(pInputCodecContex);
		avformat_close_input(&pInputFormatContext);

		avcodec_close(pOutStream->codec);
		av_free(pOutCodec);
		avcodec_close(pOutCodecContext);
		avformat_free_context(pOutputFormatContext);

		::EnterCriticalSection(&pManage->m_ReadyUploadVideoListLock);
		pManage->m_ReadyUploadVideoList.push_back(tm);
		::LeaveCriticalSection(&pManage->m_ReadyUploadVideoListLock);
		 SetEvent(pManage->m_StartUploadVideoEvent);
		 recordlog("m_StartUploadVideoEvent");
	}
	return 0;
}
	
unsigned int __stdcall CManageThreadPool::ReadCameraLiveVideoProc(PVOID arg)
{
	printlog("ReadCameraLiveVideoProc");
	BOOL bExit = FALSE;
	CManageThreadPool *pManage = (CManageThreadPool*)arg;
	while (!bExit) 
	{
		if(WAIT_OBJECT_0 == WaitForSingleObject(pManage->m_ExitEvent, 0))
			bExit = TRUE;

		if (WAIT_OBJECT_0 != WaitForSingleObject(pManage->m_StartLiveVideoEvent, 0)) {
			Sleep(50);
			continue;
		}

		AVFormatContext	*pFormatCtx;
		AVFormatContext	*pAudioFormatCtx;
		int				i, videoindex, audioindex;
		int ret, got_picture;
		AVCodecContext	*pCodecCtx;
		AVCodec			*pCodec;

		int dec_got_frame_a, enc_got_frame_a;
		int nb_samples = 0;

		AVFormatContext *pOutputFormatContext = NULL;
		AVCodecContext* pOutCodecContext = NULL;
		AVCodecContext* pAudioOutCodecContext = NULL;
		AVCodec* pOutCodec = NULL;
		AVCodec* pAudioOutCodec = NULL;
		AVStream* pOutStream = NULL;
		AVStream* pAudioOutStream = NULL;
		int64_t start_time = 0;
		AVFilterContext *buffersink_ctx;
		AVFilterContext *buffersrc_ctx;
		AVFilterGraph *filter_graph;
		const char *filter_descr = "movie=my_logo.png[wm];[in][wm]overlay=5:5[out]";

		SYSTEMTIME sys;
		GetLocalTime(&sys);
		char time_buf[50] = {0};
		sprintf(time_buf, "%d-%d-%d-%d:%d:%d.%d.flv", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
		string tm;
		tm = "E:\\tu\\";
		tm += time_buf;

		// const char *out_file = "E:\\tu\\2.flv";
		//const char* out_file = "rtmp://192.168.2.188:1935/live_video/myvideo";
		//const char* out_file = "rtmp://192.168.2.188:1935/hls/test";
		
		av_register_all();
		avformat_network_init();
		pFormatCtx = avformat_alloc_context();
		pAudioFormatCtx = avformat_alloc_context();
		
		//Open File
		//char filepath[]="src01_480x272_22.h265";
		//avformat_open_input(&pFormatCtx,filepath,NULL,NULL)

		//Register Device
		avdevice_register_all();

		avfilter_register_all();

	//Windows
	#ifdef _WIN32

		////Show Dshow Device
		//show_dshow_device();
		////Show Device Options
		//show_dshow_device_option();
	 //   //Show VFW Options
	 //   show_vfw_device();

	#if USE_DSHOW
		AVInputFormat *ifmt=av_find_input_format("dshow");
		//Set own video device's name
		if(avformat_open_input(&pFormatCtx,"video=Integrated Camera",ifmt,NULL)!=0){
			printf("Couldn't open input stream.\n");
			return -1;
		}
	#else
		AVInputFormat *ifmt=av_find_input_format("vfwcap");
		if(avformat_open_input(&pFormatCtx,"0",ifmt,NULL)!=0){
			printf("Couldn't open input stream.\n");
			return -1;
		}

		AVInputFormat *audio_ifmt = av_find_input_format("dshow");
		//if (avformat_open_input(&pAudioFormatCtx, "audio=Internal Microphone (Conexant 20751 SmartAudio HD)", audio_ifmt, &device_param) != 0)
		if (avformat_open_input(&pAudioFormatCtx, "audio=Internal Microphone (Conexant 20751 SmartAudio HD)", audio_ifmt, NULL) != 0)
		{

			printf("Couldn't open input audio stream.（无法打开输入流）\n");
			return -1;
		}

	#endif
	#elif defined linux
	    //Linux
		AVInputFormat *ifmt=av_find_input_format("video4linux2");
		if(avformat_open_input(&pFormatCtx,"/dev/video0",ifmt,NULL)!=0){
			printf("Couldn't open input stream.\n");
			return -1;
		}
	#else
	    show_avfoundation_device();
	    //Mac
	    AVInputFormat *ifmt=av_find_input_format("avfoundation");
	    //Avfoundation
	    //[video]:[audio]
	    if(avformat_open_input(&pFormatCtx,"0",ifmt,NULL)!=0){
	        printf("Couldn't open input stream.\n");
	        return -1;
	    }
	#endif

		//video
		if(avformat_find_stream_info(pFormatCtx,NULL)<0)
		{
			printf("Couldn't find stream information.\n");
			return -1;
		}
		videoindex=-1;
		for(i=0; i<pFormatCtx->nb_streams; i++) 
			if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
			{
				videoindex=i;
				break;
			}
		if(videoindex==-1)
		{
			printf("Couldn't find a video stream.\n");
			return -1;
		}
		pCodecCtx=pFormatCtx->streams[videoindex]->codec;
		pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
		if(pCodec==NULL)
		{
			printf("Codec not found.\n");
			return -1;
		}
		if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)
		{
			printf("Could not open codec.\n");
			return -1;
		}

		//audio
		if (avformat_find_stream_info(pAudioFormatCtx, NULL) < 0)
		{
			printf("Couldn't find audio stream information.（无法获取流信息）\n");
			return -1;
		}
		audioindex = -1;
		for (i = 0; i < pAudioFormatCtx->nb_streams; i++)
			if (pAudioFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
			{
				audioindex = i;
				break;
			}
		if (audioindex == -1)
		{
			printf("Couldn't find a audio stream.（没有找到视频流）\n");
			return -1;
		}
		if (avcodec_open2(pAudioFormatCtx->streams[audioindex]->codec, avcodec_find_decoder(pAudioFormatCtx->streams[audioindex]->codec->codec_id), NULL) < 0)
		{
			printf("Could not open audio codec.（无法打开解码器）\n");
			return -1;
		}

		char args[512];
		AVFilter *buffersrc = avfilter_get_by_name("buffer");
		AVFilter *buffersink = avfilter_get_by_name("ffbuffersink");
		AVFilterInOut *outputs = avfilter_inout_alloc();
		AVFilterInOut *inputs = avfilter_inout_alloc();
		enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
		AVBufferSinkParams *buffersink_params;

		filter_graph = avfilter_graph_alloc();

		/* buffer video source: the decoded frames from the decoder will be inserted here. */
		snprintf(args, sizeof(args),
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
			pCodecCtx->time_base.num, pCodecCtx->time_base.den,
			pCodecCtx->sample_aspect_ratio.num, pCodecCtx->sample_aspect_ratio.den);

		ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
			args, NULL, filter_graph);
		if (ret < 0) {
			printf("Cannot create buffer source\n");
			return ret;
		}

		/* buffer video sink: to terminate the filter chain. */
		buffersink_params = av_buffersink_params_alloc();
		buffersink_params->pixel_fmts = pix_fmts;
		ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
			NULL, buffersink_params, filter_graph);
		av_free(buffersink_params);
		if (ret < 0) {
			printf("Cannot create buffer sink\n");
			return ret;
		}

		/* Endpoints for the filter graph. */
		outputs->name = av_strdup("in");
		outputs->filter_ctx = buffersrc_ctx;
		outputs->pad_idx = 0;
		outputs->next = NULL;

		inputs->name = av_strdup("out");
		inputs->filter_ctx = buffersink_ctx;
		inputs->pad_idx = 0;
		inputs->next = NULL;

		if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_descr, &inputs, &outputs, NULL)) < 0) {
			if (ret == -2)
				return ret;
			else
				return ret;
		}

		if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
			return ret;

		AVFrame	*pFrame,*pFrameYUV, *pFrameOut;
		pFrame=av_frame_alloc();
		pFrameYUV=av_frame_alloc();
		pFrameOut = av_frame_alloc();

		pFrameYUV->format = AV_PIX_FMT_YUV420P;
		pFrameYUV->width = pCodecCtx->width;
		pFrameYUV->height = pCodecCtx->height;

		unsigned char *out_buffer=(unsigned char *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
		avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);

		AVPacket *packet=(AVPacket *)av_malloc(sizeof(AVPacket));

	#if OUTPUT_YUV420P 
	    FILE *fp_yuv=fopen("output.yuv","wb+");  
	#endif  

		struct SwsContext *img_convert_ctx;
		struct SwrContext *aud_convert_ctx;
		img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL); 
		//------------------------------

		avformat_alloc_output_context2(&pOutputFormatContext, NULL, "flv", tm.c_str()); //RTMP  
		if (!pOutputFormatContext) {
			printf("Could not create output context\n");
			ret = AVERROR_UNKNOWN;
			return -1;
		}
		//Open output URL 打开输入文件 返回AVIOContext(pFormatCtx->pb)  
		if (avio_open(&pOutputFormatContext->pb, tm.c_str(), AVIO_FLAG_READ_WRITE) < 0){
			printf("Failed to open output file! \n");
			return -1;
		}

		//创建输出流,AVStream  与  AVCodecContext一一对应  
		pOutStream = avformat_new_stream(pOutputFormatContext, 0);
		if (pOutStream == NULL)
		{
			printf("Failed create pOutStream!\n");
			return -1;
		}
		//AV_TIME_BASE_Q
		pOutCodecContext = pOutStream->codec;

		pOutCodecContext->codec_id = AV_CODEC_ID_H264;
		//type  
		pOutCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
		//像素格式,  
		pOutCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
		//size  
		pOutCodecContext->width = pCodecCtx->width;
		//pOutCodecContext->width = nWidth;
		pOutCodecContext->height = pCodecCtx->height;
		//pOutCodecContext->height = nHeight;
		//目标码率  
		pOutCodecContext->bit_rate = 400000;
		//pOutCodecContext->bit_rate = 100000;
		//每250帧插入一个I帧,I帧越小视频越小  
		//pOutCodecContext->gop_size = 100;
		pOutCodecContext->gop_size = 75;
		//Optional Param B帧  
		//pOutCodecContext->max_b_frames = 3;
		pOutCodecContext->max_b_frames = 0;

		pOutCodecContext->time_base.num = 1;
		//pOutCodecContext->time_base.den = 25;
		pOutCodecContext->time_base.den = 2;

		pOutCodecContext->lmin = 1;
		pOutCodecContext->lmax = 50;
		//最大和最小量化系数  
		pOutCodecContext->qmin = 10;
		pOutCodecContext->qmax = 51;
		pOutCodecContext->qblur = 0.0;
		pOutCodecContext->delay = 0;

		//av_dump_format(pOutputFormatContext, 0, out_file, 1);

		AVDictionary *param = 0;
		//    //H.264  
		if (pOutCodecContext->codec_id == AV_CODEC_ID_H264) {
			av_dict_set(&param, "preset", "superfast", 0);
			//av_dict_set(&param, "preset", "slow", 0);
			av_dict_set(&param, "tune", "zerolatency", 0);
			//av_dict_set(&param, "profile", "baseline", 0);
		}
		// av_dump_format(pOutputFormatContext, 0, out_file, 1);
		pOutCodec = avcodec_find_encoder(pOutCodecContext->codec_id);
		if (!pOutCodec){
			printf("Can not find encoder! \n");
			return -1;
		}

		if (avcodec_open2(pOutCodecContext, pOutCodec, &param) < 0)
			//if (avcodec_open2(pOutCodecContext, pOutCodec, NULL) < 0)
		{
			printf("Failed to open encoder! \n");
			return -1;
		}

		//audio
		pAudioOutCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
		if (!pAudioOutCodec){
			printf("Can not find output audio encoder! (没有找到合适的编码器！)\n");
			return -1;
		}
		pAudioOutCodecContext = avcodec_alloc_context3(pAudioOutCodec);
		pAudioOutCodecContext->channels = 2;
		pAudioOutCodecContext->channel_layout = av_get_default_channel_layout(2);
		pAudioOutCodecContext->sample_rate = pAudioFormatCtx->streams[audioindex]->codec->sample_rate;
		pAudioOutCodecContext->sample_fmt = pAudioOutCodec->sample_fmts[0];
		pAudioOutCodecContext->bit_rate = 32000;
		pAudioOutCodecContext->time_base.num = 1;
		pAudioOutCodecContext->time_base.den = pAudioOutCodecContext->sample_rate;
		/** Allow the use of the experimental AAC encoder */
		pAudioOutCodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
		/* Some formats want stream headers to be separate. */
		if (pOutputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
			pAudioOutCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
		if (avcodec_open2(pAudioOutCodecContext, pAudioOutCodec, NULL) < 0){
			printf("Failed to open ouput audio encoder! (编码器打开失败！)\n");
			return -1;
		}

		//Add a new stream to output,should be called by the user before avformat_write_header() for muxing
		pAudioOutStream = avformat_new_stream(pOutputFormatContext, pAudioOutCodec);
		if (pAudioOutStream == NULL){
			return -1;
		}
		pAudioOutStream->time_base.num = 1;
		pAudioOutStream->time_base.den = pAudioOutCodecContext->sample_rate;
		pAudioOutStream->codec = pAudioOutCodecContext;

		// av_dump_format(pOutputFormatContext, 0, out_file, 1);
		//Write File Header  
		int r = avformat_write_header(pOutputFormatContext, NULL);
		if (r<0)
		{
			printf("Failed write header!\n");
			return -1;
		}

		// Initialize the resampler to be able to convert audio sample formats
		aud_convert_ctx = swr_alloc_set_opts(NULL,
			av_get_default_channel_layout(pAudioOutCodecContext->channels),
			pAudioOutCodecContext->sample_fmt,
			pAudioOutCodecContext->sample_rate,
			av_get_default_channel_layout(pAudioFormatCtx->streams[audioindex]->codec->channels),
			pAudioFormatCtx->streams[audioindex]->codec->sample_fmt,
			pAudioFormatCtx->streams[audioindex]->codec->sample_rate,
			0, NULL);
		swr_init(aud_convert_ctx);

		//Initialize the FIFO buffer to store audio samples to be encoded. 
		AVAudioFifo *fifo = NULL;
		fifo = av_audio_fifo_alloc(pAudioOutCodecContext->sample_fmt, pAudioOutCodecContext->channels, 1);

		//Initialize the buffer to store converted samples to be encoded.
		uint8_t **converted_input_samples = NULL;
		/**
		* Allocate as many pointers as there are audio channels.
		* Each pointer will later point to the audio samples of the corresponding
		* channels (although it may be NULL for interleaved formats).
		*/
		if (!(converted_input_samples = (uint8_t**)calloc(pAudioOutCodecContext->channels,
			sizeof(**converted_input_samples)))) {
			printf("Could not allocate converted input sample pointers\n");
			return AVERROR(ENOMEM);
		}

		AVPacket pkt;
		int picture_size = avpicture_get_size(pOutCodecContext->pix_fmt, pOutCodecContext->width, pOutCodecContext->height);
		av_new_packet(&pkt, picture_size);

		int frame_index = 0;
		start_time = av_gettime();
		int encode_video = 1, encode_audio = 1;
		int aud_next_pts = 0;
		int vid_next_pts = 0;
		AVRational time_base_q = { 1, AV_TIME_BASE };

		while (encode_video || encode_audio)
		{
			if (encode_video && (!encode_audio || av_compare_ts(vid_next_pts, time_base_q, aud_next_pts, time_base_q) <= 0))
			{
				if ((ret = av_read_frame(pFormatCtx, packet)) >= 0) {
					AVStream *in_stream, *out_stream;
					//if (packet->stream_index == videoindex) {
						avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);

						if (got_picture)
						{
							pFrame->pts = av_frame_get_best_effort_timestamp(pFrame);
							/* push the decoded frame into the filtergraph */
							if (av_buffersrc_add_frame(buffersrc_ctx, pFrame) < 0) {
								printf("Error while feeding the filtergraph\n");
								break;
							}
							while (1) {
								ret = av_buffersink_get_frame(buffersink_ctx, pFrameOut);
								if (ret < 0)
									break;
								//if (pFrameOut->format == AV_PIX_FMT_YUV420P) {
									/*sws_scale(img_convert_ctx, (const unsigned char* const*)pFrameOut->data, pFrameOut->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
									pFrameYUV->pts = frame_index;
									frame_index++;*/

								pFrameOut->pts = av_frame_get_best_effort_timestamp(pFrameOut);
								//pFrameOut->pts = frame_index;
								//frame_index++;
								int picture;
								//真正编码  
								//int ret = avcodec_encode_video2(pOutCodecContext, &pkt, pFrameYUV, &picture);
								int ret = avcodec_encode_video2(pOutCodecContext, &pkt, pFrameOut, &picture);
								if (ret < 0){
									printf("Failed to encode! \n");
									return -1;
								}
								int64_t calc_duration;
								if (picture == 1){
									frame_index +=1;
									pkt.stream_index = pOutStream->index;

									//Write PTS
									AVRational time_base = pOutputFormatContext->streams[0]->time_base;//{ 1, 1000 };
									AVRational r_framerate1 = pFormatCtx->streams[videoindex]->r_frame_rate;//{ 50, 2 }; 
									//Duration between 2 frames (us)
									int64_t calc_duration = (double)(AV_TIME_BASE)*(1 / av_q2d(r_framerate1));	//内部时间戳
									//Parameters
									//enc_pkt.pts = (double)(framecnt*calc_duration)*(double)(av_q2d(time_base_q)) / (double)(av_q2d(time_base));
									pkt.pts = av_rescale_q(frame_index*calc_duration, time_base_q, time_base);
									pkt.dts = pkt.pts;
									pkt.duration = av_rescale_q(calc_duration, time_base_q, time_base); //(double)(calc_duration)*(double)(av_q2d(time_base_q)) / (double)(av_q2d(time_base));
									pkt.pos = -1;
									//printf("video pts : %d\n", enc_pkt.pts);

									vid_next_pts = frame_index*calc_duration; //general timebase

									//Delay
									int64_t pts_time = av_rescale_q(pkt.pts, time_base, time_base_q);
									int64_t now_time = av_gettime() - start_time;
									if ((pts_time > now_time) && ((vid_next_pts + pts_time - now_time) < aud_next_pts))
										av_usleep(pts_time - now_time);
									//ret = av_write_frame(pOutputFormatContext, &pkt);
									ret = av_interleaved_write_frame(pOutputFormatContext, &pkt);
									if (ret < 0) {
										printf("Error muxing packet\n");
										break;
									}
									av_free_packet(&pkt);
								}
								av_frame_unref(pFrameOut);
							}
						}
						av_frame_unref(pFrame);
					//}
					av_free_packet(packet);
				}
				else {
					if (ret == AVERROR_EOF)
						encode_video = 0;
					else
					{
						printf("Could not read video frame\n");
						return ret;
					}
				}
			}
			else
			{
				const int output_frame_size = pAudioOutCodecContext->frame_size;

				/*if (exit_thread)
					break;*/

				/**
				* Make sure that there is one frame worth of samples in the FIFO
				* buffer so that the encoder can do its work.
				* Since the decoder's and the encoder's frame size may differ, we
				* need to FIFO buffer to store as many frames worth of input samples
				* that they make up at least one frame worth of output samples.
				*/
				while (av_audio_fifo_size(fifo) < output_frame_size) {
					/**
					* Decode one frame worth of audio samples, convert it to the
					* output sample format and put it into the FIFO buffer.
					*/
					AVFrame *input_frame = av_frame_alloc();
					if (!input_frame)
					{
						ret = AVERROR(ENOMEM);
						return ret;
					}

					/** Decode one frame worth of audio samples. */
					/** Packet used for temporary storage. */
					AVPacket input_packet;
					av_init_packet(&input_packet);
					input_packet.data = NULL;
					input_packet.size = 0;

					/** Read one audio frame from the input file into a temporary packet. */
					if ((ret = av_read_frame(pAudioFormatCtx, &input_packet)) < 0) {
						/** If we are at the end of the file, flush the decoder below. */
						if (ret == AVERROR_EOF)
						{
							encode_audio = 0;
						}
						else
						{
							printf("Could not read audio frame\n");
							return ret;
						}
					}

					/**
					* Decode the audio frame stored in the temporary packet.
					* The input audio stream decoder is used to do this.
					* If we are at the end of the file, pass an empty packet to the decoder
					* to flush it.
					*/
					if ((ret = avcodec_decode_audio4(pAudioFormatCtx->streams[audioindex]->codec, input_frame,
						&dec_got_frame_a, &input_packet)) < 0) {
						printf("Could not decode audio frame\n");
						return ret;
					}
					av_packet_unref(&input_packet);
					/** If there is decoded data, convert and store it */
					if (dec_got_frame_a) {
						/**
						* Allocate memory for the samples of all channels in one consecutive
						* block for convenience.
						*/
						if ((ret = av_samples_alloc(converted_input_samples, NULL,
							pAudioOutCodecContext->channels,
							input_frame->nb_samples,
							pAudioOutCodecContext->sample_fmt, 0)) < 0) {
							printf("Could not allocate converted input samples\n");
							av_freep(&(*converted_input_samples)[0]);
							free(*converted_input_samples);
							return ret;
						}

						/**
						* Convert the input samples to the desired output sample format.
						* This requires a temporary storage provided by converted_input_samples.
						*/
						/** Convert the samples using the resampler. */
						if ((ret = swr_convert(aud_convert_ctx,
							converted_input_samples, input_frame->nb_samples,
							(const uint8_t**)input_frame->extended_data, input_frame->nb_samples)) < 0) {
							printf("Could not convert input samples\n");
							return ret;
						}

						/** Add the converted input samples to the FIFO buffer for later processing. */
						/**
						* Make the FIFO as large as it needs to be to hold both,
						* the old and the new samples.
						*/
						if ((ret = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + input_frame->nb_samples)) < 0) {
							printf("Could not reallocate FIFO\n");
							return ret;
						}

						/** Store the new samples in the FIFO buffer. */
						if (av_audio_fifo_write(fifo, (void **)converted_input_samples,
							input_frame->nb_samples) < input_frame->nb_samples) {
							printf("Could not write data to FIFO\n");
							return AVERROR_EXIT;
						}
					}
				}

				/**
				* If we have enough samples for the encoder, we encode them.
				* At the end of the file, we pass the remaining samples to
				* the encoder.
				*/
				if (av_audio_fifo_size(fifo) >= output_frame_size)
					/**
					* Take one frame worth of audio samples from the FIFO buffer,
					* encode it and write it to the output file.
					*/
				{
					/** Temporary storage of the output samples of the frame written to the file. */
					AVFrame *output_frame = av_frame_alloc();
					if (!output_frame)
					{
						ret = AVERROR(ENOMEM);
						return ret;
					}
					/**
					* Use the maximum number of possible samples per frame.
					* If there is less than the maximum possible frame size in the FIFO
					* buffer use this number. Otherwise, use the maximum possible frame size
					*/
					const int frame_size = FFMIN(av_audio_fifo_size(fifo),
						pAudioOutCodecContext->frame_size);

					/** Initialize temporary storage for one output frame. */
					/**
					* Set the frame's parameters, especially its size and format.
					* av_frame_get_buffer needs this to allocate memory for the
					* audio samples of the frame.
					* Default channel layouts based on the number of channels
					* are assumed for simplicity.
					*/
					output_frame->nb_samples = frame_size;
					output_frame->channel_layout = pAudioOutCodecContext->channel_layout;
					output_frame->format = pAudioOutCodecContext->sample_fmt;
					output_frame->sample_rate = pAudioOutCodecContext->sample_rate;

					/**
					* Allocate the samples of the created frame. This call will make
					* sure that the audio frame can hold as many samples as specified.
					*/
					if ((ret = av_frame_get_buffer(output_frame, 0)) < 0) {
						printf("Could not allocate output frame samples\n");
						av_frame_free(&output_frame);
						return ret;
					}

					/**
					* Read as many samples from the FIFO buffer as required to fill the frame.
					* The samples are stored in the frame temporarily.
					*/
					if (av_audio_fifo_read(fifo, (void **)output_frame->data, frame_size) < frame_size) {
						printf("Could not read data from FIFO\n");
						return AVERROR_EXIT;
					}

					/** Encode one frame worth of audio samples. */
					/** Packet used for temporary storage. */
					AVPacket output_packet;
					av_init_packet(&output_packet);
					output_packet.data = NULL;
					output_packet.size = 0;

					/** Set a timestamp based on the sample rate for the container. */
					if (output_frame) {
						nb_samples += output_frame->nb_samples / 3;
						//nb_samples += 3;
					}

					/**
					* Encode the audio frame and store it in the temporary packet.
					* The output audio stream encoder is used to do this.
					*/
					if ((ret = avcodec_encode_audio2(pAudioOutCodecContext, &output_packet,
						output_frame, &enc_got_frame_a)) < 0) {
						printf("Could not encode frame\n");
						av_packet_unref(&output_packet);
						return ret;
					}

					/** Write one audio frame from the temporary packet to the output file. */
					if (enc_got_frame_a) {

						output_packet.stream_index = 1;

						AVRational time_base = pOutputFormatContext->streams[1]->time_base;
						AVRational r_framerate1 = { pAudioFormatCtx->streams[audioindex]->codec->sample_rate, 1 };// { 44100, 1};  
						int64_t calc_duration = (double)(AV_TIME_BASE)*(1 / av_q2d(r_framerate1));  //内部时间戳  

						output_packet.pts = av_rescale_q(nb_samples*calc_duration, time_base_q, time_base);
						output_packet.dts = output_packet.pts;
						output_packet.duration = output_frame->nb_samples;

						//printf("audio pts : %d\n", output_packet.pts);
						aud_next_pts = nb_samples*calc_duration;

						int64_t pts_time = av_rescale_q(output_packet.pts, time_base, time_base_q);
						int64_t now_time = av_gettime() - start_time;
						if ((pts_time > now_time) && ((aud_next_pts + pts_time - now_time) < vid_next_pts))
							av_usleep(pts_time - now_time);
						
						if ((ret = av_interleaved_write_frame(pOutputFormatContext, &output_packet)) < 0) 
						//if ((ret = av_write_frame(pOutputFormatContext, &output_packet)) < 0)
						{
							printf("Could not write frame\n");
							av_packet_unref(&output_packet);
							return ret;
						}

						av_packet_unref(&output_packet);
					}
					av_frame_free(&output_frame);
				}
			}
			if(WAIT_OBJECT_0 != WaitForSingleObject(pManage->m_EndLiveVideoEvent, 0))
				break;
		}

		//Write file trailer  
		av_write_trailer(pOutputFormatContext);

		avfilter_graph_free(&filter_graph);
		sws_freeContext(img_convert_ctx);
		//fclose(fp_yuv);  
		av_free(out_buffer);
		av_free(pFrameYUV);
		av_free(pFrame);
		avcodec_close(pCodecCtx);
		avformat_close_input(&pFormatCtx);

		avcodec_close(pOutStream->codec);
		av_free(pOutCodec);
		avcodec_close(pOutCodecContext);
		avformat_free_context(pOutputFormatContext);
		if (fifo)
			av_audio_fifo_free(fifo);
		if (converted_input_samples) {
			av_freep(&converted_input_samples[0]);
			//free(converted_input_samples);
		}
	}
	return 0;
}

char FirstDriveFromMask(ULONG unitmask)
{
	char i;

	for (i = 0; i < 26; ++i)
	{
		if (unitmask & 0x1)
			break;
		unitmask = unitmask >> 1;
	}
	return (i + 'A');
}

LRESULT OnDeviceChange(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
	switch (wParam)
	{
	case DBT_DEVICEARRIVAL:{
		if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
		{
			PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
			U[0] = FirstDriveFromMask(lpdbv->dbcv_unitmask);
			string Utemp(U);
			Utemp += "\\";
			printlogA(Utemp.c_str());
			//Filesearch(Utemp, 0);  //搜索u盘
			//CopyPPT(FileList);    //复制ppt
			//CopyDOC(FileList);    //复制doc
		}
		break;
	}
	case DBT_DEVICEREMOVECOMPLETE:{
		printlog("u quit");
		break;
	}
	}
	return LRESULT();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

	case WM_CREATE:{
		U[1] = ':';
		SetTimer(hWnd, TIMER, 5000, 0);
		break;
	}
	case WM_TIMER:{
		SendMessage(hWnd, WM_DEVICECHANGE, 0, 0);
		return 0;
	}
	case WM_DEVICECHANGE:{
		OnDeviceChange(hWnd, wParam, lParam);
		break;
	}
	case WM_DESTROY:{
		KillTimer(hWnd, TIMER);
		PostQuitMessage(0);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

unsigned int __stdcall CManageThreadPool::MonitorUsbDriveProc(PVOID arg)
{
	printlog("MonitorUsbDriveProc");
	static TCHAR szAppName[] = TEXT("UUUUUU");
	HWND               hwnd;
	MSG                msg;
	WNDCLASS           wndclass;

	wndclass.style = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = GetModuleHandle(NULL);
	wndclass.hIcon = 0;
	wndclass.hCursor = 0;
	wndclass.hbrBackground = 0;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("Program requires Windows NT!"),
			szAppName, MB_ICONERROR);
		return 0;
	}
	hwnd = CreateWindow(szAppName, NULL,
		WS_DISABLED,
		0, 0,
		0, 0,
		NULL, NULL, GetModuleHandle(NULL), NULL);
	if (NULL == hwnd) {
		GetLastError();
	}
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}


//TODO
unsigned int __stdcall CManageThreadPool::CommunicateWithServerProc(PVOID arg)
{
	recordlog("CommunicateWithServerProc");
	int nRet = -1;
	BOOL bRet = FALSE;
	BOOL bReconnect = FALSE;
	BOOL bUpdateRule = FALSE;
	LONG idx = 0;
	/*__tag1* t = (__tag1*)malloc(sizeof(__tag1));
	CThreadPool *pThreadPool = (CThreadPool*)t->arg;
	CManageThreadPool *pManage = (CManageThreadPool*)t->arg1;*/
	CManageThreadPool *pManage = (CManageThreadPool*)arg;

	CConnector *pConnector = NULL;

RECONNECT:
	pConnector = new CConnector(pManage, pManage->g_sWebMainServerIP, pManage->g_sWebMainServerIP, atoi(pManage->g_sWebServerPort), CConnector::STATE_CLIENT);
	if (NULL == pConnector) {
		recordlog("connect web server failed!");
	} else {	
		/*CSLock lock(pManage->m_ConnClientListLock);
		pManage->m_ConnClientList.push_back(pConnector);*/
	}

	recordlog("connect web server successed!");

	SOCKET sockfd = pConnector->GetSocketHandle();
	recordlog("connect sucess");
UPDATERULE:
	if (!bUpdateRule) {
		bRet = pManage->SendRuleRequestToServer(sockfd);
		if (!bRet) {
			goto RECONNECT;
		}
	} else {
		SetEvent(pManage->m_SuccessConnectServerEvent);
	    /*bRet = pManage->SendRuleUpdateRequestToServer(sockfd);
	    if (!bRet) {
	    	goto RECONNECT;
	    }
		bUpdateRule = FALSE;
		*/	
	}

	INT nCommandLen;
	INT nPkgSizeLen = 0;
	INT nActionLen;
	INT nCheckLen;
	INT nPkgTagLen;
	char sBuffer[BUFFERSIZE] = {0};
	char sRuleData[BUFFERSIZE] = {0};
	//LONG idx = 0;
	BOOL isEnough = FALSE;
	/*PPKG_HEAD_STRUCT pHead = (PPKG_HEAD_STRUCT)malloc(sizeof(PKG_HEAD_STRUCT));
	pHead->command = PACKET_COMMAND_NONE;
	pHead->action = PACKET_ACTION_NONE;
	pHead->check = PACKET_CHECK_NONE;
	pHead->pkgSize = 0;
	pHead->pkgTag = 0;*/
	/*char buf[1024] = {0};
	int bytes_read = recv(sockfd, buf, 1024, 0);
	printlog(buf);*/

	while (WAIT_OBJECT_0 != WaitForSingleObject(pManage->m_ExitEvent, 0))
	{
		struct timeval tm;
	    int len, err = -1;
	    tm.tv_sec = 5;  
	    tm.tv_usec = 0;  
	    fd_set wset;
	    fd_set rset;
	    //FD_ZERO(&wset);  
	    FD_ZERO(&rset);  
	    //FD_SET(sockfd, &wset);  
	    FD_SET(sockfd, &rset);  
	    int retval = select(sockfd + 1, &rset, NULL, NULL, &tm);  
	    switch(retval)  
	    {  
	        case -1:  
	        {
	            perror("select");  
	            bReconnect = TRUE;
	            break;;  
	        }
	        case 0:  
	        {
	            //printlog("connect timeout\n");  
				//bReconnect = TRUE;
	            break;;  
	        }
	        case 1:
	        {
	            /*if (FD_ISSET(sockfd, &wset))  
	            {  
	                printlog("build connect successfully!\n");
	            }*/
	            if (FD_ISSET(sockfd, &rset))
	            {
					int bytes_read = 0;
					LONG nLength = 0;
					//bytes_read = recv(sockfd, buffer + idx, 1, 0 );
					bytes_read = recv(sockfd, sBuffer + idx, 1, 0);
					if (bytes_read < 0) {
						if (errno == EAGAIN || errno == EWOULDBLOCK) {
							continue;
							// return FALSE;
						}
						else {
				            bReconnect = TRUE;
							// return FALSE;
						}
					}
					else if (bytes_read == 0) {
			            bReconnect = TRUE;
						// return FALSE;
					}
					else if (bytes_read > 0) {
						idx += bytes_read;
						if (idx == sizeof(PKG_HEAD_STRUCT)) {
							//INT nCommandLen;
							memcpy(&nCommandLen, sBuffer, sizeof(INT));
							//INT nActionLen;
							memcpy(&nActionLen, sBuffer + 4, sizeof(INT));
							//INT nCheckLen;
							memcpy(&nCheckLen, sBuffer + 8, sizeof(INT));
							//INT nPkgSizeLen;
							memcpy(&nPkgSizeLen, sBuffer + 12, sizeof(INT));
							//INT nPkgTagLen;
							memcpy(&nPkgTagLen, sBuffer + 16, sizeof(INT));
							//nLength = nPkgSizeLen - sizeof(PKG_HEAD_STRUCT);
							/*if (idx == nPkgSizeLen) {
								memcpy(sRuleData, sBuffer + sizeof(PKG_HEAD_STRUCT), nLength);
								printlog(sRuleData);
								isEnough = TRUE;
							}*/
						}
						nLength = nPkgSizeLen - sizeof(PKG_HEAD_STRUCT);
						if (idx == nPkgSizeLen) {
							memcpy(sRuleData, sBuffer + sizeof(PKG_HEAD_STRUCT), nLength);
							printlog(sRuleData);
							isEnough = TRUE;
						}
					}

					/*bRet = pManage->ReceiveDataFromServer(sockfd, sBuffer, idx, pHead, sRuleData, isEnough);
	            	if (!bRet) {
			            bReconnect = TRUE;
				    	bUpdateRule = TRUE;
	            	}*/
					
	            	if (isEnough) {
						pManage->DispatchDataToHandleProc(nCommandLen, sRuleData);
						//pManage->DispatchDataToHandleProc(pHead, sRuleData);
	            	}
	            }
	            break;
	        }
	        default:  
	        {
	            /*if (FD_ISSET(sockfd, &wset))  
	            {  
	                if (getsockopt(sockfd,SOL_SOCKET,SO_ERROR, (char*)&err, (socklen_t *)&len) < 0)  
	                {  
			            bReconnect = TRUE;
	                }  
	                if (err != 0)  
	                {  
			            bReconnect = TRUE;
	                }  
	            }*/
	            break;  
	        }
	    }
	    if (bReconnect) {
	    	bUpdateRule = TRUE;
	    	//free(pHead);
	    	goto RECONNECT;
	    }
		if (WAIT_OBJECT_0 == WaitForSingleObject(pManage->m_MonitorDataListEvent, 0)) {
			//CSLock lock(pManage->m_MonitorDataListLock);
			::EnterCriticalSection(&pManage->m_MonitorDataListLock);
			if (pManage->m_MonitorDataList.empty()) {
				continue;
			}
			vector<string>::iterator it = pManage->m_MonitorDataList.begin();
			for (int i = 0; i < pManage->m_MonitorDataList.size(); i++) {
				bRet = pManage->SendMonitorDataToServer(sockfd, (pManage->m_MonitorDataList)[i]);
				if (!bRet) {
					bUpdateRule = TRUE;
					//free(pHead);
					goto RECONNECT;
				}
			}
			//pManage->m_RapidJsonPtr.GenerateJson(*it);
			pManage->m_MonitorDataList.clear();
			::LeaveCriticalSection(&pManage->m_MonitorDataListLock);
		}
	}
	//free(pHead);
	return 0;
}

unsigned int __stdcall CManageThreadPool::RecordLogProc(PVOID arg)
{
	printlog("RecordLogProc");
	BOOL bExit = FALSE;
	CManageThreadPool *pManage = (CManageThreadPool*)arg;
	FILE* pFile = NULL;
REOPEN:
    pFile = fopen(DEFAULT_AGENT_LOG_FILE_PATH, "a+");
	while (!bExit)
	{
		if(WAIT_OBJECT_0 == WaitForSingleObject(pManage->m_ExitEvent, 0))
			bExit = TRUE;
		if (WAIT_OBJECT_0 != WaitForSingleObject(pManage->m_SaveLogInfoListEvent, 0)) {
			Sleep(50);
			continue;
		}
		CSLock lock(pManage->m_SaveLogInfoListLock);
		if (pManage->m_SaveLogInfoList.empty()) {
			continue;
		}
		vector<char *>::iterator it;
		*it = pManage->m_SaveLogInfoList.front();
REWRITE:
        if (pFile == NULL)
            goto REOPEN;
        int nInfoLen = strlen(*it);
        int nRetCode = fwrite(*it, sizeof(char), nInfoLen, pFile);
        if (nRetCode != nInfoLen)
            goto REWRITE;
        else
        {
            fflush(pFile);
        }
		pManage->m_SaveLogInfoList.erase(it);
	}
	return 0;
}
BOOL CManageThreadPool::WaitForSendMonitorDataToServer()
{
	/*if (WAIT_OBJECT_0 == WaitForSingleObject(pManage->m_MonitorDataListEvent, 0)) {
		CSLock lock(pManage->m_MonitorDataListLock);
		if (pManage->m_MonitorDataList.empty()) {
		continue;
		}
		vector<string>::iterator it;
		*it = pManage->m_MonitorDataList.front();
		bRet = pManage->SendMonitorDataToServer(sockfd, *it);
		if (!bRet) {
		bUpdateRule = TRUE;
		free(pHead);
		goto RECONNECT;
		}
		pManage->m_MonitorDataList.erase(it);
	}*/
	return TRUE;
}

void CManageThreadPool::Log(char *pszFormat, ...)
{
	va_list   pArgList;
	va_start(pArgList, pszFormat);
	char buf[1024];
	size_t n = now_s(buf, 1024);
	vsnprintf_s(buf + n, 1024 - n, _TRUNCATE, pszFormat, pArgList);
	va_end(pArgList);
	CSLock lock(m_SaveLogInfoListLock);
	m_SaveLogInfoList.push_back(buf);
	SetEvent(m_SaveLogInfoListEvent);
}

CWorkThread::CWorkThread() :
	m_ThreadPool(NULL),
	m_Busy(FALSE),
	m_Exit(FALSE),
	m_HandleThr(INVALID_HANDLE_VALUE),
	//TaskFunCb(NULL),
	m_Param(NULL)
	//TaskCallFunCb(NULL)
{

}

CWorkThread::CWorkThread(void *arg, volatile LONG &num, void *tag) :
	m_ThreadPool(NULL),
	m_Busy(FALSE),
	m_Exit(FALSE),
	m_HandleThr(INVALID_HANDLE_VALUE),
	//TaskFunCb(NULL),
	m_Param(NULL)
	//TaskCallFunCb(NULL)
{
	/*__tag1* t = (__tag1*)malloc(sizeof(__tag1));
	t->arg = arg;
	t->arg1 = tag;*/
	m_HandleThr = (HANDLE)_beginthreadex(NULL, 0, CManageThreadPool::CommunicateWithServerProc, tag, CREATE_SUSPENDED, NULL);
	//t->hdThread = m_HandleThr;
	if (m_HandleThr == 0) {
		//printlog("CreateThread failed. LastError: %u\n", GetLastError());
		//free(t);
		throw exception();
	}
	::ResumeThread(m_HandleThr);
	::InterlockedIncrement(&num);
	//free(t);
}
//TODO
CWorkThread::CWorkThread(void *arg, volatile LONG &num) :
	m_ThreadPool(NULL),
	m_Busy(FALSE),
	m_Exit(FALSE),
	m_HandleThr(INVALID_HANDLE_VALUE),
	//TaskFunCb(NULL),
	m_Param(NULL)
	//TaskCallFunCb(NULL)
{
	/*__tag1* t = (__tag1*)malloc(sizeof(__tag1));
	////t->arg = arg;
	t->arg1 = arg1;*/
	// m_HandleThr = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, t, CREATE_SUSPENDED, NULL);
	m_HandleThr = (HANDLE)_beginthreadex(NULL, 0, CIOCPModel::_WorkerThread, arg, CREATE_SUSPENDED, NULL);
	//t->hdThread = m_HandleThr;
	if (m_HandleThr == 0)
	{
		printlog("CreateThread failed. LastError: %u\n", GetLastError());
		//free(t);
		throw exception();
	}
	::ResumeThread(m_HandleThr);
	::InterlockedIncrement(&num);
	//free(t);
}

CWorkThread::~CWorkThread()
{

}

void CWorkThread::Start(void *proc, void *arg)
{
	/*__tag1* t = (__tag1*)malloc(sizeof(__tag1));
	t->arg = NULL;
	t->arg1 = NULL;*/

	m_HandleThr = (HANDLE)_beginthreadex(NULL, 0, (unsigned int (__stdcall*)(void*))proc, arg, CREATE_SUSPENDED, NULL);
	//t->hdThread = m_HandleThr;
	if (m_HandleThr == 0)
	{
		printlog("CreateThread failed. LastError: %u\n", GetLastError());
		//free(t);
		throw exception();
	}
	::ResumeThread(m_HandleThr);
	// ::InterlockedIncrement(&num);
}

unsigned int __stdcall CWorkThread::ThreadProc(PVOID arg)
{
	__tag1 * t = (__tag1*)arg;
	CWorkThread * tp = (CWorkThread*)t->arg;
	HANDLE hThread = t->hdThread;
	free(t);
	tp->Run(hThread);
	return 0;
}

void CWorkThread::Run(HANDLE hThread)
{
	
}

//TODO
CThreadPool::CThreadPool(void *arg)
{
	InitCS(&m_IdleThreadListLock);
	//m_Manage = (CManageThreadPool*)arg;
	BOOL ret = Init(arg);
	if (ret) {

	}
}

CThreadPool::CThreadPool(int num, void *arg)
{
	InitCS(&m_IdleThreadListLock);
	m_Manage = (CManageThreadPool*)arg;
	BOOL ret = Init(arg, num);
	if (ret) {

	}
}

CThreadPool::~CThreadPool()
{
	DestCS(&m_IdleThreadListLock);
	if (m_Manage != NULL) 
		delete m_Manage;
}

//TODO
BOOL CThreadPool::Init(void *arg, int num, void *arg1)
{
	recordlog("init");
	assert(num >= 0);
	switch (num) {
	case 0:
		{
			/*__tag2* t = (__tag2*)malloc(sizeof(__tag2));
			__tag2* t1 = (__tag2*)malloc(sizeof(__tag2));
			t1->arg = arg;
			t1->arg1 = arg1;
			t->arg = (void*)this;
			t->arg1 = (void*)t1;*/
			int nLogicCpu, nCpu;
			GetProcessNum(nLogicCpu, nCpu);
			if (m_MinThreadNums < nLogicCpu * 2)
				//m_MinThreadNums = nLogicCpu * 2;
				m_MinThreadNums = 2;
			if (m_MaxThreadNums < m_MinThreadNums + nLogicCpu * 2)
				m_MaxThreadNums = m_MinThreadNums + nLogicCpu * 2;
			try {
				CSLock lock(m_IdleThreadListLock);
				for (int i = 0; i < m_MinThreadNums; i++) {
					//m_IdleThreadList.push_back(new CWorkThread(t, m_ThreadNum));
					m_IdleThreadList.push_back(new CWorkThread(arg, m_ThreadNum));
				}
				// CreateIdleThread(CommunicateWithClientProc);
				/*CWorkThread *thr = NULL;
				thr = new CWorkThread();
				thr->Start(CManageThreadPool::FtpVideoDataProc);*/
			}
			catch (exception &e) {
				cout << e.what() << endl;
				return FALSE;
			}
			m_PoolState = pool_opening;
			/*free(t);
			free(t1);*/
			break;
		}
	default:
		{
			m_PoolState = pool_opening;
			try {
				CSLock lock(m_IdleThreadListLock);
				for (int i = 0; i < num; i++) {
					m_IdleThreadList.push_back(new CWorkThread(this, m_ThreadNum, arg));
				}
				CWorkThread *thr = NULL;
				thr = new CWorkThread();
				thr->Start(CManageThreadPool::FtpUploadVideoDataProc, arg);
				m_IdleThreadList.push_back(thr);
				::InterlockedIncrement(&m_ThreadNum);

				thr = NULL;
				thr = new CWorkThread();
				thr->Start(CManageThreadPool::MonitorProcessProc, arg);
				m_IdleThreadList.push_back(thr);
				::InterlockedIncrement(&m_ThreadNum);

				thr = NULL;
				thr = new CWorkThread();
				thr->Start(CManageThreadPool::RecordDesktopProc, arg);
				m_IdleThreadList.push_back(thr);
				::InterlockedIncrement(&m_ThreadNum);

				thr = NULL;
				thr = new CWorkThread();
				thr->Start(CManageThreadPool::RecordLogProc, arg);
				m_IdleThreadList.push_back(thr);
				::InterlockedIncrement(&m_ThreadNum);

				thr = NULL;
				thr = new CWorkThread();
				thr->Start(CManageThreadPool::ReadCameraLiveVideoProc, arg);
				m_IdleThreadList.push_back(thr);
				::InterlockedIncrement(&m_ThreadNum);

				thr = NULL;
				thr = new CWorkThread();
				thr->Start(CManageThreadPool::MonitorUsbDriveProc, arg);
				m_IdleThreadList.push_back(thr);
				::InterlockedIncrement(&m_ThreadNum);
			}
			catch (exception &e) {
				cout << e.what() << endl;
				return FALSE;
			}
		}
		break;
	}
	return TRUE;
}

void CThreadPool::CreateIdleThread(void *proc)
{
	CWorkThread *thr = NULL;
	thr = new CWorkThread();
	thr->Start(proc);
}

void CThreadPool::MoveIdleThreadToBusyThreadList()
{

}

void CThreadPool::MoveBusyThreadToIdleThreadList()
{

}

void CThreadPool::PushTaskList()
{

}

void CThreadPool::PopTaskList()
{

}

void CThreadPool::PushIdleThreadList()
{

}

void CThreadPool::PopIdleThreadList()
{

}

void CThreadPool::PushBusyThreadList()
{

}

void CThreadPool::PopBusyThreadList()
{

}

void CThreadPool::PushTimeTaskList()
{

}

void CThreadPool::PopTimeTaskList()
{

}

void CThreadPool::PushWaitTaskList()
{

}

void CThreadPool::PopWaitTaskList()
{

}