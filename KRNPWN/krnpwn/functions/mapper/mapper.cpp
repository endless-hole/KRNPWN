#include "mapper.hpp"

/*
* Contructor for the mapper class
* 
* [in] std::shared_ptr<vdf::vdf> | _ctx   | pointer to vdf, this is used to carry out all kernel operations
*/
mapper::mapper( std::shared_ptr<krnpwn::krnpwn> _pwn )
    : pwn( _pwn ), remote_entry_point( 0 ), remote_image_base( 0 ), mapped( false )
{}

/*
* Maps a driver image into the kernel address space
* The mapper will load the image into local memory first to parse the header of the image then
* will allocate kernel memory and calculate relocation and imports based off the kernel memory
* address given.
* It will then call the entry point of the loaded image to carry out execution.
* 
* [in] std::vector<uint8_t>&     | _image | vector containing the image to be mapped into kernel space
* 
* [ret] bool | true if image was successfully mapped
*/
NTSTATUS mapper::map_image( std::vector<uint8_t>& _image )
{
    image = _image;

    auto nt_hdr = pe64::get_nt_header( image.data() );
    auto pe_hdr = pe64::get_optional_header( image.data() );

    if( pe_hdr->Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC )
    {
        log_err( "image is not a 64bit file" );
        return false;
    }

    uint32_t image_size = pe_hdr->SizeOfImage;

    void* local_image_base = VirtualAlloc( nullptr, image_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
    if( !local_image_base )
    {
        log_err( "could not allocate local image memory" );
        return false;
    }

    // allocate kernel memory for the image that is about to be mapped to kernel
    remote_image_base = allocate_pool( nt::POOL_TYPE::NonPagedPool, image_size );

    if( !remote_image_base )
    {
        log_err( "failed to allocate kernel memory for image" );
        return false;
    }

    // do while loop allows us to jump out of the loop to exit mapping attempt, saves repeated code

    log_info( "image base has been allocated at:", std::hex, remote_image_base );
    log_info( "image allocated size:            ", std::hex, image_size );
    log_info( "image entry point offset:        ", std::hex, pe_hdr->AddressOfEntryPoint );
    printf( "\n" );

    // copy headers to local buffer
    memcpy( local_image_base, image.data(), pe_hdr->SizeOfHeaders );

    // copy sections over to local buffer
    const auto curr_img_sec = IMAGE_FIRST_SECTION( nt_hdr );

    for( auto i = 0; i < nt_hdr->FileHeader.NumberOfSections; ++i )
    {
        auto sec = ( uint64_t )local_image_base + curr_img_sec[ i ].VirtualAddress;
        memcpy( ( void* )sec, image.data() + curr_img_sec[ i ].PointerToRawData, curr_img_sec[ i ].SizeOfRawData );
    }

    // resolve relocs and imports
    reloc_by_delta( pe64::get_relocs( local_image_base ), remote_image_base - pe_hdr->ImageBase );

    if( !resolve_imports( pe64::get_imports( local_image_base ) ) )
    {
        log_err( "failed to resolve imports" );
        VirtualFree( local_image_base, 0, MEM_RELEASE );
        free_pool( remote_image_base );
        return false;
    }

    // write fixed image to kernel
    if( !pwn->kmemcpy( ( void* )remote_image_base, ( void* )( ( uint64_t )local_image_base ), image_size ) )
    {
        log_err( "failed to write to remote kernel address" );
        VirtualFree( local_image_base, 0, MEM_RELEASE );
        free_pool( remote_image_base );
        return false;
    }

   
    remote_entry_point = remote_image_base + pe_hdr->AddressOfEntryPoint;

    log_info( "calling mapped driver entry point:", std::hex, remote_entry_point );

    Sleep( 1000 );

    using driver_entry_t = NTSTATUS( __stdcall* )( );

    // call driver entry point and get the return value
    NTSTATUS return_value = pwn->kcall< driver_entry_t >( ( void* )remote_entry_point );

    // free local image as it is now loaded into kernel
    VirtualFree( local_image_base, 0, MEM_RELEASE );

    return return_value;
}

/*
* Offset the relocations within PE file by the delta
* 
* [in/out] pe64::vec_relocs | relocs | vector containing relocation data 
* [in]     uint64_t         | delta  | delta ( remote base - local header image base )
*/
void mapper::reloc_by_delta( pe64::vec_relocs relocs, const uint64_t delta )
{
    log_info( "relocs:" );

    for( int i = 0; i < relocs.size(); i++ )
    {
        log_info( "\treloc[", i, "] ->" );

        for( auto j = 0u; j < relocs[ i ].count; ++j )
        {
            const uint16_t type   = relocs[ i ].item[ j ] >> 12;
            const uint16_t offset = relocs[ i ].item[ j ] & 0xfff;

            if( type == IMAGE_REL_BASED_DIR64 )
            {
                *( uint64_t* )( relocs[ i ].address + offset ) += delta;
                log_info( "\t\treloc_item[", j, "] ->", std::hex, *( uint64_t* )( relocs[ i ].address + offset ) );
            }
        }
    }
    printf( "\n" );
}

/*
* Finds the needed kernel function addresses for the imports
* 
* [in/out] pe64::vec_imports | imports | vector containing the module and function names of the imports
*/
bool mapper::resolve_imports( pe64::vec_imports imports )
{
    log_info( "imports:" );

    for( const auto& curr_mod : imports )
    {
        log_info( "\t", curr_mod.name );

        for( const auto& curr_func : curr_mod.func_data )
        {
            auto func = native::find_kernel_export( curr_mod.name, curr_func.name );

            log_info( "\t\t", curr_func.name, "->", std::hex, func);

            *curr_func.address = ( uint64_t )func;
        }
    }
    printf( "\n" );
    return true;
}

/*
* Kernel memset called from usermode syscall hook
* 
* [in] void*    | dst  | start address for the memset operation
* [in] uint32_t | data | byte value to fill memory block
* [in] size_t   | size | size of memory block
*/
void mapper::kmemset( void* dst, uint32_t data, size_t size )
{
    static const auto func = native::find_kernel_export( "ntoskrnl.exe", "memset" );

    pwn->void_kcall< decltype( &memset ) >( func, dst, data, size );
}

/*
* Kernel call, using usermode syscall hook, to ExAllocatePoolWithTag. Tag has already been set within function
* 
* [in] POOL_TYPE | type | type of pool to allocate
* [in] size_t    | size | size of pool to allocate
*/
uint64_t mapper::allocate_pool( nt::POOL_TYPE type, size_t size )
{
    static const auto func = native::find_kernel_export( "ntoskrnl.exe", "ExAllocatePoolWithTag" );

    // ExAllocatePoolWithTag prototype
    using alloc_pool_fn = uint64_t( __stdcall* )( nt::POOL_TYPE, size_t, uint32_t );

    return pwn->kcall< alloc_pool_fn >( func, type, size, 'HALB' ); // BLAH
}

/*
* Kernel call, using usermode syscall hook, to ExFreePool
* 
* [in] uint64_t | address | start address of pool to free
*/
bool mapper::free_pool( uint64_t address )
{
    static const auto func = native::find_kernel_export( "ntoskrnl.exe", "ExFreePool" );

    // ExFreePool prototype
    using free_pool_fn = bool( __stdcall* )( uint64_t );

    return pwn->kcall< free_pool_fn >( func, address );
}