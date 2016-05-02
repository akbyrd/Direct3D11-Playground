#pragma once

///
// Macros and Fundamental Types
///

#include <stdint.h>

typedef uint8_t  uint8;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int32_t int32;

typedef float  real32;
typedef double real64;

#define Kilobyte 1024LL
#define Megabyte 1024LL * Kilobyte
#define Gigabyte 1024LL * Megabyte

#define ArrayCount(a) (sizeof(a) / sizeof(a[0]))

#define Assert(condition) if (!condition) { *((uint8 *) 0) = 0; }
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

enum MouseButton
{
	Left,
	Right,
	Middle
};

struct MousePosition
{
	int32 x;
	int32 y;
};

enum InputMessage
{
	FocusGained = 1,
	FocusLost   = 2,

	//TODO: Put ticks in the queue
	//TimeTicks
	//MousePosition
	//MouseClicks
	//ButtonClicks

	Quit        = 3
};

//TODO: Expand to support passing data
struct InputQueue
{
	InputMessage messages[64] = {};
	uint8        head         = -1;
	uint8        count        = 0;
};

static inline InputMessage
GetInputMessage(InputQueue* queue)
{
	Assert(queue->count > 0);

	uint32 size = ArrayCount(queue->messages);
	uint32 tail = (queue->head + (size - queue->count) + 1) % size;
	InputMessage result = queue->messages[tail];
	--queue->count;

	return result;
}

static inline void
PutInputMessage(InputQueue* queue, InputMessage message)
{
	uint32 size = ArrayCount(queue->messages);

	++queue->head %= size;
	queue->messages[queue->head] = message;
	++queue->count;

	if (queue->count > size)
	{
		queue->count = size;
		LOG_WARNING(L"InputQueue is overflowing.");
	}
}


///
// Memory
///

struct SimMemory
{
	//NOTE: bytes are expected to be initialized to zeros.
	void* bytes;
	uint32 size;

	InputQueue input;
	uint64 tickFrequency;
};