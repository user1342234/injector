#ifndef LOGGING_H
#define LOGGING_H
#include "nt.h"
#ifdef _DEBUG
#define log(format, ...) DbgPrintEx(0, 0, "[+] " format "\n", ##__VA_ARGS__)
#else
#define log(format, ...)
#endif
#endif