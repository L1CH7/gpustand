#ifndef ERROR_H__
#define ERROR_H__

#include <iostream>
#include <string>
using std::literals::string_literals::operator""s;

// Color printing for errors and warnings

#define COLOR( is_bold, color_id ) "\033["#is_bold";"#color_id"m" // "\033[0;31m" corresponds to red
#define DARK            COLOR(0, 30)
#define RED             COLOR(0, 31) 
#define GREEN           COLOR(0, 32)
#define YELLOW          COLOR(0, 33)
#define BLUE            COLOR(0, 34)
#define MAGENTA         COLOR(0, 35)
#define CYAN            COLOR(0, 36)
#define WHITE           COLOR(0, 37)
#define GRAY            COLOR(1, 30)
#define BOLD_RED        COLOR(1, 31)
#define BOLD_GREEN      COLOR(1, 32)
#define BOLD_YELLOW     COLOR(1, 33)
#define BOLD_BLUE       COLOR(1, 34)
#define BOLD_MAGENTA    COLOR(1, 35)
#define BOLD_CYAN       COLOR(1, 36)
#define BOLD_WHITE      COLOR(1, 37)
#define DEFAULT         COLOR(0, 0)

#define ERR_            BOLD_RED
#define WARN_           BOLD_YELLOW
#define DFLT_           DEFAULT

static inline std::string error_str( const std::string & str )
{
    return ERR_ + str + DFLT_;
}

static inline std::string warn_str( const std::string & str )
{
    return WARN_ + str + DFLT_;
}

static inline std::string focus_str( const std::string & str )
{
    return GREEN + str + DFLT_;
}

#endif // ERROR_H__
