CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude -D_XOPEN_SOURCE=700

SRC = src/thread.c \
      src/queue.c \
      src/sched.c \
      src/timer.c \
      src/sync.c

OBJ = $(SRC:.c=.o)

DEMOS = basic_threads demo_person1
TESTS = test_queue test_lifecycle test_block test_validation

.PHONY: all test clean

all: $(DEMOS)

basic_threads: $(OBJ) demos/basic_threads.c
	$(CC) $(CFLAGS) $(OBJ) demos/basic_threads.c -o basic_threads

demo_person1: $(OBJ) demos/demo_person1.c
	$(CC) $(CFLAGS) $(OBJ) demos/demo_person1.c -o demo_person1

# Person 3 targets (available when implemented)
race_condition: $(OBJ) demos/race_condition.c
	$(CC) $(CFLAGS) $(OBJ) demos/race_condition.c -o race_condition

producer_consumer: $(OBJ) demos/producer_consumer.c
	$(CC) $(CFLAGS) $(OBJ) demos/producer_consumer.c -o producer_consumer

# Person 1 Unit Tests
test_queue: src/queue.o tests/test_queue.c
	$(CC) $(CFLAGS) src/queue.o tests/test_queue.c -o test_queue

test_lifecycle: $(OBJ) tests/test_lifecycle.c
	$(CC) $(CFLAGS) $(OBJ) tests/test_lifecycle.c -o test_lifecycle

test_block: $(OBJ) tests/test_block.c
	$(CC) $(CFLAGS) $(OBJ) tests/test_block.c -o test_block

test_validation: $(OBJ) tests/test_validation.c
	$(CC) $(CFLAGS) $(OBJ) tests/test_validation.c -o test_validation

test: $(TESTS)
	@echo "=========================================="
	@echo " Running Person 1 Test Suite"
	@echo "=========================================="
	./test_queue
	./test_lifecycle
	./test_block
	./test_validation
	@echo "=========================================="
	@echo " ALL PERSON 1 TESTS PASSED SUCCESSFULLY! "
	@echo "=========================================="

clean:
	rm -rf $(OBJ) $(DEMOS) $(TESTS) race_condition producer_consumer *.dSYM