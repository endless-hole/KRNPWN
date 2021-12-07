#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <vector>

namespace pe64
{
    struct reloc_info_t
    {
        uint64_t address;
        uint16_t* item;
        uint32_t count;
    };

    struct import_func_info_t
    {
        std::string name;
        uint64_t* address;
    };

    struct import_info_t
    {
        std::string name;
        std::vector<import_func_info_t> func_data;
    };

    using vec_sections  = std::vector<IMAGE_SECTION_HEADER>;
    using vec_relocs    = std::vector<reloc_info_t>;
    using vec_imports   = std::vector<import_info_t>;

    PIMAGE_NT_HEADERS get_nt_header( void* image );
    PIMAGE_OPTIONAL_HEADER64 get_optional_header( void* image );
    uint32_t get_magic( void* image );

    vec_relocs get_relocs( void* image );
    vec_imports get_imports( void* image );
}