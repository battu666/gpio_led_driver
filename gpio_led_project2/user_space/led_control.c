#include <stdio.h>      // Para entrada/salida estándar
#include <stdlib.h>     // Para EXIT_SUCCESS/FAILURE
#include <unistd.h>     // Para close()
#include <fcntl.h>      // Para open() y flags O_RDWR

#define DEVICE_PATH "/dev/gpio_led"  // Ruta del dispositivo

int main() {
    int fd;             // Descriptor de archivo
    char choice;        // Opción del usuario
    char state;         // Estado del LED
    
    // 1. Abrir dispositivo
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");  // Error si falla
        return EXIT_FAILURE;
    }
    
    // 2. Bucle principal
    while (1) {
        // Mostrar menú
        printf("\nMenu:\n");
        printf("1 - Encender LED\n");
        printf("0 - Apagar LED\n");
        printf("2 - Salir\n");
        printf("Seleccione una opcion: ");
        
        // Leer opción
        scanf(" %c", &choice);
        
        // Procesar opción
        if (choice == '2') {
            break;  // Salir del bucle
        } else if (choice == '0' || choice == '1') {
            // Escribir comando al dispositivo
            if (write(fd, &choice, 1) < 0) {
                perror("Failed to write to the device");
                close(fd);
                return EXIT_FAILURE;
            }
            
            // Leer estado para confirmación
            if (read(fd, &state, 1) < 0) {
                perror("Failed to read from the device");
                close(fd);
                return EXIT_FAILURE;
            }
            
            // Mostrar estado actual
            printf("Estado actual del LED: %s\n", 
                  (state == '1') ? "ENCENDIDO" : "APAGADO");
        } else {
            printf("Opcion no valida. Intente de nuevo.\n");
        }
    }
    
    // 3. Cerrar dispositivo y salir
    close(fd);
    return EXIT_SUCCESS;
}