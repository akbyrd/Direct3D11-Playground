// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// C++ Header Files
#include <algorithm>
#include <iostream>

#define RETURN_IF_FALSE(x, r) { if ( !(x) ) { return r; } }
#define LOG_ERROR(x) { std::cout << "Error: " << x; }