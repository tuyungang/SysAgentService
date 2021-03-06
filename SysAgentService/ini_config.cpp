/* *******************************************************
 * Call Center On Demand Product Series
 * Copyright (C) 2017 HonDa(Guangzhou.) Technology Ltd., Co.
 * All right reserved
 *
 * @file ini_config.c
 * @brief 
 * @author tuyungang
 * @version v1.0
 * @date 2017-12-01
 * 
 * TODO: 读写ini配置文件
 * 
 * *******************************************************/
#include "ini_config.h"
//#include "thread_pool.h"

//sAgentConfigInfo *g_ConfigInfo = NULL;
char g_LoginName[10] = {0};
char g_LoginPassword[128] = {0};
char g_MasterIP[32] = {0};
char g_StandbyIP[32] = {0};
char g_Port[10] = {0};
char g_SystemName[10] = {0};
char g_SafeBoxID[128] = {0};
char g_IniFilePath[256] = {0};

char g_CurAbsolutePath[256] = {0};
char g_LogAbsolutePath[256] = {0};
char g_CacheFileAbsolutePath[256] = {0};

bool InitIniConfig()
{
    bool bRet;
    //char g_CurAbsolutePath[256];
    //memset(g_CurAbsolutePath, '\0', 256);
    // memset(g_IniFilePath, '\0', 256);
    
    /*
    if (NULL == getcwd(g_CurAbsolutePath, 256)) {
        return false;
    }
    */
	sprintf(g_IniFilePath, "%s/agent.ini", g_IniFilePath);
    memcpy(g_IniFilePath, "C:\\hondavda\\agent.ini", strlen("C:\\hondavda\\agent.ini"));
    if (_access("agent.ini", 0) != 0) {
        return false;
    }

    return true;
}

void trim(char *str)
{
    int len = strlen(str);
    char *p = str + len - 1;

    while(*p == ' ')
    {
        p--;
    }

    *(p + 1) = '\0';
}

void GetIniKeyString(const char *section, const char *key, char *value)
{
    if (section == NULL || key == NULL)
        return;
    FILE *pFile = NULL;
    pFile = fopen(g_IniFilePath, "r");
    if (pFile == NULL)
        return;

    char line[256];
    while (!feof(pFile)) {
        memset(line, '\0', 256);
        fgets(line, sizeof(line), pFile);
        line[strlen(line) - 1] = '\0';
        if (line[0] == ';' || line[0] == '\r' || line[0] == '\n' || line[0] == '\0') {
            continue;
        }
        char *start = NULL;
        if ((start = strchr(line, '[')) == NULL) {
            continue;
        }

        trim(start);
        //start[strlen(start) - 1] = '\0';
		if (_stricmp(start, section) == 0) {
            char content[256];
            while (!feof(pFile)) {
                memset(content, '\0', 256);
                fgets(content, sizeof(content), pFile);
                content[strlen(content)] = '\0';
                if (content[0] == ';')
                    continue;
                trim(content);
                char *m_value = NULL;
                m_value = strpbrk(content, "=");
                *m_value++ = '\0';
                if (strncmp(content, key, strlen(content)) == 0) {
                    memcpy(value, m_value, strlen(m_value) - 1);
                    break;
                }
            }
            break;
        }
    }
    fclose(pFile);
    return;
}

void GetIniKeyInt(const char *section, const char *key, int value)
{
    if (section == NULL || key == NULL)
        return;
    FILE *pFile = NULL;
    pFile = fopen(g_IniFilePath, "r");
    if (pFile == NULL)
        return;

    char line[256];
    while (!feof(pFile)) {
        memset(line, '\0', 256);
        fgets(line, sizeof(line), pFile);
        line[strlen(line) - 1] = '\0';
        if (line[0] == ';' || line[0] == '\r' || line[0] == '\n' || line[0] == '\0') {
            continue;
        }
        char *start = NULL;
        if ((start = strchr(line, '[')) == NULL) {
            continue;
        }

        trim(start);
        //start[strlen(start) - 1] = '\0';
		if (_stricmp(start, section) == 0) {
            char content[256];
            while (!feof(pFile)) {
                memset(content, '\0', 256);
                fgets(content, sizeof(content), pFile);
                content[strlen(content) - 1] = '\0';
                if (content[0] == ';')
                    continue;
                trim(content);
                char *m_value = NULL;
                m_value = strpbrk(content, "=");
                *m_value++ = '\0';
                if (strncmp(content, key, strlen(content) - 1) == 0) {
                    //memcpy(value, m_value, strlen(m_value));
                    value = atoi(m_value);
                    break;
                }
            }
            break;
        }
    }
    fclose(pFile);
    return;
}

void GetIniKeyLong(const char *section, const char *key, long value)
{
    if (section == NULL || key == NULL)
        return;
    FILE *pFile = NULL;
    pFile = fopen(g_IniFilePath, "r");
    if (pFile == NULL)
        return;

    char line[256];
    while (!feof(pFile)) {
        memset(line, '\0', 256);
        fgets(line, sizeof(line), pFile);
        line[strlen(line) - 1] = '\0';
        if (line[0] == ';' || line[0] == '\r' || line[0] == '\n' || line[0] == '\0') {
            continue;
        }
        char *start = NULL;
        if ((start = strchr(line, '[')) == NULL) {
            continue;
        }

        trim(start);
        //start[strlen(start) - 1] = '\0';
		if (_stricmp(start, section) == 0) {
            char content[256];
            while (!feof(pFile)) {
                memset(content, '\0', 256);
                fgets(content, sizeof(content), pFile);
                content[strlen(content) - 1] = '\0';
                if (content[0] == ';')
                    continue;
                trim(content);
                char *m_value = NULL;
                m_value = strpbrk(content, "=");
                *m_value++ = '\0';
                if (strncmp(content, key, strlen(content) - 1) == 0) {
                    //memcpy(value, m_value, strlen(m_value));
                    value = atol(m_value);
                    break;
                }
            }
            break;
        }
    }
    fclose(pFile);
    return;
}
