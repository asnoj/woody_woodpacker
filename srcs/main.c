#include "woody.h"

/* fait tout le travail sur le fichier deja mappe ; renvoie le code de sortie */
static int pack(void *file, off_t size)
{
    Elf64_Phdr    *text;
    unsigned char key[KEY_SIZE];

    if (validate_elf(file, size) != 0)
        return (1);
    text = find_text_segment(file);
    if (!text)
    {
        fprintf(stderr, "Pas de segment executable trouve\n");
        return (1);
    }
    if (text->p_offset + text->p_filesz > (uint64_t)size)
    {
        fprintf(stderr, "ELF corrompu (segment de code)\n");
        return (1);
    }
    if (generate_key(key, KEY_SIZE) != 0)
    {
        fprintf(stderr, "Erreur generation cle\n");
        return (1);
    }
    print_key(key, KEY_SIZE);
    if (inject(file, size, key, KEY_SIZE) != 0)
    {
        fprintf(stderr, "injection echouee\n");
        return (1);
    }
    return (0);
}

int main(int argc, char **argv)
{
    int    fd;
    off_t  size;
    void   *file;
    int    ret;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <elf64_binary>\n", argv[0]);
        return (1);
    }
    fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return (1);
    }
    size = lseek(fd, 0, SEEK_END);
    if (size <= 0)
    {
        fprintf(stderr, "Fichier vide ou illisible\n");
        close(fd);
        return (1);
    }
    lseek(fd, 0, SEEK_SET);
    file = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);
    if (file == MAP_FAILED)
    {
        perror("mmap");
        return (1);
    }
    ret = pack(file, size);
    munmap(file, size);     // <-- UN SEUL munmap, ici
    return (ret);
}
