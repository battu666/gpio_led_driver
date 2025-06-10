# Objeto a construir (agrega .o automáticamente)
obj-m := gpio_led_driver.o

# Ruta al directorio de construcción del kernel
KDIR := /lib/modules/$(shell uname -r)/build

# Directorio actual
PWD := $(shell pwd)

# Regla por defecto (compilación)
all:
	# Invoca make en el directorio del kernel con nuestra ruta
	make -C $(KDIR) M=$(PWD) modules

# Regla de limpieza
clean:
	# Limpia los archivos generados
	make -C $(KDIR) M=$(PWD) clean