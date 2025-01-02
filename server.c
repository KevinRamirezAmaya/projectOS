#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>

#define PORT 9001
#define BUFFER_SIZE 1024

int main() {
    int servSockD, clientSockD;
    struct sockaddr_in servAddr;
    char buffer[BUFFER_SIZE];

    // Crear el socket del servidor
    servSockD = socket(AF_INET, SOCK_STREAM, 0);
    if (servSockD == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(PORT);

    // Asociar el socket a la dirección y puerto configurados
    if (bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("Bind failed");
        close(servSockD);
        exit(EXIT_FAILURE);
    }

    // Poner el servidor en modo de escucha
    if (listen(servSockD, 3) < 0) {
        perror("Listen failed");
        close(servSockD);
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando en el puerto %d...\n", PORT);

    // Aceptar una conexión entrante del cliente
    clientSockD = accept(servSockD, NULL, NULL);
    if (clientSockD < 0) {
        perror("Accept failed");
        close(servSockD);
        exit(EXIT_FAILURE);
    }

    printf("Cliente conectado.\n");

    // Bucle para recibir y procesar comandos
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSockD, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            printf("Cliente desconectado.\n");
            break;
        }

        // Verificar si el cliente desea cerrar la conexión
        if (strcmp(buffer, "salida\n") == 0) {
            printf("Cerrando conexión.\n");
            break;
        }

        printf("Comando recibido: %s", buffer);

        // Crear un proceso hijo para manejar el comando
        pid_t pid = fork();
        if (pid == 0) {
            // Código del proceso hijo
            printf("Proceso hijo creado. PID: %d\n", getpid()); // Mensaje de depuración
            FILE *fp = popen(buffer, "r");
            if (fp == NULL) {
                perror("Error ejecutando el comando");
                send(clientSockD, "Error ejecutando el comando\n", 26, 0);
                exit(EXIT_FAILURE);
            }

            // Capturar la salida del comando
            char result[BUFFER_SIZE] = {0};
            fread(result, sizeof(char), BUFFER_SIZE, fp);
            pclose(fp);

            // Enviar la salida al cliente
            send(clientSockD, result, strlen(result), 0);

            // Terminar el proceso hijo
            printf("Proceso hijo PID %d finalizado.\n", getpid()); // Mensaje de depuración
            exit(0);
        } else if (pid > 0) {
            // Código del proceso padre: esperar al hijo
            printf("Proceso padre esperando al hijo con PID: %d\n", pid); // Mensaje de depuración
            wait(NULL);
        } else {
            perror("Error al crear el proceso hijo");
        }
    }

    // Cerrar sockets
    close(clientSockD);
    close(servSockD);
    return 0;
}
