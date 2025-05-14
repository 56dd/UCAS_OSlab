#ifndef __CTYPE_H__
#define __CTYPE_H__

static inline int isupper(int ch)
{
    return 'A' <= ch && ch <= 'Z';
}

static inline int islower(int ch)
{
    return 'a' <= ch && ch <= 'z';
}

static inline int isalpha(int ch)
{
    return isupper(ch) || islower(ch);
}

static inline int isdigit(int ch)
{
    return '0' <= ch && ch <= '9';
}

static inline int isxdigit(int ch)
{
    return isdigit(ch) || ('a' <= ch && ch <= 'f') ||
                          ('A' <= ch && ch <= 'F');
}

static inline int isalnum(int ch)
{
    return isalpha(ch) || isdigit(ch);
}

static inline int isspace(int ch)
{
    return ch == ' '  || ch == '\t' || ch == '\n' ||
           ch == '\v' || ch == '\f' || ch == '\r';
}

#endif