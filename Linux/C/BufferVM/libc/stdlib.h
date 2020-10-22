#ifndef STDLIB_H
#define STDLIB_H

#include "stdint.h"
#include "stddef.h"

#define NULL 0

#define UINT64_C(c)	c ## UL
#define UINT64_MAX  (__UINT64_C(18446744073709551615))

#ifndef EOF
# define EOF (-1)
#endif

#define	false	0
#define	true	1
#define	bool	_Bool

typedef __builtin_va_list va_list;
typedef __builtin_va_list __isoc_va_list; /* wchar.h */

#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)
#define va_copy(d,s) __builtin_va_copy(d,s)

extern ssize_t write(int file, const void *buffer, size_t count);
extern ssize_t read(int fd, void *buffer, size_t length);
extern int open(const char *filename, int32_t flags, uint16_t mode);
extern int close(int32_t handle);

int fgetc(int fd);
int puts(const char *string);
void putchar(char c);
char *convert(int64_t num, size_t base);
char *getline(void);
void abort(void);

#endif
