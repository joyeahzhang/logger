#include "../include/LogFile.h"

int main()
{
    while (true) 
    {
        logger::LogFile::Create("./",false)->Append("This is a test log\n");    
    }
    return 0;
}