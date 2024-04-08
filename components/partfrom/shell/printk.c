#include "printk.h"
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define ZEROPAD 1 /* pad with zero */
#define SIGN 2 /* unsigned/signed long */
#define PLUS 4 /* show plus */
#define SPACE 8 /* space if plus */
#define LEFT 16 /* left justified */
#define SMALL 32 /* Must be 32 == 0x20 */
#define SPECIAL 64 /* 0x */

void (*output)(const char* strbuf, uint16_t len);

void set_output(void (*fun)(const char*, uint16_t))
{
    output = fun;
}

static char* number(char* str, long num, int base, int size, int precision,
    int type)
{
    /* we are called with base 8, 10 or 16, only, thus don't need "G..." */
    static const char digits[16] = "0123456789ABCDEF"; /* "GHIJKLMNOPQRSTUVWXYZ"; */
    unsigned long unum;
    char tmp[66];
    char sign, locase, padding;
    int chgsize;

    /* locase = 0 or 0x20. ORing digits or letters with 'locase'
     * produces same digits or (maybe lowercased) letters */
    locase = (type & SMALL);
    if (type & LEFT)
        type &= ~ZEROPAD;

    padding = (type & ZEROPAD) ? '0' : ' ';
    sign = 0;
    if (type & SIGN) {
        if (num < 0) {
            sign = '-';
            num = -num;
            size--;
        } else if (type & PLUS) {
            sign = '+';
            size--;
        } else if (type & SPACE) {
            sign = ' ';
            size--;
        }
    }

    if (type & SPECIAL) {
        if (base == 16)
            size -= 2;
        else if (base == 8)
            size--;
    }

    chgsize = 0;
    unum = (unsigned)num;
    do {
        tmp[chgsize] = (digits[unum % base] | locase);
        ++chgsize;
        unum /= base;
    } while (unum);

    if (precision < chgsize)
        precision = chgsize;

    size -= precision;
    if (!(type & (ZEROPAD + LEFT))) {
        for (; size > 0; --size)
            *str++ = ' ';
    }

    if (sign)
        *str++ = sign;

    if (type & SPECIAL) {
        if (base == 8)
            *str++ = '0';
        else if (base == 16) {
            *str++ = '0';
            *str++ = ('X' | locase);
        }
    }

    if (!(type & LEFT)) {
        for (; size > 0; --size)
            *str++ = padding;
    }

    for (; chgsize < precision; --precision)
        *str++ = '0';

    for (; chgsize > 0; *str++ = tmp[--chgsize])
        ;

    for (; size > 0; --size)
        *str++ = ' ';

    return str;
}

static char* float2string(char* str, float num, int size, int precision, int type)
{
    char tmp[66];
    char sign, padding;
    int chgsize;
    unsigned int ipart;

    if (type & LEFT)
        type &= ~ZEROPAD;

    padding = (type & ZEROPAD) ? '0' : ' ';

    if (precision < 0 || precision > 6) // 精度，此处精度限制为最多 6 位小数
        precision = 6;

    if (num < 0.0f) { // 如果是负数，则先转换为正数，并占用一个字节存放负号
        sign = '-';
        num = -num;
        size--;
    } else
        sign = 0;

    chgsize = 0;
    ipart = (unsigned int)num; // 整数部分

    if (precision) { // 如果有小数转换，则提取小数部分
        static const float mulf[7] = {
            1.0f, 10.0f, 100.0f, 1000.0f, 10000.0f, 100000.0f, 1000000.0f
        };
        unsigned int fpart = (unsigned int)((num - (float)ipart) * mulf[precision]);

        for (int i = 0; i < precision; ++i) {
            tmp[chgsize++] = (char)(fpart % 10 + '0');
            fpart /= 10;
        }
        tmp[chgsize++] = '.';
    }

    do {
        tmp[chgsize++] = (char)(ipart % 10 + '0');
        ipart /= 10;
    } while (ipart);

    size -= chgsize; // 剩余需要填充的大小

    if (!(type & LEFT)) { // 右对齐
        if ('0' == padding && sign) { // 如果是填充 0 且为负数，先放置负号
            *str++ = sign;
            sign = 0;
        }
        for (; size > 0; --size) // 填充 0
            *str++ = padding;
    }

    if (sign)
        *str++ = sign;

    for (; chgsize > 0; *str++ = tmp[--chgsize])
        ;

    for (; size > 0; --size) // 左对齐时，填充右边的空格
        *str++ = ' ';

    return str;
}
void vprintk(const char* fmt, va_list args)
{
    if (!output) // 无硬件输出，返回
        return;

    char tmp[88]; // 此段内存仅用于缓存数字转换成的字符串
    char* substr;
    unsigned long num;
    int len, base;
    int flags; /* flags to number() */
    int field_width; /* width of output field */
    int precision; /* min. # of digits for integers; max
                  number of chars for from string */
    int qualifier; /* 'h', 'l', or 'L' for integer fields */

    char* fmthead = (char*)fmt;
    char* fmtout = fmthead;

    for (; *fmtout; ++fmtout) {
        if (*fmtout == '%') {
            char* str = tmp;

            if (fmthead != fmtout) { // 先把 % 前面的部分输出
                output(fmthead, fmtout - fmthead);
                fmthead = fmtout;
            }

            /* process flags */
            flags = 0;
            base = 0;
            do {
                ++fmtout; /* this also skips first '%' */
                switch (*fmtout) {
                case '-':
                    flags |= LEFT;
                    break;
                case '+':
                    flags |= PLUS;
                    break;
                case ' ':
                    flags |= SPACE;
                    break;
                case '#':
                    flags |= SPECIAL;
                    break;
                case '0':
                    flags |= ZEROPAD;
                    break;
                default:
                    base = 1;
                }
            } while (!base);

            /* get field width */
            if (isdigit((int)*fmtout)) {
                field_width = 0;
                do {
                    field_width = field_width * 10 + *fmtout - '0';
                    ++fmtout;
                } while (isdigit((int)*fmtout));
                if (field_width > sizeof(tmp))
                    field_width = sizeof(tmp);
            } else
                field_width = -1;

            /* get the precision */
            if (*fmtout == '.') {
                precision = 0;
                for (++fmtout; isdigit((int)*fmtout); ++fmtout)
                    precision = precision * 10 + *fmtout - '0';
                if (precision > sizeof(tmp))
                    precision = sizeof(tmp);
            } else
                precision = -1;

            /* get the conversion qualifier *fmt == 'h' ||  || *fmt == 'L'*/
            if (*fmtout == 'l') {
                qualifier = *fmtout;
                ++fmtout;
            } else
                qualifier = -1;

            /* default base */
            base = 10;

            switch (*fmtout) {
            case 'c':
                if (!(flags & LEFT))
                    for (; --field_width > 0; *str++ = ' ')
                        ; // 右对齐，补全左边的空格
                *str++ = (char)va_arg(args, int);
                for (; --field_width > 0; *str++ = ' ')
                    ; // 左对齐，补全右边的空格
                output(tmp, str - tmp);
                fmthead = fmtout + 1;
                continue;

            case 's':
                substr = va_arg(args, char*);
                if (!substr)
                    substr = "(NULL)";
                str = substr;
                if (precision > 0)
                    while (*str++ && --precision)
                        ;
                else
                    while (*str++)
                        ;
                len = str - substr; // 其实就是为了实现 strnlen ，此处不希望再进行函数压栈
                str = tmp;
                if ((!(flags & LEFT)) && (len < field_width)) { // 右对齐且需要补全空格
                    do {
                        *str++ = ' ';
                    } while (len < --field_width); // 填充空格串
                    output(tmp, str - tmp);
                }
                output(substr, len); // 输出子字符串
                if (len < field_width) { // 左对齐且需要补全右边空格
                    do {
                        *str++ = ' ';
                    } while (len < --field_width);
                    output(tmp, str - tmp);
                }
                fmthead = fmtout + 1;
                continue;

            case 'p':
                if (field_width == -1) {
                    field_width = 2 * sizeof(void*);
                    flags |= ZEROPAD;
                }
                str = number(tmp,
                    (size_t)va_arg(args, void*), 16,
                    field_width, precision, flags);
                output(tmp, str - tmp);
                fmthead = fmtout + 1;
                continue;

            case 'f':
                str = float2string(tmp, va_arg(args, double), field_width, precision, flags);
                output(tmp, str - tmp);
                fmthead = fmtout + 1;
                continue;

            case '%':
                *str++ = '%';
                output(tmp, str - tmp);
                fmthead = fmtout + 1;
                continue;

            /* integer number formats - set up the flags and "break" */
            case 'o':
                base = 8;
                break;

            case 'x':
                flags |= SMALL;
            case 'X':
                base = 16;
                break;

            case 'd':
            case 'i':
                flags |= SIGN;
            case 'u':
                break;

            default:
                continue;
            } // switch()

            if (qualifier == 'l')
                num = va_arg(args, unsigned long);
            else
                num = va_arg(args, int);

            str = number(tmp, num, base, field_width, precision, flags);
            output(tmp, str - tmp);
            fmthead = fmtout + 1;
        } // if (*fmtout == '%')
    }

    if (fmthead != fmtout)
        output(fmthead, fmtout - fmthead);
}
void printk(const char* fmt, ...)
{
    if (!output) // 无硬件输出，返回
        return;

    char tmp[88]; // 此段内存仅用于缓存数字转换成的字符串
    char* substr;
    unsigned long num;
    int len, base;
    int flags; /* flags to number() */
    int field_width; /* width of output field */
    int precision; /* min. # of digits for integers; max
                  number of chars for from string */
    int qualifier; /* 'h', 'l', or 'L' for integer fields */

    char* fmthead = (char*)fmt;
    char* fmtout = fmthead;

    va_list args;
    va_start(args, fmt);

    for (; *fmtout; ++fmtout) {
        if (*fmtout == '%') {
            char* str = tmp;

            if (fmthead != fmtout) { // 先把 % 前面的部分输出
                output(fmthead, fmtout - fmthead);
                fmthead = fmtout;
            }

            /* process flags */
            flags = 0;
            base = 0;
            do {
                ++fmtout; /* this also skips first '%' */
                switch (*fmtout) {
                case '-':
                    flags |= LEFT;
                    break;
                case '+':
                    flags |= PLUS;
                    break;
                case ' ':
                    flags |= SPACE;
                    break;
                case '#':
                    flags |= SPECIAL;
                    break;
                case '0':
                    flags |= ZEROPAD;
                    break;
                default:
                    base = 1;
                }
            } while (!base);

            /* get field width */
            if (isdigit((int)*fmtout)) {
                field_width = 0;
                do {
                    field_width = field_width * 10 + *fmtout - '0';
                    ++fmtout;
                } while (isdigit((int)*fmtout));
                if (field_width > sizeof(tmp))
                    field_width = sizeof(tmp);
            } else
                field_width = -1;

            /* get the precision */
            if (*fmtout == '.') {
                precision = 0;
                for (++fmtout; isdigit((int)*fmtout); ++fmtout)
                    precision = precision * 10 + *fmtout - '0';
                if (precision > sizeof(tmp))
                    precision = sizeof(tmp);
            } else
                precision = -1;

            /* get the conversion qualifier *fmt == 'h' ||  || *fmt == 'L'*/
            if (*fmtout == 'l') {
                qualifier = *fmtout;
                ++fmtout;
            } else
                qualifier = -1;

            /* default base */
            base = 10;

            switch (*fmtout) {
            case 'c':
                if (!(flags & LEFT))
                    for (; --field_width > 0; *str++ = ' ')
                        ; // 右对齐，补全左边的空格
                *str++ = (char)va_arg(args, int);
                for (; --field_width > 0; *str++ = ' ')
                    ; // 左对齐，补全右边的空格
                output(tmp, str - tmp);
                fmthead = fmtout + 1;
                continue;

            case 's':
                substr = va_arg(args, char*);
                if (!substr)
                    substr = "(NULL)";
                str = substr;
                if (precision > 0)
                    while (*str++ && --precision)
                        ;
                else
                    while (*str++)
                        ;
                len = str - substr; // 其实就是为了实现 strnlen ，此处不希望再进行函数压栈
                str = tmp;
                if ((!(flags & LEFT)) && (len < field_width)) { // 右对齐且需要补全空格
                    do {
                        *str++ = ' ';
                    } while (len < --field_width); // 填充空格串
                    output(tmp, str - tmp);
                }
                output(substr, len); // 输出子字符串
                if (len < field_width) { // 左对齐且需要补全右边空格
                    do {
                        *str++ = ' ';
                    } while (len < --field_width);
                    output(tmp, str - tmp);
                }
                fmthead = fmtout + 1;
                continue;

            case 'p':
                if (field_width == -1) {
                    field_width = 2 * sizeof(void*);
                    flags |= ZEROPAD;
                }
                str = number(tmp,
                    (size_t)va_arg(args, void*), 16,
                    field_width, precision, flags);
                output(tmp, str - tmp);
                fmthead = fmtout + 1;
                continue;

            case 'f':
                str = float2string(tmp, va_arg(args, double), field_width, precision, flags);
                output(tmp, str - tmp);
                fmthead = fmtout + 1;
                continue;

            case '%':
                *str++ = '%';
                output(tmp, str - tmp);
                fmthead = fmtout + 1;
                continue;

            /* integer number formats - set up the flags and "break" */
            case 'o':
                base = 8;
                break;

            case 'x':
                flags |= SMALL;
            case 'X':
                base = 16;
                break;

            case 'd':
            case 'i':
                flags |= SIGN;
            case 'u':
                break;

            default:
                continue;
            } // switch()

            if (qualifier == 'l')
                num = va_arg(args, unsigned long);
            else
                num = va_arg(args, int);

            str = number(tmp, num, base, field_width, precision, flags);
            output(tmp, str - tmp);
            fmthead = fmtout + 1;
        } // if (*fmtout == '%')
    }

    if (fmthead != fmtout)
        output(fmthead, fmtout - fmthead);

    va_end(args);
}