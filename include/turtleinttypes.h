#ifndef T_INTTYPES_H
#define T_INTTYPES_H

#ifdef _MSC_VER
#include <inttypes.h>

typedef uint8_t tu8;
typedef uint16_t tu16;
typedef uint32_t tu32;
typedef uint64_t tu64;

typedef int8_t ts8;
typedef int16_t ts16;
typedef int32_t ts32;
typedef int64_t ts64;

typedef void* tpointer;
#else
typedef unsigned char tu8;
typedef unsigned short tu16;
typedef unsigned int tu32;
typedef unsigned long long tu64;

typedef signed char ts8;
typedef signed short ts16;
typedef signed int ts32;
typedef signed long long ts64;

typedef void* tpointer;
#endif

#define T_NULLPOINTER ((void*)0)
#define T_NULL (0)
#define T_TRUE (1)
#define T_FALSE (0)

#endif
