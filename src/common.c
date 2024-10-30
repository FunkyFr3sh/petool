#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>

#include "cleanup.h"
#include "common.h"


// Caller cleans up fh and image, length is optional
int open_and_read(FILE **fh, int8_t **image, uint32_t *length,
                  const char* executable, const char *fopen_attr)
{
    int ret = EXIT_SUCCESS;

    *fh = fopen(executable, fopen_attr);

    if (!*fh) {
        // Somehow this does fail a lot on windows - retry after 1 second does fix it
        sleep(1);
        *fh = fopen(executable, fopen_attr);
    }
    FAIL_IF_PERROR(!*fh, "Could not open executable");

    FAIL_IF_PERROR(fseek(*fh, 0L, SEEK_END),
                   "Need seekable file for executable, not stream");

    uint32_t len = ftell(*fh);
    rewind(*fh);

    *image = malloc(len);
    FAIL_IF(!*image, "Failed to allocate memory to read executable with\n");

    FAIL_IF_PERROR(fread(*image, len, 1, *fh) != 1, "Error reading executable");

    if (length) *length = len;

cleanup:
    return ret;
}

bool file_exists(const char *path)
{
    FILE *fh = fopen(path, "r");

    if (fh)
    {
        fclose(fh);
        return true;
    }

    return false;
}

const char *file_basename(const char *path)
{
    static char basebuf[4096];
    size_t i;

    if (path == NULL)
        return NULL;

    for (i = strlen(path); i > 0; i--)
    {
        if (path[i] == '/' || path[i] == '\\')
        {
            i++;
            break;
        }
    }

    strncpy(basebuf, path + i, sizeof basebuf - 1);
    basebuf[sizeof basebuf - 1] = '\0';

    char* p = strchr(basebuf, ' ');
    if (p)
    {
        do 
        {
            *p = '_';
        } while ((p = strchr(p, ' ')));
    }

    return basebuf;
}

const char* file_escaped_basename(const char* path)
{
    static char basebuf[4096];
    size_t i;

    if (path == NULL)
        return NULL;

    for (i = strlen(path); i > 0; i--)
    {
        if (path[i] == '/' || path[i] == '\\')
        {
            i++;
            break;
        }
    }

    strncpy(basebuf, path + i, sizeof basebuf - 1);
    basebuf[sizeof basebuf - 1] = '\0';

    char* p = strchr(basebuf, ' ');
    if (p)
    {
        do
        {
            memmove(p + 1, p, strlen(p) + 1);

            *p++ = '\\';
        } while ((p = strchr(++p, ' ')));
    }

    return basebuf;
}

int file_copy(const char* from, const char *to)
{
    int ret = EXIT_SUCCESS;
    FILE *from_fh = NULL, *to_fh = NULL;
    static char buf[4096];
    size_t numread;

    from_fh = fopen(from, "rb");
    FAIL_IF_PERROR(!from_fh, "Could not open input file");

    FAIL_IF(file_exists(to), "Output file already exists.\n");

    to_fh = fopen(to, "wb");
    FAIL_IF_PERROR(!to_fh, "Could not open output file");

    while ((numread = fread(buf, 1, sizeof buf, from_fh)) > 0)
    {
        FAIL_IF_PERROR(fwrite(buf, 1, numread, to_fh) != numread, "Output file truncated");
    }

cleanup:
    if (from_fh) fclose(from_fh);
    if (to_fh) fclose(to_fh);
    return ret;
}

int extract_resource(const char* src, char* file_path)
{
    int     ret = EXIT_SUCCESS;
    FILE* fh = NULL;

    FAIL_IF(file_exists(file_path), "Output file already exists.\n");

    fh = fopen(file_path, "wb");
    FAIL_IF_PERROR(!fh, "Could not open output file");

    fputs(src, fh);

    if (ferror(fh))
    {
        ret = EXIT_FAILURE;
    }

    fclose(fh);

cleanup:
    return ret;
}
