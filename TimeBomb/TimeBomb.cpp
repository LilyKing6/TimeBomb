/* 
 * TimeBomb
 * Copyright (C) 2024 Lily King
 */

#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include <thread>
#include <chrono>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "utils.h"
#include "TimeBomb.h"
#include "Itm.h"

#define USE_COLOR_PRINT

#ifndef USE_COLOR_PRINT
#define PrintColor TMB_Print
#endif

#define TMB_DATA_FILE     "tmb.dat"

#define LANG_EVAL               "Informal version. "
#define LANG_EXPIRES            "Expires "
#define LANG_TESTONLY           "For testing purposes only."
#define LANG_BOMB               "The evaluation period for this copy of Loong has ended. Loong cannot start."
#define LANG_BOMB2              "To continue using Loong, please purchase and install a retail copy of the product."               

#pragma warning(disable:4996)

struct InstallTime
{
    int y;
    int m;
    int d;
    int h;
    int mnt;
    int s;
    int number;
} InstallTime, *IT = &InstallTime;

struct ExpireTime
{
    int y;
    int m;
    int d;
    int h;
    int mnt;
    int s;
};

void TMB_Print(std::string str, int c)
{
    std::cout << str;
}

// 计算过期时间
void GetExpireTime(ExpireTime* ETime)
{
    int Start_y;
    int Start_m;
    int Start_d;
    int Start_h;
    int Start_min;
    int Start_s;

    // 编译时间
    Start_y = BUILD_YEAR;
    Start_m = BUILD_MONTH;
    Start_d = BUILD_DAY;
    Start_h = BUILD_HOUR;
    Start_min = BUILD_MINUTE;
    Start_s = BUILD_SECOND;

    // 每个月份的天数
    int mon[13] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };

    // 加上有效期
    Start_d += DAYS;
    Start_h += HOURS;
    Start_min += MINUTES;

    // 跨分处理
    while (Start_min >= 60)
    {
        Start_min -= 60;
        Start_h++;
    }

    // 跨天处理
    while (Start_h >= 24)
    {
        Start_h -= 24;
        Start_d++;
    }

    // 二月份的天数重新计算
    if (Start_m == 2)                             
        mon[2] = 28 + (Start_y % 4 == 0 && Start_y % 100 != 0 || Start_y % 400 == 0);

    // 跨月处理
    while (Start_d > mon[Start_m]) 
    {
        Start_d -= mon[Start_m];
        Start_m = (Start_m == 12) ? 1 : Start_m + 1;

        if (Start_m == 1) {
            Start_y++;
        }

        // 更新二月份的天数
        if (Start_m == 2) 
        { 
            mon[2] = 28 + (Start_y % 4 == 0 && Start_y % 100 != 0 || Start_y % 400 == 0);
        }
    }

    ETime->y = Start_y;
    ETime->m = Start_m;
    ETime->d = Start_d;
    ETime->h = Start_h;
    ETime->mnt = Start_min;
    ETime->s = Start_s;
}


TimeBomb::TimeBomb()
    : m_isRunning(false), m_isExpired(false)
{
    isEcho = 0;
    hideexpireTime = 1;
    checkSystemTime = 1;
}

TimeBombError TimeBomb::removeDataFile()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::ifstream file(TMB_DATA_FILE);
    if (file)
    {
        file.close();
        if (remove(TMB_DATA_FILE) != 0)
        {
            return TimeBombError::FILE_ERROR;
        }
    }
    return TimeBombError::SUCCESS;
}

void TimeBomb::logError(TimeBombError error, const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cerr << "[ERROR] Code: " << static_cast<int>(error) << ", Message: " << message << std::endl;
}

TimeBomb::~TimeBomb()
{
}

// 判断是否过期 过期返回true 未过期返回false
bool TimeBomb::IsExpire()
{
#if TIMEBOMB == 1 /* 时间炸弹启用则编译 */
    // 检查是否已缓存过期状态
    if (m_isExpired)
    {
        return true;
    }

    ExpireTime ExpireTime;
    GetExpireTime(&ExpireTime);

    // 获取当前时间
    time_t now = time(0);
    if (now == -1)
    {
        logError(TimeBombError::TIME_ERROR, "Failed to get current time");
        return true; // 时间获取失败时，视为过期以确保安全
    }

    // 使用UTC时间进行比较，避免时区问题
    struct tm *ntm = gmtime(&now);
    if (ntm == nullptr)
    {
        logError(TimeBombError::TIME_ERROR, "Failed to convert time to UTC");
        return true; // 转换失败时，视为过期以确保安全
    }
    
    // 将时间转换为可比较的元组结构
    auto current = std::make_tuple(
        1900 + ntm->tm_year,
        1 + ntm->tm_mon,
        ntm->tm_mday,
        ntm->tm_hour,
        ntm->tm_min,
        ntm->tm_sec
    );
    
    auto expire = std::make_tuple(
        ExpireTime.y,
        ExpireTime.m,
        ExpireTime.d,
        ExpireTime.h,
        ExpireTime.mnt,
        ExpireTime.s
    );

    bool isExpired = current > expire;
    m_isExpired = isExpired; // 更新缓存的过期状态

    return isExpired;  // 当前时间超过过期时间时返回true
    
#endif /* 时间炸弹启用则编译 */
    return false; // 时间炸弹未启用时，始终返回未过期
}


// 监控线程函数
void TimeBomb::monitoringThread()
{
    m_isRunning = true;
    m_isExpired = IsExpire();

    while (m_isRunning && !m_isExpired)
    {
        // 等待指定间隔
        std::this_thread::sleep_for(std::chrono::seconds(CHECK_INTERVAL_SECONDS));
        
        m_isExpired = IsExpire();
        
        if (m_isExpired)
        {
            // 删除SPP文件
            TimeBombError err = removeDataFile();
            if (err != TimeBombError::SUCCESS)
            {
                logError(err, "Failed to remove loong data file");
            }

            // 输出过期提示信息
            PrintColor(LANG_BOMB, BRIGHT_RED);
            std::cout << std::endl;
            PrintColor(LANG_BOMB2, BRIGHT_RED);
            std::cout << std::endl;

            m_isRunning = false;
            exit(1);
        }
    }
}

// 启动监控线程
void TimeBomb::startMonitoring()
{
    if (!m_isRunning)
    {
        try
        {
            std::thread monitor(&TimeBomb::monitoringThread, this);
            monitor.detach(); // 分离线程，使其在后台运行
        }
        catch (const std::system_error& e)
        {
            logError(TimeBombError::THREAD_ERROR, "Failed to start monitoring thread: " + std::string(e.what()));
        }
    }
}

// 主函数
void TimeBomb::Run()
{
/* 如果时间炸弹启用 */
#if TIMEBOMB == 1

    // 检查系统时间是否准确
    if (CHKSYSTEMTIME && checkSystemTime == 1 && !SysTimeCheck()) 
    {
        std::cerr << "System time is not accurate. Exiting..." << std::endl;
        exit(1);
    }

    ExpireTime ExpireTime;
    GetExpireTime(&ExpireTime);

    // 判断是否过期
    if (IsExpire()) 
    {
        //如果过期

        // 检查SPP文件是否存在
        std::ifstream file(TMB_DATA_FILE);
        if (file) 
        {
            file.close(); // 关闭文件
            // 删除文件
            remove(TMB_DATA_FILE);
        }

        //输出过期提示信息
        PrintColor(LANG_BOMB, BRIGHT_RED);
        std::cout << std::endl;
        PrintColor(LANG_BOMB2, BRIGHT_RED);
        std::cout << std::endl;

        //如果隐藏输出过期时间
        if(hideexpireTime == 1)
        {
            exit(1);
        }
        else
        {
            PrintColor(LANG_EXPIRES, BRIGHT_RED);

            std::ostringstream oss;
            oss << std::setw(2) << std::setfill('0') << ExpireTime.y << "/"
                << std::setw(2) << std::setfill('0') << ExpireTime.m << "/"
                << std::setw(2) << std::setfill('0') << ExpireTime.d << " "
                << std::setw(2) << std::setfill('0') << ExpireTime.h << ":"
                << std::setw(2) << std::setfill('0') << ExpireTime.mnt;
            PrintColor(oss.str(), BRIGHT_RED);

            PrintColor(".", BRIGHT_RED);
            std::cout << std::endl << std::endl;
        }
        exit(1);

    }
    else
    {
        // 如果未过期
        startMonitoring(); // 启动监控线程

        if (isEcho == 1)
        {
            PrintColor(LANG_EVAL, YELLOW);
            PrintColor(LANG_EXPIRES, YELLOW);

            std::ostringstream oss;
            oss << std::setw(2) << std::setfill('0') << ExpireTime.y << "/"
                << std::setw(2) << std::setfill('0') << ExpireTime.m << "/"
                << std::setw(2) << std::setfill('0') << ExpireTime.d << " "
                << std::setw(2) << std::setfill('0') << ExpireTime.h << ":"
                << std::setw(2) << std::setfill('0') << ExpireTime.mnt;
            PrintColor(oss.str(), YELLOW);

            PrintColor(".", YELLOW);

            std::cout << std::endl;
            
            PrintColor(LANG_TESTONLY, MAGENTA);
            std::cout << std::endl << std::endl;
        }
    }

#endif /* 条件编译是否包含时间炸弹代码 */

}

