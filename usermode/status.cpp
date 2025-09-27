#pragma once
#include "status.h"

void fail(const char* failure_message) {
	
	printf("[+] ");
	printf(failure_message);
	printf("\n");
	std::getchar();
	std::abort();
}