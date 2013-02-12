#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ortp/ortp.h>
#include <libmemcached/memcached.h>

RtpSession *session;

void ssrc_cb(RtpSession *session)
{
        printf("hey, the ssrc has changed !\n");
}

static void signal_term_handler(int signum)
{
        fprintf(stderr, "Terminate...\n");

        // Destroy transport session
        rtp_session_destroy(session);
        ortp_exit();
        ortp_global_stats_display();

        fprintf(stderr, "\nok\n");
        exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
        int sock;
        uint32_t ts = 0;
        char buf[65000];

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
        ortp_init();
        ortp_scheduler_init();
        ortp_set_log_level_mask(ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR);
        session = rtp_session_new(RTP_SESSION_RECVONLY);	
        rtp_session_set_scheduling_mode(session,1);
        rtp_session_set_blocking_mode(session,1);
        rtp_session_set_local_addr(session,"0.0.0.0",atoi(argv[1]),-1);
        rtp_session_set_connected_mode(session,TRUE);
        rtp_session_set_symmetric_rtp(session,TRUE);
        //rtp_session_enable_adaptive_jitter_compensation(session,TRUE);
        //rtp_session_set_jitter_compensation(session,100);
        rtp_session_set_payload_type(session,0);
        //rtp_session_signal_connect(session,"ssrc_changed",(RtpCallback)ssrc_cb,0);
        //rtp_session_signal_connect(session,"ssrc_changed",(RtpCallback)rtp_session_reset,0);

        // Create memcached connection
        memcached_st *memc = memcached_create(NULL);
        memcached_server_add(memc, "localhost", 11211);

        for (;;) 
        {
                int have_more = 1, bytes;
                while (have_more)
                    bytes = rtp_session_recv_with_ts(session, buf, 650000, ts, &have_more);
                if (bytes)
                {
                        // Set timestamp
                        char ts_str[14];
                        sprintf(ts_str, "%12u", ts);
                        memcached_set(memc, uuid_ts, uuid_ts_len, ts_str, strlen(ts_str), 0, 0);

                        fprintf(stderr, "Received %d bytes :: timestamp %s\n", bytes, ts_str);

                        // Set image
                        memcached_set(memc, uuid, uuid_len, buf, bytes, 0, 0);
                }
                ++ts;
        }
        return 0;
}
