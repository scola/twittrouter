# build twittrouter executable when user executes "make"
CC=gcc
LDFLAGS=-lpthread -lm -lcurl
CFLAGS=-std=gnu99 -Wall
twittrouter: twittrouter.o TCPServerUtility.o DieWithMessage.o AddressUtility.o utils.o jconf.o json.o arplinklist.o twitter.o hash.o oauth.o xmalloc.o
	$(CC) $(LDFLAGS) twittrouter.o TCPServerUtility.o DieWithMessage.o AddressUtility.o utils.o jconf.o json.o arplinklist.o twitter.o hash.o oauth.o xmalloc.o -o twittrouter
twittrouter.o: twittrouter.c
	$(CC) $(CFLAGS) -c twittrouter.c
TCPServerUtility.o: TCPServerUtility.c
	$(CC) $(CFLAGS) -c TCPServerUtility.c
DieWithMessage.o: DieWithMessage.c
	$(CC) $(CFLAGS) -c DieWithMessage.c
AddressUtility.o: AddressUtility.c
	$(CC) $(CFLAGS) -c AddressUtility.c
utils.o: utils.c
	$(CC) $(CFLAGS) -c utils.c
jconf.o: jconf.c
	$(CC) $(CFLAGS) -c jconf.c
json.o: json.c
	$(CC) $(CFLAGS) -c json.c
arplinklist.o: arplinklist.c
	$(CC) $(CFLAGS) -c arplinklist.c
twitter.o: twitter.c
	$(CC) $(CFLAGS) -c twitter.c
hash.o: hash.c
	$(CC) $(CFLAGS) -c hash.c
oauth.o: oauth.c
	$(CC) $(CFLAGS) -c oauth.c
xmalloc.o: xmalloc.c
	$(CC) $(CFLAGS) -c xmalloc.c

# remove object files and executable when user executes "make clean"
clean:
	rm *.o twittrouter
