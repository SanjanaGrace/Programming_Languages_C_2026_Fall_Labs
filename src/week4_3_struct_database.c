#include <stdio.h>
#include <stdlib.h>

// Define the Student structure.
struct Student {
    char name[50];
    int id;
    float grade;
};

int main(void)
{
    int n;

    // Read and validate the number of students.
    printf("Enter number of students: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid number.\n");
        return 1;
    }

    // Allocate an array of Student structures.
    struct Student *students =
        malloc((size_t)n * sizeof(struct Student));

    if (students == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    // Read each student's name, ID, and grade.
    for (int i = 0; i < n; i++) {
        printf("Enter data for student %d: ", i + 1);

        if (scanf("%49s %d %f",
                  students[i].name,
                  &students[i].id,
                  &students[i].grade) != 3) {
            free(students);
            printf("Invalid input.\n");
            return 1;
        }
    }

    // Print a blank line followed by the student table.
    printf("\n");
    printf("%-6s %-11s %s\n", "ID", "Name", "Grade");

    for (int i = 0; i < n; i++) {
        printf("%-6d %-11s %.1f\n",
               students[i].id,
               students[i].name,
               students[i].grade);
    }

    // Release the dynamically allocated array.
    free(students);

    return 0;
}
