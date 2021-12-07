#pragma once

#include <ntddk.h>

void log_error_( const char* fmt, ... );
void log_debug_( const char* fmt, ... );
void log_success_( const char* fmt, ... );

#define log_error log_error_
#define log_debug log_debug_
#define log_success log_success_

