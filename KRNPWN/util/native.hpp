#pragma once

#include <stdint.h>
#include <vector>
#include <string>

namespace native
{
    typedef struct mem_region_t
    {
        uint64_t address;
        uint64_t size;
    } mem_region, *pmem_region;

    void* find_export( void* mod, const std::string& func, bool rva = false );
    void* find_kernel_module( const std::string& module_name );
    void* find_kernel_export( const std::string& name, const std::string& func );
    std::vector<mem_region> parse_resource_map( const std::string& key, const std::string& value );
}