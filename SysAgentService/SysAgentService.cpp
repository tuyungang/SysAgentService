// SysAgentService.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "ManageThreadPool.h"

#pragma comment(linker,"/subsystem:windows /entry:mainCRTStartup")

CManageThreadPool *pGlobalManage;

int main(int argc, _TCHAR* argv[])
{
	CManageThreadPool *m_Manage = CManageThreadPool::CreateInstance();
	pGlobalManage = m_Manage;
	m_Manage->RunServer();
	while (1) {
		Sleep(10000);
	}
	return 0;
}

