#ifndef PATTERN_H
#define PATTERN_H
#include "nt.h"
#include "pe.h"
static int CheckMask(unsigned char* base, unsigned char* pattern, unsigned char* mask);
void* FindPatternEx(unsigned char* base, UINT64 size, unsigned char* pattern, unsigned char* mask);
UINT64 FindPattern(UINT64 base, unsigned char* pattern, unsigned char* mask);
#endif