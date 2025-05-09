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


/*
* Atualmente esta tudo em disco, tenho de mudar para utilizar a cache tambem
*/


int get_number_of_lines_with_keyword(Document* doc, char* keyword) {
    if (doc == NULL) {
        return -1; // Document not found
    }

    char full_path[4096];
    snprintf(full_path, sizeof(full_path), "%s/%s", pathToFile, doc->path);

    // Check if the file exists
    struct stat buffer;
    if (stat(full_path, &buffer) != 0) {
        perror("File does not exist");
        return -1;
    }

    // Create a pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (pid == 0) {  
        // Child process
        close(pipefd[0]);  // Close read end
        
        // Redirect stdout to pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        
        // Execute grep and wc
        char cmd[4096];
        long unsigned int size = snprintf(cmd, sizeof(cmd), "grep \"%s\" \"%s\" | wc -l", keyword, full_path);
        if (size >= sizeof(cmd)) {
            fprintf(stderr, "Command too long\n");
            exit(EXIT_FAILURE);
        }
        execlp("sh", "sh", "-c", cmd, NULL);
        
        perror("execlp");
        exit(EXIT_FAILURE); // If execlp fails, exit child process

    } else {  // Parent process
        close(pipefd[1]);  // Close write end
        
        char result[32] = {0};
        ssize_t bytes_read = read(pipefd[0], result, sizeof(result) - 1);
        close(pipefd[0]);

        // Wait for child
        int status;
        waitpid(pid, &status, 0);
        
        if (bytes_read <= 0) {
            perror("read from pipe failed");
            return -1;
        }
        
        int count = atoi(result);
        if (count > 0) {
            printf("Keyword '%s' found in file: %s\n", keyword, full_path);
        }
        
        return count;
    }
}

int get_number_of_docs_with_keyword(char* keyword, int nr_process, int max_key) {
    int work_load = max_key / nr_process;
    int rest = max_key % nr_process;
    pid_t pids[nr_process];
    int pipes[nr_process][2]; // Array of pipes for communication
    int total_count = 0;

    for (int i = 0; i < nr_process; i++) {
        // Create pipe before fork
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return -1;
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return -1;
        }

        if (pid == 0) {  
            // Child process
            close(pipes[i][0]); // Close read end
            
            int start = i * work_load;
            int end = start + work_load;
            if (i == nr_process - 1) {
                end += rest; // Add the remainder to the last process
            }

            int docs_with_keyword = 0;
            for (int j = start; j < end; j++) {
                Document* doc = consult_document(j);
                int count = get_number_of_lines_with_keyword(doc, keyword);
                if (count > 0) {
                    printf("Document %d contains %d lines with keyword '%s'\n", j, count, keyword);
                    docs_with_keyword++;
                }
            }
            
            // Write the count to the pipe
            ssize_t bytes_written = write(pipes[i][1], &docs_with_keyword, sizeof(int));
            if (bytes_written != sizeof(int)) {
                perror("write to pipe failed");
                exit(1);
            }
            close(pipes[i][1]);
            exit(0);
        } else {  
            // Parent process
            close(pipes[i][1]); // Close write end
            pids[i] = pid;
        }
    }
    
    // Parent waits for all children to finish and collects results
    for (int i = 0; i < nr_process; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        
        int child_count = 0;
        ssize_t bytes_read = read(pipes[i][0], &child_count, sizeof(int));
        if (bytes_read != sizeof(int)) {
            perror("read from pipe failed");
            child_count = 0; // Set to 0 in case of error
        }
        close(pipes[i][0]);
        
        total_count += child_count;
        
        if (WIFEXITED(status)) {
            printf("Child process %d with pid %d finished with status %d\n",i, pids[i], WEXITSTATUS(status));
        } else { 
            printf("Child process %d with pid %d did not terminate normally\n",i, pids[i]);
        }
    }
    
    printf("Total documents containing keyword '%s': %d\n", keyword, total_count);
    return total_count;
}
