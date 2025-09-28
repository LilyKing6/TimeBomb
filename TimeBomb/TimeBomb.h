#pragma once

#include <mutex>
#include <atomic>

#ifdef _TIMEBOMB_OFF_
#define TIMEBOMB 0
#else
#define TIMEBOMB 1
#endif

// 是否联网校验系统时间(全局开关)
#define CHKSYSTEMTIME 0

// 配置参数
#define DAYS 3
#define HOURS 0
#define MINUTES 0
#define CHECK_INTERVAL_SECONDS 60  // 检查间隔(秒)
#define MAX_TIME_DIFF_SECONDS 300  // 最大允许时间差(秒)

// 错误码定义
enum class TimeBombError {
    SUCCESS = 0,
    FILE_ERROR = 1,
    NETWORK_ERROR = 2,
    TIME_ERROR = 3,
    THREAD_ERROR = 4
};

class TimeBomb
{
private:
    std::mutex m_mutex;              // 互斥锁，保护共享资源
    std::atomic<bool> m_isRunning;   // 原子变量，指示监控线程是否运行
    std::atomic<bool> m_isExpired;   // 原子变量，指示是否已过期

public:
    TimeBomb();
    TimeBomb(int echo) : isEcho(echo){};
    ~TimeBomb();

    int EchoExpireTime();
    bool SysTimeCheck();
	bool IsExpire();
	void Run();
    void monitoringThread(); // 时间监控线程函数
    void startMonitoring(); // 启动时间监控线程
    
    /* 用于输出炸弹信息显示 */
    int isEcho;         // 是否输出炸弹信息 0
    int hideexpireTime; // 如果过期 是否隐藏输出过期时间 1

    /* 用于开关是否检查系统时间 */
    int checkSystemTime;// 是否检查系统时间 1

private:
    TimeBombError removeDataFile(); // 删除数据文件
    void logError(TimeBombError error, const std::string& message); // 记录错误日志
};

