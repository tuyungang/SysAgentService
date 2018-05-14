#pragma once

#include "Util.h"
#include "ProcDataDef.h"
//#include "ManageRapidJson.h"

class CRule
{
public:
	CRule(){
		m_Id = -1;	
	}
	virtual ~CRule(){}
public:
	string m_ActionName;
	string m_TriggleExpr;
	string m_Table;
	INT    m_Id;
};

class CPolicy
{
public:
	CPolicy() {
		// m_CycleCount = 0;
		m_StartTime = 0;
		m_EndTime = 0;
		// m_CurrentCount = 0;
	}
	virtual ~CPolicy(){}
public:
	// int m_CycleCount;
	// int m_CurrentCount;
	LONG m_StartTime;
	LONG m_EndTime;
};

template<class T>
class Compare
{
public:
	int operator()(const T& x, const T& y)const
	{
		if (x.m_ActionName.size() >= y.m_ActionName.size())
			return 0;
		return 1;
	}
};

class CObject
{
public:
	CObject(){}
	CObject(PCHAR arg) : m_ObjectName(arg)
	{
		m_RulePolicyMap.clear();
		m_IsInjected = FALSE;
		m_IsExist = FALSE;
		m_ObjectPid = -1;
		// m_CurObjectPid = -1;
		// m_LastObjectPid = -1;
		m_InjectModeIndex = -1;
		// m_Count = 0;
	}
	virtual ~CObject(){}
public:
	map<CRule, CPolicy, Compare<CRule>> m_RulePolicyMap;
	string m_ObjectName;
	string m_ObjectInstallPath;
	BOOL m_IsInjected;
	BOOL m_IsExist;
	DWORD m_ObjectPid;
	UINT m_InjectModeIndex;
	// UINT m_Count;
	// CPolicy m_Policy;
};
class CQqObject : public CObject
{
public:
	CQqObject(PCHAR arg) : CObject(arg)
	{
		m_FirstObjectPid = -1;
		m_SecondObjectPid = -1;
		m_Count = 0;
	}
	virtual ~CQqObject(){}
public:
	DWORD m_FirstObjectPid;
	DWORD m_SecondObjectPid;
	UINT m_Count;
};

class CWebChatObject : public CObject
{
public:
	CWebChatObject() : CObject(){}
	virtual ~CWebChatObject(){}
public:

};

class CFetionObject : public CObject
{
public:
	CFetionObject() :CObject(){}
	virtual ~CFetionObject(){}
public:

};

class CUSBObject : public CObject
{
public:
	CUSBObject() :CObject(){}
	virtual ~CUSBObject(){}
public:

};

class CFoxmailObject : public CObject
{
public:
	CFoxmailObject() :CObject(){}
	virtual ~CFoxmailObject(){}
public:

};

class CWebObject : public CObject
{
public:
	CWebObject() :CObject(){}
	virtual ~CWebObject(){}
public:

};

class CNetDiskObject : public CObject
{
public:
	CNetDiskObject() :CObject(){}
	virtual ~CNetDiskObject(){}
public:

};

//class CCRapidJson;
//class CRuleGenerator
//{
//public:
//	CRuleGenerator() :js(){}
//	CRuleGenerator(void *arg);
//	~CRuleGenerator();
//private:
//	//CManageThreadPool *pManage;
//	CCRapidJson js;
//public:
//	//BOOL GenerateObject(char *data, vector<CObject*> &objList);
//	BOOL GenerateObject(char *data);
//	BOOL GenerateDataToServer(PCHAR type, void *in_data, string &out_data, void *buf = NULL, UINT bufLen = 0); 
//};
