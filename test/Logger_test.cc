#include "Logger.h"
#include "AsyncLog.h"

#include <chrono>
#include <functional>
#include <string_view>
#include <thread>

using namespace doggy;

int main()
{
    AsyncLog asyc_log("./");
    Logger::SetOutput(std::bind(&AsyncLog::Append, &asyc_log, std::placeholders::_1));
    LOG_TRACE << "hello world";
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}