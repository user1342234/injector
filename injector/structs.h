#pragma once
#ifndef STRUCTS_H
#define STRUCTS_H
#define BUILD_22H2
#ifdef BUILD_22H2
namespace WINDOWS { 
#include "Win10_22H2.h" 
} 
#endif

#define ACCESS_FIELD(structure, field, ptr) \
		(decltype(WINDOWS::structure::field)*)(((PUCHAR)ptr + UFIELD_OFFSET(WINDOWS::structure, field)))

#endif