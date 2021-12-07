#include "pe.hpp"

PIMAGE_NT_HEADERS pe64::get_nt_header( void* image )
{
    PIMAGE_DOS_HEADER dos = ( PIMAGE_DOS_HEADER )image;
    return ( PIMAGE_NT_HEADERS64 )( ( uint8_t* )dos + dos->e_lfanew );
}

PIMAGE_OPTIONAL_HEADER64 pe64::get_optional_header( void* image )
{
    return &get_nt_header( image )->OptionalHeader;
}

uint32_t pe64::get_magic( void* image )
{
    return get_optional_header( image )->Magic;
}

pe64::vec_relocs pe64::get_relocs( void* image )
{
    const auto pe_hdr = get_optional_header( image );

    if( !pe_hdr )
        return {};

    vec_relocs relocs;

    // get directory address and size for relocs
    uint64_t reloc_va = pe_hdr->DataDirectory[ IMAGE_DIRECTORY_ENTRY_BASERELOC ].VirtualAddress;
    uint64_t reloc_size = pe_hdr->DataDirectory[ IMAGE_DIRECTORY_ENTRY_BASERELOC ].Size;

    // ensure there are relocs
    if( reloc_va == 0 && reloc_size == 0 )
        return {};

    auto curr_reloc = ( PIMAGE_BASE_RELOCATION )( ( uint64_t )image + reloc_va );
    auto end_reloc = ( uint64_t )curr_reloc + reloc_size;

    // loop through all the relocs
    while( curr_reloc->VirtualAddress && curr_reloc->VirtualAddress < end_reloc && curr_reloc->SizeOfBlock )
    {
        reloc_info_t info;

        // push the data into the vector
        info.address = ( uint64_t )image + curr_reloc->VirtualAddress;
        info.item = ( uint16_t* )( ( uint64_t )curr_reloc + sizeof( IMAGE_BASE_RELOCATION ) );
        info.count = ( curr_reloc->SizeOfBlock - sizeof( IMAGE_BASE_RELOCATION ) ) / sizeof( uint16_t );

        relocs.push_back( info );

        // go to next reloc location
        curr_reloc = curr_reloc + curr_reloc->SizeOfBlock;
    }

    return relocs;
}

pe64::vec_imports pe64::get_imports( void* image )
{
    const auto pe_hdr = get_optional_header( image );

    if( !pe_hdr )
        return {};

    vec_imports imports;

    // get directory address and size for imports
    auto imports_va = pe_hdr->DataDirectory[ IMAGE_DIRECTORY_ENTRY_IMPORT ].VirtualAddress;
    auto imports_size = pe_hdr->DataDirectory[ IMAGE_DIRECTORY_ENTRY_IMPORT ].Size;

    // ensure there is an imports directory
    if( imports_va == 0 && imports_size == 0 )
        return {};

    // get the first import in list
    auto curr_import = ( PIMAGE_IMPORT_DESCRIPTOR )( ( uint8_t* )image + imports_va );

    // loop through all modules in import list
    while( curr_import->FirstThunk )
    {
        import_info_t import_info;
        import_info.name = std::string( ( char* )( ( uint64_t )image + curr_import->Name ) );

        auto thunk_first = ( PIMAGE_THUNK_DATA64 )( ( uint64_t )image + curr_import->FirstThunk );
        auto thunk_org = ( PIMAGE_THUNK_DATA64 )( ( uint64_t )image + curr_import->OriginalFirstThunk );

        // loop through each function within module in import list
        while( thunk_org->u1.Function )
        {
            import_func_info_t func_info;

            auto thunk = ( PIMAGE_IMPORT_BY_NAME )( ( uint64_t )image + thunk_org->u1.AddressOfData );

            func_info.name = thunk->Name;
            func_info.address = &thunk_first->u1.Function;

            import_info.func_data.push_back( func_info );

            // go to the next function
            ++thunk_org;
            ++thunk_first;
        }

        imports.push_back( import_info );

        // go to the next module
        ++curr_import;
    }

    return imports;
}
