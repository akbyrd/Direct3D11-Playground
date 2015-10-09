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

// For some GUID magic in the DirectX headers. Must be included before them.
#include <InitGuid.h>

// Direct3D 11 Header Files
#include <d3d11.h>
#include <DirectXMath.h>

// Project Header Files
#include "ExitCode.h"
#include "Logging.h"

// Macros
#define RETURN_IF_FALSE(x, r) { if ( !(x) ) { return r; } }
#define RETURN_IF_FAILED(x)   { long ret = (x); if ( ret < 0 ) { return ret; } }
#define CHECK_HR(hr) if ( LOG_IF_FAILED(hr) ) { goto Cleanup; }
#define CHECK_RET(ret) if ( (ret) < 0 ) { goto Cleanup; }

// Convert char* to wchar_t*
#define _WIDE(x) L ## x
#define WIDE(x) _WIDE(x)