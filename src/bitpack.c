#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static int fail(FILE *in, FILE *out, const char *msg) {
    if (msg) fprintf(stderr, "bitpack: %s\n", msg);
    if (in) (void)fclose(in);
    if (out) (void)fclose(out);
    return 2;
}

int main(int ac, char **av) {
    if (ac != 3) {
        fprintf(stderr, "usage: bitpack in.bits out.bin\n");
        return 2;
    }
    FILE *in = fopen(av[1], "rb");
    if (!in) { perror("bitpack input"); return 2; }
    FILE *out = fopen(av[2], "wb");
    if (!out) { perror("bitpack output"); (void)fclose(in); return 2; }

    unsigned value = 0, count = 0;
    int c;
    while ((c = fgetc(in)) != EOF) {
        if (c == '0' || c == '1') {
            value = (value << 1) | (unsigned)(c - '0');
            if (++count == 8) {
                if (fputc((int)value, out) == EOF) return fail(in, out, "write failed");
                value = 0;
                count = 0;
            }
        } else if (!isspace((unsigned char)c)) {
            return fail(in, out, "invalid character");
        }
    }
    if (ferror(in)) return fail(in, out, "read failed");
    if (count) return fail(in, out, "incomplete byte");
    if (fclose(in) != 0) { (void)fclose(out); return 2; }
    if (fclose(out) != 0) return 2;
    return 0;
}
