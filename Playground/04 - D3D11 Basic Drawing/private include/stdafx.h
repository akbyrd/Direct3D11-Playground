// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

// Modify Windows Header Files
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "targetver.h"

// Windows Header Files:
#include <windows.h>

// C++ Header Files
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

// For some GUID magic in the DirectX/DXGI headers. Must be included before them.
#include <InitGuid.h>

// Direct3D 11 Header Files
#include <d3d11.h>
#include <DirectXMath.h>
#ifdef _DEBUG
#include <dxgidebug.h>
#endif

// Macros
#define CHECK_HR(hr) if ( LOG_IF_FAILED(hr) ) { goto Cleanup; }
#define CHECK_RET(ret) if ( (ret) < 0 ) { goto Cleanup; }