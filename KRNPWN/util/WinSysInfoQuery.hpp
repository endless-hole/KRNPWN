#pragma once
#include "nt.hpp"
#include <Windows.h>
#include <vector>
#include <string>
#include <iostream>

// https://www.geoffchappell.com/studies/windows/km/ntoskrnl/api/ex/sysinfo/query.htm

// base class to setup NtQuerySystemInformation
// class template input is the output structure of the query
// input of the class is the enum class ID of the query carried out

template< class T >
class WinSystemInfoQuery
{
private:
    bool                        m_initialised;
    SYSTEMINFOCLASS2            m_si;
    std::vector< uint8_t >      m_buffer;

public:
    WinSystemInfoQuery( SYSTEMINFOCLASS2 sysinfo_class ) :
        m_initialised( false ), m_si( sysinfo_class )
    {}

    NTSTATUS exec()
    {
        NTSTATUS status;
        ULONG return_size;

        // set up an inital buffer size
        ULONG buffer_size = ( ULONG )max( m_buffer.size(), 0x1000 );

        // loop until the output buffer size is correct
        do
        {
            // reserve the buffer size
            m_buffer.reserve( buffer_size );

            // carry out the query
            status = NtQuerySystemInformation(
                ( SYSTEM_INFORMATION_CLASS )m_si,
                m_buffer.data(),
                buffer_size,
                &return_size
            );

            // if status returns invalid buffer size then we set
            // buffer size to the return size then loop through again
            if( status == STATUS_INFO_LENGTH_MISMATCH )
            {
                buffer_size = return_size;
            }
        }
        while( status == STATUS_INFO_LENGTH_MISMATCH );

        // set init to output of NT_SUCCESS
        m_initialised = NT_SUCCESS( status );

        return status;
    }

    // simple template get function that will return the buffer as templated buffer
    // if uninitialised return nullptr
    T* get()
    {
        return m_initialised ? ( T* )m_buffer.data() : nullptr;
    }
};

// https://www.geoffchappell.com/studies/windows/km/ntoskrnl/api/rtl/ldrreloc/process_modules.htm
// https://www.geoffchappell.com/studies/windows/km/ntoskrnl/api/rtl/ldrreloc/process_module_information.htm
class SysModInfoQuery :
    public WinSystemInfoQuery<RTL_PROCESS_MODULES>
{
public:
    SysModInfoQuery() :
        WinSystemInfoQuery<RTL_PROCESS_MODULES>
        ( SYSTEMINFOCLASS2::SystemModuleInformation2 )
    {}

    // Find a certain module and get info of module back
    bool find_module( const std::string& name, RTL_PROCESS_MODULE_INFORMATION& info_out );

    // Get the kernel address for the module
    void* get_module_base( const std::string& name );

    // simple function to print all info of all modules
    void print_info();
};

// https://www.geoffchappell.com/studies/windows/km/ntoskrnl/api/ex/sysinfo/process.htm
class SystemProcessInformationQuery :
    public WinSystemInfoQuery<SYSTEM_PROCESS_INFORMATION>
{
public:
    SystemProcessInformationQuery() :
        WinSystemInfoQuery<SYSTEM_PROCESS_INFORMATION>
        ( SYSTEMINFOCLASS2::SystemProcessInformation2 )
    {}

    PVOID get_proc_id( const std::wstring& name );

    void print_info();

};