# Makefile
CC = gcc
DEBUG_FLAG = -O0
DEPLOY_FLAG = -O2
OPT_FLAG = $(DEPLOY_FLAG) # -O0 for use with GDB, -O2 for testing performance and is the default setting
CFLAGS = -Wall $(OPT_FLAG) -Werror -g3

all: runner heap_runner performance bump_test
# gprof_performance
support.o: support.c support.h
# I changed so csbrk is using my now
csbrk.o: csbrk.c csbrk.h
	$(CC) $(CFLAGS) -DTRACK_CSBRK -o csbrk.o -c csbrk.c
err_handler.o: err_handler.c err_handler.h 
# csbrk_tracked.o: csbrk.c csbrk.h
# 	$(CC) $(CFLAGS) -DTRACK_CSBRK -o csbrk_tracked.o -c csbrk.c
umalloc.o: umalloc.c umalloc.h
check_heap.o: check_heap.c check_heap.h

deploy: OPT_FLAG=$(DEPLOY_FLAG)
deploy: clean all

debug: OPT_FLAG=$(DEBUG_FLAG)
debug: clean all

runner: runner.c csbrk.o umalloc.o check_heap.o err_handler.o support.o
	$(CC) $(CFLAGS) -o runner runner.c  umalloc.h csbrk.o umalloc.o check_heap.o err_handler.o support.o

heap_runner: heap_runner.o check_heap.o csbrk.o umalloc.o err_handler.o support.o
	$(CC) $(CFLAGS) -o heap_runner heap_runner.o check_heap.o csbrk.o umalloc.o err_handler.o support.o

bump_test: bump_test.o check_heap.o csbrk.o umalloc.o err_handler.o support.o
	$(CC) $(CFLAGS) -o bump_test bump_test.o check_heap.o csbrk.o umalloc.o err_handler.o support.o

debug_heap_runner: OPT_FLAG=$(DEBUG_FLAG)
debug_heap_runner: heap_runner

bump_test.o: bump_test.c bump_test.h
	$(CC) $(CFLAGS) -c bump_test.c

heap_runner.o: heap_runner.c check_heap.h
	$(CC) $(CFLAGS) -c heap_runner.c

check_heap.o: check_heap.c check_heap.h
	$(CC) $(CFLAGS) -c check_heap.c

performance: performance.c csbrk.o umalloc.o support.o err_handler.o
	$(CC) $(CFLAGS) -o performance performance.c umalloc.h csbrk.o umalloc.o err_handler.o support.o

unittest: unittest.o support.o umalloc.o csbrk.o err_handler.o check_heap.o
	$(CC) $(CFLAGS) -o unittest unittest.c umalloc.h umalloc.o support.o csbrk.o err_handler.o check_heap.o

# GPROF
# gprof_csbrk.o: csbrk.c csbrk.h
# 	$(CC) -O0 -c -fprofile-arcs -g -pg -o gprof_csbrk.o csbrk.c 

# gprof_umalloc.o: umalloc.c umalloc.h
# 	$(CC) -O0 -c -fprofile-arcs -g -pg -o gprof_umalloc.o umalloc.c	

# gprof_performance: performance.c gprof_umalloc.o support.o gprof_csbrk.o
# 	$(CC) -O0 -fprofile-arcs -g -pg -o gprof_performance performance.c umalloc.h gprof_umalloc.o gprof_csbrk.o err_handler.o support.o

clean:
	rm -f *.so runner heap_runner bump_test gprof_performance performance *.gcda gmon.out \
		support.o err_handler.o umalloc.o check_heap.o gprof_umalloc.o heap_runner.o bump_test.o csbrk.o