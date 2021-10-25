// An utility program for verifying that the integers in a data file
// are sorted in the ascending order.
#include <cstdio>

int main() {
    FILE *fp;
    int n_bytes, val1, val2, i;
    int intSize = sizeof(int);

    if (!(fp = fopen("output", "r"))) {
        printf("Can't open datafile %s\n", "output");
        return 0;
    }

    n_bytes = fread(&val1, intSize, 1, fp);
    if (!n_bytes) {
        printf("No data in %s\n", "output");
        return 0;
    }
    i = 1;
    while (true) {
        n_bytes = fread(&val2, intSize, 1, fp);
        if (n_bytes == 0)
            break;
        i++;
        if (val1 > val2) {
            printf("Failed: items %u, %u: %u > %u\n",
                   i - 1, i, val1, val2);
            return 0;
        }
        val1 = val2;
    }
    printf("Data in %s are sorted\n", "output");
}

