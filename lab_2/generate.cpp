#include <cstdlib>
#include <cstdio>
#include <iostream>

int main() {
    int *array;
    int N = 240, j;
    std::freopen("input", "w", stdout);

    array = new int[N];
    for (int i = 0; i < N; i++)
        array[i] = i + 1;
    for (int i = 0; i < N; i++) {
        j = rand() % N;
        std::swap(array[i], array[j]);
    }
    fwrite(array, sizeof(int), N, stdout);
}

