#pragma once

#include <cstdint>

//Underscores are stupid
typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint_fast8_t  uint8f;
typedef uint_fast16_t uint16f;
typedef uint_fast32_t uint32f;
typedef uint_fast64_t uint64f;

typedef int_fast8_t  int8f;
typedef int_fast16_t int16f;
typedef int_fast32_t int32f;
typedef int_fast64_t int64f;

// Macros
#define IGNORE
#define IS_FALSE !

#define IF(expression, check, action) \
do {                                  \
	if ( check(expression) )          \
	{                                 \
		action;                       \
	}                                 \
} while ( 0 )                         