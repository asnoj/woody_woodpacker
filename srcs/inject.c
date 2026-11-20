#include "woody.h"
#include "stub_bin.h"      // la DEFINITION de woody_stub vit maintenant ICI

#define STUB_VADDR_GAP 0x4000000   // ecart vaddr choisi pour le stub (64 Mo)

/* cherche la valeur magic (8 octets) dans le stub et ecrit value a la place */
static int patch_u64(unsigned char *s, size_t len, uint64_t magic, uint64_t value)
{
    size_t   i;
    uint64_t v;

    i = 0;
    while (i + 8 <= len)
    {
        memcpy(&v, s + i, 8);          // memcpy -> pas de souci d'alignement
        if (v == magic)
        {
            memcpy(s + i, &value, 8);
            return (0);
        }
        i++;
    }
    return (-1);
}

/* cherche keylen octets 0x44 et ecrit la vraie cle a la place */
static int patch_key(unsigned char *s, size_t len, unsigned char *key, int keylen)
{
    size_t i;
    int    j;

    i = 0;
    while (i + keylen <= len)
    {
        j = 0;
        while (j < keylen && s[i + j] == 0x44)
            j++;
        if (j == keylen)
        {
            memcpy(s + i, key, keylen);
            return (0);
        }
        i++;
    }
    return (-1);
}

int inject(void *file, size_t size, unsigned char *key, int keylen)
{
    unsigned char *out;
    size_t        total;
    Elf64_Ehdr    *ehdr;
    Elf64_Phdr    *phdr;
    Elf64_Phdr    *text;
    Elf64_Phdr    *note;
    t_rc4         rc4;
    int           i;
    int           ret;

    // --- 1. buffer agrandi = original + stub ---
    total = size + woody_stub_len;
    out = malloc(total);
    if (!out)
        return (-1);
    memcpy(out, file, size);                          // copie de l'original
    memcpy(out + size, woody_stub, woody_stub_len);   // stub a la fin

    // pointeurs ELF dans la COPIE
    ehdr = (Elf64_Ehdr *)out;
    phdr = (Elf64_Phdr *)(out + ehdr->e_phoff);

    // retrouver le segment de code (PT_LOAD+X) et le PT_NOTE
    text = NULL;
    note = NULL;
    i = 0;
    while (i < ehdr->e_phnum)
    {
        if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & PF_X))
            text = &phdr[i];
        if (phdr[i].p_type == PT_NOTE)
            note = &phdr[i];
        i++;
    }
    if (!text || !note)
    {
        free(out);
        return (-1);
    }

    // --- 2. sauvegarder l'entry point ORIGINAL (avant de l'ecraser) ---
    uint64_t orig_entry = ehdr->e_entry;
    uint64_t code_vaddr = text->p_vaddr;
    uint64_t code_off   = text->p_offset;
    uint64_t code_len   = text->p_filesz;
    uint64_t stub_off   = size;                        // le stub commence a la fin de l'original
    uint64_t stub_vaddr = stub_off + STUB_VADDR_GAP;   // vaddr du stub

    // --- 3. chiffrer le code dans la copie ---
    rc4_init(&rc4, key, keylen);
    rc4_crypt(&rc4, out + code_off, code_len);

    // --- 4. patcher le stub (dans out, a partir de stub_off) ---
    unsigned char *s = out + stub_off;
    if (patch_key(s, woody_stub_len, key, keylen) != 0
        || patch_u64(s, woody_stub_len, 0x1111111111111111ULL, code_vaddr - stub_vaddr) != 0
        || patch_u64(s, woody_stub_len, 0x2222222222222222ULL, code_len) != 0
        || patch_u64(s, woody_stub_len, 0x3333333333333333ULL, orig_entry - stub_vaddr) != 0)
    {
        free(out);
        return (-1);
    }

    // --- 5. recycler PT_NOTE -> PT_LOAD decrivant le stub ---
    note->p_type   = PT_LOAD;
    note->p_flags  = PF_R | PF_X;      // lecture + execution
    note->p_offset = stub_off;
    note->p_vaddr  = stub_vaddr;
    note->p_paddr  = stub_vaddr;
    note->p_filesz = woody_stub_len;
    note->p_memsz  = woody_stub_len;
    note->p_align  = 0x1000;

    // --- 6. detourner l'entry point vers le stub ---
    ehdr->e_entry = stub_vaddr;

    // --- 7. ecrire woody ---
    ret = write_woody(out, total);
    free(out);
    return (ret);
}
