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
