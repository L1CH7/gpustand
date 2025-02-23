#ifndef ERROR_H__
#define ERROR_H__

#include <iostream>
#include <string>

// Color print for errors
#define PRINT_ERROR( error )                                \
{                                                           \
    std::cerr << "\033[0;31m\n";                            \
    std::cerr << __PRETTY_FUNCTION__ << ":\n\t" << error;   \
    std::cerr << "\033[0;0m\n";                             \
}

// user-defined literal for printing red characters
inline std::string operator ""_red( const char * str, const size_t len )
{
    std::string s = "\033[0;31m" // red text colour
    + std::string( str )
    + "\033[0;0m"; // default text colour
    return s;
}

#endif // ERROR_H__
