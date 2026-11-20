#ifndef WOODY_H
# define WOODY_H

# include <stdio.h>      // printf, perror, fprintf
# include <stdlib.h>     // exit
# include <unistd.h>     // close, lseek
# include <fcntl.h>      // open
# include <sys/mman.h>   // mmap, munmap
# include <elf.h>        // les structures Elf64_Ehdr, etc.
# include <string.h>     // memcmp (ou ton ft_memcmp)
# include <stdint.h>

typedef struct s_rc4 {
    unsigned char S[256];
    int           i;
    int           j;
}   t_rc4;

void rc4_init(t_rc4 *rc4, const unsigned char *key, int keylen);
void rc4_crypt(t_rc4 *rc4, unsigned char *data, size_t len);
# define KEY_SIZE 16

int  generate_key(unsigned char *key, int keylen);
void print_key(unsigned char *key, int keylen);
int write_woody(void *data, size_t size);
int inject(void *file, size_t size, unsigned char *key, int keylen);

extern unsigned char woody_stub[];
extern unsigned int  woody_stub_len;
int         validate_elf(void *file, off_t size);
Elf64_Phdr  *find_text_segment(void *file);

#endif