// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

// Modify Window Header Files
#define WIN32_LEAN_AND_MEAN
#include "targetver.h"

// Windows Header Files:
#include <windows.h>

// ATL Header Files
#include <atlbase.h>

// For some GUID magic in the DirectX/DXGI headers. Must be included before them.
#include <InitGuid.h>

// Direct3D 11 Header Files
#include <d3d11.h>
#include <DirectXMath.h>
#ifdef _DEBUG
#include <dxgidebug.h>
#endif

// Project Header Files
#include "Logging.h"

// Macros
#define CHECK(x)             \
do {                         \
	if ( !(x) )              \
	{                        \
		return false;        \
	}                        \
} while (0)


#define CHECK_HR(hr)         \
do {                         \
	if ( FAILED(hr) )        \
	{                        \
		LOG_IF_FAILED(hr);   \
		return false;        \
	}                        \
} while (0)