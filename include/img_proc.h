void transpose(unsigned char* dest_data, unsigned char* src_data,
               int src_width, int src_height,
               int width, int height, int startx, int starty);

void flip(unsigned char* dest_data, unsigned char* src_data,
          int src_width, int src_height,
          int width, int height, int startx, int starty, int flipcode);

void rgb24_rgb565(unsigned char*, unsigned char*,
                  unsigned char*, unsigned char*, int, int);

