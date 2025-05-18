#include <mailbox.h>

const uint32_t MOD_ADLER = 65521;

/* adler32 algorithm implementation from: https://en.wikipedia.org/wiki/Adler-32 */
uint32_t adler32(char *data, size_t len)
{
    uint32_t a = 1, b = 0;
    size_t index;

    // Process each byte of the data in order
    for (index = 0; index < len; ++index)
    {
        a = (a + data[index]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }

    return (b << 16) | a;
}

void generateRandomString(char* buf, int len)
{
    static const char alpha[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-={};|[],./<>?!@#$%^&*";
    static const int alphaSz = sizeof(alpha) / sizeof(char);
    int i = len - 2;
    buf[len - 1] = '\0';
    while (i >= 0) {
        buf[i] = alpha[rand() % alphaSz];
        --i;
    }
}