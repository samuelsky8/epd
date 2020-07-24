#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define SWAP_32(x) ((((x)&0xff000000)>>24)|(((x)&0x00ff0000)>> 8)| \
                    (((x)&0x0000ff00)<< 8)|(((x)&0x000000ff)<<24))

int perr(char const *format, ...)
{
        va_list args;
        int r;

        va_start (args, format);
        r = vfprintf(stderr, format, args);
        va_end(args);

        return r;
}

void display_buffer_hex(unsigned char *buffer, unsigned size)
{
        unsigned i, j, k;

        for (i=0; i<size; i+=16) {
                printf("\n  %08x  ", i);
                for(j=0,k=0; k<16; j++,k++) {
                        if (i+j < size) {
                                printf("%02x", buffer[i+j]);
                        } else {
                                printf("  ");
                        }
                        printf(" ");
                }
                printf(" ");
                for(j=0,k=0; k<16; j++,k++) {
                        if (i+j < size) {
                                if ((buffer[i+j] < 32) || (buffer[i+j] > 126)) {
                                        printf(".");
                                } else {
                                        printf("%c", buffer[i+j]);
                                }
                        }
                }
        }
        printf("\n" );
}

void EndianLittleBig(unsigned int* buf, unsigned int size)
{
   int i;

   for(i=0; i<size/sizeof(unsigned int); i++){
      buf[i]=SWAP_32(buf[i]);
   }
}

