# Controlador de Dispositivo de Carácter para LED en GPIO 23

## Descripción del Proyecto

Este proyecto implementa un controlador de dispositivo de carácter para Linux que permite controlar un LED conectado al pin GPIO 23 desde aplicaciones de usuario. El sistema consta de dos componentes principales:

1. **Módulo del kernel**: Driver que gestiona el hardware directamente
2. **Aplicación de usuario**: Interfaz para controlar el LED mediante un menú interactivo

## Estructura del Proyecto
gpio_led_control/
├── driver/ # Código del controlador del kernel
│ ├── gpio_led_driver.c # Implementación del driver
│ └── Makefile # Script de compilación del módulo
│
├── user_app/ # Aplicación de espacio de usuario
│ ├── led_control.c # Programa de control interactivo
│ └── Makefile # Script de compilación
│
└── README.md # Documentación principal

text

## Explicación Detallada del Driver

### 1. Inicialización del Módulo

Cuando el módulo se carga con `insmod`, se ejecuta la función `led_init()` que realiza:

1. **Registro del dispositivo de carácter**:
   - Solicita un número mayor dinámico al kernel
   - Crea una estructura cdev para representar el dispositivo
   - Registra las operaciones de archivo soportadas

2. **Configuración del sistema de archivos**:
   - Crea una clase de dispositivo en sysfs
   - Genera el nodo del dispositivo en /dev automáticamente

3. **Configuración del GPIO**:
   - Verifica que el GPIO 23 esté disponible
   - Solicita el control exclusivo del pin
   - Configura el pin como salida
   - Establece el estado inicial en apagado (0)

### 2. Operaciones del Dispositivo

El driver implementa estas operaciones básicas:

- **open()**: Se ejecuta cuando un proceso abre el dispositivo. Registra el acceso en el log del kernel.

- **release()**: Se llama al cerrar el dispositivo. Limpia los recursos del proceso.

- **read()**: Permite consultar el estado actual del LED. Devuelve '1' (encendido) o '0' (apagado).

- **write()**: Controla el LED. Acepta:
  - '1' para encender el LED
  - '0' para apagarlo
  - Otros valores son rechazados

### 3. Gestión del GPIO

El driver utiliza las funciones estándar del kernel para:
- Verificar disponibilidad del pin (gpio_is_valid)
- Reservar el GPIO (gpio_request)
- Configurar dirección (gpio_direction_output)
- Cambiar estado (gpio_set_value)

### 4. Limpieza del Módulo

Al ejecutar `rmmod`, `led_exit()` realiza:
1. Apaga el LED como medida de seguridad
2. Libera el control del GPIO
3. Elimina el nodo del dispositivo
4. Desregistra el número mayor
5. Libera la clase de dispositivo

## Instrucciones de Instalación

### Requisitos Previos
- Sistema Linux con headers del kernel instalados
- Acceso root para cargar módulos
- GPIO 23 disponible y sin usar

### Pasos de Implementación

Compilación del Módulo del Kernel  
cd driver  
make  

Carga del Módulo  
sudo insmod gpio_led_driver.ko  

Verificar que el Dispositivo fue Creado  
dmesg | tail  
ls -l /dev/gpio_led  

Si no existe /dev/gpio_led, se crea manualmente:  
sudo mknod /dev/gpio_led c $(grep gpio_led /proc/devices | awk '{print $1}') 0  
sudo chmod 666 /dev/gpio_led  

Compilación de la Aplicación de Usuario  
cd ../user_app  
make  

Ejecución de la Aplicación  
sudo ./led_control  

Diagrama de Funcionamiento  
+-------------------+     +-------------------+     +-------+  
| Aplicación Usuario| <-> | Dispositivo /dev  | <-> | GPIO  |  
| (led_control)     |     | (gpio_led)        |     |  23   |  
+-------------------+     +-------------------+     +-------+  
       ↑                                             ↓  
       |                                    +---------------------+  
       +------------------------------------| LED con resistencia |  
                                            +---------------------+
