#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libmemcached/memcached.h>
#include "proto.h"

int main(int argc, char **argv)
{
        int sock, bytes_read;
        vpkg buf;
        struct sockaddr_in addr;
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(struct sockaddr_in);

        if (argc < 3)
        {
                printf("Too few args!\n USAGE: vcaps [PORT] [UUID]\n");
                exit(1);
        }

        // Save UUID
        char *uuid = argv[2];
        int uuid_len = strlen(uuid);

        // Save UUID-ts
        char uuid_ts[40];
        sprintf(uuid_ts, "%s-ts", uuid);
        int uuid_ts_len = strlen(uuid_ts);



        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if(sock < 0)
        {
                perror("socket");
                exit(1);
        }

        // Bind UDP socket
        addr.sin_family = AF_INET;
        addr.sin_port = htons(atoi(argv[1]));
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
                perror("bind");
                exit(2);
        }

        // Create memcached connection
        memcached_st *memc = memcached_create(NULL);
        memcached_server_add(memc, "localhost", 11211);

        for (;;) 
        {
            bytes_read = recvfrom(sock, &buf, sizeof(buf),
                        0, (struct sockaddr *)&from, &fromlen); 

            // Set timestamp
            memcached_set(memc, uuid_ts, uuid_ts_len, 
                            buf.timestamp, sizeof(buf.timestamp), 0, 0);
            
            fprintf(stderr, "Received %d bytes :: timestamp %s\n", bytes_read, buf.timestamp);
            
            // Set image
            memcached_set(memc, uuid, uuid_len, 
                            buf.image, bytes_read - sizeof(buf.timestamp), 0, 0);
        }
        return 0;
}
