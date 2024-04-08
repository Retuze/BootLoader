#ifndef _LOG_H_
#define _LOG_H_

#include "partfrom.h"
#include "printk.h"

class LOG_LOG {
    
    enum {
        LOG_DEBUG,
        LOG_INFO,
        LOG_WARN,
        LOG_ERROR,
    };
    const char* level_colors[4] = {
        "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m"
    };
    const char* level_strings[4] = {
        "DEBUG", "INFO", "WARN", "ERROR"
    };
    int level;
    bool quiet;
    void (*lock)(bool);

public:
    void set_level(int level) { this->level = level; }
    void set_quiet(bool quiet) { this->quiet = quiet; }
    void set_lock(void (*lock)(bool)) { this->lock = lock; }
    void log(int level, const char* file, int line, const char* func, const char* fmt, va_list args)
    {
        if (lock) {
            lock(true);
        }
        if (!quiet && level >= this->level) {
            printk("%s%-5s\x1b[0m \x1b[90m%s:%d [%s]:\x1b[0m ", level_colors[level], level_strings[level], file, line, func);
            vprintk(fmt, args);
        }
        if (lock) {
            lock(false);
        }
    }
    void warn(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        log(LOG_WARN, __BASE_FILE__, __LINE__, __func__, fmt, args);
        va_end(args);
    }
    void info(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        log(LOG_INFO, __BASE_FILE__, __LINE__, __func__, fmt, args);
        va_end(args);
    }
    void debug(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        log(LOG_DEBUG, __BASE_FILE__, __LINE__, __func__, fmt, args);
        va_end(args);
    }
    void error(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        log(LOG_ERROR, __BASE_FILE__, __LINE__, __func__, fmt, args);
        va_end(args);
    }
};

#endif // _LOG_H_