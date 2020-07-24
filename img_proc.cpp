#include <stdint.h>

void transpose(unsigned char* dest_data, unsigned char* src_data,
               int src_width, int src_height,
               int width, int height, int startx, int starty)
{
   int row, col;

   for(row=0; row<height; row++){
      for(col=0; col<width; col++){
         *(dest_data+col*height+row)=*(src_data+row*src_width+(startx+col));
      }
   }
}

void flip(unsigned char* dest_data, unsigned char* src_data,
          int src_width, int src_height,
          int width, int height, int startx, int starty, int flipcode)
{
   int row, col;

   for(row=0; row<height; row++){
      for(col=0; col<width; col++){
         *(dest_data+row*width+col)=*(src_data+(height-row-1)*src_width+(startx+col));
      }
   }
}

void rgb24_rgb565(unsigned char* data565, unsigned char* data_r,
                  unsigned char* data_g, unsigned char* data_b,
                  int width, int height)
{
   uint16_t data;

   for(int i=0; i<height; i++){
      for(int j=0; j<width; j++){
         data=((*(data_b+i*width+j)>>3)&0x1F)    |
              ((*(data_g+i*width+j)>>2)&0x3F)<< 5|
              ((*(data_r+i*width+j)>>3)&0x1F)<<11;
         *(data565+(i*2)  *width+j)=(data>>8)&0xFF;
         *(data565+(i*2+1)*width+j)=data&0xFF;
      }
   }
}

