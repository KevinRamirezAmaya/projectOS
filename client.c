#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9001
#define BUFFER_SIZE 1024

int main() {
    // Declaración de variables principales del cliente
    int sockD;
    struct sockaddr_in servAddr;
    char buffer[BUFFER_SIZE];

    // Crear el socket del cliente
    sockD = socket(AF_INET, SOCK_STREAM, 0);
    if (sockD == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor al que se conectará
    servAddr.sin_family = AF_INET; // Familia de direcciones IPv4
    servAddr.sin_port = htons(PORT); // Puerto del servidor
    servAddr.sin_addr.s_addr = INADDR_ANY; // Dirección del servidor

    // Conectarse al servidor
    if (connect(sockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("Connection failed");
        close(sockD);
        exit(EXIT_FAILURE);
    }

    printf("Conectado al servidor. Escribe comandos o 'salida' para terminar.\n");

    // Bucle para enviar comandos al servidor
    while (1) {
        printf(">> ");
        fgets(buffer, BUFFER_SIZE, stdin); // Leer comando del usuario

        send(sockD, buffer, strlen(buffer), 0); // Enviar comando al servidor

        // Verificar si el usuario desea cerrar la conexión
        if (strcmp(buffer, "salida\n") == 0) {
            printf("Cerrando conexión.\n");
            break;
        }

        // Recibir la respuesta del servidor
        memset(buffer, 0, BUFFER_SIZE); // Limpiar el buffer
        int bytesReceived = recv(sockD, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            printf("Servidor desconectado.\n");
            break;
        }

        // Mostrar la respuesta al usuario
        printf("Respuesta del servidor:\n%s\n", buffer);
    }

    // Cerrar el socket
    close(sockD);
    return 0;
}