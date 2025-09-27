#pragma once
#ifndef STATUS_H
#define STATUS_H
#include <iostream>
#define FAIL(a) (a == 0)
#define NT_SUCCESS(a) (a == ERROR_SUCCESS)
#define log(format, ...) printf("[+] " format "\n", ##__VA_ARGS__)

void fail(const char* failure_message);

#endif