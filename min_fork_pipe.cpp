#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int main() {
    int arr[20];
    int N = 20;

    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        arr[i] = rand() % 1000;
    }

    printf("Array elements:\n");
    for (int i = 0; i < N; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n\n");

    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    int id = fork();
    if (id == -1) {
        perror("fork");
        return 2;
    }

    if (id == 0) {
        // child: second half 10-19
        close(fd[0]);

        int childMin = arr[N/2];
        for (int i = N/2; i < N; i++) {
            if (arr[i] < childMin) childMin = arr[i];
        }

        printf("child process id is %d\n", getpid());
        printf("child min (second half 10-19): %d\n", childMin);

        write(fd[1], &childMin, sizeof(childMin));
        close(fd[1]);
        return 0;
    } else {
        // parent: first half 0-9
        close(fd[1]);

        int parentMin = arr[0];
        for (int i = 0; i < N/2; i++) {
            if (arr[i] < parentMin) parentMin = arr[i];
        }

        printf("parent process id is %d\n", getpid());
        printf("parent min (first half 0-9): %d\n", parentMin);

        int childMin;
        read(fd[0], &childMin, sizeof(childMin));
        close(fd[0]);

        wait(NULL);

        int overallMin = parentMin;
        if (childMin < overallMin) overallMin = childMin;

        printf("minimum of whole array: %d\n", overallMin);
        return 0;
    }
}