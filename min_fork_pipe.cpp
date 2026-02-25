#include <iostream>
#include <unistd.h>     // fork, pipe, read, write, getpid, close
#include <sys/wait.h>   // waitpid
#include <cstdlib>      // rand, srand
#include <ctime>        // time
#include <algorithm>    // std::min

int main() {
    const int N = 20;
    int arr[N];

    // Seed RNG and fill array
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    for (int i = 0; i < N; i++) {
        arr[i] = std::rand() % 1000; // 0..999
    }

    std::cout << "Array elements:\n";
    for (int i = 0; i < N; i++) {
        std::cout << arr[i] << (i == N - 1 ? "\n" : " ");
    }
    std::cout << "\n";

    int fd[2];
    if (pipe(fd) == -1) {
        std::perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        std::perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child process 
        close(fd[0]); // close read end

        int childMin = arr[N / 2];
        for (int i = N / 2; i < N; i++) {
            if (arr[i] < childMin) childMin = arr[i];
        }

        std::cout << "[Child] PID: " << getpid()
                  << " | Min (second half index 10-19): " << childMin << "\n";

        // Send childMin to parent through pipe
        if (write(fd[1], &childMin, sizeof(childMin)) != sizeof(childMin)) {
            std::perror("write");
            close(fd[1]);
            return 1;
        }

        close(fd[1]);
        return 0;
    } else {
        // Parent process 
        close(fd[1]); // close write end

        int parentMin = arr[0];
        for (int i = 0; i < N / 2; i++) {
            if (arr[i] < parentMin) parentMin = arr[i];
        }

        std::cout << "[Parent] PID: " << getpid()
                  << " | Min (first half index 0-9): " << parentMin << "\n";

        // Read child minimum from pipe
        int childMin = 0;
        ssize_t bytesRead = read(fd[0], &childMin, sizeof(childMin));
        if (bytesRead != sizeof(childMin)) {
            std::perror("read");
            close(fd[0]);
            return 1;
        }
        close(fd[0]);

        // Wait for child to finish
        int status = 0;
        waitpid(pid, &status, 0);

        int overallMin = std::min(parentMin, childMin);
        std::cout << "[Parent] Received child min via pipe: " << childMin << "\n";
        std::cout << "Overall minimum of array: " << overallMin << "\n";

        return 0;
    }
}
