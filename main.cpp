#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "include/ejump_tcon.h"
#include "include/epd.h"
#include "include/usb_epd.h"

#define DELAY_NO   500000

#define DEVICE_MAX 16
#define IMAGE_MAX  100

using namespace std;

int main(int argc, char** argv)
{
   string photo_lib_dir; // Directory of Photo Library
   string playlist_fn, device_fn;   // File Name of Playlist, Device

   int dev_no;
   epd *epd_handler;
   struct{
      epd *handler;
      int panel_no;
      int id;
      int width, height;
   }epd_dev[DEVICE_MAX];

   ifstream playlist, device;
   string line, str;
   int img_no;
   struct{
      char fn[80];
      int startx, starty;
      int width, height;
      short transition;
      int delay_sec;
   }img[IMAGE_MAX];

   char *img_name_no[10];

   int count_no[5];
   int ret, i, j, k, step, step_x, step_y, str_ptr;

   // Default Parameters Setup
   photo_lib_dir="/photo_library";
   playlist_fn  =photo_lib_dir+"/"+"playlist.txt";
   device_fn    =photo_lib_dir+"/"+"device.txt";

   // Variables Initial
   dev_no=0;

   device.open(device_fn);
   while(getline(device, line)){
      str_ptr=line.find(" ", 0);
      str=line.substr(0, str_ptr);
      epd_dev[dev_no].panel_no=atoi(str.c_str());

      line=line.substr(str_ptr+1, line.length()-(str_ptr+1));
      str_ptr=line.find(" ", 0);
      str=line.substr(0, str_ptr);
      epd_dev[dev_no].id=atoi(str.c_str());

      dev_no++;
   };

   if(dev_no==0){
      cout << "No Device Assigned!!" << endl;
      exit(1);
   }

   // USB Initial
   usb_init();

   // EPD Initial
   for(i=0; i<dev_no; i++){
      epd_dev[i].handler=new epd(epd_dev[i].panel_no, epd_dev[i].id);
      epd_dev[i].width  =epd_dev[i].handler->getWidth();
      epd_dev[i].height =epd_dev[i].handler->getHeight();
   }

   // EPD Information
   for(i=0; i<dev_no; i++){
      if(epd_dev[i].handler->getDevID()<0){
         cout << "Device[" << epd_dev[i].id << "] Open Failed!!" << endl;
         exit(1);
      }

      cout << "EPD[" << epd_dev[i].id << "] Width=" << epd_dev[i].width
           << endl;
      cout << "EPD[" << epd_dev[i].id << "] Width=" << epd_dev[i].height
           << endl;
   }


   for(i=0; i<dev_no; i++)
      epd_dev[i].handler->clrScreen();   // Clear the Screen
   cout << "Wait for " << "2-" << 2+dev_no*2 << " Seconds" << endl;

   do{
      // Read Playlist
      img_no=0;
      playlist.open(playlist_fn);
      while(getline(playlist, line)){
         str_ptr=line.find(" ", 0);
         str=line.substr(0, str_ptr);
         strcpy(img[img_no].fn, (photo_lib_dir+"/"+str).c_str());

         line=line.substr(str_ptr+1, line.length()-(str_ptr+1));
         str_ptr=line.find(" ", 0);
         str=line.substr(0, str_ptr);
         img[img_no].delay_sec=atoi(str.c_str())*1000000;

         line=line.substr(str_ptr+1, line.length()-(str_ptr+1));
         str=line;
         img[img_no].transition=(short)atoi(str.c_str());

         img[img_no].startx=0;
         img[img_no].starty=0;
         img[img_no].width=0;
         img[img_no].height=0;
         img_no++;
      };
      playlist.close();

      // Load Image and Display
      for(j=0; j<img_no; j++){
         // Load Image to Each Device's Buffer
         for(i=0; i<dev_no; i++)
            ret=epd_dev[i].handler->ldImg2Buf(img[j].fn);

         cout << "Display Image File : " << img[j].fn << endl;
         // Each Device Display the Image in Buffer
         for(i=0; i<dev_no; i++)
            ret=epd_dev[i].handler->display(img[j].transition);
         usleep(img[j].delay_sec);
      }

   }while(1);

   usb_close();

   return 0;
}


