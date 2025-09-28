/* 
 * TimeBomb 测试程序
 * Copyright (C) 2024 Lily King
 */

#include "TimeBomb.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

// 测试正常情况
void testNormalCase() {
    cout << "=== 测试正常情况 ===" << endl;
    TimeBomb tmb(1); // 启用回显
    tmb.Run();
    cout << "测试正常情况完成" << endl;
    cout << "------------------------" << endl;
}

// 测试命令行参数情况
void testCommandLineCase() {
    cout << "=== 测试命令行参数情况 ===" << endl;
    // 模拟命令行参数
    int argc = 2;
    char* argv[] = {(char*)"test_timebomb", (char*)"--silent"};
    
    TimeBomb tmb(1);
    if (argc > 1) {
        tmb.isEcho = 0;
        tmb.Run();
        cout << "命令行参数模式: 静默输出" << endl;
    }
    cout << "测试命令行参数情况完成" << endl;
    cout << "------------------------" << endl;
}

// 测试过期情况（模拟）
void testExpiredCase() {
    cout << "=== 测试过期情况 ===" << endl;
    cout << "注意: 此测试会模拟时间过期并强制退出程序" << endl;
    cout << "5秒后开始测试..." << endl;
    this_thread::sleep_for(chrono::seconds(5));
    
    // 这里我们无法直接修改系统时间，
    // 但可以通过修改TimeBomb类的BUILD_*宏来模拟
    // 或者在实际测试时调整系统时间
    
    TimeBomb tmb(1);
    tmb.Run();
    // 如果程序没有退出，说明测试失败
    cout << "测试过期情况失败: 程序未退出" << endl;
    cout << "------------------------" << endl;
}

int main() {
    cout << "TimeBomb 测试程序启动" << endl;
    cout << "======================" << endl;
    
    // 测试正常情况
    testNormalCase();
    
    // 等待2秒
    this_thread::sleep_for(chrono::seconds(2));
    
    // 测试命令行参数情况
    testCommandLineCase();
    
    // 等待2秒
    this_thread::sleep_for(chrono::seconds(2));
    
    // 测试过期情况 (可选，会导致程序退出)
    // 取消注释下面一行来执行过期测试
    // testExpiredCase();
    
    cout << "所有测试完成" << endl;
    return 0;
}