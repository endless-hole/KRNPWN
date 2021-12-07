#pragma once
#include <Windows.h>
#include <string>
#include <vector>

namespace krnpwn
{
    class krnpwn_base
    {
    protected:
        handle_t                driver_handle;
        std::string             service_name;
        std::string             file_path;
        bool                    driver_opened = false;

        krnpwn_base( std::string _service_name );
        ~krnpwn_base();    

    public:
        bool open_service();

        virtual bool read_physical_memory( uint64_t address, void* buffer, size_t size ) = 0;
        virtual bool write_physical_memory( uint64_t address, void* buffer, size_t size ) = 0;

        std::string get_service_name()
        {
            return service_name;
        }

        bool has_handle()
        {
            return driver_opened;
        }
    };
}