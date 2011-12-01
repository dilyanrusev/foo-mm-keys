#include "stdafx.h"
#include "common.hxx"

void log_message(const char *message) {
	puts(message);
}

void log_message(const std::string &message) {
	puts(message.c_str());
}
