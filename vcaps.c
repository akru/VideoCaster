#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
        FILE *outf;
        int sock, bytes_read;
        char buf[60000], filename[200], convertcmd[240];
        struct sockaddr_in addr;
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(struct sockaddr_in);

        if (argc < 3)
        {
                printf("Too few args!\n USAGE: vcaps [PORT] [DESTDIR]\n");
                exit(1);
        }

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

        for (;;) 
        {
            bytes_read = recvfrom(sock, buf, sizeof(buf), 
                        0, (struct sockaddr *)&from, &fromlen); 

            sprintf(filename, "%s/%s.ppm", argv[2], buf);
            
            fprintf(stderr,"Received %d bytes :: %s timestamp\n", bytes_read, buf);

            outf = fopen(filename, "w");
            fwrite(buf + 32, 1, bytes_read - 32, outf);
            fclose(outf);

            sprintf(convertcmd, "convert %s %s.png", filename, filename);
            system(convertcmd);
        }
        return 0;
}
