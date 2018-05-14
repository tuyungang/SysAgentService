#include "Util.h"

CRITICAL_SECTION g_cs_log;

void recordlog(char *arg)
{
	/*FILE* pfile = NULL;
	pfile = fopen("C:\\test.txt", "a+");
	fwrite(arg, sizeof(char), strlen(arg), pfile);
	fwrite("\n", sizeof(char), strlen("\n"), pfile);
	fclose(pfile);*/
}
void CLog::Log(char *logInfo, LOG_ERROR_LEVEL err)
{

}

void CLog::PushLogInQueue(string logInfo)
{

}

void CLog::PopLogOutQueue()
{

}

char *dup_wchar_to_utf8(const wchar_t *w)
{
	char *s = NULL;
	int l = WideCharToMultiByte(CP_UTF8, 0, w, -1, 0, 0, 0, 0);
	s = (char *)av_malloc(l);
	if (s)
		WideCharToMultiByte(CP_UTF8, 0, w, -1, s, l, 0, 0);
	return s;
}

//Show Dshow Device
void show_dshow_device()
{
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary* options = NULL;
	av_dict_set(&options, "list_devices", "true", 0);
	AVInputFormat *iformat = av_find_input_format("dshow");
	printf("========Device Info=============\n");
	avformat_open_input(&pFormatCtx, "video=dummy", iformat, &options);
	printf("================================\n");
}

//Show Dshow Device Option
void show_dshow_device_option()
{
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary* options = NULL;
	av_dict_set(&options, "list_options", "true", 0);
	AVInputFormat *iformat = av_find_input_format("dshow");
	printf("========Device Option Info======\n");
	avformat_open_input(&pFormatCtx, "video=Integrated Camera", iformat, &options);
	//avformat_open_input(&pFormatCtx, "audio=Internal Microphone (Conexant 20751 SmartAudio HD)", iformat, &options);
	printf("================================\n");
}

//Show VFW Device
void show_vfw_device()
{
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVInputFormat *iformat = av_find_input_format("vfwcap");
	printf("========VFW Device Info======\n");
	avformat_open_input(&pFormatCtx, "list", iformat, NULL);
	printf("=============================\n");
}

//Show AVFoundation Device
void show_avfoundation_device()
{
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary* options = NULL;
	av_dict_set(&options, "list_devices", "true", 0);
	AVInputFormat *iformat = av_find_input_format("avfoundation");
	printf("==AVFoundation Device Info===\n");
	avformat_open_input(&pFormatCtx, "", iformat, &options);
	printf("=============================\n");
}

//void Filesearch(string Path, int Layer)
//{
//	FileInfo FileInfomation;
//	struct _finddata_t filefind;
//	string Current = Path + "\\*.*";                            // 修改此处改变搜索条件
//	int Done = 0, i, Handle;
//	string FullPath;
//	if ((Handle = _findfirst(Current.c_str(), &filefind)) != -1)
//	{
//		while (!(Done = _findnext(Handle, &filefind)))
//		{
//			if (strcmp(filefind.name, "..") == 0)
//				continue;
//			for (i = 0; i <Layer; i++)
//				printf("\t");
//			if ((_A_SUBDIR == filefind.attrib))                // 是目录
//			{
//				printf("[Dir]:\t%s\n", filefind.name);
//				Current = Path + "\\" + filefind.name;
//				Filesearch(Current, Layer + 1);                    // 递归遍历子目录
//			}
//			else
//			{
//				printf("[File]:\t%s\n", filefind.name);
//				FullPath = Path + "\\" + filefind.name;
//				FileInfomation.Name = filefind.name;
//				FileInfomation.FullPath = FullPath;
//				FileList.push_back(FileInfomation);
//				FullPath.empty();
//			}
//		}
//		_findclose(Handle);
//	}
//}
//
//void CopyPPT(vector<FileInfo> FileList)
//{
//	mkdir("C:\\Destest");
//	for (auto iterator = FileList.cbegin(); iterator != FileList.cend(); ++iterator)
//	{
//		string TempPath;
//		//printf("%s\r\n", iterator->);
//
//		if ((int)(iterator->Name.find(".ppt")) > 0)
//		{
//			TempPath = DesPath + "\\" + iterator->Name;
//			BOOL bOk = CopyFileA(iterator->FullPath.c_str(), TempPath.c_str(), false);
//			cout << GetLastError();
//			if (bOk == TRUE)
//			{
//				//    MessageBox(0, "Copy Success", "Notice!", MB_OK);
//
//			}
//			else
//			{
//				//    MessageBox(0, "Copy Fail", "Notice!", MB_OK);
//			}
//		}
//
//	}
//}
//
//void CopyDOC(vector<FileInfo> FileList)
//{
//	mkdir("C:\\Destest");
//	for (auto iterator = FileList.cbegin(); iterator != FileList.cend(); ++iterator)
//	{
//		string TempPath;
//		//printf("%s\r\n", iterator->);
//
//		if ((int)(iterator->Name.find(".doc")) > 0)
//		{
//			TempPath = DesPath + "\\" + iterator->Name;
//			BOOL bOk = CopyFileA(iterator->FullPath.c_str(), TempPath.c_str(), false);
//			cout << GetLastError();
//			if (bOk == TRUE)
//			{
//				//    MessageBox(0, "Copy Success", "Notice!", MB_OK);
//
//			}
//			else
//			{
//				//    MessageBox(0, "Copy Fail", "Notice!", MB_OK);
//			}
//		}
//	}
//}

DWORD CountSetBits(ULONG_PTR bitMask)
{
	DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
	DWORD bitSetCount = 0;
	ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
	DWORD i;

	for (i = 0; i <= LSHIFT; ++i)
	{
		bitSetCount += ((bitMask & bitTest) ? 1 : 0);
		bitTest /= 2;
	}
	return bitSetCount;
}

int GetProccessNum()
{
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	return siSysInfo.dwNumberOfProcessors;
}

typedef BOOL(WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

void GetProcessNum(int & logicNum, int & procCore)
{
	LPFN_GLPI glpi;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
	DWORD returnLength = 0;

	glpi = (LPFN_GLPI)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");
	if (NULL == glpi)
	{
		printlog("GetLogicalProcessorInformation is not supported.\n");
		//goto fail;
		logicNum = procCore = GetProccessNum();
		if ((NULL != buffer) && buffer) free(buffer);
		return;
	}
	
	glpi(0, &returnLength);
	ptr = buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);
	if (NULL == buffer)
	{
		printlog("\nError: Allocation failure\n");
		//goto fail;
		logicNum = procCore = GetProccessNum();
		if (buffer) free(buffer);
		return;
	}
	DWORD rc = glpi(buffer, &returnLength);
	if (FALSE == rc)
	{
		printlog("Error %d\n", GetLastError());
		//goto fail;
		logicNum = procCore = GetProccessNum();
		if (buffer) free(buffer);
		return;
	}

	DWORD byteOffset = 0;
	DWORD logicalProcessorCount = 0;
	DWORD processorCoreCount = 0;

	while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
	{
		switch (ptr->Relationship)
		{
		case RelationProcessorCore:
			processorCoreCount++;
			logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
			break;
		}
		byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
		ptr++;
	}
	logicNum = logicalProcessorCount;
	procCore = processorCoreCount;
	if (buffer) free(buffer);
	return;
//fail:
//	logicNum = procCore = GetProccessNum();
//	if ((NULL != buffer) && buffer) free(buffer);
}

BOOL CSysConfig::CheckFolderExist()
{
//	WIN32_FIND_DATA fd;
//	size_t length_of_arg;
//	HANDLE hFind = INVALID_HANDLE_VALUE;
//	DWORD dwError = 0;
//	volatile unsigned int nCount;
//	BOOL bLogFolderExistState;
//	/*StringCchLength(DEFAULT_AGENT_PATH, MAX_AGENT_PATH, &length_of_arg);
//	if (length_of_arg > (MAX_AGENT_PATH - 4))
//    {
//      // _tprintf(TEXT("\nDirectory path is too long.\n"));
//    	printlog("Directory path is too long.");
//		return FALSE;
//    }
//	*/	
//	bLogFolderExistState = FALSE;
//	m_LisenceFileExistState = FALSE;
//	m_RuleFileExistState = FALSE;
//
//	hFind = ::FindFirstFile(L"C:\\agent\\*", &fd);
//	if (INVALID_HANDLE_VALUE == hFind) {
//		printlog("");
//		goto GOERROR;
//	}
//
//	do
//	{
//		int iLen = WideCharToMultiByte(CP_ACP, 0, fd.cFileName, -1, NULL, 0, NULL, NULL);
//		char *pstr = new char[iLen*sizeof(char)];
//		WideCharToMultiByte(CP_ACP, 0, fd.cFileName, -1, pstr, iLen, NULL, NULL);
//		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (_stricmp("log", pstr) == 0)) {
//			bLogFolderExistState = TRUE;
//		} else {
//			if (_stricmp("lisence.bin", pstr) == 0) {
//				m_LisenceFileExistState = TRUE;
//			}
//			else if (_stricmp("rule.bin", pstr) == 0) {
//				m_RuleFileExistState = TRUE;
//			}
//		}
//		delete pstr;
//	} while (FindNextFile(hFind, &fd) != 0);
//	if (!bLogFolderExistState) {
//		printlog("log folder is not exist!, agent create it");
//		if (!::CreateDirectory(L"C:\\agent\\log", NULL)) {
//			char szDir[MAX_PATH];
//			sprintf_s(szDir, sizeof(szDir), "create directory[%s]fail, please check your authority", "C:\\agent\\log");
//			printlog(szDir);
//			goto GOERROR;
//		}
//	}
//
//	if (!m_RuleFileExistState) {
//		printlog("rule.bin file is not found!");
//	}
//
//	if (!m_LisenceFileExistState) {
//		printlog("lisence.bin file is not found!, please contact with provider");
//		goto GOERROR;
//	}
//
//	FindClose(hFind);
//	return TRUE;
//
//GOERROR:
//	dwError = GetLastError();
//	if (dwError != ERROR_NO_MORE_FILES) 
//	{
//		printlog("");
//		// DisplayErrorBox(TEXT("FindFirstFile"));
//	}
//
//	FindClose(hFind);
	return FALSE;
}

BOOL CSysConfig::GetSysConfigInfoFromLocal()
{
	return TRUE;
}

BOOL CSysConfig::GetRulesInfoFromLocal()
{
//	 char* pszPvaFile = NULL;
//    //char* pszPvaFile = "pvabuffer.bin";
//    FILE* pFile = NULL;
//	if (access("pvabuffer.bin", 0) != 0)
//        printlog("pvabuffer.bin is not exist");
//    else {
//        pszPvaFile = "pvabuffer.bin";
//        goto BINREADY;
//    }
//	if (access("/var/pva/pvabuffer.bin", 0) != 0) {
//        printlog("/var/pva/pvabuffer.bin is not exist");
//		memcpy(pszErrorInfo, "/var/pva/pvabuffer.bin is not exist", strlen("/var/pva/pvabuffer.bin is not exist"));
//		return false;
//    }
//    pszPvaFile = "/var/pva/pvabuffer.bin";
//
//BINREADY:
//    pFile = fopen(pszPvaFile, "rb");
//    if (pFile == NULL)
//    {
//        printlog("open password file failed");
//		return FALSE;
//    }
//	
//    fseek(pFile, 0, SEEK_END);
//    long lFileSize = ftell(pFile);
//    if (lFileSize == 0)
//    {
//        printlog("get password file size failed");
//		fclose(pFile);
//		remove(pszPvaFile);
//        return FALSE;
//    }
//    rewind(pFile);
//	
//    long nFileType = 0;
//    fread(&nFileType, sizeof(long), 1, pFile);
//    if (ntohl(nFileType) != 0x1100) //file type identify
//    {
//        printlog("get password file head type failed");
//		fclose(pFile);
//		return FALSE;
//    }
//
//    lFileSize -= sizeof(long);
//    char* pszDataBuffer= (char*)malloc(lFileSize + 1);
//    if (pszDataBuffer == NULL)
//    {
//        printlog("malloc save file buffer failed");
//		fclose(pFile);
//		return FALSE;
//    }
//	
//    memset(pszDataBuffer, 0, lFileSize + 1);
//    long nDataLen = 0;
//    while (nDataLen < lFileSize)
//    {
//       char szBuffer[40960] = {0};
//       int nReadLen = fread(szBuffer, sizeof(char), 40960, pFile);
//       if (nReadLen > 0)
//       {
//	   		memcpy(pszDataBuffer, szBuffer, nReadLen);
//	   		nDataLen += nReadLen;
//       }
//       else
//	   		break;
//    }
//	fclose(pFile);
//	
//	if (nDataLen < lFileSize)
//	{
//        printlog("read password file buffer failed");
//		free(pszDataBuffer);
//		return FALSE;
//	}
//	
//	char* pszDecodeData = (char*)malloc(lFileSize * 2);
//	if (pszDecodeData == NULL)
//	{
//        printlog("malloc decode buffer failed");
//		free(pszDataBuffer);
//		return FALSE;
//	}
//	memset(pszDecodeData, 0, lFileSize * 2);
//	
//	unsigned char szEncodeKey[16] = {0};
//	unsigned char szEndData[16] = { 0xe1, 0x02, 0xa3, 0x04, 0x15, 0xb6, 0x07, 0x08, 0xc9, 0x0a, 0xab, 0x0c, 0x6d, 0x0e, 0x2f, 0x01 };
//	int nIndex = 0;
//    for (nIndex = 0; nIndex < 16; nIndex++)
//	{
//		if (nIndex % 3 == 0)
//		{
//		   szEncodeKey[nIndex] = (0x0f) & (szEndData[nIndex] >> 2);
//		}
//		else if (nIndex % 3 == 1)
//		{
//		   szEncodeKey[nIndex] = (0x1f) & (szEndData[nIndex] >> 3);
//		}
//		else
//		{
//		   szEncodeKey[nIndex] = (0x3f) & (szEndData[nIndex] >> 4);
//		}
//	}
//	int nDecodeLen = 0;
//	AES_DecryptDataEVP((unsigned char*)pszDataBuffer, lFileSize, szEncodeKey, pszDecodeData, &nDecodeLen);
//	if (nDecodeLen == 0)
//	{
//        printlog("decode file buffer failed");
//		free(pszDataBuffer);
//		free(pszDecodeData);
//		return FALSE;
//	}
//	free(pszDataBuffer);
//	
//	char szPassword[64];
//	nDataLen = 0;
//	long lAppID = 0;
//	int nBufferLen = 0;
//	while (nDataLen < nDecodeLen)
//	{
//		memcpy(&lAppID, pszDecodeData + nDataLen, sizeof(long));
//		nDataLen += sizeof(long);
//		
//		memcpy(&nBufferLen, pszDecodeData + nDataLen, sizeof(int));
//		nDataLen += sizeof(int);
//		
//		memset(szPassword, 0, sizeof(szPassword));
//		if (nBufferLen > 63)	//password buffer size is 64;
//			break;
//		
//		memcpy(szPassword, pszDecodeData + nDataLen, nBufferLen);
//		nDataLen += nBufferLen;
//
//		sLocalPswCacheInfo *ps = (sLocalPswCacheInfo *)malloc(sizeof(sLocalPswCacheInfo));
//        //ps = NULL;
//        ps->lAppID = lAppID;
//        memset(ps->szVaultID, 0, 128);
//        memcpy(ps->szVaultID, g_SafeBoxID, strlen(g_SafeBoxID));
//        //memcpy(ps->szVaultID, "2478cbd7fe8f4f22810664407e01f437", strlen("2478cbd7fe8f4f22810664407e01f437"));
//        memset(ps->szPassword, 0, 64);
//        memcpy(ps->szPassword, szPassword, nBufferLen);
//        ps->tmChange = (time_t)0;
//        //ps->next = NULL;
//        ps->handler = NULL;
//        ps->lock = 0;
//        pthread_mutex_lock(&pool->LocalPswCacheQueueMutex);
//        TAILQ_INSERT_TAIL(&pool->LocalPswCacheQueue, ps, next);
//        pthread_mutex_unlock(&pool->LocalPswCacheQueueMutex);
//	}
//
//	free(pszDecodeData);
    return TRUE;
}

BOOL CSysConfig::SaveCurrentRulesInfoToLocal()
{
	//if (!pswNew)
 //       return FALSE;

 //   //char g_CurAbsolutePath[256];
 //   //char g_CacheFileAbsolutePath[256];

 //   //memset(g_CurAbsolutePath, '\0', 256);
 //   //memset(g_CacheFileAbsolutePath, '\0', 256);
 //   /*
 //   if (NULL == getcwd(g_CurAbsolutePath, 256)) {
 //   }
 //   */

 //   sprintf(g_CacheFileAbsolutePath, "%s/%s/%ld.dat",g_CurAbsolutePath, "pswcache", pswNew->lAppID);
 //   if (access(g_CacheFileAbsolutePath, F_OK) == 0)
 //       remove(g_CacheFileAbsolutePath);

 //   FILE* pFile = NULL;
 //   pFile = fopen(g_CacheFileAbsolutePath, "wb");
 //   if (pFile == NULL)
 //       return FALSE;

 //   long lFileType = htonl(0x1100);
 //   fwrite(&lFileType, sizeof(long), 1, pFile);

 //   unsigned char szEncodeKey[16] = {0};
 //   unsigned char szEndData[16] = { 0xe1, 0x02, 0xa3, 0x04, 0x15, 0xb6, 0x07, 0x08, 0xc9, 0x0a, 0xab, 0x0c, 0x6d, 0x0e, 0x2f, 0x01 };
 //   int nIndex = 0;
 //   for (nIndex = 0; nIndex < 16; nIndex++)
 //   {
 //       if (nIndex % 3 == 0)
 //       {
 //           szEncodeKey[nIndex] = (0x0f) & (szEndData[nIndex] >> 2);
 //       }
 //       else if (nIndex % 3 == 1)
 //       {
 //           szEncodeKey[nIndex] = (0x1f) & (szEndData[nIndex] >> 3);
 //       }
 //       else
 //       {
 //           szEncodeKey[nIndex] = (0x3f) & (szEndData[nIndex] >> 4);
 //       }
 //   }

 //   int  nBufferIndex = 0, nPasswordLen = 0;
 //   char szBuffer[2048] = {0};
 //   int i = 0;

 //   memcpy(szBuffer + nBufferIndex, &(pswNew->lAppID), sizeof(long));
 //   nBufferIndex += sizeof(long);
 //   nPasswordLen = strlen(pswNew->szPassword);
 //   memcpy(szBuffer + nBufferIndex, &nPasswordLen, sizeof(int));
 //   nBufferIndex += sizeof(int);
 //   memcpy(szBuffer + nBufferIndex, pswNew->szPassword, nPasswordLen);
 //   nBufferIndex += nPasswordLen;
 //   time_t tmTimeChange = time(0);
 //   memcpy(szBuffer + nBufferIndex, &tmTimeChange, sizeof(time_t));
 //   nBufferIndex += sizeof(time_t);

 //   char uszEncodeData[4096] = {0};
 //   int nEncodeLen = 0;
 //   AES_CryptDataEVP((unsigned char*)szBuffer, nBufferIndex, szEncodeKey, (unsigned char*)uszEncodeData, &nEncodeLen);

 //   int nDataLen = 0;
 //   while (nEncodeLen > 0)
 //   {
 //       int nWriteLen = fwrite(uszEncodeData + nDataLen, sizeof(char), nEncodeLen, pFile);
 //       if (nWriteLen <= 0)
 //           break;
 //       nEncodeLen -= nWriteLen;
 //       nDataLen += nWriteLen;
 //   }

 //   fclose(pFile);
    return TRUE;
}

//static DatabaseManage *pSqlite = NULL;

int OpenDB(const char *path)
{
	//pSqlite = DatabaseManage::Instance(path);
	return 0;
}

void GetSysConfigInfoFromDB()
{

}

void UpdateSysConfigInfoFromDB()
{

}

void GetRulesInfoFromDB()
{

}

void UpdateRulesInfoFromDB()
{

}

void GetAgentExpiredDateFromDB()
{

}

void UpdateAgentExpiredDateFromDB()
{

}

void GetSensitiveInfoFromDB()
{

}

void InsertSensitiveInfoIntoDB()
{

}

void DeleteSensitiveInfoFromDB()
{

}

void CloseDB()
{
	//pSqlite->CloseDatabase();
}

/*
int opendatabase(const char *path)
{
	pSqlite = DatabaseManage::Instance(path);
	return 0;
}

char** getinfofromblacklist(const char *sql, int &num)
{
	map<string, string> buffer;
	map<string, string>::iterator It;
	char **temp = (char**)malloc(sizeof(char*) * 100);
	int i = 0;
	pSqlite->SqlPrepare(sql, buffer);
	for (It = buffer.begin(); It != buffer.end(); It++) {
		temp[i] = const_cast<char*>((*It).first.c_str());
		num = i;
		i++;
	}
	return temp;

}

void getnumbersofrowsfromblacklist(int &sum)
{
	map<string, string> buffer;
	sum = pSqlite->SqlPrepareParam("select count() from mytable;", NULL, NULL, buffer, "count()");

}

void addnewintoblacklist(const char *ip, const char *reason)
{
	map<string, string> buffer;
	pSqlite->SqlPrepareParam("insert into mytable(ip_address,forbidden_reason) values(?,?);", ip, reason, buffer, NULL);

}

void deleteenpiredinblacklist(const char *ip)
{
	map<string, string> buffer;
	pSqlite->SqlPrepareParam("delete from mytable where ip_address=?;", ip, NULL, buffer, NULL);

}

int checkenpiredinblacklist()
{

}

void closedatabase()
{
	pSqlite->CloseDatabase();
}
*/