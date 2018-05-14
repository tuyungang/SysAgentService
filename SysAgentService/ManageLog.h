#pragma once

//#include <iostream>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <windows.h>
//#include <vector>
//#include <list>
//#include <map>
//#include <io.h>
//
//using namespace std;
//
//class CLog;
//
//class CLogManage
//{
//private:
//	CLogManage();
//	/*virtual ~CLogManage();
//	CLogManage(const CLogManage &);
//	CLogManage & operator = (const CLogManage &) const;*/
//
//protected:
//
//public:
//	static CLogManage *GetInstance();
//	class FreeInstance
//	{
//	public:
//		~FreeInstance()
//		{
//			if (CLogManage::pInstance != NULL) {
//				delete CLogManage::pInstance;
//			}
//		}
//	};
//	static FreeInstance _free;
//
//private:
//	static CLogManage *pInstance;
//
//	
//	
//public:
//	void StartLog();
//	void ExitLog();
//	void Log();
//	void PushLogInList();
//	void PopLogOutList();
//
//};
//
//class CLog
//{
//public :
//	CLog();
//	~CLog();
//
//protected:
//
//private:
//	void _log();
//};