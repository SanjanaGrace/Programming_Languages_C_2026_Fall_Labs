#include <stdio.h>

int is_prime(int n) {
    if (n < 2) {
        return 0;
    }

    for (int divisor = 2; divisor <= n / divisor; divisor++) {
        if (n % divisor == 0) {
            return 0;
        }
    }

    return 1;
}

int main(void) {
    int n;

    printf("Enter n: ");

    if (scanf("%d", &n) != 1) {
        printf("Error: please enter an integer.\n");
        return 1;
    }

    if (n < 2) {
        printf("Error: n must be at least 2.\n");
        return 1;
    }

    printf("Prime numbers up to %d:\n", n);

    for (int number = 2; number <= n; number++) {
        if (is_prime(number)) {
            printf("%d ", number);
        }
    }

    printf("\n");

    return 0;
}
