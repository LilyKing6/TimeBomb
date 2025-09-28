/* 
 * Loong Programming Language
 * Copyright (C) 2023-2025 Lily King
 */

#include "TimeBomb.h"
#include "utils.h"

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <csignal>
#include <iomanip>     // std::quoted
#include <thread>
#include <chrono>
#include <ctime>

using std::cout;
using std::endl;
using std::cerr;
using std::string;

#define FMTPRINT fmt::print

int main(int argc, char* argv[])
{
/* Timebomb */
    TimeBomb tmb(1);
    tmb.Run();

	/* 命令行参数处理 */
    if (argc > 1)
    {
        tmb.isEcho = 0;
        tmb.Run();
    }

    while(1)
    {
        // dosomething...
        std::this_thread::sleep_for(std::chrono::seconds(2));
        cout << "Now time: " << std::chrono::system_clock::now().time_since_epoch().count() << endl;
    }

    return 0;
}

