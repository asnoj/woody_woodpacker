#include "woody.h"

/* 0 si ELF64 valide, -1 sinon (avec message) */
int validate_elf(void *file, off_t size)
{
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)file;

    if (size < (off_t)sizeof(Elf64_Ehdr)
        || memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0)
    {
        fprintf(stderr, "Not an ELF file\n");
        return (-1);
    }
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64)
    {
        fprintf(stderr, "File architecture not suported. x86_64 only\n");
        return (-1);
    }
    if (ehdr->e_phentsize != sizeof(Elf64_Phdr)
        || ehdr->e_phnum == 0
        || ehdr->e_phoff > (uint64_t)size
        || ehdr->e_phoff + (uint64_t)ehdr->e_phnum * sizeof(Elf64_Phdr) > (uint64_t)size)
    {
        fprintf(stderr, "ELF corrompu (program headers)\n");
        return (-1);
    }
    return (0);
}

/* renvoie le segment de code (PT_LOAD + executable), ou NULL */
Elf64_Phdr *find_text_segment(void *file)
{
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)file;
    Elf64_Phdr *phdr = (Elf64_Phdr *)((char *)file + ehdr->e_phoff);
    int        i = 0;

    while (i < ehdr->e_phnum)
    {
        if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & PF_X))
            return (&phdr[i]);
        i++;
    }
    return (NULL);
}