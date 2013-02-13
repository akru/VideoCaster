#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <libmemcached/memcached.h>

// UDP-socket
static int          sock = -1;
// Memcached connection
static memcached_st *memc;

static void signal_term_handler(int signum)
{
        fprintf(stderr, "\nTerminate...");

        // Close transport socket
        close(sock);

        // Free memcached
        memcached_free(memc);

        fprintf(stderr, "ok\n");
        exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
        int sock, bytes;
        char buf[65000];
        struct sockaddr_in addr;
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(struct sockaddr_in);

        if (argc < 3)
        {
                printf("Too few args!\n USAGE: vserv [PORT] [UUID]\n");
                exit(1);
        }

        // Enabling OS signals
        signal(SIGINT, signal_term_handler);
        signal(SIGTERM, signal_term_handler);

        // Save UUID
        char *uuid = argv[2];
        int uuid_len = strlen(uuid);

        // Save UUID-ts
        char uuid_ts[40];
        sprintf(uuid_ts, "%s-ts", uuid);
        int uuid_ts_len = strlen(uuid_ts);

        // Init transport protocol
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if(sock < 0)
        {
                perror("socket");
                exit(1);
        }
        addr.sin_family = AF_INET;
        addr.sin_port = htons(atoi(argv[1]));
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
                perror("bind");
                exit(2);
        }

        // Create memcached connection
        memc = memcached_create(NULL);
        memcached_server_add(memc, "localhost", 11211);

        for (;;) 
        {
                // Receive packet
                bytes = recvfrom(sock, &buf, sizeof(buf),
                                0, (struct sockaddr *)&from, &fromlen);

                // Send reply (for jitter compensation)
                sendto(sock, &buf, 14, 0,
                                (struct sockaddr *)&from, sizeof(from));

                // Set timestamp
                memcached_set(memc, uuid_ts, uuid_ts_len, buf, 14, 0, 0);

                fprintf(stderr, "Received %d bytes :: timestamp %s\n", bytes, buf);

                // Set image
                memcached_set(memc, uuid, uuid_len, buf + 14, bytes - 14, 0, 0);
        }
        return 0;
}
