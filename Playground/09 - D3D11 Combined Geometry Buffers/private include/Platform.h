#pragma once

///
// Macros and Fundamental Types
///

#include <stdint.h>

typedef uint8_t  uint8;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int32_t int32;
typedef int64_t int64;

typedef float  real32;
typedef double real64;

typedef wchar_t wchar;

#define Kilobyte 1024LL
#define Megabyte 1024LL * Kilobyte
#define Gigabyte 1024LL * Megabyte

#define ArrayCount(a) (sizeof(a) / sizeof(a[0]))

#define Assert(condition) if (!(condition)) { *((uint8 *) 0) = 0; }
#define InvalidCodePath Assert(!"Invalid code path")
#define InvalidDefaultCase default: InvalidCodePath; break

//TODO: Confirm the loop is optimized away
#define IF(expression, check, action) \
do {                                  \
	if ( check(expression) )          \
	{                                 \
		action;                       \
	}                                 \
} while ( 0 )                         

#ifdef IGNORE
	//WinBase
	#undef IGNORE
#endif

#define IGNORE
#define IS_FALSE !


///
// Global Functionality
///

#include "Logging.h"

///
// Input
///

struct ButtonState
{
	bool  isDown;
	uint8 transitionCount;
};

struct InputState
{
	uint32 newTicks;

	int32 mouseX;
	int32 mouseY;

	union
	{
		ButtonState buttons[2];
		struct
		{
			ButtonState mouseLeft;
			ButtonState mouseRight;
		};
	};
};


///
// Memory
///

struct SimMemory
{
	//NOTE: bytes are expected to be initialized to zeros.
	void*  bytes;
	uint32 size;

	InputState input;
	uint64     tickFrequency;
};