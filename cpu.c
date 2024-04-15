#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_BUF 1024

// Función para leer estadísticas de la CPU para un proceso específico
void read_process_cpu_stats(int pid) {
    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (child_pid == 0) {
        // Proceso hijo
        char path[MAX_BUF];
        snprintf(path, MAX_BUF, "/proc/%d/stat", pid);

        FILE* file = fopen(path, "r");
        if (file == NULL) {
            perror("Failed to open /proc/[PID]/stat");
            exit(EXIT_FAILURE);
        }

        char comm[256]; // Buffer para el nombre del comando
        unsigned long utime, stime; // Tiempo de usuario y tiempo de sistema
        long int total_time;

        // Leemos los campos relevantes del archivo /proc/[PID]/stat
        if (fscanf(file, "%*d %s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %*lu %lu %lu", comm, &utime, &stime) != 3) {
            perror("Failed to read /proc/[PID]/stat");
            exit(EXIT_FAILURE);
        }

        fclose(file);

        // Calculamos el tiempo total de CPU utilizado por el proceso
        total_time = utime + stime;

        // Obtenemos el tiempo de CPU total del sistema
        file = fopen("/proc/stat", "r");
        if (file == NULL) {
            perror("Failed to open /proc/stat");
            exit(EXIT_FAILURE);
        }

        unsigned long long int total_cpu_time;
        char buffer[MAX_BUF];
        unsigned long long int user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;

        if (fgets(buffer, MAX_BUF, file) == NULL) {
            perror("Failed to read /proc/stat");
            exit(EXIT_FAILURE);
        }

        if (strncmp(buffer, "cpu ", 4) == 0) {
            sscanf(buffer, "%*s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);
            total_cpu_time = user + nice + system + idle + iowait + irq + softirq + steal;
        } else {
            perror("Failed to parse /proc/stat");
            exit(EXIT_FAILURE);
        }

        fclose(file);

        // Calculamos el porcentaje de utilización de CPU para el proceso
        float cpu_usage = ((float) total_time / (float) total_cpu_time) * 100.0;

        // Imprimimos la información del proceso
        printf("PID: %d\n", pid);
        printf("Command: %s\n", comm);
        printf("CPU Usage: %.2f%%\n", cpu_usage);

        exit(EXIT_SUCCESS);
    } else {
        // Proceso padre
        int status;
        waitpid(child_pid, &status, 0);
    }
}

// Función para leer estadísticas de la CPU para todos los procesos
void read_all_processes_cpu_stats() {
    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (child_pid == 0) {
        // Proceso hijo
        FILE* file = fopen("/proc/stat", "r");
        if (file == NULL) {
            perror("Failed to open /proc/stat");
            exit(EXIT_FAILURE);
        }

        char buffer[MAX_BUF];
        printf("CPU Statistics for All Processes:\n");
        printf("-----------------------------------\n");
        while (fgets(buffer, MAX_BUF, file) != NULL) {
            if (strncmp(buffer, "cpu ", 4) != 0) {
                printf("%s", buffer);
            }
        }

        fclose(file);

        exit(EXIT_SUCCESS);
    } else {
        // Proceso padre
        int status;
        waitpid(child_pid, &status, 0);
    }
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        read_all_processes_cpu_stats();
    } else if (argc == 2) {
        int pid = atoi(argv[1]);
        if (pid <= 0) {
            fprintf(stderr, "Invalid PID\n");
            return EXIT_FAILURE;
        }
        read_process_cpu_stats(pid);
    } else {
        fprintf(stderr, "Usage: %s [PID]\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
