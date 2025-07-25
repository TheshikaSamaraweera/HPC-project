# Makefile to build both serial and OpenMP chatbot versions

CC = gcc
CFLAGS = -Wall
OMP_FLAGS = -fopenmp

# Targets
TARGET_SERIAL = chatbot_serial
TARGET_OMP = chatbot_omp

# Source files
COMMON_SRCS = metrics.c deepseek_Chatbot.c

SERIAL_SRCS = main_serial.c $(COMMON_SRCS)
OMP_SRCS = main.c OMP_parallel.c $(COMMON_SRCS)

SERIAL_OBJS = $(SERIAL_SRCS:.c=.o)
OMP_OBJS = $(OMP_SRCS:.c=.o)

all: $(TARGET_SERIAL) $(TARGET_OMP)

$(TARGET_SERIAL): $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_OMP): $(OMP_OBJS)
	$(CC) $(CFLAGS) $(OMP_FLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(TARGET_SERIAL) $(TARGET_OMP)
