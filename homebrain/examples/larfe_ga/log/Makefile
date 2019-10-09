CC := arm-linux-gcc
log-test: log.o
	$(CC) -o $@ $^
.c.o:
	$(CC) -c $<
clean:
	rm -rf *.o log-test
