// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// For some GUI magic in the DirectX headers. Must be included before them.
#include <InitGuid.h>

// Direct3D 11 Header Files

// Macros
#define RETURN_IF_FALSE(x, r) { if ( !(x) ) { return r; } }