#ifndef INCLUDE_STRING_H_
#define INCLUDE_STRING_H_

#include <type.h>

void memcpy(uint8_t *dest, const uint8_t *src, uint32_t len);
void memset(void *dest, uint8_t val, uint32_t len);
void bzero(void *dest, uint32_t len);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, uint32_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, int n);
char *strcat(char *dest, const char *src);
int strlen(const char *src);

#endif
