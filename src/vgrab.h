#ifndef VGRAB_H
#define VGRAB_H

/*
 * vgrab :: Video (V4L2) grabber library
 *
 */

// Frame buffer
typedef struct {
        void   *start;
        size_t length;
} vgrab_buffer;

// Init device by name and resolution
void        vgrab_init(
const char  *dev_name,
int         width,
int         height
);

// Close device
void vgrab_close();

// Get pointer to frame buffer and sizeof data
int vgrab_get_frame(vgrab_buffer **buffer_pointer);

#endif // VGRAB_H
