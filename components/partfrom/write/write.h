#ifndef _WRITE_H_
#define _WRITE_H_

#include "usart.h"
#include "partfrom.h"

#ifdef __cplusplus
extern "C" {
#endif

int _write(int fd, char* pBuffer, int size);

#ifdef __cplusplus
}

#endif

#endif // _WRITE_H_



