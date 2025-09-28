/* 
 * System Abstraction Layer (SAL)
 * 操作系统抽象层，实现程序可移植
 * 存放系统相关的函数和类
 * Loong Programming Language
 * Copyright (C) 2024 Lily King
 */

#ifndef SAL_H
#define SAL_H

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstring>
#include <cctype>
#include <vector>
#include <stdexcept>

/* 构建时间 */
#define BUILD_YEAR     ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 \
                                     + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))

#define BUILD_MONTH    (__DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 1 : 6) \
                                 : __DATE__ [2] == 'b' ? 2 \
                                 : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
                                 : __DATE__ [2] == 'y' ? 5 \
                                 : __DATE__ [2] == 'l' ? 7 \
                                 : __DATE__ [2] == 'g' ? 8 \
                                 : __DATE__ [2] == 'p' ? 9 \
                                 : __DATE__ [2] == 't' ? 10 \
                                 : __DATE__ [2] == 'v' ? 11 : 12)

#define BUILD_DAY      ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 \
                                 + (__DATE__ [5] - '0'))


#define BUILD_HOUR     ((__TIME__ [0] - '0') * 10 + (__TIME__ [1] - '0'))

#define BUILD_MINUTE   ((__TIME__ [3] - '0') * 10 + (__TIME__ [4] - '0'))

#define BUILD_SECOND   ((__TIME__ [6] - '0') * 10 + (__TIME__ [7] - '0'))


/* 平台 */
#ifdef _WIN32
    #define SYSTEM_NAME "Windows"
    #ifdef _WIN64
        #define _PLATFORM "WIN64"
    #else
        #define _PLATFORM "WIN32"
    #endif
#else
    #ifdef __linux__
        #define SYSTEM_NAME "Linux"
        #ifdef __x86_64__
            #define _PLATFORM "LINUX64"
        #elif __i386__
            #define _PLATFORM "LINUX32"
        #endif
    #else
        #define SYSTEM_NAME "MacOS"
        #ifdef __x86_64__
            #define _PLATFORM "MAC64"
        #elif __i386__
            #define _PLATFORM "MAC32"
        #endif
    #endif
#endif

namespace GetCh
{
    char GetCh();
}

enum Color 
{
    BLACK,           // 黑色        0
    RED,             // 红色        1
    GREEN,           // 绿色        2
    YELLOW,          // 黄色        3
    BLUE,            // 蓝色        4
    MAGENTA,         // 品红色      5
    CYAN,            // 青色        6
    WHITE,           // 白色        7
    GRAY,            // 灰色        8
    BRIGHT_RED,      // 亮红色      9
    BRIGHT_GREEN,    // 亮绿色      10
    BRIGHT_YELLOW,   // 亮黄色      11
    BRIGHT_BLUE,     // 亮蓝色      12
    BRIGHT_MAGENTA,  // 亮品红色    13
    BRIGHT_CYAN,     // 亮青色      14
    BRIGHT_WHITE     // 亮白色      15
};
/* 打印颜色文字 */
void PrintColor(const std::string& text, Color color);

std::string Get_Loong_Dir();

void clearLineAndMoveCursorToStart();

/* 获取CPU名称 */
void GetProcessorName(char* processorName, size_t size);

namespace PigLatin
{
    std::string ConvertSentenceToPigLatin(const std::string& sentence, bool firstWordUpperCase);
}

namespace SHA1 
{
    typedef unsigned char BYTE;
    typedef unsigned int WORD;
    typedef unsigned long long DWORD;
    class Sha1 
    {
        public:
        Sha1();
        void reset();
        bool update(const BYTE * input, DWORD size);
        bool update(const char * input);
        void getDigest(BYTE * output);
        void getDigestString(char * output, bool toUpperCase = false);
        private:
        WORD A, B, C, D, E;
        bool isBigEnd;
        bool isFinish;
        BYTE data[64];
        DWORD data_size;
        DWORD total_size;
        bool computerOneBlock();
        bool padingDataBlock();
    };
}

#ifdef _WIN32
namespace MultiEncoding
{
    //UTF-8转Unicode 
    std::wstring Utf82Unicode(const std::string& utf8string);

    //unicode 转为 ascii 
    std::string WideByte2Acsi(std::wstring& wstrcode);

    //utf-8 转 ascii 
    std::string UTF_82ASCII(std::string& strUtf8Code);

    //ascii 转 Unicode 
    std::wstring Acsi2WideByte(std::string& strascii);

    //Unicode 转 Utf8 
    std::string Unicode2Utf8(const std::wstring& widestring);

    //ascii 转 Utf8 
    std::string ASCII2UTF_8(std::string& strAsciiCode);
}
#endif

#endif /* SAL_H */
