#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <vector>
#include <stdint.h>
#include <fstream>

#define COLUMN_WIDTH 15

enum e_arg_type
{
    path,
    input_file,
    output_file,
    array_name,
    count
};

bool dump_array_to_file(
    char* file_name, char* array_name, uint8_t* data, int data_size, int column_width )
{
    FILE* dest_file = fopen( file_name, "w+" );

    if( !dest_file )
    {
        printf( "[!] Error opening output file.\n" );
        return false;
    }

    fprintf( dest_file, "#include <stdint.h>\n\n" );

    fprintf( dest_file, "static uint8_t %s[ %d ] = \n{\n\t", array_name, data_size );

    for( int i = 0; i < data_size; i++ )
    {
        fprintf( dest_file, "0x%02X", data[ i ] );

        if( i != data_size - 1 )
            fprintf( dest_file, "," );

        if( ( i + 1 ) % column_width == 0 )
            fprintf( dest_file, "\n\t" );
    }

    fprintf( dest_file, "\n};" );
    fclose( dest_file );

    return true;
}

int main( int argc, char* argv[] )
{
    FILE* input_file;
    void* input_buffer;
    int input_file_size;

    if( argc != e_arg_type::count )
    {
        printf( "[!] Invalid number of arguments.\n" );
        printf( "[!] Found %d, expected %d.\n", argc, e_arg_type::count );
        return 1;
    }

    input_file = fopen( argv[ e_arg_type::input_file ], "rb" );

    if( !input_file )
    {
        printf( "[!] Error opening input file.\n" );
        return 1;
    }

    fseek( input_file, 0, SEEK_END );
    input_file_size = ftell( input_file );
    rewind( input_file );
    input_buffer = malloc( input_file_size );

    if( !input_buffer )
    {
        printf( "[!] Error allocating input buffer memory.\n" );
        return 1;
    }

    fread( input_buffer, 1, input_file_size, input_file );

    if( dump_array_to_file(
        argv[ e_arg_type::output_file ],
        argv[ e_arg_type::array_name ],
        ( uint8_t* )input_buffer,
        input_file_size,
        COLUMN_WIDTH 
        ) )
    {
        printf( "[*] Successfully generated %s", argv[ e_arg_type::output_file ] );
    }

    return 0;
}