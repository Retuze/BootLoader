#ifndef _PRINTK_H_
#define _PRINTK_H_

#include "stdarg.h"
#include <sys/_stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

// 使用printk前需要设定好dev_out;
void printk(const char* fmt, ...);
void vprintk(const char* fmt, va_list args);
void set_output(void (*fun)(const char* strbuf, uint16_t len));
extern void (*output)(const char* strbuf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif // _PRINTK_H_