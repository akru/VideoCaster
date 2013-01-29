#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <linux/videodev2.h>
#include <libv4l2.h>
#include <signal.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer {
        void   *start;
        size_t length;
} *buffers;

struct vpkg {
        char timestamp[32];
        char image_header[15];
        char image[57600];
} outbuf;

int                             sock, fd = -1;
unsigned int                    i, n_buffers;
enum v4l2_buf_type              type;

static void xioctl(int fh, int request, void *arg)
{
        int r;

        do {
                r = v4l2_ioctl(fh, request, arg);
        } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

        if (r == -1) {
                fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
}

static void signal_term_handler(int signum)
{
        printf("Terminate...");

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        xioctl(fd, VIDIOC_STREAMOFF, &type);
        for (i = 0; i < n_buffers; ++i)
                v4l2_munmap(buffers[i].start, buffers[i].length);
        v4l2_close(fd);
        close(sock);

        printf("ok\n");
        exit(0);
}


int main(int argc, char **argv)
{
        struct v4l2_format              fmt;
        struct v4l2_buffer              buf;
        struct v4l2_requestbuffers      req;
        fd_set                          fds;
        struct timeval                  tv;
        int                             r;
        char                            *dev_name = "/dev/video0";
        char                            out_name[256];
        FILE                            *fout;
        struct sockaddr_in              addr;
        struct timeval                  now;

        if (argc < 3)
        {
                printf("Too few args!\n USAGE: vcap [HOST] [PORT]\n");
                exit(1);
        }

        signal(SIGINT, signal_term_handler);

        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if(sock < 0)
        {
                perror("socket");
                exit(1);
        }

        addr.sin_family = AF_INET;
        addr.sin_port = htons(atoi(argv[2]));
        addr.sin_addr.s_addr = inet_addr(argv[1]);;

        fd = v4l2_open(dev_name, O_RDWR | O_NONBLOCK, 0);
        if (fd < 0) {
                perror("Cannot open device");
                exit(EXIT_FAILURE);
        }

        CLEAR(fmt);
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width       = 160;
        fmt.fmt.pix.height      = 120;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
        fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
        xioctl(fd, VIDIOC_S_FMT, &fmt);
        if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24) {
                printf("Libv4l didn't accept RGB24 format. Can't proceed.\n");
                exit(EXIT_FAILURE);
        }
        if ((fmt.fmt.pix.width != 160) || (fmt.fmt.pix.height != 120))
                printf("Warning: driver is sending image at %dx%d\n",
                                fmt.fmt.pix.width, fmt.fmt.pix.height);

        CLEAR(req);
        req.count = 10;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;
        xioctl(fd, VIDIOC_REQBUFS, &req);

        buffers = calloc(req.count, sizeof(*buffers));
        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                CLEAR(buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = n_buffers;

                xioctl(fd, VIDIOC_QUERYBUF, &buf);

                buffers[n_buffers].length = buf.length;
                buffers[n_buffers].start = v4l2_mmap(NULL, buf.length,
                                PROT_READ | PROT_WRITE, MAP_SHARED,
                                fd, buf.m.offset);

                if (MAP_FAILED == buffers[n_buffers].start) {
                        perror("mmap");
                        exit(EXIT_FAILURE);
                }
        }

        for (i = 0; i < n_buffers; ++i) {
                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;
                xioctl(fd, VIDIOC_QBUF, &buf);
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        xioctl(fd, VIDIOC_STREAMON, &type);
        
        // Write image header into buffer
        sprintf(outbuf.image_header,"P6\n%d %d 255\n", 
                        fmt.fmt.pix.width, fmt.fmt.pix.height);

        for (;;) {
                do {
                        FD_ZERO(&fds);
                        FD_SET(fd, &fds);

                        /* Timeout. */
                        tv.tv_sec = 2;
                        tv.tv_usec = 0;

                        r = select(fd + 1, &fds, NULL, NULL, &tv);
                } while ((r == -1 && (errno = EINTR)));
                if (r == -1) {
                        perror("select");
                        return errno;
                }

                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                xioctl(fd, VIDIOC_DQBUF, &buf);
                
                // Copy timestamp into buffer
                gettimeofday(&now, 0);
                sprintf(outbuf.timestamp,"%ld", now.tv_sec);

                // Copy image into buffer
                memcpy(outbuf.image, buffers[buf.index].start, buf.bytesused);
                
                // Send buffer to server
                i = sendto(sock, &outbuf, sizeof(struct vpkg), 0,
                                (struct sockaddr *)&addr, sizeof(addr));
                printf("%ld: sended %d\n", now.tv_sec, i);
                
                xioctl(fd, VIDIOC_QBUF, &buf);
        }


        return 0;
}
