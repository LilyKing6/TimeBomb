/* 
 * TimeBomb
 * Copyright (C) 2023-2025 Lily King. All Rights Reserved.
 */


#include "utils.h"

#ifdef _WIN32
    #include <windows.h>
#endif

#ifdef __linux__
    #include <limits.h>
    #include <unistd.h>
#endif

/* 打印颜色文字 */
// Windows系统，需要在命令行下运行程序，否则颜色设置无效
void PrintColor(const std::string& text, Color color) 
{
#ifdef _WIN32
    // Windows系统
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD originalColorAttrs = 0;
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    // 获取当前的控制台颜色属性
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        originalColorAttrs = csbi.wAttributes;
    }

    WORD colorCode;
    switch (color) {
        case BLACK:         colorCode = 0; break;
        case RED:           colorCode = FOREGROUND_RED; break;
        case GREEN:         colorCode = FOREGROUND_GREEN; break;
        case YELLOW:        colorCode = FOREGROUND_RED | FOREGROUND_GREEN; break;
        case BLUE:          colorCode = FOREGROUND_BLUE; break;
        case MAGENTA:       colorCode = FOREGROUND_RED | FOREGROUND_BLUE; break;
        case CYAN:          colorCode = FOREGROUND_GREEN | FOREGROUND_BLUE; break;
        case WHITE:         colorCode = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
        case GRAY:          colorCode = FOREGROUND_INTENSITY; break;
        case BRIGHT_RED:    colorCode = FOREGROUND_INTENSITY | FOREGROUND_RED; break;
        case BRIGHT_GREEN:  colorCode = FOREGROUND_INTENSITY | FOREGROUND_GREEN; break;
        case BRIGHT_YELLOW: colorCode = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN; break;
        case BRIGHT_BLUE:   colorCode = FOREGROUND_INTENSITY | FOREGROUND_BLUE; break;
        case BRIGHT_MAGENTA:colorCode = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE; break;
        case BRIGHT_CYAN:   colorCode = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
        case BRIGHT_WHITE:  colorCode = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
        default:            colorCode = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
    }
    SetConsoleTextAttribute(hConsole, colorCode);
    std::cout << text;
    // 重置颜色
    SetConsoleTextAttribute(hConsole, originalColorAttrs);
#else
    // 非Windows系统
    int colorCode;
    switch (color) {
        case BLACK:         colorCode = 30; break;
        case RED:           colorCode = 31; break;
        case GREEN:         colorCode = 32; break;
        case YELLOW:        colorCode = 33; break;
        case BLUE:          colorCode = 34; break;
        case MAGENTA:       colorCode = 35; break;
        case CYAN:          colorCode = 36; break;
        case WHITE:         colorCode = 37; break;
        case GRAY:          colorCode = 90; break;
        case BRIGHT_RED:    colorCode = 91; break;
        case BRIGHT_GREEN:  colorCode = 92; break;
        case BRIGHT_YELLOW: colorCode = 93; break;
        case BRIGHT_BLUE:   colorCode = 94; break;
        case BRIGHT_MAGENTA:colorCode = 95; break;
        case BRIGHT_CYAN:   colorCode = 96; break;
        case BRIGHT_WHITE:  colorCode = 97; break;
        default:            colorCode = 37; break;
    }
    std::cout << "\033[" << colorCode << "m" << text << "\033[0m"; // 重置颜色
#endif
}

// 获取解释器目录
std::string Get_Loong_Dir() 
{
    // 获取解释器目录
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    std::wstring wbuffer(buffer);
    std::wstring::size_type pos = wbuffer.find_last_of(L"\\/");
	std::string dir = std::string(wbuffer.begin(), wbuffer.begin() + pos + 1);
	// printf("Get_Loong_Dir: %s\n", dir.c_str());
    return dir;
    // return std::string(wbuffer.begin(), wbuffer.begin() + pos + 1);
#else
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        std::string::size_type pos = std::string(buffer).find_last_of("/");
        return std::string(buffer).substr(0, pos + 1);
    }
    // 错误处理，例如抛出异常或返回错误码
    // throw std::runtime_error("Failed to get interpreter directory");
	return "";
#endif
}

/* 获取处理器名称 */
#if defined(_MSC_VER)
    // For MSVC (Microsoft Visual C++)
    #include <intrin.h>

    void GetCPUInfo(unsigned int CPUInfo[4], unsigned int InfoType) {
        __cpuidex(reinterpret_cast<int*>(CPUInfo), InfoType, 0);
    }

#elif defined(__GNUC__) || defined(__clang__)
    // For GCC and Clang
    void GetCPUInfo(unsigned int CPUInfo[4], unsigned int InfoType) {
        __asm__ __volatile__ (
            "cpuid"
            : "=a" (CPUInfo[0]), "=b" (CPUInfo[1]), "=c" (CPUInfo[2]), "=d" (CPUInfo[3])
            : "a" (InfoType)
        );
    }
#else
    #error "Unsupported compiler"
#endif
void GetProcessorName(char* processorName, size_t size) 
{
    unsigned int CPUInfo[4] = {0};

    if (size < 49) 
    {
        // 处理数组太小的情况
        return;
    }

    // Get Processor Name
    GetCPUInfo(CPUInfo, 0x80000002);
    memcpy(processorName, CPUInfo, 16);

    GetCPUInfo(CPUInfo, 0x80000003);
    memcpy(processorName + 16, CPUInfo, 16);

    GetCPUInfo(CPUInfo, 0x80000004);
    memcpy(processorName + 32, CPUInfo, 16);
}

namespace PigLatin
{
    // 判断字母是否为元音
    bool isVowel(char c) 
    {
        char lowerC = std::tolower(c);
        return lowerC == 'a' || lowerC == 'e' || lowerC == 'i' || lowerC == 'o' || lowerC == 'u';
    }

    // 检查单词是否只包含字母
    bool isAlphabetic(const std::string& word) 
    {
        for (char c : word) {
            if (!std::isalpha(c)) {
                return false; // 如果有任何非字母字符，则返回false
            }
        }
        return true;
    }

    // 将单词的第一个字母转换为相应的大小写
    std::string applyOriginalCase(const std::string& pigLatinWord, const std::string& originalWord, bool isFirstWord, bool firstWordUpperCase) 
    {
        // 如果是句子的第一个单词，检查是否需要大写处理
        if (isFirstWord && firstWordUpperCase) {
            std::string result = pigLatinWord;
            result[0] = std::toupper(result[0]); // 首字母大写
            return result;
        }
        
        // 原样模式，保持单词大小写不变
        return pigLatinWord;
    }

    // 将单词转换为 Pig Latin 并保留标点符号
    std::string toPigLatin(const std::string& word, bool isFirstWord, bool firstWordUpperCase) 
    {
        if (word.empty()) {
            return word;
        }

        std::string coreWord;
        std::string suffix;

        // 分离标点符号
        size_t i = 0;
        while (i < word.size() && std::isalnum(word[i])) {
            coreWord += word[i++];
        }
        suffix = word.substr(i);

        // 如果 coreWord 是空的，直接返回原始单词
        if (coreWord.empty()) {
            return word;
        }

        // 转换为 Pig Latin
        std::string pigLatinWord;
        if (isVowel(coreWord[0])) {
            pigLatinWord = coreWord + "ay";
        } else {
            if (coreWord.size() > 1) {
                pigLatinWord = coreWord.substr(1) + coreWord[0] + "ay";
            } else {
                pigLatinWord = coreWord + "ay";  // 处理单字母单词
            }
        }

        // 保持或转换大小写格式
        pigLatinWord = applyOriginalCase(pigLatinWord, coreWord, isFirstWord, firstWordUpperCase);

        return pigLatinWord + suffix;
    }

    // 处理带连字符的单词
    std::string toPigLatinWithHyphen(const std::string& word, bool isFirstWord, bool firstWordUpperCase) {
        std::string result;
        std::istringstream iss(word);
        std::string part;
        bool firstPart = true;
        
        while (std::getline(iss, part, '-')) {
            result += toPigLatin(part, isFirstWord && firstPart, firstWordUpperCase) + "-";
            firstPart = false;
        }
        result.pop_back(); // 去掉多余的 "-"
        
        return result;
    }

    // 将句子中的每个单词转换为 Pig Latin
    /* 
    是否要让句子中的第一个单词首字母大写
    如果为 true，则句子的第一个单词首字母会大写
    如果为 false，则不改变大小写格式
    */
    std::string ConvertSentenceToPigLatin(const std::string& sentence, bool firstWordUpperCase) 
    {
        std::istringstream iss(sentence);
        std::ostringstream oss;
        std::string word;
        bool isFirstWord = true;

        while (iss >> word) {
            // 只有完全由字母组成的单词才进行 Pig Latin 转换
            if (isAlphabetic(word)) {
                // 如果单词中有连字符，则使用带连字符处理的函数
                if (word.find('-') != std::string::npos) {
                    oss << toPigLatinWithHyphen(word, isFirstWord, firstWordUpperCase) << " ";
                } else {
                    oss << toPigLatin(word, isFirstWord, firstWordUpperCase) << " ";
                }
            } else {
                // 如果单词包含非字母字符，直接保留原样
                oss << word << " ";
            }

            isFirstWord = false; // 处理完第一个单词后设置为 false
        }

        std::string result = oss.str();
        if (!result.empty()) {
            result.pop_back();  // 去掉末尾多余的空格
        }

        return result;
    }

}

namespace SHA1
{
    // 机器大小端模式是个问题，主要是位移运算的不一致性
    bool isBigEndian(){
        union{
            int a;  
            char b;  
        }num;  
        num.a = 0x1234;  
        if( num.b == 0x12){  
            return true;  
        }
        return false;  
    }

    WORD bigMode(WORD value) {
        return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 | (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24; 
    }

    DWORD bigMode(DWORD value) {
        DWORD high_uint64 = (DWORD)bigMode((WORD)value);
        DWORD low_uint64 = (DWORD)bigMode((WORD)(value >> 32));
        return (high_uint64 << 32) + low_uint64;
    }

    Sha1::Sha1() {
        this->isBigEnd = isBigEndian();
        this->reset();
    }

    void Sha1::reset() {
        this->A = isBigEnd ? 0x67452301 : bigMode((WORD)0x67452301);
        this->B = isBigEnd ? 0xEFCDAB89 : bigMode((WORD)0xEFCDAB89);
        this->C = isBigEnd ? 0x98BADCFE : bigMode((WORD)0x98BADCFE);
        this->D = isBigEnd ? 0x10325476 : bigMode((WORD)0x10325476);
        this->E = isBigEnd ? 0xC3D2E1F0 : bigMode((WORD)0xC3D2E1F0);

        this->isFinish = false;
        memset(data, 0, 64);
        data_size = 0;
        total_size = 0;
    }

    bool Sha1::computerOneBlock() {
        WORD k[4] = {0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6};
        if (!isBigEnd) {
            for (int i = 0; i < 4; i++) k[i] = bigMode(k[i]);
        }
        
        WORD sub_group[80] = {0};
        WORD * temp_sub_group = (WORD*)(data);  // 定位子组 0-15 的数据
        
        // 生成子组
        for (int j = 0; j < 80; j++) {
            if (j < 16) sub_group[j] = temp_sub_group[j];
            else {
                WORD temp = sub_group[j - 3] ^ sub_group[j - 8] ^ sub_group[j - 14] ^ sub_group[j - 16];
                temp = isBigEnd ? temp : bigMode(temp);
                sub_group[j] = isBigEnd ? temp << 1 | temp >> 31 : bigMode(temp << 1 | temp >> 31);
            }
        }

        WORD a = A, b = B, c = C, d = D, e = E;

        for (int j = 0; j < 80; j++) {
            WORD temp, temp2 = isBigEnd ? a << 5 | a >> 27 : bigMode(bigMode(a) << 5 | bigMode(a) >> 27);
            switch (j / 20)
            {
            case 0:
                temp = (b & c) | ((~b) & d);
                temp = isBigEnd ? k[0] + temp : bigMode(bigMode(k[0]) + bigMode(temp));
                break;
            case 1:
                temp = (b ^ c ^ d);
                temp = isBigEnd ? k[1] + temp : bigMode(bigMode(k[1]) + bigMode(temp));
                break;
            case 2:
                temp = (b & c) | (b & d) | (c & d);
                temp = isBigEnd ? k[2] + temp : bigMode(bigMode(k[2]) + bigMode(temp));
                break;
            case 3:
                temp = (b ^ c ^ d);
                temp = isBigEnd ? k[3] + temp : bigMode(bigMode(k[3]) + bigMode(temp));
                break;
            }
            temp = isBigEnd ? temp + temp2 + e + sub_group[j] : bigMode(bigMode(temp) + bigMode(temp2) + bigMode(e) + bigMode(sub_group[j]));
            e = d;
            d = c;
            c = isBigEnd ? b << 30 | b >> 2 : bigMode(bigMode(b) << 30 | bigMode(b) >> 2);
            b = a;
            a = temp;
        }
        A = isBigEnd? A + a  : bigMode(bigMode(a) + bigMode(A));
        B = isBigEnd? B + b  : bigMode(bigMode(b) + bigMode(B));
        C = isBigEnd? C + c  : bigMode(bigMode(c) + bigMode(C));
        D = isBigEnd? D + d  : bigMode(bigMode(d) + bigMode(D));
        E = isBigEnd? E + e  : bigMode(bigMode(e) + bigMode(E));

        return true;
    }

    bool Sha1::update(const BYTE * input, DWORD size) {
        if (isFinish) return false;
        DWORD index = 0;
        while (data_size + size > 64) {
            memcpy(data + data_size, input + index, 64 - data_size);
            computerOneBlock();
            size -= 64 - data_size;
            index += 64 - data_size;
            total_size += 64 - data_size;
            data_size = 0;
        }
        memcpy(data + data_size, input + index, size);
        data_size += size;
        total_size += size;
        if (data_size == 64) {
            computerOneBlock();
            data_size = 0;
        }
        return true;
    }

    bool Sha1::update(const char * input) {
        return update((const BYTE *)input, strlen(input));
    }

    void Sha1::getDigest(BYTE * output) {
        if (!isFinish) {
            padingDataBlock();
            computerOneBlock();
        }

        *(WORD *)output = A;
        *(WORD *)(output + 4) = B;
        *(WORD *)(output + 8) = C;
        *(WORD *)(output + 12) = D;
        *(WORD *)(output + 16) = E;
    }

    void Sha1::getDigestString(char * output, bool toUpperCase) {
        BYTE origin_output[20];
        getDigest(origin_output);

        size_t index = 0;
        for (BYTE c : origin_output) {
            char high = c / 16, low = c % 16;
            output[index] = high < 10 ? '0' + high : (toUpperCase ? 'A' : 'a' + high - 10);
            output[index + 1] = low < 10 ? '0' + low : (toUpperCase ? 'A' : 'a' + low - 10);
            index += 2;
        }
    }

    bool Sha1::padingDataBlock() {
        if (isFinish) return false;
        if (data_size != 56) {
            DWORD pad_size = (64 + 56 - data_size) % 64;
            if (data_size + pad_size < 64) {
                data[data_size] = 0x80;
                memset(data + data_size + 1, 0x0, pad_size - 1);
            } else {
                data[data_size] = 0x80;
                memset(data + data_size + 1, 0x0, 64 - data_size - 1);
                computerOneBlock();
                memset(data, 0x0, 56);
            }
        }

        *(DWORD*)(data + 56) = isBigEnd ? total_size * 8 : bigMode(total_size * 8);
        isFinish = true;
        return true;
    }

}

#ifdef _WIN32
namespace MultiEncoding
{
    //UTF-8转Unicode 
    std::wstring Utf82Unicode(const std::string& utf8string) 
    {
        int widesize = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, NULL, 0);
        if (widesize == ERROR_NO_UNICODE_TRANSLATION)
        {
            throw std::logic_error("Invalid UTF-8 sequence.");
        }
        if (widesize == 0)
        {
            throw std::logic_error("Error in conversion.");
        }
        std::vector<wchar_t> resultstring(widesize);
        int convresult = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, &resultstring[0], widesize);
        if (convresult != widesize)
        {
            throw std::logic_error("La falla!");
        }
        return std::wstring(&resultstring[0]);
    }

    //unicode 转为 ascii 
    std::string WideByte2Acsi(std::wstring& wstrcode)
    {
        int asciisize = ::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, NULL, 0, NULL, NULL);
        if (asciisize == ERROR_NO_UNICODE_TRANSLATION)
        {
            throw std::logic_error("Invalid UTF-8 sequence.");
        }
        if (asciisize == 0)
        {
            throw std::logic_error("Error in conversion.");
        }
        std::vector<char> resultstring(asciisize);
        int convresult = ::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, &resultstring[0], asciisize, NULL, NULL);
        if (convresult != asciisize)
        {
            throw std::logic_error("La falla!");
        }
        return std::string(&resultstring[0]);
    }

    //utf-8 转 ascii 
    std::string UTF_82ASCII(std::string& strUtf8Code)
    {
        using namespace std;
        string strRet = "";
        //先把 utf8 转为 unicode 
        wstring wstr = Utf82Unicode(strUtf8Code);
        //最后把 unicode 转为 ascii 
        strRet = WideByte2Acsi(wstr);
        return strRet;
    }

    //ascii 转 Unicode 
    std::wstring Acsi2WideByte(std::string& strascii)
    {
        using namespace std;
        int widesize = MultiByteToWideChar(CP_ACP, 0, (char*)strascii.c_str(), -1, NULL, 0);
        if (widesize == ERROR_NO_UNICODE_TRANSLATION)
        {
            throw std::logic_error("Invalid UTF-8 sequence.");
        }
        if (widesize == 0)
        {
            throw std::logic_error("Error in conversion.");
        }
        std::vector<wchar_t> resultstring(widesize);
        int convresult = MultiByteToWideChar(CP_ACP, 0, (char*)strascii.c_str(), -1, &resultstring[0], widesize);
        if (convresult != widesize)
        {
            throw std::logic_error("La falla!");
        }
        return std::wstring(&resultstring[0]);
    }

    //Unicode 转 Utf8 
    std::string Unicode2Utf8(const std::wstring& widestring)
    {
        using namespace std;
        int utf8size = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, NULL, 0, NULL, NULL);
        if (utf8size == 0)
        {
            throw std::logic_error("Error in conversion.");
        }
        std::vector<char> resultstring(utf8size);
        int convresult = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, &resultstring[0], utf8size, NULL, NULL);
        if (convresult != utf8size)
        {
            throw std::logic_error("La falla!");
        }
        return std::string(&resultstring[0]);
    }

    //ascii 转 Utf8 
    std::string ASCII2UTF_8(std::string& strAsciiCode) 
    {
        using namespace std;
        string strRet("");
        //先把 ascii 转为 unicode 
        wstring wstr = Acsi2WideByte(strAsciiCode);
        //最后把 unicode 转为 utf8 
        strRet = Unicode2Utf8(wstr);
        return strRet;
    }
}
#endif
