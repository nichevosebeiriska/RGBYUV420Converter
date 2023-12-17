#include"xassert.h"

void xassert(bool condition,std::string what)
{
    if(!condition)
    {
        std::cout<<what<<"\n";
        exit(-1);
    }
}
