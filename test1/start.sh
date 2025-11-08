

export LD_LIBRARY_PATH=./lib/

gcc server.c -o server -lpthread

make

./MQTT_Demo.o
