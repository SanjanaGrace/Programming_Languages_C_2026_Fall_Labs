
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int n;

    printf("Enter number of elements: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid size.\n");
        return 1;
    }

    int *numbers = malloc((size_t)n * sizeof(int));
    if (numbers == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    printf("Enter %d integers: ", n);

    long long sum = 0;

    for (int i = 0; i < n; i++) {
        if (scanf("%d", &numbers[i]) != 1) {
            free(numbers);
            printf("Invalid input.\n");
            return 1;
        }

        sum += numbers[i];
    }

    double average = (double)sum / n;

    printf("Sum = %lld\n", sum);
    printf("Average = %.2f\n", average);

    free(numbers);

    return 0;
}
