#include "WinSysInfoQuery.hpp"

bool SysModInfoQuery::find_module( const std::string& name, nt::RTL_PROCESS_MODULE_INFORMATION& info_out )
{

    auto buffer = get();

    for( uint32_t i = 0; i < buffer->NumberOfModules; i++ )
    {
        nt::RTL_PROCESS_MODULE_INFORMATION* info = &buffer->Modules[ i ];
        UCHAR* file_name = info->FullPathName + info->OffsetToFileName;

        if( name == ( char* )file_name )
        {
            info_out = *info;
            return true;
        }
    }

    return false;
}

void* SysModInfoQuery::get_module_base( const std::string& name )
{
    auto buffer = get();

    for( uint32_t i = 0; i < buffer->NumberOfModules; i++ )
    {
        nt::RTL_PROCESS_MODULE_INFORMATION* info = &buffer->Modules[ i ];
        UCHAR* file_name = info->FullPathName + info->OffsetToFileName;

        if( name == ( char* )file_name )
        {
            return info->ImageBase;
        }
    }

    return nullptr;
}

void SysModInfoQuery::print_info()
{
    nt::RTL_PROCESS_MODULES* buffer = get();

    for( uint32_t i = 0; i < buffer->NumberOfModules; i++ )
    {
        nt::RTL_PROCESS_MODULE_INFORMATION* info = &buffer->Modules[ i ];

        UCHAR* file_name = info->FullPathName + info->OffsetToFileName;

        std::cout << file_name << std::endl;
        std::cout << "\tfile_path:   " << info->FullPathName << std::endl;
        std::cout << "\tmapped_base: " << info->MappedBase << std::endl;
        std::cout << "\timage_base:  " << info->ImageBase << std::endl;
        std::cout << "\timage_size:  " << info->ImageSize << std::endl;
        std::cout << "\tflags:       " << info->Flags << std::endl;
        std::cout << std::endl;
    }
}

PVOID SystemProcessInformationQuery::get_proc_id( const std::wstring& name )
{
    auto buffer = get();

    while( buffer->NextEntryOffset )
    {
        if( buffer->ImageName.Buffer == NULL )
        {
            buffer = ( PSYSTEM_PROCESS_INFORMATION )(
                ( BYTE* )buffer + buffer->NextEntryOffset );

            continue;
        }

        if( name == buffer->ImageName.Buffer )
        {
            return buffer->UniqueProcessId;
        }

        buffer = ( PSYSTEM_PROCESS_INFORMATION )(
            ( BYTE* )buffer + buffer->NextEntryOffset );
    }
}

void SystemProcessInformationQuery::print_info()
{
    auto buffer = get();

    while( buffer->NextEntryOffset )
    {
        if( buffer->ImageName.Buffer == NULL )
        {
            buffer = ( PSYSTEM_PROCESS_INFORMATION )(
                ( BYTE* )buffer + buffer->NextEntryOffset );

            continue;
        }

        std::wcout << buffer->ImageName.Buffer << std::endl;
        std::cout << "\tproc_id: " << buffer->UniqueProcessId << std::endl;

        buffer = ( PSYSTEM_PROCESS_INFORMATION )(
            ( BYTE* )buffer + buffer->NextEntryOffset );
    }
}
