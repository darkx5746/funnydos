/*
**      File BIN2C.C
**      ¤a·¡áŸ¡ -> C •A·¡Èá ¤µi ¥eÑÅ‹¡
**
**      ¸b¬÷i¼a : 99‘e 12¶© 18·©
**      ¸b ¬÷ ¸a : ·¡¬â£¥ (PCHacker@hitel.net)
**
**      ÄñÌa·©œá : Turbo C 2.01 ™¡“e ‹a ·¡¬w, DJGPP V2.02
**
*/

#include <stdio.h>

typedef unsigned char byte;

long filesize(FILE *stream)
{
    long curpos, length;

    curpos = ftell(stream);
    fseek(stream, 0L, SEEK_END);
    length = ftell(stream);
    fseek(stream, curpos, SEEK_SET);

    return length;
}

int main(int argc, char *argv[])
{
    byte buffer[16];
    int i, j;
    long length;
    FILE *fp1, *fp2;

    if (argc < 4) {
        puts("Binary to C Converter 1.0 (Programmed by PCHacker)");
        puts("Usage: bin2c.exe infile symname outfile\n");
        puts("   Ex) bin2c.exe han.fnt _hanfont hanfont.c");
        return -1;
    }

    fp1 = fopen(argv[1], "rb");
    if (fp1 == NULL) {
        puts("File open error...");
        return -1;
    }
    length = filesize(fp1);

    /* -------------------------------------------------------------------- */

    fp2 = fopen(argv[3], "wb");
    fprintf(fp2, "const unsigned char %s[%ld] = {\n", argv[2], length);
    for (i = 0; i < (length / 16); i++) {
        fread(buffer, 16, 1, fp1);
        for (j = 0; j < 16; j++)
            fprintf(fp2, "0x%02X,", buffer[j]);
        fprintf(fp2, "\n");
    }

    if (length % 16 != 0) {
        fread(buffer, (size_t)(length % 16), 1, fp1);
        for (j = 0; j < length % 16; j++)
            fprintf(fp2, "0x%02X,", buffer[j]);
        fprintf(fp2, "\n};");
    } else fprintf(fp2, "};");

    fclose(fp1);
    fclose(fp2);

    return 0;
}

