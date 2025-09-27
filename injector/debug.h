#pragma once
#ifndef DEBUG_H
#define DEBUG_H
#include "nt.h"
#ifdef _DEBUG 
#define DEBUG_BREAK __debugbreak()
#define fail(msg) do { \
	DbgPrintEx(0, 0, msg), DEBUG_BREAK; \
} while (0)
#else
#define DEBUG_BREAK
#define fail(a) KeBugCheck(0xBAD); 

#endif

#endif