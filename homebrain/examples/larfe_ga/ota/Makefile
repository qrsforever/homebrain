CC := arm-linux-gcc
ota-test: ota.o
	$(CC) -o $@ $^
.c.o:
	$(CC) -c $<
clean:
	rm -rf *.o ota-test
