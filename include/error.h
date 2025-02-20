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

#endif // ERROR_H__
