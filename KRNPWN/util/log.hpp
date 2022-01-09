#pragma once
#include <Windows.h>
#include <string>
#include <iostream>

/*
 *
 * Simple logging class
 *
 */

enum class e_log_type
{
    dbg,
    err,
    info
};

class log
{
public:
    template< typename ... args >
    explicit log( e_log_type log_type, args ... to_print )
    {
        std::ios state( NULL );
        state.copyfmt( std::cout );
        std::string type;

        switch( log_type )
        {
        case e_log_type::dbg:
            type = "?";
            break;
        case e_log_type::err:
            type = "-";
            break;
        case e_log_type::info:
            type = "+";
            break;
        }

        std::cout << "[" << type << "] ";
        print( to_print ... );
        std::cout.copyfmt( state );
    }

private:
    template <typename T>
    static void print( T only )
    {
        std::cout << only << std::endl;
    }

    template <typename T, typename ... args>
    void print( T current, args ... next )
    {
        std::cout << current << ' ';
        print( next... );
    }
};

#define log_dbg( ... ) log::log( e_log_type::dbg, __VA_ARGS__ )
#define log_err( ... ) log::log( e_log_type::err, __VA_ARGS__ )
#define log_info( ... ) log::log( e_log_type::info, __VA_ARGS__ )