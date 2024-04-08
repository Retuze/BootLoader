/*
  Copyright (c) 2014 Arduino.  All right reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "Arduino.h"

#include "Print.h"

// Public Methods //////////////////////////////////////////////////////////////


// size_t Print::print(const char *str)
// {
//   return write(str);
// }

size_t Print::print(const char str[])
{
  return write(str);
}

size_t Print::print(char c)
{
  return write(c);
}

size_t Print::print(unsigned char b, int base)
{
  return print((unsigned long) b, base);
}

size_t Print::print(int n, int base)
{
  return print((long) n, base);
}

size_t Print::print(unsigned int n, int base)
{
  return print((unsigned long) n, base);
}

size_t Print::print(long n, int base)
{
  if (base == 0) {
    return write(n);
  } else if (base == 10) {
    if (n < 0) {
      int t = print('-');
      n = -n;
      return printNumber(n, 10) + t;
    }
    return printNumber(n, 10);
  } else {
    return printNumber(n, base);
  }
}

size_t Print::print(unsigned long n, int base)
{
  if (base == 0) {
    return write(n);
  } else {
    return printNumber(n, base);
  }
}

size_t Print::print(long long n, int base)
{
  if (base == 0) {
    return write(n);
  } else if (base == 10) {
    if (n < 0) {
      int t = print('-');
      n = -n;
      return printULLNumber(n, 10) + t;
    }
    return printULLNumber(n, 10);
  } else {
    return printULLNumber(n, base);
  }
}

size_t Print::print(unsigned long long n, int base)
{
  if (base == 0) {
    return write(n);
  } else {
    return printULLNumber(n, base);
  }
}

size_t Print::print(double n, int digits)
{
  return printFloat(n, digits);
}

// size_t Print::println(const char *ifsh)
// {
//   size_t n = print(ifsh);
//   n += println();
//   return n;
// }

size_t Print::print(const Printable &x)
{
  return x.printTo(*this);
}

size_t Print::println(void)
{
  return write("\r\n");
}

// size_t Print::println(const String &s)
// {
//   size_t n = print(s);
//   n += println();
//   return n;
// }

size_t Print::println(const char c[])
{
  size_t n = print(c);
  n += println();
  return n;
}

size_t Print::println(char c)
{
  size_t n = print(c);
  n += println();
  return n;
}

size_t Print::println(unsigned char b, int base)
{
  size_t n = print(b, base);
  n += println();
  return n;
}

size_t Print::println(int num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

size_t Print::println(unsigned int num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

size_t Print::println(long num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

size_t Print::println(unsigned long num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

size_t Print::println(long long num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

size_t Print::println(unsigned long long num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

size_t Print::println(double num, int digits)
{
  size_t n = print(num, digits);
  n += println();
  return n;
}

size_t Print::println(const Printable &x)
{
  size_t n = print(x);
  n += println();
  return n;
}

#define ZEROPAD 1 /* pad with zero */
#define SIGN 2 /* unsigned/signed long */
#define PLUS 4 /* show plus */
#define SPACE 8 /* space if plus */
#define LEFT 16 /* left justified */
#define SMALL 32 /* Must be 32 == 0x20 */
#define SPECIAL 64 /* 0x */

char* Print::number(char* str, long num, int base, int size, int precision,
    int type)
{
  /* we are called with base 8, 10 or 16, only, thus don't need "G..." */
  static const char *digits = "0123456789ABCDEF"; /* "GHIJKLMNOPQRSTUVWXYZ"; */
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

 char* Print::float2string(char* str, float num, int size, int precision, int type)
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

void Print::printf(const char* fmt, ...)
{
  char tmp[88]; // 此段内存仅用于缓存数字转换成的字符串
  const char* substr;
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
          write(fmthead, fmtout - fmthead);
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
      if (isdigit(*fmtout)) {
          field_width = 0;
          do {
              field_width = field_width * 10 + *fmtout - '0';
              ++fmtout;
          } while (isdigit(*fmtout));
          if (field_width > (int)sizeof(tmp))
              field_width = (int)sizeof(tmp);
      } else
          field_width = -1;

      /* get the precision */
      if (*fmtout == '.') {
          precision = 0;
          for (++fmtout; isdigit(*fmtout); ++fmtout)
              precision = precision * 10 + *fmtout - '0';
          if (precision > (int)sizeof(tmp))
              precision = (int)sizeof(tmp);
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
          write(tmp, str - tmp);
          fmthead = fmtout + 1;
          continue;

      case 's':
          substr = va_arg(args, char*);
          if (!substr)
              substr = "(NULL)";
          str = (char *)substr;
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
              write(tmp, str - tmp);
          }
          write(substr, len); // 输出子字符串
          if (len < field_width) { // 左对齐且需要补全右边空格
              do {
                  *str++ = ' ';
              } while (len < --field_width);
              write(tmp, str - tmp);
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
          write(tmp, str - tmp);
          fmthead = fmtout + 1;
          continue;

      case 'f':
          str = float2string(tmp, va_arg(args, double), field_width, precision, flags);
          write(tmp, str - tmp);
          fmthead = fmtout + 1;
          continue;

      case '%':
          *str++ = '%';
          write(tmp, str - tmp);
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
      write(tmp, str - tmp);
      fmthead = fmtout + 1;
    } // if (*fmtout == '%')
  }

  if (fmthead != fmtout)
    write(fmthead, fmtout - fmthead);

  va_end(args);
}

// Private Methods /////////////////////////////////////////////////////////////

size_t Print::printNumber(unsigned long n, uint8_t base)
{
  char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
  char *str = &buf[sizeof(buf) - 1];

  *str = '\0';

  // prevent crash if called with base == 1
  if (base < 2) {
    base = 10;
  }

  do {
    unsigned long m = n;
    n /= base;
    char c = m - base * n;
    *--str = c < 10 ? c + '0' : c + 'A' - 10;
  } while (n);

  return write(str);
}

/*
void Print::printULLNumber(uint64_t n, uint8_t base)
{
  unsigned char buf[16 * sizeof(long)];
  unsigned int i = 0;

  if (n == 0) {
    print((char)'0');
    return;
  }

  while (n > 0) {
    buf[i++] = n % base;
    n /= base;
  }

  for (; i > 0; i--) {
    print((char)(buf[i - 1] < 10 ? '0' + buf[i - 1] : 'A' + buf[i - 1] - 10));
  }
}
*/
// REFERENCE IMPLEMENTATION FOR ULL
// size_t Print::printULLNumber(unsigned long long n, uint8_t base)
// {
// // if limited to base 10 and 16 the bufsize can be smaller
// char buf[65];
// char *str = &buf[64];

// *str = '\0';

// // prevent crash if called with base == 1
// if (base < 2) base = 10;

// do {
// unsigned long long t = n / base;
// char c = n - t * base;  // faster than c = n%base;
// n = t;
// *--str = c < 10 ? c + '0' : c + 'A' - 10;
// } while(n);

// return write(str);
// }

// FAST IMPLEMENTATION FOR ULL
size_t Print::printULLNumber(unsigned long long n64, uint8_t base)
{
  // if limited to base 10 and 16 the bufsize can be 20
  char buf[64];
  uint8_t i = 0;
  uint8_t innerLoops = 0;

  // prevent crash if called with base == 1
  if (base < 2) {
    base = 10;
  }

  // process chunks that fit in "16 bit math".
  uint16_t top = 0xFFFF / base;
  uint16_t th16 = 1;
  while (th16 < top) {
    th16 *= base;
    innerLoops++;
  }

  while (n64 > th16) {
    // 64 bit math part
    uint64_t q = n64 / th16;
    uint16_t r = n64 - q * th16;
    n64 = q;

    // 16 bit math loop to do remainder. (note buffer is filled reverse)
    for (uint8_t j = 0; j < innerLoops; j++) {
      uint16_t qq = r / base;
      buf[i++] = r - qq * base;
      r = qq;
    }
  }

  uint16_t n16 = n64;
  while (n16 > 0) {
    uint16_t qq = n16 / base;
    buf[i++] = n16 - qq * base;
    n16 = qq;
  }

  size_t bytes = i;
  for (; i > 0; i--) {
    write((char)(buf[i - 1] < 10 ?
                 '0' + buf[i - 1] :
                 'A' + buf[i - 1] - 10));
  }
  return bytes;
}

size_t Print::printFloat(double number, uint8_t digits)
{
  size_t n = 0;

  if (isnan(number)) {
    return print("nan");
  }
  if (isinf(number)) {
    return print("inf");
  }
  if (number > 4294967040.0) {
    return print("ovf");  // constant determined empirically
  }
  if (number < -4294967040.0) {
    return print("ovf");  // constant determined empirically
  }

  // Handle negative numbers
  if (number < 0.0) {
    n += print('-');
    number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for (uint8_t i = 0; i < digits; ++i) {
    rounding /= 10.0;
  }

  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  n += print(int_part);

  // Print the decimal point, but only if there are digits beyond
  if (digits > 0) {
    n += print('.');
  }

  // Extract digits from the remainder one at a time
  while (digits-- > 0) {
    remainder *= 10.0;
    unsigned int toPrint = (unsigned int)remainder;
    n += print(toPrint);
    remainder -= toPrint;
  }

  return n;
}
