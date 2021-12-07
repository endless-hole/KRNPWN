#pragma once

#pragma once
#include "krnpwn_base.hpp"

// https://github.com/vaptu/winio/blob/master/Source/Drv/winio_nt.h#L51

#pragma pack(push, 1)
struct winio64_mem_t
{
    uint64_t physical_memory_size;
    uint64_t physical_address;
    uint64_t physical_memory_handle;
    uint64_t linear_address;
    uint64_t physical_section;
};
#pragma pack(pop)

namespace krnpwn
{
    class krnpwn_winio64 : public krnpwn_base
    {
    public:
        krnpwn_winio64()
            : krnpwn_base( "WinIo" )
        {}

        bool read_physical_memory( uint64_t address, void* buffer, size_t size );
        bool write_physical_memory( uint64_t address, void* buffer, size_t size );

    private:
        uint64_t map_physical_to_linear( winio64_mem_t& winio_struct );
        bool unmap_physical_memory( winio64_mem_t& winio_struct );
    };
}