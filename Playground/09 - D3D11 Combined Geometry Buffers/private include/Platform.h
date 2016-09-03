#pragma once

///
// Macros and Fundamental Types
///

#include <stdint.h>

typedef uint8_t  u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t i32;
typedef int64_t i64;

typedef float  r32;
typedef double r64;

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
	bool isDown;
	u8   transitionCount;
};

struct InputState
{
	u32 newTicks;

	i32 mouseX;
	i32 mouseY;

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
	u32    size;

	InputState input;
	u64        tickFrequency;
};