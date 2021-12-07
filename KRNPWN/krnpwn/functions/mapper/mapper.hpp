#pragma once
#include "../../krnpwn.hpp"
#include "nt_defs.hpp"
#include "pe.hpp"

#include <memory>
#include <vector>

class mapper
{
private:
    const std::shared_ptr< krnpwn::krnpwn > pwn;
    std::vector< uint8_t > image;

    uint64_t remote_image_base;
    uint64_t remote_entry_point;

    bool mapped;

public:
    mapper( std::shared_ptr< krnpwn::krnpwn > _pwn );
    ~mapper() {}

    bool map_image( std::vector< uint8_t >& _image );

    uint64_t get_image_base() { return remote_image_base; }
    uint64_t get_entry_point() { return remote_entry_point; }
    bool is_mapped() { return mapped; }

private:
    void reloc_by_delta( pe64::vec_relocs relocs, const uint64_t delta );
    bool resolve_imports( pe64::vec_imports imports );

    void kmemset( void* dst, uint32_t data, size_t size );

    uint64_t allocate_pool( nt::POOL_TYPE type, size_t size );
    bool free_pool( uint64_t address );
};