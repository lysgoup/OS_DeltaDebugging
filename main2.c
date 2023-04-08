#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#define PIPE_READ_END 0
#define PIPE_WRITE_END 1

void timeout_handler(int sig) {
    printf("Child process timed out. Killing...\n");
    kill(getpid(), SIGKILL);
}

void read_input_file(char* filename, char* input_str, size_t max_size) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Unable to open input file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    size_t n = 0;
    int c;
    while ((c = fgetc(fp)) != EOF) {
        if (n >= max_size - 1) {
            fprintf(stderr, "Error: Input file %s is too large\n", filename);
            exit(EXIT_FAILURE);
        }
        input_str[n++] = (char) c;
    }
    input_str[n] = '\0';

    fclose(fp);
}

void write_output_file(char* filename, char* output_str) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        // File does not exist, create a new file
        fp = fopen(filename, "w+");
        if (fp == NULL) {
            fprintf(stderr, "Error: Unable to open output file %s\n", filename);
            exit(EXIT_FAILURE);
        }
    }

    if (fprintf(fp, "%s", output_str) < 0) {
        fprintf(stderr, "Error: Unable to write to output file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fclose(fp);
}

int main(int argc, char *argv[]) {
    char input_str[65536];
    char message[65536] = "";
    char output_str[65536] = "";
    char target_pro[20] = "";
    if (argc < 5 || (strcmp(argv[1], "-i") != 0 && strcmp(argv[1], "-m") != 0 && strcmp(argv[1], "-o") != 0)) {
        fprintf(stderr, "Usage: %s -i <input_file> -m <message> -o <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc-1; i += 2) {
        if (strcmp(argv[i], "-i") == 0) {
            read_input_file(argv[i+1], input_str, sizeof(input_str));
            printf("Input read from file: %s\n", input_str);
        } else if (strcmp(argv[i], "-m") == 0) {
            strcpy(message, argv[i+1]);
            printf("Message stored: %s\n", message);
        } else if (strcmp(argv[i], "-o") == 0) {
            strcpy(output_str, argv[i+1]);
        } else {
            fprintf(stderr, "Usage: %s -i <input_file> -m <message> -o <output_file>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    strcpy(target_pro, argv[argc-1]);
    printf("target program name: %s\n", target_pro);

    // Create unnamed pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Use fork to create a new process that duplicates the parent and child
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        // Close read end of pipe from parent process because child process will only be writing to the pipe
        close(pipefd[PIPE_READ_END]);
        // Redirect stdout and stderr to the pipe
        dup2(pipefd[PIPE_WRITE_END], STDOUT_FILENO);
        dup2(pipefd[PIPE_WRITE_END], STDERR_FILENO);
        // Replace the child process with the target program
        signal(SIGALRM, timeout_handler);
        // Set alarm for 3 seconds
        alarm(3);
        execlp("./build.sh", "./build.sh", NULL); // Replace with your target program's name
        exit(EXIT_FAILURE); // If exec() fails, exit with failure status

    } else {
        // Parent process
        close(pipefd[PIPE_WRITE_END]); // Close unused write end
        char buffer[65536];
        ssize_t num_read;
        while ((num_read = read(pipefd[PIPE_READ_END], buffer, sizeof(buffer))) > 0) {
            buffer[num_read] = '\0';
            printf("Received: %s", buffer);
            write_output_file(output_str, input_str);
            if (strstr(buffer, message) != NULL) {
                printf("Message found in stderr. Copying input to output file.\n");
                write_output_file(output_str, input_str);
                break;
            }
        }
        close(pipefd[PIPE_READ_END]);
        wait(NULL); // Wait for child process to finish
        exit(EXIT_SUCCESS);
    }
}