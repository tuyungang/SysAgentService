#include "ManageRapidJson.h"
#include "ManageThreadPool.h"
#include "ManageRules.h"
#include "Packet.h"
#include <cstring>

extern CManageThreadPool *pGlobalManage;
extern CHAR g_sTimeBuf[50];
Document m_Document;
CCRapidJson::CCRapidJson(void *arg)
{
	//m_pManage = (CManageThreadPool*)arg;
	MapToHandler();
}

CCRapidJson::~CCRapidJson()
{
	m_RuleProcMap.clear();
}

void CCRapidJson::MapToHandler()
{
	m_RuleProcMap["QQ.exe"] = &CCRapidJson::Handle_QQ_Proc;
	m_RuleProcMap["WeChat.exe"] = &CCRapidJson::Handle_WeChat_Proc;
	m_RuleProcMap["Fetion.exe"] = &CCRapidJson::Handle_Fetion_Proc;
	m_RuleProcMap["USB"] = &CCRapidJson::Handle_USB_Proc;
	m_RuleProcMap["Web"] = &CCRapidJson::Handle_Web_Proc;
	m_RuleProcMap["NetDisk"] = &CCRapidJson::Handle_NetDisk_Proc;

	m_GenerateProcMap["record_time"] = &CCRapidJson::Generate_Time_Proc;
	m_GenerateProcMap["QQ.exe"] = &CCRapidJson::Generate_QQ_Proc;
	m_GenerateProcMap["WeChat.exe"] = &CCRapidJson::Generate_WeChat_Proc;
	m_GenerateProcMap["Fetion.exe"] = &CCRapidJson::Generate_Fetion_Proc;
	m_GenerateProcMap["USB"] = &CCRapidJson::Generate_USB_Proc;
	m_GenerateProcMap["Web"] = &CCRapidJson::Generate_Web_Proc;
	m_GenerateProcMap["NetDisk"] = &CCRapidJson::Generate_NetDisk_Proc;
}

BOOL CCRapidJson::Handle_QQ_Proc(Value &t, vector<CObject*> &objList)
{
	// CObject *obj = new CObject("QQ.exe");
	CQqObject *obj = new CQqObject("QQ.exe");
	for (int i = 0; i < t.Size(); i++) {
		Value &t_child = t[i];
		CRule r; CPolicy p;
		r.m_ActionName = t_child["rule_action"].GetString();
		r.m_TriggleExpr = t_child["rule_expression"].GetString();
		r.m_Table = t_child["action_detail_tb"].GetString();
		r.m_Id = t_child["policy_id"].GetInt();
		p.m_StartTime = t_child["start_time"].GetInt();
		p.m_EndTime = t_child["end_time"].GetInt();
		obj->m_RulePolicyMap.insert(map<CRule, CPolicy>::value_type(r, p));
		obj->m_InjectModeIndex = 0;
	}
	pGlobalManage->m_MonitorObjectList.push_back(obj);
	//objList.push_back(obj);
	return TRUE;
}

BOOL CCRapidJson::Handle_WeChat_Proc(Value &t, vector<CObject*> &objList)
{
	/*CObject *obj = new CObject("WeChat.exe");
	for (int i = 0; i < t.Size(); i++) {
		Value &t_child = t[i];
		CRule r; CPolicy p;
		r.m_ActionName = t_child["rule_action"].GetString();
		r.m_TriggleExpr = t_child["rule_expression"].GetString();
		r.m_Table = t_child["action_detail_tb"].GetString();
		r.m_Id = t_child["policy_id"].GetInt();
		p.m_StartTime = t_child["start_time"].GetInt();
		p.m_EndTime = t_child["end_time"].GetInt();
		obj->m_RulePolicyMap.insert(map<CRule, CPolicy>::value_type(r, p));
		obj->m_InjectModeIndex = 0;
	}
	pGlobalManage->m_MonitorObjectList.push_back(obj);*/

	/*CObject *obj = new CObject("WeChat.exe");
	for (int i = 0; i < t.Size(); i++) {
		Value &t_child = t[i];
		CRule r; CPolicy p;
		r.m_ActionName = t_child["rule_action"].GetString();
		r.m_TriggleExpr = t_child["rule_expression"].GetString();
		p.m_StartTime = t_child["policy_starttime"].GetString();
		p.m_EndTime = t_child["policy_endtime"].GetString();
		obj->m_RulePolicyMap.insert(map<CRule, CPolicy>::value_type(r, p));
		obj->m_InjectModeIndex = 1;
	}
	objList.push_back(obj);*/
	return TRUE;
}

BOOL CCRapidJson::Handle_Fetion_Proc(Value &t, vector<CObject*> &objList)
{
	/*CObject *obj = new CObject("Fetion.exe");
	for (int i = 0; i < t.Size(); i++) {
		Value &t_child = t[i];
		CRule r; CPolicy p;
		r.m_ActionName = t_child["rule_action"].GetString();
		r.m_TriggleExpr = t_child["rule_expression"].GetString();
		r.m_Table = t_child["action_detail_tb"].GetString();
		r.m_Id = t_child["policy_id"].GetInt();
		p.m_StartTime = t_child["start_time"].GetInt();
		p.m_EndTime = t_child["end_time"].GetInt();
		obj->m_RulePolicyMap.insert(map<CRule, CPolicy>::value_type(r, p));
		obj->m_InjectModeIndex = 0;
	}
	pGlobalManage->m_MonitorObjectList.push_back(obj);*/

	/*CObject *obj = new CObject("Fetion.exe");
	for (int i = 0; i < t.Size(); i++) {
		Value &t_child = t[i];
		CRule r; CPolicy p;
		r.m_ActionName = t_child["rule_action"].GetString();
		r.m_TriggleExpr = t_child["rule_expression"].GetString();
		p.m_StartTime = t_child["policy_starttime"].GetString();
		p.m_EndTime = t_child["policy_endtime"].GetString();
		obj->m_RulePolicyMap.insert(map<CRule, CPolicy>::value_type(r, p));
	}
	objList.push_back(obj);*/
	return TRUE;
}

BOOL CCRapidJson::Handle_USB_Proc(Value &t, vector<CObject*> &objList)
{
	/*CObject *obj = new CObject("USB");
	for (int i = 0; i < t.Size(); i++) {
		Value &t_child = t[i];
		CRule r; CPolicy p;
		r.m_ActionName = t_child["rule_action"].GetString();
		r.m_TriggleExpr = t_child["rule_expression"].GetString();
		p.m_StartTime = t_child["policy_starttime"].GetString();
		p.m_EndTime = t_child["policy_endtime"].GetString();
		obj->m_RulePolicyMap.insert(map<CRule, CPolicy>::value_type(r, p));
	}
	objList.push_back(obj);*/
	return TRUE;
}

BOOL CCRapidJson::Handle_Web_Proc(Value &t, vector<CObject*> &objList)
{
	/*CObject *obj = new CObject("web");
	for (int i = 0; i < t.Size(); i++) {
		Value &t_child = t[i];
		CRule r; CPolicy p;
		r.m_ActionName = t_child["rule_action"].GetString();
		r.m_TriggleExpr = t_child["rule_expression"].GetString();
		p.m_StartTime = t_child["policy_starttime"].GetString();
		p.m_EndTime = t_child["policy_endtime"].GetString();
		obj->m_RulePolicyMap.insert(map<CRule, CPolicy>::value_type(r, p));
	}
	objList.push_back(obj);*/
	return TRUE;
}

BOOL CCRapidJson::Handle_NetDisk_Proc(Value &t, vector<CObject*> &objList)
{
	/*CObject *obj = new CObject("QQ.exe");
	for (int i = 0; i < t.Size(); i++) {
		Value &t_child = t[i];
		CRule r; CPolicy p;
		r.m_ActionName = t_child["rule_action"].GetString();
		r.m_TriggleExpr = t_child["rule_expression"].GetString();
		p.m_StartTime = t_child["policy_starttime"].GetString();
		p.m_EndTime = t_child["policy_endtime"].GetString();
		obj->m_RulePolicyMap.insert(map<CRule, CPolicy>::value_type(r, p));
	}
	objList.push_back(obj);*/
	return TRUE;
}

BOOL CCRapidJson::ParseJson(PCHAR data, vector<CObject*> &objList)
{
	if (m_Document.Parse(data).HasParseError())
	        return FALSE;
	assert(m_Document.IsObject());

	map<PCHAR, pHandler>::iterator it = m_RuleProcMap.begin();
	while (it != m_RuleProcMap.end()) {
		if (m_Document.HasMember((*it).first)) {
			Value& t = m_Document[(*it).first];
			if (t.IsArray()) {
				pHandler pFunction = (*it).second;
				(this->*pFunction)(t, objList);
			} else {
				return FALSE;
			}
		} 
		it++;
	}

	/*if (m_Document.HasMember("QQ.exe")) {
		Value& t = m_Document["QQ.exe"];
		if (t.IsArray()) {
			Handle_QQ_Proc(t, objList);
		} else {
			return FALSE;
		}
	}
	
	if (m_Document.HasMember("WeChat.exe")) {
		Value& t = m_Document["WeChat.exe"];
		if (t.IsArray()) {
			Handle_WeChat_Proc(t, objList);
		} else {
			return FALSE;
		}

	}
	
	if (m_Document.HasMember("Fetion.exe")) {
		Value& t = m_Document["Fetion.exe"];
		if (t.IsArray()) {
			Handle_Fetion_Proc(t, objList);
		} else {
			return FALSE;
		}

	}
	
	if (m_Document.HasMember("USB")) {
		Value& t = m_Document["USB"];
		if (t.IsArray()) {
			Handle_USB_Proc(t, objList);
		} else {
			return FALSE;
		}

	}
	
	if (m_Document.HasMember("NetDisk")) {
		Value& t = m_Document["NetDisk"];
		if (t.IsArray()) {
			Handle_NetDisk_Proc(t, objList);
		} else {
			return FALSE;
		}

	}
	
	if (m_Document.HasMember("Web")) {
		Value& t = m_Document["Web"];
		if (t.IsArray()) {
			Handle_Web_Proc(t, objList);
		} else {
			return FALSE;
		}

	}
	*/

	return TRUE;
}

BOOL CCRapidJson::GenerateJson(PCHAR type, void *data, string &out_data, void *buf, UINT bufLen)
{
	map<PCHAR, pHandlerGenerate>::iterator it = m_GenerateProcMap.find(type);
	if (it == m_GenerateProcMap.end()) {
		return FALSE;
	}
	pHandlerGenerate pFunction = (*it).second;
	(this->*pFunction)(data, out_data, buf, bufLen);
	return TRUE;
}

BOOL CCRapidJson::Generate_Time_Proc(void *data, string &out_data, void *buf, UINT bufLen)
{
	if (NULL == data) {
		return FALSE;
	}
	char sTime[20] = {0};
	PKG_CLIENT_HEAD_STRUCT* pHeadClient = reinterpret_cast<PKG_CLIENT_HEAD_STRUCT*>((char*)data);
	sprintf(sTime, "%ld", time(0));
    Document::AllocatorType &allocator = m_Document.GetAllocator();
    m_Document.SetObject();   
    Value Obj(kObjectType);
	Value arg(kStringType);
	arg.SetString(sTime, allocator);
	switch (pHeadClient->pkgTag)
	{
		case 0:{
			Obj.AddMember("QQ_sendfile_starttime", arg, allocator);
		    m_Document.AddMember("QQ.exe", Obj, allocator); 
			break;
		}
		case 1:{
			Obj.AddMember("WeChat_sendfile_starttime", arg, allocator);
		    m_Document.AddMember("WeChat.exe", Obj, allocator); 
			break;
		}
		case 2:{
			Obj.AddMember("Fetion_sendfile_starttime", arg, allocator);
		    m_Document.AddMember("Fetion.exe", Obj, allocator); 
			break;
		}
		case 3:{
			break;
		}
		case 4:{
			break;
		}
		case 5:{
			break;
		}
	}
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    m_Document.Accept(writer);
	out_data = buffer.GetString(); 
	return TRUE;
}

BOOL CCRapidJson::Generate_QQ_Proc(void *data, string &out_data, void *buf, UINT bufLen)
{
	if (NULL == data) {
		return FALSE;
	}
	struct stat file_info;
	curl_off_t fsize;
    Document::AllocatorType &allocator = m_Document.GetAllocator();
    m_Document.SetObject();   
    Value Obj(kObjectType);
	Value ObjChild(kObjectType);
	Value arg(kStringType);

	PKG_CLIENT_HEAD_STRUCT* pHeadClient = reinterpret_cast<PKG_CLIENT_HEAD_STRUCT*>((char*)data);
	PKG_BODY_STRUCT *pBody = reinterpret_cast<PKG_BODY_STRUCT*>((char*)data + sizeof(PKG_CLIENT_HEAD_STRUCT));
	INT index = 0;
	char sLocalIP[50] = {0};
	char sSenderQQ[50] = {0};
	char sReceiverIP[50] = {0};
	char sSendFilePath[100] = {0};
	int nTriggleTime;
	char sHostName[50] = {0};
	if (pHeadClient->pkgInfo.pkg_qq.nHostName != 0) {
		memcpy(sHostName, pBody->data + index, pHeadClient->pkgInfo.pkg_qq.nHostName);
		index += pHeadClient->pkgInfo.pkg_qq.nHostName;
		arg.SetString(sHostName, allocator);
		m_Document.AddMember("resultMachineName", arg, allocator);
	}
	else {
		arg.SetNull();
		m_Document.AddMember("resultMachineName", arg, allocator);
	}
	if (pHeadClient->pkgInfo.pkg_qq.nLocalIPLength != 0) {
		memcpy(sLocalIP, pBody->data + index, pHeadClient->pkgInfo.pkg_qq.nLocalIPLength);
		index += pHeadClient->pkgInfo.pkg_qq.nLocalIPLength;
		arg.SetString(sLocalIP, allocator);
		m_Document.AddMember("resultOriginIp", arg, allocator);
	}
	else {
		arg.SetNull();
		m_Document.AddMember("resultOriginIp", arg, allocator);
	}
	if (pHeadClient->pkgInfo.pkg_qq.nReceiverIPLength != 0) {
		memcpy(sReceiverIP, pBody->data + index, pHeadClient->pkgInfo.pkg_qq.nReceiverIPLength);
		index += pHeadClient->pkgInfo.pkg_qq.nReceiverIPLength;
		arg.SetString(sReceiverIP, allocator);
		m_Document.AddMember("resultTargetIp", arg, allocator);
	}
	else {
		arg.SetNull();
		m_Document.AddMember("resultTargetIp", arg, allocator);
	}
	if (pHeadClient->pkgInfo.pkg_qq.nSenderQQLength != 0) {
		memcpy(sSenderQQ, pBody->data + index, pHeadClient->pkgInfo.pkg_qq.nSenderQQLength);
		index += pHeadClient->pkgInfo.pkg_qq.nSenderQQLength;
		arg.SetString(sSenderQQ, allocator);
		ObjChild.AddMember("chatOriginAccount", arg, allocator);
	}
	else {
		arg.SetNull();
		ObjChild.AddMember("chatOriginAccount", arg, allocator);
	}
	if (pHeadClient->pkgInfo.pkg_qq.nSendFilePathLength != 0) {
		memcpy(sSendFilePath, pBody->data + index, pHeadClient->pkgInfo.pkg_qq.nSendFilePathLength);
		index += pHeadClient->pkgInfo.pkg_qq.nSendFilePathLength;
		arg.SetString(sSendFilePath, allocator);
		ObjChild.AddMember("chatFileName", arg, allocator);
	}
	else {
		arg.SetNull();
		ObjChild.AddMember("chatFileName", arg, allocator);
	}
	if (stat(sSendFilePath, &file_info)) {
		printlog("Couldn't open '%s': %s\n", sSendFilePath, strerror(errno));
	}
	fsize = (curl_off_t)file_info.st_size;
	if (fsize <= 0) {
		printlog("");
	}
	Value arg1(kNumberType);
	arg1.SetInt(fsize);
	ObjChild.AddMember("chatFileLength", arg1, allocator);
	arg.SetNull();
	ObjChild.AddMember("chatTargetAccount", arg, allocator);
	if (pHeadClient->pkgInfo.pkg_qq.nTriggleTime != 0) {
		memcpy(&nTriggleTime, pBody->data + index, pHeadClient->pkgInfo.pkg_qq.nTriggleTime);
		index += pHeadClient->pkgInfo.pkg_qq.nTriggleTime;
		//arg.SetString(sLocalIP, allocator);
		m_Document.AddMember("resultOperateTime", nTriggleTime, allocator);
	}
	else {
		arg.SetNull();
		m_Document.AddMember("resultOperateTime", arg, allocator);
	}
	for (int i = 0; i < pGlobalManage->m_MonitorObjectList.size(); i++)
	{
		if (strcmp((pGlobalManage->m_MonitorObjectList)[i]->m_ObjectName.c_str(), "QQ.exe") == 0) {
			map<CRule, CPolicy, Compare<CRule>>::iterator it = (pGlobalManage->m_MonitorObjectList)[i]->m_RulePolicyMap.begin();
			m_Document.AddMember("policyId", (*it).first.m_Id, allocator);
			arg.SetString((*it).first.m_Table.c_str(), allocator);
			m_Document.AddMember("resultDetailTb", arg, allocator);
		}
	}
	//arg.SetString(sReceiverIP, allocator);
	m_Document.AddMember("resultTrigger", 0, allocator);
	arg.SetNull();
	m_Document.AddMember("resultAccessory", arg, allocator);
	arg.SetString(g_sTimeBuf, allocator);
	m_Document.AddMember("videoName", arg, allocator);

	/*if (index != (pHeadClient->pkgSize - sizeof(PKG_CLIENT_HEAD_STRUCT))) {
		return FALSE;
	}*/

	m_Document.AddMember("detailContent", ObjChild, allocator);

	/*StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    document.Accept(writer);   
    printlogA(sb.GetString().c_str());
	*/    
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    m_Document.Accept(writer);
	out_data = buffer.GetString(); 

	return TRUE;
}

BOOL CCRapidJson::Generate_WeChat_Proc(void *data, string &out_data, void *buf, UINT bufLen)
{
	if (NULL == data) {
		return FALSE;
	}
	PKG_CLIENT_HEAD_STRUCT* pHeadClient = reinterpret_cast<PKG_CLIENT_HEAD_STRUCT*>((char*)data);
	PKG_BODY_STRUCT *pBody = reinterpret_cast<PKG_BODY_STRUCT*>((char*)data + sizeof(PKG_CLIENT_HEAD_STRUCT));
	INT index = 0;
	char sLocalIP[50] = { 0 };
	char sReceiverIP[50] = { 0 };
	char sSendFilePath[100] = { 0 };
	memcpy(sLocalIP, pBody->data + index, pHeadClient->pkgInfo.pkg_qq.nLocalIPLength);
	index += pHeadClient->pkgInfo.pkg_qq.nLocalIPLength;
	memcpy(sReceiverIP, pBody->data + index, pHeadClient->pkgInfo.pkg_qq.nReceiverIPLength);
	index += pHeadClient->pkgInfo.pkg_qq.nReceiverIPLength;
	memcpy(sSendFilePath, pBody->data + index, pHeadClient->pkgInfo.pkg_qq.nSendFilePathLength);
	index += pHeadClient->pkgInfo.pkg_qq.nSendFilePathLength;
	if (index != (pHeadClient->pkgSize - sizeof(PKG_CLIENT_HEAD_STRUCT))) {
		return FALSE;
	}
	Document::AllocatorType &allocator = m_Document.GetAllocator();
	m_Document.SetObject();
	Value Obj(kObjectType);

	Value arg(kStringType);
	arg.SetString(sLocalIP, allocator);
	Obj.AddMember("LocalIP", arg, allocator);
	arg.SetString(sReceiverIP, allocator);
	Obj.AddMember("ReceiverIP", arg, allocator);
	arg.SetString(sSendFilePath, allocator);
	Obj.AddMember("SendFilePath", arg, allocator);
    m_Document.AddMember("WeChat.exe", Obj, allocator); 

	/*StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    document.Accept(writer);   
    printlogA(sb.GetString().c_str());
	*/    
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    m_Document.Accept(writer);
	out_data = buffer.GetString(); 
	
	return TRUE;
}

BOOL CCRapidJson::Generate_Fetion_Proc(void *data, string &out_data, void *buf, UINT bufLen)
{
	if (NULL == data) {
		return FALSE;
	}
	PKG_CLIENT_HEAD_STRUCT* pHeadClient = reinterpret_cast<PKG_CLIENT_HEAD_STRUCT*>((char*)data);
	PKG_BODY_STRUCT *pBody = reinterpret_cast<PKG_BODY_STRUCT*>((char*)data + sizeof(PKG_CLIENT_HEAD_STRUCT));
	INT index = 0;
	char sLocalIP[50] = { 0 };
	char sReceiverIP[50] = { 0 };
	char sSendFilePath[100] = { 0 };
	memcpy(sLocalIP, pBody->data + index, pHeadClient->pkgInfo.pkg_qq.nLocalIPLength);
	index += pHeadClient->pkgInfo.pkg_qq.nLocalIPLength;
	memcpy(sReceiverIP, pBody->data + index, pHeadClient->pkgInfo.pkg_qq.nReceiverIPLength);
	index += pHeadClient->pkgInfo.pkg_qq.nReceiverIPLength;
	memcpy(sSendFilePath, pBody->data + index, pHeadClient->pkgInfo.pkg_qq.nSendFilePathLength);
	index += pHeadClient->pkgInfo.pkg_qq.nSendFilePathLength;
	if (index != (pHeadClient->pkgSize - sizeof(PKG_CLIENT_HEAD_STRUCT))) {
		return FALSE;
	}
	Document::AllocatorType &allocator = m_Document.GetAllocator();
	m_Document.SetObject();
	Value Obj(kObjectType);

	Value arg(kStringType);
	arg.SetString(sLocalIP, allocator);
	Obj.AddMember("LocalIP", arg, allocator);
	arg.SetString(sReceiverIP, allocator);
	Obj.AddMember("ReceiverIP", arg, allocator);
	arg.SetString(sSendFilePath, allocator);
	Obj.AddMember("SendFilePath", arg, allocator);
    m_Document.AddMember("Fetion.exe", Obj, allocator); 

	/*StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    document.Accept(writer);   
    printlogA(sb.GetString().c_str());
	*/    
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    m_Document.Accept(writer);
	out_data = buffer.GetString(); 
	
	return TRUE;
}

BOOL CCRapidJson::Generate_USB_Proc(void *data, string &out_data, void *buf, UINT bufLen)
{
	//if (NULL == data) {
	//	return FALSE;
	//}
	//usb *p = (usb*)data;
 //   Document::AllocatorType &allocator = m_Document.GetAllocator();
 //   m_Document.SetObject();   
 //   Value Obj(kObjectType);
	//Value arg(kStringType);
	//Obj.AddMember("UsbInsert", p->bUsbInsert, allocator);
	//arg.SetString(p->sCopyFile, allocator);
 //   Obj.AddMember("sCopyFile", arg, allocator);
 //   m_Document.AddMember("USB", Obj, allocator); 

	///*StringBuffer sb;
 //   PrettyWriter<StringBuffer> writer(sb);
 //   document.Accept(writer);   
 //   printlogA(sb.GetString().c_str());
	//*/    
 //   StringBuffer buffer;
 //   Writer<StringBuffer> writer(buffer);
 //   m_Document.Accept(writer);
	//out_data = buffer.GetString(); 
	
	return TRUE;
}

BOOL CCRapidJson::Generate_Web_Proc(void *data, string &out_data, void *buf, UINT bufLen)
{
	//if (NULL == data) {
	//	return FALSE;
	//}
	//web *p = (web*)data;
 //   Document::AllocatorType &allocator = m_Document.GetAllocator();
 //   m_Document.SetObject();   
 //   Value Obj(kObjectType);
	//Value arg(kStringType);
	//arg.SetString(p->sAccessURL, allocator);
 //   Obj.AddMember("AccessURL", arg, allocator);
 //   m_Document.AddMember("Web", Obj, allocator); 
 //   if (bufLen == 0) {
 //   	return FALSE;
 //   }
	//arg.SetString((PCHAR)buf, allocator);
 //   Obj.AddMember("WebContent", arg, allocator);
 //   Obj.AddMember("WebContentLength", bufLen, allocator);

	///*StringBuffer sb;
 //   PrettyWriter<StringBuffer> writer(sb);
 //   document.Accept(writer);   
 //   printlogA(sb.GetString().c_str());
	//*/    
 //   StringBuffer buffer;
 //   Writer<StringBuffer> writer(buffer);
 //   m_Document.Accept(writer);
	//out_data = buffer.GetString(); 
	
	return TRUE;
}

BOOL CCRapidJson::Generate_NetDisk_Proc(void *data, string &out_data, void *buf, UINT bufLen)
{
	//if (NULL == data) {
	//	return FALSE;
	//}
	//netdisk *p = (netdisk*)data;
 //   Document::AllocatorType &allocator = m_Document.GetAllocator();
 //   m_Document.SetObject();   
 //   Value Obj(kObjectType);
	//Value arg(kStringType);
	//arg.SetString(p->sNetDiskName, allocator);
	//Obj.AddMember("NetDiskName", arg, allocator);
	//arg.SetString(p->sUploadFilePath, allocator);
	//Obj.AddMember("UploadFilePath", arg, allocator);
 //   m_Document.AddMember("NetDisk", Obj, allocator); 

	///*StringBuffer sb;
 //   PrettyWriter<StringBuffer> writer(sb);
 //   document.Accept(writer);   
 //   printlogA(sb.GetString().c_str());
	//*/    
 //   StringBuffer buffer;
 //   Writer<StringBuffer> writer(buffer);
 //   m_Document.Accept(writer);
	//out_data = buffer.GetString(); 
	
	return TRUE;
}

BOOL CCRapidJson::Generate_Foxmail_Proc(void *data, string &out_data, void *buf = NULL, UINT bufLen = 0)
{
	//if (NULL == data) {
	//	return FALSE;
	//}
	//foxmail *p = (foxmail*)data;
 //   Document::AllocatorType &allocator = m_Document.GetAllocator();
 //   m_Document.SetObject();   
 //   Value Obj(kObjectType);
	//Value arg(kStringType);
	//arg.SetString(p->sUserMailAddress, allocator);
	//Obj.AddMember("UserMailAddress", arg, allocator);
	//arg.SetString(p->sReceiverMailAddress, allocator);
	//Obj.AddMember("ReceiverMailAddress", arg, allocator);
	//arg.SetString(p->sAttachFilePath, allocator);
	//Obj.AddMember("AttachFilePath", arg, allocator);
 //   m_Document.AddMember("Foxmail.exe", Obj, allocator); 

	///*StringBuffer sb;
 //   PrettyWriter<StringBuffer> writer(sb);
 //   document.Accept(writer);   
 //   printlogA(sb.GetString().c_str());
	//*/    
 //   StringBuffer buffer;
 //   Writer<StringBuffer> writer(buffer);
 //   m_Document.Accept(writer);
	//out_data = buffer.GetString(); 
	
	return TRUE;
}

void CCRapidJson::AddString(const char *key, const char *value)
{
	m_Document.AddMember(rapidjson::StringRef(key), rapidjson::StringRef(key), m_Document.GetAllocator());
}

void CCRapidJson::AddNumber(char *key, int value)
{
	m_Document.AddMember(rapidjson::StringRef(key), value, m_Document.GetAllocator());
}

void CCRapidJson::AddBool()
{

}

void CCRapidJson::AddAarry()
{

}
