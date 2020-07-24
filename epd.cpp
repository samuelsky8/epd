#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <vector>
#include "include/epd.h"
#include "include/img_proc.h"

using namespace cv;

epd::epd(int panel_name)
{
   epd(panel_name, 0);
}

epd::epd(int panel_name, int id)
{
   int i;

   this->extID=-1;
   this->panel_name=panel_name;
   switch(this->panel_name){
      case ED133UT2:
         it8951_No=1;
         break;
      case ED312TT2: case EC312TT2:
         it8951_No=4;
         break;
      case ED420TT1:
         it8951_No=2;
      default:
         break;
   }
   for(i=0; i<4; i++) it8951_dev[i]=NULL;

   for(i=0; i<it8951_No; i++){
      if(this->panel_name==EC312TT2)
         it8951_dev[i]=new it8951_dev_usb(i, id, 1);
      else
         it8951_dev[i]=new it8951_dev_usb(i, id, 0);
   }

   if(it8951_dev[0]->getWidth()<0) return;

   if((this->panel_name==ED312TT2)||(this->panel_name==EC312TT2)){
      epd_width =it8951_dev[0]->getHeight()*it8951_No;
      epd_height=it8951_dev[0]->getWidth();
   }
   else{
      epd_width =it8951_dev[0]->getWidth();
      epd_height=it8951_dev[0]->getHeight()*it8951_No;
   }

   extID=it8951_dev[0]->getExtID();
}

epd::~epd()
{
}

int epd::ldImgData2Buf(unsigned char* pixel_data,
                       int width, int height, int startx, int starty)
{
   unsigned char* data[4];
   int w[4], h[4], sx[4], sy[4];
   int i, cur_x, cur_y, cur_endx, cur_endy, img_x, img_y, segment;

   memset(w, 0, sizeof(w));
   switch(this->panel_name){
      case ED133UT2: case ED420TT1:
         segment=epd_height/it8951_No;
         cur_y=starty;
         cur_endy=starty+height;
         img_y=0;
         for(i=0; i<it8951_No; i++){
            if(cur_endy>0){
               if((cur_y-segment)<0){
                  sx[i]=startx;
                  w[i]=width;
                  sy[i]=cur_y;
                  if((cur_endy-segment)<0){
                     data[i]=(unsigned char*)malloc(width*(cur_endy-cur_y));
                     if(i==1){
                        flip(data[i], pixel_data+img_y*width,
                             width, height,
                             width, cur_endy-cur_y, 0, img_y, 1);
                     }
                     else{
                        memcpy(data[i], pixel_data+img_y*width,
                               width*(cur_endy-cur_y));
                     }
                     h[i]=cur_endy-cur_y;
                  }
                  else{
                     data[i]=(unsigned char*)malloc(width*(segment-cur_y));
                     if(i==1){
                        flip(data[i], pixel_data+img_y*width,
                             width, height,
                             width, segment-cur_y, 0, img_y, 1);
                     }
                     else{
                        memcpy(data[i], pixel_data+img_y*width,
                               width*(segment-cur_y));
                     }
                     h[i]=segment-cur_y;
                     img_y+=segment-cur_y;
                  }
               }
            }
            if((cur_y-=segment)<0) cur_y=0;
            cur_endy-=segment;
         }
         break;
      case ED312TT2:
         segment=epd_width/it8951_No;
         cur_x=startx;
         cur_endx=startx+width;
         img_x=0;
         for(i=0; i<it8951_No; i++){
            if(cur_endx>0){
               if((cur_x-segment)<0){
                  w[3-i]=height;
                  sx[3-i]=starty;
                  sy[3-i]=cur_x;
                  if((cur_endx-segment)<0){
                     data[3-i]=(unsigned char*)malloc((cur_endx-cur_x)*height);
                     transpose(data[3-i], pixel_data,
                               width, height,
                               cur_endx-cur_x, height, img_x, 0);
                     h[3-i]=cur_endx-cur_x;
                  }
                  else{
                     data[3-i]=(unsigned char*)malloc((segment-cur_x)*height);
                     transpose(data[3-i], pixel_data,
                               width, height,
                               segment-cur_x, height, img_x, 0);
                     h[3-i]=segment-cur_x;

                     img_x+=segment-cur_x;
                  }
               }
            }
            if((cur_x-=segment)<0) cur_x=0;
            cur_endx-=segment;
         }
         break;
      default:
         return 101;
   }

   for(i=0; i<it8951_No; i++){
      if(w[i]==0) ;
      else{
         it8951_dev[i]->ldPtr2Buf(data[i], w[i], h[i], sx[i], sy[i], 0);
      }
   }

   free(*data);
}

int epd::ldImgData2Buf(unsigned char* pixel_data_r, unsigned char* pixel_data_g,
                       unsigned char* pixel_data_b,
                       int width, int height, int startx, int starty)
{
   unsigned char* pixel_data;
   unsigned char* data[4];
   int w[4], h[4], sx[4], sy[4];
   int i, cur_x, cur_endx, img_x, segment;

   pixel_data=(unsigned char*)malloc(width*(height*2));
   memset(pixel_data, 0x00, width*(height*2));
   rgb24_rgb565(pixel_data,
                pixel_data_r, pixel_data_g, pixel_data_b, width, height);

   memset(w, 0, sizeof(w));
   segment=epd_width/it8951_No;
   switch(this->panel_name){
      case EC312TT2:
         cur_x=startx;
         cur_endx=startx+width;
         img_x=0;
         for(i=0; i<it8951_No; i++){
            if(cur_endx>0){
               if((cur_x-segment)<0){
                  w [3-i]=height*2;
                  sx[3-i]=starty;
                  sy[3-i]=cur_x;
                  if((cur_endx-segment)<0){
                     data[3-i]=(unsigned char*)malloc((cur_endx-cur_x)*
                                                      (height*2));
                     transpose(data[3-i], pixel_data,
                               width, height*2,
                               cur_endx-cur_x, height*2, img_x, 0);
                     h[3-i]=cur_endx-cur_x;
                  }
                  else{
                     data[3-i]=(unsigned char*)malloc((segment-cur_x)*
                                                      (height*2));
                     transpose(data[3-i], pixel_data,
                               width, height*2,
                               segment-cur_x, height*2, img_x, 0);
                     h[3-i]=segment-cur_x;

                     img_x+=segment-cur_x;
                  }
               }
            }
            if((cur_x-=segment)<0) cur_x=0;
            cur_endx-=segment;
         }
         break;
      default:
         return 101;
   }

   for(i=0; i<it8951_No; i++){
      if(w[i]==0) ;
      else{
         it8951_dev[i]->ldPtr2Buf(data[i], w[i], h[i], sx[i], sy[i], 1);
      }
   }

   free(*data);

   return 0;
}

int epd::ldImg2Buf(char* img_name,
                   int width, int height, int startx, int starty)
{
   Mat img, img_r, img_g, img_b;
   vector<Mat> channels;

   switch(this->panel_name){
      case ED133UT2: case ED312TT2: case ED420TT1:
         img=imread(img_name, CV_LOAD_IMAGE_GRAYSCALE);
         resize(img, img, cv::Size(width, height), 0, 0, CV_INTER_LINEAR);
         ldImgData2Buf(img.data, width, height, startx, starty);
         break;
      case EC312TT2:
         img=imread(img_name, CV_LOAD_IMAGE_COLOR);
         resize(img, img, cv::Size(width, height), 0, 0, CV_INTER_LINEAR);
         split(img, channels);
         img_r=channels.at(2);
         img_g=channels.at(1);
         img_b=channels.at(0);
         ldImgData2Buf(img_r.data, img_g.data,img_b.data,
                       width, height, startx, starty);
         break;
      default:
         return 101;
   }

   return 0;
}

int epd::ldImg2Buf(char* img_name)
{
   ldImg2Buf(img_name, epd_width, epd_height, 0, 0);
}

int epd::display(int width, int height, int startx, int starty, int mode)
{
   int w[4], h[4], sx[4], sy[4];
   int i, cur_x, cur_y, cur_endx, cur_endy, segment;

   memset(w, 0, sizeof(w));
   memset(h, 0, sizeof(h));
   memset(sx, 0, sizeof(sx));
   memset(sy, 0, sizeof(sy));

   switch(this->panel_name){
      case ED133UT2: case ED420TT1:
         segment=epd_height/it8951_No;
         cur_y=starty;
         cur_endy=starty+height;
         for(i=0; i<it8951_No; i++){
            if(cur_endy>0){
               if((cur_y-segment)<0){
                  sx[i]=startx;
                  w[i]=width;
                  if((cur_endy-segment)<0)
                     h[i]=cur_endy-cur_y;
                  else
                     h[i]=segment-cur_y;
                  if(i==1) sy[i]=segment-cur_y-h[i];
                  else sy[i]=cur_y;
               }
            }
            if((cur_y-=segment)<0) cur_y=0;
            cur_endy-=segment;
         }
         break;
      case ED312TT2: case EC312TT2:
         segment=epd_width/it8951_No;
         cur_x=startx;
         cur_endx=startx+width;
         for(i=0; i<4; i++){
            if(cur_endx>0){
               if((cur_x-segment)<0){
                  w[3-i]=height;
                  sx[3-i]=starty;
                  sy[3-i]=cur_x;
                  if((cur_endx-segment)<0)
                     h[3-i]=cur_endx-cur_x;
                  else
                     h[3-i]=segment-cur_x;
               }
            }
            if((cur_x-=segment)<0) cur_x=0;
            cur_endx-=segment;
         }
         break;
      default:
         return 102;
   }

/*   for(i=0; i<it8951_No; i++){
      if(w[i]==0){
         it8951_dev[i]->writeReg(1);
      }
   }*/

   for(i=it8951_No-1; i>=0; i--){
      if(w[i]!=0)
         it8951_dev[i]->display(w[i], h[i], sx[i], sy[i], mode);
   }

/*   for(i=0; i<it8951_No; i++){
      if(w[i]==0)
         it8951_dev[i]->writeReg(0);
   }*/

   return 0;
}

int epd::display(int width, int height, int startx, int starty,
                 short transition)
{
   int n;
   int step=80, step_x=40, step_y=20;
   size_t step_count=step_x*step_y+1;
   int numset[step_count];

   switch(transition){
      case 10:
         for(n=0; n<step; n++){
            this->display(width, height/step, startx, starty+height/step*n, 2);
            usleep(80000);
         }
         break;
      case 100:
         for(n=0; n<step_count; n++) numset[n]=n;
         for(n=0; n<step_count; n++)
            std::swap(numset[n], numset[rand()%step_count]);
         for(n=0; n<step_count; n++)
            this->display(width/step_x, height/step_y,
                          (numset[n]%step_x)*(width/step_x),
                          (numset[n]/step_x)*(height/step_y),
                          2);
         break;
      default:
         this->display(width, height, startx, starty, 2);
         break;
   }

   return 0;
}

int epd::display(short transition)
{
   this->display(epd_width, epd_height, 0, 0, transition);
   return 0;
}

void epd::clrScreen(void)
{
   display(epd_width, epd_height, 0, 0, (int)0);
}

int epd::getWidth(void)
{
   return this->epd_width;
}

int epd::getHeight(void)
{
   return this->epd_height;
}

int epd::getDevID(void)
{
   return this->extID;
}

int epd::getPanelName(void)
{
   return this->panel_name;
}

int epd::setPmicOff(void)
{
   int i, ret;

   for(i=0; i<it8951_No; i++){
      printf("it8951[%d] Power Off\n", i);
      ret=it8951_dev[i]->setPower(0);
   }

   return 0;
}

int epd::setPmicON()
{
   int i, ret;

   for(i=0; i<it8951_No; i++){
      printf("it8951[%d] Power ON\n", i);
      ret=it8951_dev[i]->setPower(1);
   }

   return 0;
}

/*void epd::transpose(unsigned char* dest_data, unsigned char* src_data,
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

void epd::rgb24_rgb565(unsigned char* data565, unsigned char* data_r,
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
*/
