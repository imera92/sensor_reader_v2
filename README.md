## Sistemas Operativos - Proyecto del 2do parcial, período 2018-2
Compilar el archivo del sensor de ejemplo (triple_sensor.c) y el lector de datos (threads_reader.c):
```
gcc -o reader threads_reader.c -lm -pthread
```
Luego ejecutar los archivos binarios. Los resultados (vehiculo/obstaculo) se iran almacenando en un archivo llamado *tmp_result.txt*.
Los parametros se pueden cambiar de forma interactiva desde el menu en la consola.
