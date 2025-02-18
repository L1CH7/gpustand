#ifndef ERROR_H__
#define ERROR_H__

#include <iostream>
#include <string>

#define PRINT_ERROR( error )                                \
{                                                           \
    std::cerr << "\033[0;31m\n";                            \
    std::cerr << __PRETTY_FUNCTION__ << ":\n\t" << error;   \
    std::cerr << "\033[0;0m\n";                             \
}
// inline void printError( const std::string & error )
// {
//     std::cerr << "\033[0;31m" << std::endl;
//     std::cerr << __PRETTY_FUNCTION__ << ":\n\t" << error;
//     std::cerr << "\033[0;0m" << std::endl;
// }

#endif // ERROR_H__
