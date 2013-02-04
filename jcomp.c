#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jpeglib.h>

long jpeg_compress(char *src, unsigned char *dst, int width, int height, int quality)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
	
  JSAMPROW row_pointer[1];

  long bufsize = 65000;

  // create jpeg data
  cinfo.err = jpeg_std_error( &jerr );
  jpeg_create_compress(&cinfo);
  jpeg_mem_dest(&cinfo, &dst, &bufsize);

  // set image parameters
  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  // set jpeg compression parameters to default
  jpeg_set_defaults(&cinfo);
  // and then adjust quality setting
  jpeg_set_quality(&cinfo, quality, TRUE);

  // start compress 
  jpeg_start_compress(&cinfo, TRUE);

  // feed data
  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = src + cinfo.next_scanline * cinfo.image_width * cinfo.input_components;
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  // finish compression
  jpeg_finish_compress(&cinfo);

  // destroy jpeg data
  jpeg_destroy_compress(&cinfo);

  return bufsize;
}
