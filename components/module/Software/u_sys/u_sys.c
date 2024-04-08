/*******************************************
# Author        : Retuze 
# Date          : 2023-11-09 19:44:13
# LastEditors   : Retuze 
# LastEditTime  : 2024-01-11 03:48:54
# Description   : 
*********************************************/
#include "u_sys.h"

/// @brief 该函数为printf底层发送函数，通过重写该函数完成printf重定向
/// @param fd
/// @param pBuffer
/// @param size
/// @return
__attribute__((weak)) int _write(int fd, char *pBuffer, int size)
{
    return 0;
}

__attribute__((weak)) int _isatty(int fd)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
        return 1;

    errno = EBADF;
    return 0;
}

__attribute__((weak)) int _close(int fd)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
        return 0;

    errno = EBADF;
    return -1;
}

__attribute__((weak)) int _lseek(int fd, int ptr, int dir)
{
    (void)fd;
    (void)ptr;
    (void)dir;

    errno = EBADF;
    return -1;
}

__attribute__((weak)) int _fstat(int fd, struct stat *st)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
    {
        st->st_mode = S_IFCHR;
        return 0;
    }

    errno = EBADF;
    return 0;
}

__attribute__((weak)) int _getpid(void)
{
    return 0;
}

__attribute__((weak)) int _kill(int pid, int sig)
{
    return 0;
}

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
    return len;
}