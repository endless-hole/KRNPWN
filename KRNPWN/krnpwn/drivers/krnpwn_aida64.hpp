#pragma once
#include "krnpwn_base.hpp"

namespace krnpwn
{
    class krnpwn_aida64 : public krnpwn_base
    {
    public:
        krnpwn_aida64()
            : krnpwn_base( "AIDA64Driver" )
        {}

        bool read_physical_memory( uint64_t address, void* buffer, size_t size );
        bool write_physical_memory( uint64_t address, void* buffer, size_t size );
    };
}
