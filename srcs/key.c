#include "woody.h"

#define KEY_SIZE 16

/* Remplit `key` avec keylen octets tirés de /dev/urandom */
int generate_key(unsigned char *key, int keylen)
{
    int fd;
    int ret;

    fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0)
        return (-1);
    ret = read(fd, key, keylen);
    close(fd);
    if (ret != keylen)       // on exige d'avoir lu TOUS les octets demandés
        return (-1);
    return (0);
}

/* Affiche la clé en hexadécimal, format imposé par le sujet */
void print_key(unsigned char *key, int keylen)
{
    int i;

    printf("key_value: ");
    i = 0;
    while (i < keylen)
    {
        printf("%02x", key[i]);   // %02x : 2 chiffres hex, avec un 0 devant si besoin
        i++;
    }
    printf("\n");
}