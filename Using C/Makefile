
###### for serial version ######

CC = gcc
CFLAGS = -Wall
LIBS = -lcurl -lcjson

OBJS = deepseek_Chatbot.o metrics.o

all: chatbot

chatbot: $(OBJS)
	$(CC) -o deepseek_chatbot $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o deepseek_chatbot
########################################

###############for parallel version using OpenMP ###############
# CC = gcc
# CFLAGS = -Wall -fopenmp
# LIBS = -lcurl -lcjson

# OBJS = deepseek_omp_Chatbot.o metrics.o omp_parallel.o

# all: parallel_chatbot

# parallel_chatbot: $(OBJS)
# 	$(CC) -o $@ $(OBJS) $(LIBS)

# %.o: %.c
# 	$(CC) $(CFLAGS) -c $<

# clean:
# 	rm -f *.o parallel_chatbot
