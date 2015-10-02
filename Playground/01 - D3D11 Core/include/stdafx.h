// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "targetver.h"

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN

// Windows Header Files:
#include <windows.h>

// C++ Header Files
#include <iostream>
#include <sstream>
#include <string>

// C RunTime Header Files
//#include <stdlib.h>
//#include <malloc.h>
//#include <memory.h>
//#include <tchar.h>

// For some GUID magic in the DirectX headers. Must be included before them.
#include <InitGuid.h>

// Direct3D 11 Header Files
#include <d3d11_2.h>

// Macros
#define RETURN_IF_FALSE(x, r) { if ( !(x) ) { return r; } }

// Convert char* to wchar_t*
#define _WIDE(x) L ## x
#define WIDE(x) _WIDE(x)