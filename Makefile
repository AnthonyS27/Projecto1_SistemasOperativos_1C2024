CC=gcc
CFLAGS=-Wall -Wextra

all: programa_cpu programa_memoria programa_disco

programa_cpu: cpu.c
    $(CC) $(CFLAGS) cpu.c -o programa_cpu

programa_memoria: memoria.c
    $(CC) $(CFLAGS) memoria.c -o programa_memoria

programa_disco: disco.c
    $(CC) $(CFLAGS) disco.c -o programa_disco

clean:
    rm -f programa_cpu programa_memoria programa_disco


