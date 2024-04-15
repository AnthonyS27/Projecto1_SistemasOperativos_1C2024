#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_BUF 1024

// Función para obtener el porcentaje de utilización de memoria virtual de todos los procesos
void get_virtual_memory_usage() {
    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (child_pid == 0) {
        FILE *file = fopen("/proc/meminfo", "r");
        if (file == NULL) {
            perror("Failed to open /proc/meminfo");
            exit(EXIT_FAILURE);
        }

        char buffer[MAX_BUF];
        unsigned long long total_memory = 0;
        unsigned long long free_memory = 0;

        while (fgets(buffer, MAX_BUF, file) != NULL) {
            if (strncmp(buffer, "MemTotal:", 9) == 0) {
                sscanf(buffer, "MemTotal: %llu kB", &total_memory);
            } else if (strncmp(buffer, "MemFree:", 8) == 0) {
                sscanf(buffer, "MemFree: %llu kB", &free_memory);
            }
        }

        fclose(file);

        // Calcular la utilización de memoria virtual
        float virtual_memory_usage = ((float)(total_memory - free_memory) / total_memory) * 100.0;

        printf("Virtual Memory Usage: %.2f%%\n", virtual_memory_usage);

        exit(EXIT_SUCCESS);
    } else {
        int status;
        waitpid(child_pid, &status, 0);
    }
}

// Función para obtener el porcentaje de utilización de memoria real de todos los procesos
void get_real_memory_usage() {
    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (child_pid == 0) {
        DIR *dir = opendir("/proc");
        if (dir == NULL) {
            perror("Failed to open /proc");
            exit(EXIT_FAILURE);
        }

        char buffer[MAX_BUF]; // <-- Variable buffer agregada aquí
        unsigned long long total_memory = 0; // <-- Variable total_memory agregada aquí

        printf("Real Memory Usage:\n");
        printf("-------------------\n");

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR) {
                char path[MAX_BUF];
                snprintf(path, MAX_BUF, "/proc/%s/status", entry->d_name);

                FILE *file = fopen(path, "r");
                if (file != NULL) {
                    char comm[256]; // Buffer para el nombre del proceso
                    unsigned long vmsize = 0;

                    // Leemos los campos relevantes del archivo /proc/[PID]/status
                    while (fgets(buffer, MAX_BUF, file) != NULL) { // <-- buffer utilizado aquí
                        if (strncmp(buffer, "Name:", 5) == 0) {
                            sscanf(buffer, "Name: %s", comm);
                        } else if (strncmp(buffer, "VmSize:", 7) == 0) {
                            sscanf(buffer, "VmSize: %lu kB", &vmsize);
                        }
                    }

                    fclose(file);

                    // Imprimir el PID, nombre y porcentaje de utilización de memoria real
                    if (vmsize > 0) {
                        printf("PID: %s\n", entry->d_name);
                        printf("Name: %s\n", comm);
                        printf("Virtual Memory Usage: %.2f%%\n", ((float)vmsize / total_memory) * 100.0);
                        printf("-------------------\n");
                    }
                }
            }
        }

        closedir(dir);

        exit(EXIT_SUCCESS);
    } else {
        int status;
        waitpid(child_pid, &status, 0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [-v/-r]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-v") == 0) {
        get_virtual_memory_usage();
    } else if (strcmp(argv[1], "-r") == 0) {
        get_real_memory_usage();
    } else {
        fprintf(stderr, "Invalid argument\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
