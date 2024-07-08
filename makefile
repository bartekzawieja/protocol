CC     = gcc
CFLAGS = -Wall -Wextra -O2 -std=gnu17

TARGETS = ppcbs ppcbc
all: $(TARGETS)

ppcbs: ppcbs.o common.o udp_server_helper.o tcp_server_helper.o
	$(CC) $(CFLAGS) $^ -o $@

ppcbc: ppcbc.o common.o udp_client_helper.o tcp_client_helper.o client_buffering_helper.o
	$(CC) $(CFLAGS) $^ -o $@

ppcbs.o: server.c server.h common.h protconst.h size_const.h packets.h server_structures.h tcp_server_helper.h udp_server_helper.h
	$(CC) $(CFLAGS) -c server.c -o ppcbs.o

ppcbc.o: client.c client.h common.h protconst.h size_const.h packets.h client_structures.h tcp_client_helper.h udp_client_helper.h client_buffering_helper.h
	$(CC) $(CFLAGS) -c client.c -o ppcbc.o

common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c -o common.o

client_buffering_helper.o: client_buffering_helper.c client_buffering_helper.h client_structures.h
	$(CC) $(CFLAGS) -c client_buffering_helper.c -o client_buffering_helper.o

tcp_server_helper.o: tcp_server_helper.c tcp_server_helper.h common.h packets.h server_structures.h
	$(CC) $(CFLAGS) -c tcp_server_helper.c -o tcp_server_helper.o

udp_server_helper.o: udp_server_helper.c udp_server_helper.h common.h packets.h server_structures.h
	$(CC) $(CFLAGS) -c udp_server_helper.c -o udp_server_helper.o

tcp_client_helper.o : tcp_client_helper.c tcp_client_helper.h common.h packets.h client_structures.h
	$(CC) $(CFLAGS) -c tcp_client_helper.c -o tcp_client_helper.o

udp_client_helper.: udp_client_helper.c udp_client_helper.h common.h packets.h client_structures.h
	$(CC) $(CFLAGS) -c udp_client_helper.c -o udp_client_helper.o

clean:
	rm -f $(TARGETS) *.o
