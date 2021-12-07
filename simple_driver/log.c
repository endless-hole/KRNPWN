#include "log.h"
#include <stdarg.h>


void log_error_( const char* fmt, ... )
{
    va_list args;
    va_start( args, fmt );

    vDbgPrintExWithPrefix( "[!] ", DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, fmt, args );

    va_end( args );
}

void log_success_( const char* fmt, ... )
{
    va_list args;
    va_start( args, fmt );

    vDbgPrintExWithPrefix( "[*] ", DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, fmt, args );

    va_end( args );
}

void log_debug_( const char* fmt, ... )
{
    va_list args;
    va_start( args, fmt );

    vDbgPrintExWithPrefix( "[?] ", DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, fmt, args );

    va_end( args );
}