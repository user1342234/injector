#pragma once
#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include "nt.h"

enum class operation_t {
    op_initialized = 0xA0,
    op_finish
};

typedef struct _REQUEST {
    operation_t op;
    UINT64 dll_base; 
    size_t size;
    char target[40];
}REQUEST, * PREQUEST;
#endif