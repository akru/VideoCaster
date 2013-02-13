#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "jcomp.h"
#include "vgrab.h"

// UDP socket
int sock = -1;

static void signal_term_handler(int signum)
{
        fprintf(stderr, "\nTerminate...");

        // Close video device
        vgrab_close();

        // Close transport socket
        close(sock);

        fprintf(stderr,"ok\n");
        exit(EXIT_SUCCESS);
}


int main(int argc, char **argv)
{
        uint32_t            ts, ts_c, ts_j;
        struct timeval      now, timeout;
        int                 height, width, quality, fps,
                            bytes, delta, jitter;
        char                outbuf[65000], rcvbuf[65000];
        vgrab_buffer        *buf;
        struct sockaddr_in  addr;
        socklen_t fromlen = sizeof(struct sockaddr_in);

        if (argc < 6)
        {
                printf("Too few args!\n USAGE: vcast [DEVICE] [FPS] [XRES] [YRES] [QUALITY] [HOST] [PORT]\n");
                exit(1);
        }

        // Enabling OS signals
        signal(SIGINT, signal_term_handler);
        signal(SIGTERM, signal_term_handler);

        // Getting parametres
        fps =     atoi(argv[2]);
        width =   atoi(argv[3]);
        height =  atoi(argv[4]); 
        quality = atoi(argv[5]);

        // Init video device
        vgrab_init(argv[1], width, height);

        // Init transport protocol
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if(sock < 0)
        {
                perror("socket");
                exit(1);
        }
        addr.sin_family = AF_INET;
        addr.sin_port = htons(atoi(argv[7]));
        addr.sin_addr.s_addr = inet_addr(argv[6]);;
        // Enabling recv timeout
        timeout.tv_sec = 0; timeout.tv_usec = 1000000 / fps - 20000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, 
                        (const char*)&timeout, sizeof(struct timeval));

        // Main loop
        for (;;) {
                // Get current timestamp
                gettimeofday(&now, 0);
                ts = now.tv_sec * 1000 + (uint32_t) now.tv_usec / 1000;
                fprintf(stderr, "ts=%u ", ts);
                sprintf(outbuf, "%13u", ts);

                // Get next frame
                bytes = vgrab_get_frame(&buf);
                fprintf(stderr, "read=%d ", bytes);

                // Compress image
                bytes = jpeg_compress(buf->start, outbuf + 14, width, height, quality);
                fprintf(stderr, "compress=%d ", bytes);

                // Send image
                bytes = sendto(sock, &outbuf, bytes + 14, 0,
                                (struct sockaddr *)&addr, sizeof(addr));
                fprintf(stderr, "send=%d ", bytes);

                // Counts creation time
                gettimeofday(&now, 0);
                ts_c = now.tv_sec * 1000 + (int) now.tv_usec / 1000;
                delta = ts_c - ts;
                fprintf(stderr, "delta=%dms ", delta);

                // Receive reply (for jitter compensation)
                do
                    bytes = recvfrom(sock, &rcvbuf, sizeof(rcvbuf),
                                0, (struct sockaddr *)&addr, &fromlen);
                while (bytes > 0 && strcmp(outbuf, rcvbuf));

                // Counts jitter time
                gettimeofday(&now, 0);
                ts_j = now.tv_sec * 1000 + (int) now.tv_usec / 1000;
                jitter = ts_j - ts_c;
                fprintf(stderr, "jitter=%dms ", jitter);
                if (bytes < 0)
                        fprintf(stderr, "(timeout) ");

                // Make delay (FPS ~ 2)
                int delay = 1000000 / fps - (delta + jitter) * 1000;
                fprintf(stderr, "delay=%dus\n", delay);
                if (delay > 0) usleep(delay);
        }

        return 0;
}
