CC := arm-linux-gcc
hostdiscovery: hostdiscovery.o
	$(CC) -o $@ $^
.c.o:
	$(CC) -c $<
clean:
	rm -rf *.o hostdiscovery 
