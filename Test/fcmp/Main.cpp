#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdint.h>

char *GetFilteredBuffer(const char *fname, int64_t &len)
{
    FILE *fp = fopen(fname, "rb");
    if (!fp)
    {
        len = 0;
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    if (len <= 0)
    {
        len = 0;
        return NULL;
    }

    char *bufA = new char[len + 1];
    char *bufB = new char[len + 1];
    fread(bufB, 1, len, fp);
    fclose(fp);

    int64_t i, j = 0;
    for (i = 0; i < len; ++i)
    {
        char ch = bufB[i];
        if (ch != '\r' && ch != '\n')
            bufA[j++] = bufB[i];
        bufA[j] = 0;
    }

    if (j == 0)
    {
        delete[] bufA;
        delete[] bufB;
        bufA = NULL;
    }
    else
    {
        delete[] bufB;
    }
    len = j;
    return bufA;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: fcmp file1 file2\n");
        return 1;
    }
    int64_t a, b;
    char * fileA = GetFilteredBuffer(argv[1], a);
    char * fileB = GetFilteredBuffer(argv[2], b);

    int rc = 0;
    if (a != b)
    {
        rc = 1;
        goto done;
    }

    if (fileA == NULL && fileB == NULL)
    {
        goto done;
    }
    else if (fileA != NULL && fileB == NULL)
    {
        rc = 1;
        goto done;
    }
    else if (fileA == NULL && fileB != NULL)
    {
        rc = 1;
        goto done;
    }
    else
    {
        if (memcmp(fileA, fileB, a) == 0)
            goto done;
    }

    rc = 1;
done:
    delete[] fileA;
    delete[] fileB;

    if (rc)
        printf("NEQ\n");
    else
        printf("EQ\n");
    return rc;
}
