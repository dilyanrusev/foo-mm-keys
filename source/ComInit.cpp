#include "StdAfx.h"
#include "ComInit.hxx"
#include "common.hxx"


ComInit::ComInit(void) {
	_initialized = SUCCEEDED(::CoInitialize(NULL));
	if (!_initialized) {
		throw std::runtime_error("Failed to initialize COM");
	}
	log_message("COM was successfully initialized");
}


ComInit::~ComInit(void) {
	if (_initialized) {
		::CoUninitialize();
		log_message("COM was uninitialized");
	}
}
