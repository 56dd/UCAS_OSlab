CC := gcc
TARGET := pktRxTx
SRC := $(wildcard ./Source/*.c)
CFLAGS := -g -O0
LFLAGS := -IInclude -lpcap -lpthread

.PHONY: all
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

.PHONY: clean
clean:
	rm -f ./Source/*.o $(TARGET)
