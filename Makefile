# single_chan_pkt_fwd
# Single Channel LoRaWAN Gateway

CC = g++
CFLAGS = -std=c++11 -c -Wall
LIBS = -lwiringPi -lmariadb -L /usr/lib/arm-linux-gnueabihf/
INC = -I include/ -I /usr/include/mariadb -I /usr/include/mariadb/mysql

all: single_chan_pkt_fwd

single_chan_pkt_fwd: LoRa.o single_chan_pkt_fwd.o
	$(CC) LoRa.o single_chan_pkt_fwd.o $(LIBS) $(INC) -o single_chan_pkt_fwd

single_chan_pkt_fwd.o: single_chan_pkt_fwd.cpp
	$(CC) $(CFLAGS) $(INC) single_chan_pkt_fwd.cpp

LoRa.o: LoRa.cpp
	$(CC) $(CFLAGS) LoRa.cpp

clean:
	rm *.o single_chan_pkt_fwd

install:
	sudo cp -f ./single_chan_pkt_fwd.service /lib/systemd/system/
	sudo systemctl enable single_chan_pkt_fwd.service
	sudo systemctl daemon-reload
	sudo systemctl start single_chan_pkt_fwd
	sudo systemctl status single_chan_pkt_fwd -l

uninstall:
	sudo systemctl stop single_chan_pkt_fwd
	sudo systemctl disable single_chan_pkt_fwd.service
	sudo rm -f /lib/systemd/system/single_chan_pkt_fwd.service