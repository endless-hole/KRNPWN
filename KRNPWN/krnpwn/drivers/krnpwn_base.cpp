#include "krnpwn_base.hpp"

#include "../../util/log.hpp"

#include <fstream>
#include <filesystem>

#include <winternl.h>

namespace krnpwn
{
    krnpwn_base::krnpwn_base( std::string _service_name )
        : service_name( _service_name )
    {}


    krnpwn_base::~krnpwn_base()
    {}

    bool krnpwn_base::open_service()
    {
        std::string path = "\\\\.\\";
        path += service_name;

        driver_handle = CreateFileA(
            path.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            NULL,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if( driver_handle == INVALID_HANDLE_VALUE )
        {
            log_err( "cannot get handle to device driver" );
            return false;
        }

        log_info( "successfully got driver handle" );
        log_info( "driver_handle: ", std::hex, driver_handle, "\n" );
        driver_opened = true;

        return true;
    }
}