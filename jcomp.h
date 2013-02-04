/* jcomp :: JPEG compressor
 *   src     - RGB24 image buffer
 *   dst     - destination buffer
 *   width   - image width
 *   height  - image height
 *   quality - image quality
 *
 *   returned value - size of result image in bytes
 */

unsigned long   jpeg_compress(
char            *src, 
unsigned char   *dst, 
int             width, 
int             height, 
int             quality
);

