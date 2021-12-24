#include <iostream>
#include <ostream>


template <typename Arg, typename... Args>
void print(std::ostream& out, Arg&& arg, Args&&... args)
{
    out << std::forward<Arg>(arg);
    ((out << ' ' << std::forward<Args>(args)), ...);
    out << std::endl;
}

#define DEBUG(...) print(std::cerr, "[DEBUG]", __VA_ARGS__)
