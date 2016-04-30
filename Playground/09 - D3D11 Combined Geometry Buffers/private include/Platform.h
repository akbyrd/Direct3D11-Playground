#pragma once

///
// Macros and Fundamental Types
///

#include <stdint.h>

typedef uint8_t  uint8;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int32_t int32;

typedef float  float32;
typedef double float64;

#define Kilobyte 1024LL
#define Megabyte 1024LL * Kilobyte
#define Gigabyte 1024LL * Megabyte

#define ArrayCount(a) (sizeof(a) / sizeof(a[0]))

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
// Input
///

enum InputMessage
{
	FocusGained = 1,
	FocusLost   = 2,

	Quit        = 3
};

//TODO: This is broken as fuck
//TODO: Expand to support passing data
struct InputQueue
{
	InputMessage messages[64] = {};
	uint8        head         = -1;
	uint8        tail         = 0;

	inline InputMessage
	GetInputMessage()
	{
		InputMessage result = messages[tail];
		++tail &= ArrayCount(messages);
		return result;
	}

	inline bool
	PutWindowMessage(InputMessage message)
	{
		++head &= ArrayCount(messages);
		messages[head] = message;

		if ( head == tail )
		{
			//TODO: Log
			++tail &= ArrayCount(messages);
			return true;
		}

		return false;
	}
};


///
// Memory
///

struct SimMemory
{
	void* bytes;
	uint32 size;

	InputQueue input;
	uint64 ticks;
	uint64 tickFrequency;
};