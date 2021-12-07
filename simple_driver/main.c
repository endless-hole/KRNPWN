#pragma warning(disable : 4201)
#pragma warning(disable : 4214)

#include <ntddk.h>

#include "log.h"

NTSTATUS DriverEntry( PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath )
{
    KeEnterGuardedRegion();

    UNREFERENCED_PARAMETER( DriverObject );
    UNREFERENCED_PARAMETER( RegistryPath );

    log_error( "Called from mapped driver\n" );

    PVOID alloc_mem = ExAllocatePoolWithTag( NonPagedPool, 100, 'LOL' );

    if( alloc_mem )
    {
        log_error( "Allocated memory at: 0x%016X\n", alloc_mem );
        log_error( "Address of pointer:  0x%016X\n", &alloc_mem );
        ExFreePoolWithTag( alloc_mem, 'LOL' );
    }

    KeLeaveGuardedRegion();

    return 1337;
}