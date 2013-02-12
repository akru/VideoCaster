#include <ortp/payloadtype.h>
#include <ortp/ortp.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "jcomp.h"
#include "vgrab.h"

RtpSession *session;

static void signal_term_handler(int signum)
{
        fprintf(stderr, "Terminate...\n");

        // Close video device
        vgrab_close();

        // Destroy transport session
        rtp_session_destroy(session);
        ortp_exit();
        ortp_global_stats_display();

        fprintf(stderr,"\nok\n");
        exit(EXIT_SUCCESS);
}


int main(int argc, char **argv)
{
        struct timeval now;
        int            height, width, quality; 
        uint32_t       user_ts = 0;
        char           outbuf[65000];
        vgrab_buffer   *buf;

        if (argc < 6)
        {
                printf("Too few args!\n USAGE: vcast [DEVICE] [XRES] [YRES] [QUALITY] [HOST] [PORT]\n");
                exit(1);
        }

        // Enabling OS signals
        signal(SIGINT, signal_term_handler);
        signal(SIGTERM, signal_term_handler);

        // Getting parametres
        width =   atoi(argv[2]);
        height =  atoi(argv[3]); 
        quality = atoi(argv[4]);

        // Init video device
        vgrab_init(argv[1], width, height);

        // Init transport protocol
        ortp_init();
        ortp_scheduler_init();
        ortp_set_log_level_mask(ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR);
        session=rtp_session_new(RTP_SESSION_SENDONLY);	

        rtp_session_set_scheduling_mode(session,1);
        rtp_session_set_blocking_mode(session,1);
        rtp_session_set_connected_mode(session,TRUE);
        rtp_session_set_payload_type(session,0);
        rtp_session_set_remote_addr(session,argv[5],atoi(argv[6]));
        
        // Main loop
        for (;;) {
                // Get next frame
                int bytes = vgrab_get_frame(&buf);

                // Print readed bytes
                fprintf(stderr, "read=%d ", bytes);

                // Compress image
                size_t bufsize = jpeg_compress(buf->start, outbuf, width, height, quality);
                fprintf(stderr, "compressed=%d ", bufsize);

                // Send image
                rtp_session_send_with_ts(session, outbuf, bufsize, user_ts);

                fprintf(stderr, "ts=%u\n", user_ts);
                usleep(100000);
                ++user_ts;
        }

        return 0;
}
