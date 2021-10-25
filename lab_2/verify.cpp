#include <cstdio>

int main() {
    FILE *fp;
    int n_bytes, val1, val2, i;

    if (!(fp = fopen("output", "r"))) {
        printf("Can't open datafile %s\n", "output");
        return 1;
    }

    n_bytes = fread(&val1, sizeof(int), 1, fp);
    if (!n_bytes) {
        printf("No data in %s\n", "output");
        return 1;
    }
    i = 1;
    while (true) {
        n_bytes = fread(&val2, sizeof(int), 1, fp);
        if (n_bytes == 0)
            break;
        i++;
        if (val1 > val2) {
            printf("Failed: items %u, %u: %u > %u\n",
                   i - 1, i, val1, val2);
            return 1;
        }
        val1 = val2;
    }
    printf("Data in %s are sorted\n", "output");
    return 0;
}

