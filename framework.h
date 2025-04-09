#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>
// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <shellapi.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#include <uxtheme.h>  // 需要链接 uxtheme.lib
#pragma comment(lib, "uxtheme.lib")
#include <dwrite.h>
#pragma comment(lib, "dwrite.lib")

