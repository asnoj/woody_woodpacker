#include "woody.h"

/* Écrit `size` octets de `data` dans un fichier nomme "woody", executable */
int write_woody(void *data, size_t size)
{
    int     fd;
    ssize_t written;

    // O_CREAT : cree le fichier s'il n'existe pas
    // O_TRUNC : le vide s'il existe deja
    // 0755    : permissions rwxr-xr-x -> le fichier sera executable
    fd = open("woody", O_WRONLY | O_CREAT | O_TRUNC, 0755);
    if (fd < 0)
    {
        perror("open woody");
        return (-1);
    }
    written = write(fd, data, size);
    close(fd);
    if (written != (ssize_t)size)      // on exige d'avoir tout ecrit
    {
        fprintf(stderr, "write woody incomplet\n");
        return (-1);
    }
    return (0);
}
