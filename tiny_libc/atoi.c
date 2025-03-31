#include <ctype.h>
#include <stddef.h>

/**
 * Convert a string into a decimal long
 */
long atol(const char *str)
{
    long ret = 0;
    int negative = 0;
    int base = 10;


    // Check if str pointer is NULL
    if (NULL == str) {
        return 0;
    }

    // Skip blanks until reaching the first non-blank char
    while (isspace(*str)) {
        ;
    }
    if ('+' == *str) {
        negative = 0;
        ++str;
    }
    else if ('-' == *str) {
        negative = 1;
        ++str;
    }
    else if (isdigit(*str)) {
        negative = 0;
    }
    else {
        return 0;
    }

    // 0x or 0X for hexadecimal
    if ((str[0] == '0' && str[1] == 'x') ||
        (str[0] == '0' && str[1] == 'X')) {
        base = 16;
        ++str;
        ++str;
    }

    // Start converting ...
    while (*str != '\0') {
        if (isdigit(*str)) {
            ret = ret * base + (*str - '0');
        } else if (base == 16) {
            if ('a' <= *str && *str <= 'f'){
                ret = ret * base + (*str - 'a' + 10);
            } else if ('A' <= *str && *str <= 'F') {
                ret = ret * base + (*str - 'A' + 10);
            } else {
                return 0;
            }            
        } else {
            return 0;
        }
        ++str;
    }

    return negative ? -ret : ret;
}

/**
 * Convert a string into a decimal int
 */
int atoi(const char *str)
{
    return (int)atol(str);
}