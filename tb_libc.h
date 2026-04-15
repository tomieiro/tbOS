#ifndef TB_LIBC_H
#define TB_LIBC_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t ebx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eip;
} tb_jmpbuf;

int tb_setjmp(tb_jmpbuf *buffer);
void tb_longjmp(tb_jmpbuf *buffer, int value) __attribute__((noreturn));

void *memcpy(void *destination, const void *source, size_t size);
void *memmove(void *destination, const void *source, size_t size);
void *memset(void *destination, int value, size_t size);
int memcmp(const void *left, const void *right, size_t size);
size_t strlen(const char *text);
int strcmp(const char *left, const char *right);
char *strcpy(char *destination, const char *source);
char *strchr(const char *text, int value);
char *strpbrk(const char *text, const char *accept);
size_t strspn(const char *text, const char *accept);
int strcoll(const char *left, const char *right);
int abs(int value);
double ldexp(double value, int exponent);
float ldexpf(float value, int exponent);
double frexp(double value, int *exponent);
float frexpf(float value, int *exponent);
void abort(void);

float tb_floorf(float value);
float tb_fmodf(float left, float right);
float tb_powf(float base, float exponent);
float tb_lua_str2number(const char *text, char **endptr);
float tb_lua_strx2number(const char *text, char **endptr);
int tb_lua_number2str(char *buffer, size_t size, float value);
int tb_lua_integer2str(char *buffer, size_t size, int32_t value);
int tb_lua_pointer2str(char *buffer, size_t size, const void *pointer);

#endif
