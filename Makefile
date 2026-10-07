CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude -D_XOPEN_SOURCE=700

SRC = src/thread.c \
      src/queue.c \
      src/sched.c \
      src/timer.c \
      src/sync.c

OBJ = $(SRC:.c=.o)

all: basic_threads race_condition producer_consumer preemption_demo

basic_threads: $(OBJ) demos/basic_threads.c
	$(CC) $(CFLAGS) $(OBJ) demos/basic_threads.c -o basic_threads

race_condition: $(OBJ) demos/race_condition.c
	$(CC) $(CFLAGS) $(OBJ) demos/race_condition.c -o race_condition

producer_consumer: $(OBJ) demos/producer_consumer.c
	$(CC) $(CFLAGS) $(OBJ) demos/producer_consumer.c -o producer_consumer

preemption_demo: $(OBJ) demos/preemption_demo.c
	$(CC) $(CFLAGS) $(OBJ) demos/preemption_demo.c -o preemption_demo

clean:
	rm -f $(OBJ) basic_threads race_condition producer_consumer preemption_demo