# Compiles Spider Robot program

TARGET = spider
TARGET2 = spiderleg

CROSS_COMPILE = arm-linux-gnueabihf-
CFLAGS = -g -Wall -std=gnu++11 -I ${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include
LDFLAGS =  -g -Wall  -lstdc++  -lrt
CC = $(CROSS_COMPILE)g++

all: $(TARGET) $(TARGET2)

server: server.o
	$(CC) $(LDFLAGS) $^ -o $@

client: client.o
	$(CC) $(LDFLAGS) $^ -o $@

$(TARGET): Main.o MMap.o LEDControl.o Motor.o
	$(CC) $(LDFLAGS)  $^ -o $@

$(TARGET2): MainSpider.o MMap.o LEDControl.o Motor.o SpiderLeg.o
	$(CC) $(LDFLAGS) $^ -o $@

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) $(TARGET2) *.a *.o *~
