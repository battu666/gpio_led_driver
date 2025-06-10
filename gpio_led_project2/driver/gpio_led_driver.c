#include <linux/module.h>       // Para módulos del kernel
#include <linux/fs.h>           // Para operaciones de archivo
#include <linux/uaccess.h>      // Para copy_to/from_user
#include <linux/gpio.h>         // Para control GPIO
#include <linux/cdev.h>         // Para dispositivos de carácter
#include <linux/device.h>       // Para sysfs y clases de dispositivo

#define DEVICE_NAME "gpio_led"  // Nombre del dispositivo
#define GPIO_LED 23             // GPIO a usar (23)

static int major;               // Número mayor asignado
static struct class *led_class; // Puntero a la clase del dispositivo
static struct cdev led_cdev;    // Estructura del dispositivo de carácter

// Función que se ejecuta al abrir el dispositivo
static int led_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "GPIO_LED: Device opened\n");  // Log de apertura
    return 0;  // Retorno exitoso
}

// Función que se ejecuta al cerrar el dispositivo
static int led_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "GPIO_LED: Device closed\n");  // Log de cierre
    return 0;  // Retorno exitoso
}

// Función para leer el estado del LED
static ssize_t led_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    char state;
    int value = gpio_get_value(GPIO_LED);  // Obtiene estado actual del GPIO
    
    state = (value) ? '1' : '0';  // Convierte a carácter '1' o '0'
    
    if (copy_to_user(buf, &state, 1)) {  // Copia a espacio de usuario
        return -EFAULT;  // Error si falla la copia
    }
    
    return 1;  // Retorna 1 byte leído
}

// Función para escribir comandos al LED
static ssize_t led_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    char command;
    
    if (copy_from_user(&command, buf, 1)) {  // Copia comando desde usuario
        return -EFAULT;  // Error si falla la copia
    }
    
    if (command == '0') {
        gpio_set_value(GPIO_LED, 0);  // Apaga el LED
        printk(KERN_INFO "GPIO_LED: LED turned OFF\n");
    } else if (command == '1') {
        gpio_set_value(GPIO_LED, 1);  // Enciende el LED
        printk(KERN_INFO "GPIO_LED: LED turned ON\n");
    } else {
        printk(KERN_WARNING "GPIO_LED: Invalid command '%c'\n", command);
        return -EINVAL;  // Comando inválido
    }
    
    return 1;  // Retorna 1 byte escrito
}

// Estructura con las operaciones del dispositivo
static struct file_operations fops = {
    .open = led_open,     // Función open
    .release = led_release, // Función close
    .read = led_read,     // Función read
    .write = led_write,   // Función write
};

// Función de inicialización del módulo
static int __init led_init(void)
{
    dev_t dev;
    int ret;
    
    // 1. Registrar dispositivo de carácter
    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ALERT "GPIO_LED: Failed to allocate major number\n");
        return ret;
    }
    major = MAJOR(dev);  // Guarda el número mayor asignado
    
    // 2. Inicializar estructura cdev
    cdev_init(&led_cdev, &fops);
    ret = cdev_add(&led_cdev, dev, 1);
    if (ret < 0) {
        printk(KERN_ALERT "GPIO_LED: Failed to add cdev\n");
        unregister_chrdev_region(dev, 1);
        return ret;
    }
    
    // 3. Crear clase de dispositivo
    led_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(led_class)) {
        printk(KERN_ALERT "GPIO_LED: Failed to create class\n");
        cdev_del(&led_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(led_class);
    }
    
    // 4. Crear nodo del dispositivo
    device_create(led_class, NULL, dev, NULL, DEVICE_NAME);
    
    // 5. Configurar GPIO
    if (gpio_is_valid(GPIO_LED) == false) {
        printk(KERN_ALERT "GPIO_LED: Invalid GPIO %d\n", GPIO_LED);
        return -ENODEV;
    }
    
    ret = gpio_request(GPIO_LED, "LED_GPIO");
    if (ret) {
        printk(KERN_ALERT "GPIO_LED: Failed to request GPIO %d\n", GPIO_LED);
        return ret;
    }
    
    ret = gpio_direction_output(GPIO_LED, 0);
    if (ret) {
        printk(KERN_ALERT "GPIO_LED: Failed to set GPIO direction\n");
        gpio_free(GPIO_LED);
        return ret;
    }
    
    printk(KERN_INFO "GPIO_LED: Module initialized, LED on GPIO %d\n", GPIO_LED);
    return 0;
}

// Función de limpieza del módulo
static void __exit led_exit(void)
{
    dev_t dev = MKDEV(major, 0);
    
    // 1. Apagar LED y liberar GPIO
    gpio_set_value(GPIO_LED, 0);
    gpio_free(GPIO_LED);
    
    // 2. Eliminar dispositivo
    device_destroy(led_class, dev);
    class_destroy(led_class);
    cdev_del(&led_cdev);
    unregister_chrdev_region(dev, 1);
    
    printk(KERN_INFO "GPIO_LED: Module unloaded\n");
}

// Macros para especificar funciones de inicio/salida
module_init(led_init);  // Función a ejecutar al cargar
module_exit(led_exit);  // Función a ejecutar al descargar

// Metadatos del módulo
MODULE_LICENSE("GPL");  // Licencia GPL
MODULE_AUTHOR("Tu Nombre");  // Autor
MODULE_DESCRIPTION("Simple GPIO LED driver");  // Descripción