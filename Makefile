CC ?= gcc
CFLAGS = -std=gnu99 -g

TARGET = test

OBJS = tarfs.o test.o

include common.mk

all: $(TARGET)

$(TARGET): $(OBJS)
	$(VECHO) "   LD\t$@\n"
	$(Q)$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(VECHO) "   CC\t$@\n"
	$(Q)$(CC) -c $< -o $@ $(CFLAGS)

clean:
	$(RM) $(OBJS)
.PHONY: clean
