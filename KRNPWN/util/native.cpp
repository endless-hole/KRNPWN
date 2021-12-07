#include "native.hpp"
#include "hash.hpp"
#include "log.hpp"
#include "WinSysInfoQuery.hpp"

#include <Windows.h>

namespace native
{
    void* find_export( void* mod, const std::string& func, bool rva )
    {
        // get export directory from DOS Header -> NT Header -> Optional Header
        auto dos = ( IMAGE_DOS_HEADER* )mod;
        auto nt_hdr = ( IMAGE_NT_HEADERS* )( ( uint8_t* )mod + dos->e_lfanew );
        auto exports = &( nt_hdr->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ] );

        // get virtual address from directory header and add it to image base to get pointer to export directory
        auto exports_dir = ( IMAGE_EXPORT_DIRECTORY* )( exports->VirtualAddress + ( uint8_t* )mod );

        // loop through export directory using NumberOfNames
        for( uint32_t i = 0; i < exports_dir->NumberOfNames; i++ )
        {
            // get the rva of the module names
            uint32_t name_rva = *( uint32_t* )( exports_dir->AddressOfNames +
                ( uint8_t* )mod + i * sizeof( uint32_t ) );

            // resolve rva of module name and compare with input
            if( ( char* )( name_rva + ( uint8_t* )mod ) == func )
            {
                // get the export function rva 
                uint16_t name_idx = *( uint16_t* )( exports_dir->AddressOfNameOrdinals +
                    ( uint8_t* )mod + i * sizeof( uint16_t ) );

                uint32_t func_rva = *( uint32_t* )( exports_dir->AddressOfFunctions +
                    ( uint8_t* )mod + name_idx * sizeof( uint32_t ) );

                if( rva )
                {
                    // only return the offset of the function
                    // double cast to remove warning, basetsd.h has UIntToPtr
                    // #define UIntToPtr( ui )  ((VOID*)(UINT_PTR)((unsigned int)ui))
                    return  ( void* )( uintptr_t )func_rva;
                }
                else
                {
                    // return the offset + module address given in args
                    return ( void* )( ( uint8_t* )mod + func_rva );
                }
            }
        }

        return nullptr;
    }

    void* find_kernel_module( const std::string& module_name )
    {
        SysModInfoQuery query;
        NTSTATUS status;

        // execute the query to load the result into the query's buffer
        if( ( status = query.exec() ) != ERROR_SUCCESS )
        {
            log_err( "failed to query SystemModuleInformation:", status );
            return nullptr;
        }

        // return module kernel memory address
        return query.get_module_base( module_name );
    }

    void* find_kernel_export( const std::string& module_name, const std::string& func )
    {
        SysModInfoQuery query;
        nt::RTL_PROCESS_MODULE_INFORMATION info;
        NTSTATUS status;

        // execute the query to load the result into the query's buffer
        if( ( status = query.exec() ) != ERROR_SUCCESS )
        {
            log_err( "failed to query SystemModuleInformation:", status );
            return 0;
        }

        // find the wanted module in the query buffer, returns true if found and copy's data into info
        if( query.find_module( module_name, info ) )
        {
            // get full path of the module
            std::string full_path = ( char* )info.FullPathName;

            // resolve the symbolic link to an absolute path
            full_path.replace( full_path.find( "\\SystemRoot\\" ),
                sizeof( "\\SystemRoot\\" ) - 1, std::string( getenv( "SYSTEMROOT" ) ).append( "\\" ) );

            // load the module into memory using DONT_RESOLVE_DLL_REFERENCES to disabled DllEntry execution
            const auto mod =
                LoadLibraryExA(
                full_path.c_str(),
                NULL,
                DONT_RESOLVE_DLL_REFERENCES
                );

            // find the rva of the export
            auto rva = find_export( mod, func, true );

            if( rva != 0 )
            {
                // add the rva to the current memory location of the module to get the function address
                return ( void* )( ( uint64_t )info.ImageBase + ( uint64_t )rva );
            }
        }
        return 0;
    }

    std::vector<mem_region> parse_resource_map( const std::string& key, const std::string& value )
    {
        std::vector<mem_region> regions;
        HKEY key_handle;
        LSTATUS result;
        DWORD ret_length, type;

        uint8_t* data = nullptr;

        // get a handle to the resource map reg key
        if( ( result = RegOpenKeyA( HKEY_LOCAL_MACHINE, key.c_str(), &key_handle ) ) != ERROR_SUCCESS )
        {
            log_err( "failed to get memory map key", result );
            return std::vector<mem_region>();
        }

        // get size of resource map by querying the key without a buffer to get return size
        if( ( result = RegQueryValueExA( key_handle, value.c_str(), NULL, &type, NULL, &ret_length ) )
            != ERROR_SUCCESS )
        {
            log_err( "failed to query memory map key", result );
            return std::vector<mem_region>();
        }

        data = ( uint8_t* )malloc( ret_length );

        // query the key once again but with our buffer pointer
        RegQueryValueExA( key_handle, value.c_str(), 0, &type, data, &ret_length );

        // cast the buffer to CM_RESOURCE_LIST
        nt::CM_RESOURCE_LIST* resource_list = ( nt::CM_RESOURCE_LIST* )data;

        // loop through the resource lists in the resource map
        for( uint32_t i = 0; i < resource_list->Count; i++ )
        {
            // each resource list has a nest list within
            for( uint32_t j = 0; j < resource_list->List[ 0 ].PartialResourceList.Count; j++ )
            {
                // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_cm_partial_resource_descriptor
                // prd = partial resource descriptor
                auto prd = resource_list->List[ i ].PartialResourceList.PartialDescriptors[ j ];

                // if resource type is of memory
                if( prd.type == CmResourceTypeMemory || prd.type == CmResourceTypeMemoryLarge )
                {
                    mem_region region;

                    // start address of region
                    region.address = prd.address;

                    // the large memory regions have flags set depending on type
                    // they need to be left shifted as the data is located on the highest bits
                    if( prd.flags & CmResourceTypeMemoryLarge40 )
                        region.size = uint64_t{ prd.size } << 8;
                    else if( prd.flags & CmResourceTypeMemoryLarge48 )
                        region.size = uint64_t{ prd.size } << 16;
                    else if( prd.flags & CmResourceTypeMemoryLarge64 )
                        region.size = uint64_t{ prd.size } << 32;
                    else
                        region.size = uint64_t{ prd.size };

                    regions.push_back( region );
                }
            }
        }

        return regions;
    }
}