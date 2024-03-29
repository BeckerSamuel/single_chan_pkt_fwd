# single_chan_gateway
# Single Channel LoRaWAN Gateway

CC = g++
CFLAGS = -std=c++11 -c -Wall
LIBS = -lwiringPi -lmariadb -L /usr/lib/arm-linux-gnueabihf/
INC = -I include/rapidjson/ -I include/wiringPi/ -I /usr/include/mariadb/ -I /usr/include/mariadb/mysql/

all: single_chan_gateway

single_chan_gateway: wiringPiSPIAdjusted.o LoRa.o single_chan_gateway.o
	$(CC) wiringPiSPIAdjusted.o LoRa.o single_chan_gateway.o $(LIBS) $(INC) -o single_chan_gateway

single_chan_gateway.o: single_chan_gateway.cpp LoRa.h
	$(CC) $(CFLAGS) $(INC) single_chan_gateway.cpp

LoRa.o: LoRa.cpp LoRa.h
	$(CC) $(CFLAGS)  $(INC) LoRa.cpp

wiringPiSPIAdjusted.o: wiringPiSPIAdjusted.c wiringPiSPIAdjusted.h
	$(CC) $(CFLAGS) $(INC) wiringPiSPIAdjusted.c

clean:
	rm *.o single_chan_gateway

install:
	sudo cp -f ./single_chan_gateway.service /lib/systemd/system/
	sudo systemctl enable single_chan_gateway.service
	sudo systemctl daemon-reload
	sudo systemctl start single_chan_gateway
	sudo systemctl status single_chan_gateway -l

uninstall:
	sudo systemctl stop single_chan_gateway
	sudo systemctl disable single_chan_gateway.service
	sudo rm -f /lib/systemd/system/single_chan_gateway.service