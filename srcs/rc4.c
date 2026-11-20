#include "woody.h"

#define RC4_N 256

typedef struct s_rc4 {
    unsigned char S[RC4_N];
    int           i;
    int           j;
}   t_rc4;

/* KSA : prépare l'état S à partir de la clé */
void rc4_init(t_rc4 *rc4, const unsigned char *key, int keylen)
{
    int           i;
    int           j;
    unsigned char tmp;

    i = 0;
    while (i < RC4_N)              // S = 0,1,2,...,255
    {
        rc4->S[i] = (unsigned char)i;
        i++;
    }
    i = 0;
    j = 0;
    while (i < RC4_N)             // mélange piloté par la clé
    {
        j = (j + rc4->S[i] + key[i % keylen]) % RC4_N;
        tmp = rc4->S[i];          // swap S[i], S[j]
        rc4->S[i] = rc4->S[j];
        rc4->S[j] = tmp;
        i++;
    }
    rc4->i = 0;
    rc4->j = 0;
}

/* PRGA : chiffre OU déchiffre len octets de data, sur place */
void rc4_crypt(t_rc4 *rc4, unsigned char *data, size_t len)
{
    size_t        n;
    unsigned char tmp;
    unsigned char k;

    n = 0;
    while (n < len)
    {
        rc4->i = (rc4->i + 1) % RC4_N;
        rc4->j = (rc4->j + rc4->S[rc4->i]) % RC4_N;
        tmp = rc4->S[rc4->i];                 // swap S[i], S[j]
        rc4->S[rc4->i] = rc4->S[rc4->j];
        rc4->S[rc4->j] = tmp;
        k = rc4->S[(rc4->S[rc4->i] + rc4->S[rc4->j]) % RC4_N];  // octet de keystream
        data[n] = data[n] ^ k;                // le XOR
        n++;
    }
}
