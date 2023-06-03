#include "../include/AsyncLog.h"

int main()
{
    doggy::AsyncLog async_log {"./", 3};
    while(true)
    {
        async_log.Append("this is a test log.\n");
    }
    return 0;
}