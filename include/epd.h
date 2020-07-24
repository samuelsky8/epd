#include "ejump_tcon.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "it8951_dev_usb.h"

class epd
{
private:
   int panel_name;
   int it8951_No;

   int epd_width;
   int epd_height;
   int extID;

   it8951_dev_usb *it8951_dev[4];

   int display(int width, int height, int startx, int starty, int mode);
//   void transpose(unsigned char*, unsigned char*, int, int, int, int, int, int);
//   void rgb24_rgb565(unsigned char*, unsigned char*, unsigned char*, unsigned char*, int, int);

public:
   // Constructor
   epd(int panel_no);
   epd(int panel_no, int id);
   ~epd();

   // Initial
   int init(int id);

   // Load Image Data to Memory Buffer
   int ldImgData2Buf(unsigned char *pixel_data, int width, int height,
                     int startx, int starty);
   int ldImgData2Buf(unsigned char *pixel_data_r, unsigned char *pixel_data_g,
                     unsigned char *pixel_data_b,
                     int width, int height, int startx, int starty);


   // Load Image File to Memory Buffer
   int ldImg2Buf(char *img_name);
   int ldImg2Buf(char *img_name, int width, int height,
                 int startx, int starty);

   // Display to EPD from Momory Buffer
   int display(short transition);
   int display(int width, int height, int startx, int starty, short transition);

   // Clear Screen
   void clrScreen(void);

   // Paramater Acquirement
   int getWidth(void);
   int getHeight(void);
   int getDevID(void);
   int getPanelName(void);

   int setPmicOff(void);
   int setPmicON(void);
};
