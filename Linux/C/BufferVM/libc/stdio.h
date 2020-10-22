//
// Created by ben on 30/12/17.
//

#ifndef LIBC_STDIO_H
#define LIBC_STDIO_H

#define FILE void

extern FILE *stderr;

extern FILE *stdin;

extern FILE *stdout; 

int printf(char* format,...);

int fprintf(FILE *, const char *, ...);

#endif //LIBC_STDIO_H
