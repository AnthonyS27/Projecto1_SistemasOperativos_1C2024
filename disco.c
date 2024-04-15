#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/wait.h>

// Función para obtener y mostrar la utilización del disco
void get_disk_usage(char *option) {
    char command[50];
    if (strcmp(option, "fm") == 0) {
        strcpy(command, "df -h / | awk 'NR==2{print $4}'");
    } else if (strcmp(option, "um") == 0) {
        strcpy(command, "df -h / | awk 'NR==2{print $3}'");
    } else {
        fprintf(stderr, "Invalid argument\n");
        exit(EXIT_FAILURE);
    }

    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (child_pid == 0) {
        // Proceso hijo
        execl("/bin/sh", "sh", "-c", command, NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else {
        // Proceso padre
        int status;
        waitpid(child_pid, &status, 0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [fm/um]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *option = argv[1];

    if (strcmp(option, "fm") == 0 || strcmp(option, "um") == 0) {
        get_disk_usage(option);
    } else {
        fprintf(stderr, "Invalid argument\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
