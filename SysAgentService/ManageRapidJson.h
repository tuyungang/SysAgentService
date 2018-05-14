#pragma once

// #include "ManageThreadPool.h"
#include "Util.h"
#include "ProcDataDef.h"
//#include "packet.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
//#include "rapidjson/filestream.h"

using namespace rapidjson;

class CObject;
class CManageThreadPool;
class CCRapidJson
{
public:
	CCRapidJson(void *arg = NULL);
	~CCRapidJson();

private:
   //Document m_Document;
	//Document::AllocatorType& allocator;
	//CManageThreadPool *m_pManage;
   typedef BOOL(CCRapidJson::*pHandler)(Value& t, vector<CObject*> &obj);
   map<PCHAR, pHandler> m_RuleProcMap;
   typedef BOOL(CCRapidJson::*pHandlerGenerate)(void *data, string &out_data, void *buf, UINT bufLen);
   map<PCHAR, pHandlerGenerate> m_GenerateProcMap;
private:
	void MapToHandler();
	BOOL Handle_QQ_Proc(Value& t, vector<CObject*> &objList);
	BOOL Handle_WeChat_Proc(Value& t, vector<CObject*> &objList);
	BOOL Handle_Fetion_Proc(Value& t, vector<CObject*> &objList);
	BOOL Handle_USB_Proc(Value& t, vector<CObject*> &objList);
	BOOL Handle_Web_Proc(Value& t, vector<CObject*> &objList);
	BOOL Handle_NetDisk_Proc(Value& t, vector<CObject*> &objList);

	BOOL Generate_Time_Proc(void *data, string &out_data, void *buf, UINT bufLen);
	BOOL Generate_QQ_Proc(void *data, string &out_data, void *buf, UINT bufLen);
	BOOL Generate_WeChat_Proc(void *data, string &out_data, void *buf, UINT bufLen);
	BOOL Generate_Fetion_Proc(void *data, string &out_data, void *buf, UINT bufLen);
	BOOL Generate_USB_Proc(void *data, string &out_data, void *buf, UINT bufLen);
	BOOL Generate_Web_Proc(void *data, string &out_data, void *buf, UINT bufLen);
	BOOL Generate_NetDisk_Proc(void *data, string &out_data, void *buf, UINT bufLen);
	BOOL Generate_Foxmail_Proc(void *data, string &out_data, void *buf, UINT bufLen);
public:
	BOOL ParseJson(PCHAR data, vector<CObject*> &objList);
	BOOL GenerateJson(PCHAR type, void *data, string &out_data, void *buf = NULL, UINT bufLen = 0);
private:
	void AddString(const char *key, const char *value);
	void AddNumber(char *key, int value);
	void AddBool();
	void AddAarry();
};