/* 
 * 时间验证模块
 * Copyright (c) 2024 Lily King. All rights reserved.
 */

#ifndef ITM_H
#define ITM_H

#include <cstdint>
#include <ctime>
#include <chrono>
#include <iostream>
#include <cstring>
#include <random>
#include <vector>
#include <thread>
#include <chrono>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif

#define JAN_1970                2208988800UL

#define TIME_DIFF_THRESHOLD     10          // 时间差阈值 单位秒
#define MAX_RETRIES             3           // 最大重试次数

struct NTPPacket 
{
    uint8_t li_vn_mode;      // 2 bits 闰秒指示器, 3 bits 版本号, 3 bits 模式
    uint8_t stratum;         // 本地时钟的层级
    uint8_t poll;            // 连续消息之间的最大间隔
    uint8_t precision;       // 本地时钟的精度

    uint32_t rootDelay;      // 总往返延迟时间
    uint32_t rootDispersion; // 主要时钟源的最大允许误差
    uint32_t refId;          // 参考时钟标识符

    uint32_t refTm_s;        // 参考时间戳秒数
    uint32_t refTm_f;        // 参考时间戳秒的小数部分
    uint32_t origTm_s;       // 原始时间戳秒数
    uint32_t origTm_f;        // 原始时间戳秒的小数部分
    uint32_t rxTm_s;         // 接收时间戳秒数
    uint32_t rxTm_f;         // 接收时间戳秒的小数部分
    uint32_t txTm_s;         // 发送时间戳秒数
    uint32_t txTm_f;         // 发送时间戳秒的小数部分
};

bool isNetworkAvailable = false;

std::vector<std::pair<std::string, std::string>> ntpServers = 
{
    {"time.pool.aliyun.com", "203.107.6.88"}, // 阿里云时间服务器及其IP地址
    {"time.windows.com", "64.4.0.50"},        // Windows时间服务器及其IP地址
    {"time.google.com", "216.239.35.0"}       // Google时间服务器及其IP地址
};

void cleanupSocket(int sockfd) 
{
#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif
}

int Get_time_t(time_t& ttime, const std::string& server, const std::string& ip) 
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        std::cerr << "WSAStartup failed for " << server << std::endl;
        return -1;
    }
#endif

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
    {
        std::cerr << "Error creating socket for " << server << std::endl;
        cleanupSocket(sockfd);
        return -1;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(123);
#ifdef _WIN32
    servaddr.sin_addr.s_addr = inet_addr(ip.c_str());
#else
    if (inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr) <= 0) 
    {
        std::cerr << "inet_pton error for " << ip << std::endl;
        cleanupSocket(sockfd);
        return -1;
    }
#endif

    NTPPacket ntpSend = {0x1B}; // LI=0, VN=3, Mode=3
    if (sendto(sockfd, (char*)&ntpSend, sizeof(NTPPacket), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) 
    {
        std::cerr << "Error sending request to " << server << " (" << ip << ")" << std::endl;
        cleanupSocket(sockfd);
        return -1;
    }

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    if (select(sockfd + 1, &readfds, nullptr, nullptr, &tv) < 0) 
    {
        std::cerr << "Error in select for " << server << " (" << ip << ")" << std::endl;
        cleanupSocket(sockfd);
        return -1;
    }

    if (FD_ISSET(sockfd, &readfds)) 
    {
        NTPPacket ntpRecv;

        // int len = sizeof(servaddr);
        socklen_t len = sizeof(servaddr);

        if (recvfrom(sockfd, (char*)&ntpRecv, sizeof(NTPPacket), 0, (struct sockaddr *)&servaddr, &len) < 0) 
        {
            std::cerr << "Error receiving response from " << server << " (" << ip << ")" << std::endl;
            cleanupSocket(sockfd);
            return -1;
        }

        ttime = ntohl(ntpRecv.txTm_s) - JAN_1970;
        cleanupSocket(sockfd);
        return 0; // Success
    } else {
        std::cerr << "Timeout waiting for response from " << server << " (" << ip << ")" << std::endl;
        cleanupSocket(sockfd);
        return -1;
    }
}

bool checkNetworkAvailability() 
{
    for (const auto& server : ntpServers) 
    {
        for (int retry = 0; retry < MAX_RETRIES; ++retry) 
        {
            time_t ttime;
            int result = Get_time_t(ttime, server.first, server.second);
            if (result == 0) 
            {
                isNetworkAvailable = true;
                return true;
            }
            auto start = std::chrono::steady_clock::now();
            auto delay = std::chrono::seconds(1);
            while (std::chrono::steady_clock::now() - start < delay) 
            {
                // 空循环，等待1秒
            }
        }
    }
    isNetworkAvailable = false;
    return false;
}

// 随机数生成器
unsigned int generateSeedFromTimestamp() 
{
    auto now = std::chrono::system_clock::now(); // 获取当前时间点
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()); // 转换为毫秒级的时间戳
    
    return static_cast<unsigned int>(timestamp.count()); // 将时间戳转换为整数种子值
}

// 根据随机数是否检测时间
bool shouldPerformCheck() 
{
    unsigned int seed = generateSeedFromTimestamp(); // 生成种子值
    
    std::mt19937 engine(seed); // 使用种子值初始化伪随机数生成器

    int random_value = engine() % 5; // 生成0到4之间的随机数

    return (random_value == 0);
}

bool TimeBomb::SysTimeCheck() 
{
    static bool networkChecked = false;
    if (!networkChecked) 
    {
        networkChecked = true;
        if (!checkNetworkAvailability()) 
        {
            std::cerr << "Network is not available. Exiting..." << std::endl;
            return false;
        }
    }

    if (isNetworkAvailable && shouldPerformCheck()) 
    {
        time_t now = time(0);
        tm ntm;
#ifdef _WIN32
        localtime_s(&ntm, &now); // Windows
#else
        localtime_r(&now, &ntm); // POSIX
#endif

        std::vector<time_t> networkTimes;
        for (const auto& server : ntpServers) 
        {
            for (int retry = 0; retry < MAX_RETRIES; ++retry) 
            {
                time_t rawtime;
                int filecode = Get_time_t(rawtime, server.first, server.second); // 获取时间戳
                if (filecode == 0) 
                {
                    networkTimes.push_back(rawtime);
                    break; // 只要有一个服务器成功即可
                } else {
                    std::cerr << "Network error with server " << server.first << " (" << server.second << ")! ErrorCode: " << filecode << std::endl;
                }
                auto start = std::chrono::steady_clock::now();
                auto delay = std::chrono::seconds(1);
                while (std::chrono::steady_clock::now() - start < delay) 
                {
                    // 空循环，等待1秒
                }
            }
            if (!networkTimes.empty()) break; // 只要有一个服务器成功即可
        }

        if (networkTimes.empty()) 
        {
            std::cerr << "Failed to get time from all servers." << std::endl;
            return false;
        }

        time_t networkTimeAvg = 0;
        for (const auto &time : networkTimes) 
        {
            networkTimeAvg += time;
        }
        networkTimeAvg /= networkTimes.size();

        tm itm;
#ifdef _WIN32
        localtime_s(&itm, &networkTimeAvg); // Windows
#else
        localtime_r(&networkTimeAvg, &itm); // POSIX
#endif

        double timeDiff = std::difftime(now, networkTimeAvg);

        if (std::abs(timeDiff) <= TIME_DIFF_THRESHOLD) 
        {
            // std::cout << "System time is correct! Time difference: " << timeDiff << " seconds" << std::endl;
            return true;
        } else {
            std::cerr << "System time is incorrect! Time difference: " << timeDiff << " seconds" << std::endl;
            std::cout << "Please check your system time." << std::endl;
            return false;
        }
    }
    return true;
}

#endif // ITM_H
