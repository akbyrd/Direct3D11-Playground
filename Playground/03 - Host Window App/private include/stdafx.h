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

// Windows Header Files:
#include <Windows.h>

// C++ Header Files
#include <iostream>

// Project Header Files
#include "Logging.h"

// Macros
#define RETURN_IF_FALSE(x, r) { if ( !(x) ) { return r; } }
#define LOG_ERROR(x) { std::cout << "Error: " << x; }