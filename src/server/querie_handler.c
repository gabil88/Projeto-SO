#include "../../include/server/document_manager.h"
#include "../../include/server/parsing.h"
#include "../../include/server/cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define pathToFile "DatasetTest/Gdataset"

int get_number_of_lines_with_keyword(int key, char* keyword) {
    Document* doc = consult_document(key);
    if (doc == NULL) {
        return -1; // Document not found
    }

    char full_path[4096];
    snprintf(full_path, sizeof(full_path), "%s/%s", pathToFile, doc->path);
    printf("Full path: %s\n", full_path);
    int fd = open(full_path, O_RDONLY, 0644);
    if (fd == -1) {
        perror("Error opening file");
        return -1;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("lseek");
        close(fd);
        return -1;
    }
    lseek(fd, 0, SEEK_SET);

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        close(fd);
        return -1;
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork");
        close(fd);
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (pid1 == 0) {
        // Child 1: process first half
        close(pipefd[0]);
        lseek(fd, 0, SEEK_SET);
        off_t half = file_size / 2;
        char buffer[4096];
        int count = 0;
        off_t read_bytes = 0;
        int in_line = 0;
        int matched = 0;
        size_t klen = strlen(keyword);

        while (read_bytes < half) {
            ssize_t n = read(fd, buffer, sizeof(buffer));
            if (n <= 0) break;
            if (read_bytes + n > half) n = half - read_bytes;
            for (ssize_t i = 0; i < n; ++i) {
                if (buffer[i] == '\n') {
                    if (matched) count++;
                    matched = 0;
                    in_line = 0;
                } else {
                    if (!in_line && strncmp(&buffer[i], keyword, klen) == 0) {
                        matched = 1;
                    }
                    in_line = 1;
                }
            }
            read_bytes += n;
        }
        write(pipefd[1], &count, sizeof(count));
        close(pipefd[1]);
        close(fd);
        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork");
        close(fd);
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (pid2 == 0) {
        // Child 2: process second half
        close(pipefd[0]);
        off_t half = file_size / 2;
        lseek(fd, half, SEEK_SET);
        char buffer[4096];
        int count = 0;
        int in_line = 0;
        int matched = 0;
        size_t klen = strlen(keyword);

        // Skip to next newline to avoid splitting a line
        char c;
        while (read(fd, &c, 1) == 1 && c != '\n');

        ssize_t n;
        while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
            for (ssize_t i = 0; i < n; ++i) {
                if (buffer[i] == '\n') {
                    if (matched) count++;
                    matched = 0;
                    in_line = 0;
                } else {
                    if (!in_line && strncmp(&buffer[i], keyword, klen) == 0) {
                        matched = 1;
                    }
                    in_line = 1;
                }
            }
        }
        write(pipefd[1], &count, sizeof(count));
        close(pipefd[1]);
        close(fd);
        exit(0);
    }

    // Parent
    close(pipefd[1]);
    close(fd);

    int total = 0, c;
    for (int i = 0; i < 2; ++i) {
        if (read(pipefd[0], &c, sizeof(c)) == sizeof(c)) {
            total += c;
        }
    }
    close(pipefd[0]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return total;
}