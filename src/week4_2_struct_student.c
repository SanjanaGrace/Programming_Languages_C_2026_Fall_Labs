#include <stdio.h>
#include <string.h>

// Define the Student structure.
struct Student {
    char name[50];
    int id;
    float grade;
};

int main(void)
{
    // Declare two Student variables.
    struct Student student1;
    struct Student student2;

    // Assign the first student's information.
    strcpy(student1.name, "Alice Johnson");
    student1.id = 1001;
    student1.grade = 9.1f;

    // Assign the second student's information.
    strcpy(student2.name, "Bob Smith");
    student2.id = 1002;
    student2.grade = 8.7f;

    // Print both student records.
    printf("Student 1: %s, ID: %d, Grade: %.1f\n",
           student1.name, student1.id, student1.grade);

    printf("Student 2: %s, ID: %d, Grade: %.1f\n",
           student2.name, student2.id, student2.grade);

    return 0;
}
