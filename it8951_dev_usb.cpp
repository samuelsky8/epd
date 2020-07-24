#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "include/it8951_dev_usb.h"
#include "include/usb_epd.h"
#include "include/tool.h"


it8951_dev_usb::it8951_dev_usb(int dev_no, int dev_id, int type)
{
   int i;

   width=-1;
   for(i=0; i<tcon_no; i++){
      dev_usb_no=i;
      extID=readExtID();

      getSystemInfo();
      intID=systemInfo.uiMultiPanelIndex;
      height=systemInfo.uiHeight;

      if(extID!=dev_id) ;
      else{
         if(intID!=dev_no) ;
         else{
            width=systemInfo.uiWidth;
            if(type==1) getColorInfo();

            i=tcon_no;
         }
      }
   }
}

it8951_dev_usb::~it8951_dev_usb()
{
   dev_usb_close(dev_usb_no);
}

int it8951_dev_usb::readExtID()
{
   uint8_t cdb[16];
   uint8_t data[sizeof(T_F_SET_BOARDID)];

   memset(cdb, 0x00, sizeof(cdb));
   cdb[ 0]=0xFE;
   cdb[ 1]=0x00;
   cdb[ 2]=0x00;
   cdb[ 3]=0x00;
   cdb[ 4]=0x00;
   cdb[ 5]=0x00;
   cdb[ 6]=0xA8;
   cdb[ 7]=0x00;
   cdb[ 8]=0x00;
   cdb[ 9]=0x00;
   cdb[10]=0x00;
   cdb[11]=0x00;
   cdb[12]=0x00;
   cdb[13]=0x00;
   cdb[14]=0x00;
   cdb[15]=0x00;

   memset(data, 0x00, sizeof(data));
   usb_cbw_cmd(dev_usb_no, 0, cdb, 0x80, sizeof(data));
   usb_ReadNData(dev_usb_no, data, sizeof(data));

//   display_buffer_hex(data, sizeof(data));

   return (int)data[0];
}

int it8951_dev_usb::getSystemInfo()
{
   uint8_t cdb[16];
   unsigned char data[sizeof(_TRSP_SYSTEM_INFO_DATA)];
   _TRSP_SYSTEM_INFO_DATA *pstSystemInfo=&systemInfo;
   unsigned int *pi=(unsigned int*)pstSystemInfo;
   int i;

   memset(cdb, 0x00, sizeof(cdb));
   cdb[ 0]=0xFE;
   cdb[ 1]=0x00;
   cdb[ 2]=0x38;
   cdb[ 3]=0x39;
   cdb[ 4]=0x35;
   cdb[ 5]=0x31;
   cdb[ 6]=IT8951_USB_OP_GET_SYS;
   cdb[ 7]=0x00;
   cdb[ 8]=0x01;
   cdb[ 9]=0x00;
   cdb[10]=0x02;
   cdb[11]=0x00;
   cdb[12]=0x00;
   cdb[13]=0x00;
   cdb[14]=0x00;
   cdb[15]=0x00;

   usb_cbw_cmd(dev_usb_no, 0, cdb, 0x80, sizeof(data));
   usb_ReadNData(dev_usb_no, data, sizeof(data));

   memcpy((unsigned char*)pi, data, sizeof(data));
   EndianLittleBig(pi, sizeof(data));

//   display_buffer_hex((unsigned char*)pi, sizeof(data));
}

int it8951_dev_usb::getColorInfo()
{
   uint8_t cdb[16];
   uint8_t data[8];
   int i;

   // Get Color Image Buffer Base
   memset(cdb, 0x00, sizeof(cdb));
   cdb[ 0]=0xFE;
   cdb[ 1]=0x00;
   cdb[ 2]=0x07;
   cdb[ 3]=0x00;
   cdb[ 4]=0x00;
   cdb[ 5]=0x00;
   cdb[ 6]=0xAE;
   cdb[ 7]=0x00;
   cdb[ 8]=0x00;
   cdb[ 9]=0x00;
   cdb[10]=0x00;
   cdb[11]=0x00;
   cdb[12]=0x00;
   cdb[13]=0x00;
   cdb[14]=0x00;
   cdb[15]=0x00;

   usb_cbw_cmd(dev_usb_no, 0, cdb, 0x80, sizeof(data));
   usb_ReadNData(dev_usb_no, data, sizeof(data));

   EndianLittleBig((unsigned int*)data, sizeof(data));
   for(i=0; i<sizeof(cImgBufBase); i++){
      cImgBufBase[i]=data[7-i];
   }

//   display_buffer_hex(cImgBufBase, sizeof(cImgBufBase));

   // Get Color Image Buffer Size
   memset(cdb, 0x00, sizeof(cdb));
   cdb[ 0]=0xFE;
   cdb[ 1]=0x00;
   cdb[ 2]=0x08;
   cdb[ 3]=0x00;
   cdb[ 4]=0x00;
   cdb[ 5]=0x00;
   cdb[ 6]=0xAE;
   cdb[ 7]=0x00;
   cdb[ 8]=0x00;
   cdb[ 9]=0x00;
   cdb[10]=0x00;
   cdb[11]=0x00;
   cdb[12]=0x00;
   cdb[13]=0x00;
   cdb[14]=0x00;
   cdb[15]=0x00;

   usb_cbw_cmd(dev_usb_no, 0, cdb, 0x80, sizeof(data));
   usb_ReadNData(dev_usb_no, data, sizeof(data));

   EndianLittleBig((unsigned int*)data, sizeof(data));
   for(i=0; i<sizeof(cImgBufSize); i++){
      cImgBufSize[i]=data[7-i];
   }

//   display_buffer_hex(cImgBufSize, sizeof(cImgBufSize));
}

void it8951_dev_usb::colorTransform(int width, int height,
                                    int startx, int starty)
{
   uint8_t cdb[16];
   uint8_t data[8];

   // Get Color Image Buffer Size
   memset(cdb, 0x00, sizeof(cdb));
   cdb[ 0]=0xFE;
   cdb[ 1]=0x00;
   cdb[ 2]=0x09;
   cdb[ 3]=0x00;
   cdb[ 4]=0x00;
   cdb[ 5]=0x00;
   cdb[ 6]=0xAE;
   cdb[ 7]=0x00;
   cdb[ 8]=(uint8_t)((startx>>8)&0xFF);
   cdb[ 9]=(uint8_t)(startx&0xFF);
   cdb[10]=(uint8_t)((starty>>8)&0xFF);
   cdb[11]=(uint8_t)(starty&0xFF);
   cdb[12]=(uint8_t)((width>>8)&0xFF);
   cdb[13]=(uint8_t)(width&0xFF);
   cdb[14]=(uint8_t)((height>>8)&0xFF);
   cdb[15]=(uint8_t)(height&0xFF);

   usb_cbw_cmd(dev_usb_no, 0, cdb, 0x80, sizeof(data));
   usb_ReadNData(dev_usb_no, data, sizeof(data));

   EndianLittleBig((unsigned int*)data, sizeof(data));
}

int it8951_dev_usb::ldPtr2Buf(unsigned char* ptr, int width, int height,
                              int startx, int starty, int type)
{
   unsigned char data[width];
   unsigned int BufferBase;
   int BufferWidth, i, row;

   if(type==1){
      BufferBase=(cImgBufBase[0]<<24)|
                 (cImgBufBase[1]<<16)|
                 (cImgBufBase[2]<< 8)|
                 (cImgBufBase[3]    );
      BufferWidth=1440*2;
   }
   else{
      BufferBase=systemInfo.uiImageBufBase;
      BufferWidth=systemInfo.uiWidth;
   }

   for(i=0, row=1; i<height; i++, row++){
      memcpy(data, ptr+width*i, width);

      if(type==1){
         fastWriteMem(BufferBase+width*(row-1),
                      data, width);

         if((row<160)&&((i+1)!=height)) ;
         else{
            colorTransform(width/2, row, startx, starty+i+1-row);
            row=0;
         }
      }
      else{
         fastWriteMem(BufferBase+BufferWidth*(starty+row-1)+
                      startx,
                      data, width);
      }
   }

   return 0;
}

int it8951_dev_usb::ldBmp2Buf(unsigned char* img, int img_w, int img_h,
                              int startx, int starty)
{
   uint8_t cdb[16];
   int cmd_data_length=0, data_length=0;
   unsigned char *data;
   unsigned char cmd_data[sizeof(_LOAD_IMG_AREA)+SPT_BUF_SIZE];
   _LOAD_IMG_AREA stLdImgInfo;
   unsigned int ulSendLineCnt;
   int i;

   memset(cdb, 0x00, sizeof(cdb));
   cdb[ 0]=0xFE;
   cdb[ 1]=0x00;
   cdb[ 2]=0x00;
   cdb[ 3]=0x00;
   cdb[ 4]=0x00;
   cdb[ 5]=0x00;
   cdb[ 6]=IT8951_USB_OP_LD_IMG_AREA;
   cdb[ 7]=0x00;
   cdb[ 8]=0x00;
   cdb[ 9]=0x00;
   cdb[10]=0x00;
   cdb[11]=0x00;
   cdb[12]=0x00;
   cdb[13]=0x00;
   cdb[14]=0x00;
   cdb[15]=0x00;

   ulSendLineCnt=SPT_BUF_SIZE/img_w;
   for(i=0; i<img_h; i+=ulSendLineCnt){
     if(ulSendLineCnt>(img_h-i)){
         ulSendLineCnt=img_h-i;
      }

      stLdImgInfo.iAddress=systemInfo.uiImageBufBase;
      stLdImgInfo.iX      =startx;
      stLdImgInfo.iY      =starty+i;
      stLdImgInfo.iW      =img_w;
      stLdImgInfo.iH      =ulSendLineCnt;

      EndianLittleBig((unsigned int*)&stLdImgInfo, sizeof(stLdImgInfo));
//      display_buffer_hex((unsigned char*)&stLdImgInfo, sizeof(_LOAD_IMG_AREA));

      data=img+(i*img_w);
      data_length=ulSendLineCnt*img_w;

      memcpy(cmd_data, (unsigned int*)&stLdImgInfo, sizeof(cmd_data));
      memcpy(&cmd_data[sizeof(_LOAD_IMG_AREA)], data, data_length);
      cmd_data_length=sizeof(_LOAD_IMG_AREA)+data_length;
//      display_buffer_hex((unsigned char*)cmd_data, 32);

      usb_cbw_cmd(dev_usb_no, 0, cdb, 0x00, cmd_data_length);
      usb_WriteNData(dev_usb_no, cmd_data, cmd_data_length);
   }

   return 0;
}

int it8951_dev_usb::fastWriteMem(unsigned int ulBufAddr, unsigned char* data,
                                 int data_length)
{
   uint8_t cdb[16];

   if(data_length>(60*1024)) return 101;

   memset(cdb, 0x00, sizeof(cdb));
   cdb[ 0]=0xFE;
   cdb[ 1]=0x00;
   cdb[ 2]=(ulBufAddr>>24)&0xFF;
   cdb[ 3]=(ulBufAddr>>16)&0xFF;
   cdb[ 4]=(ulBufAddr>>8 )&0xFF;
   cdb[ 5]= ulBufAddr     &0xFF;
   cdb[ 6]=IT8951_USB_OP_FAST_WRITE_MEM;
   cdb[ 7]=(data_length>>8)&0xFF;
   cdb[ 8]= data_length    &0xFF;
   cdb[ 9]=0x00;
   cdb[10]=0x00;
   cdb[11]=0x00;
   cdb[12]=0x00;
   cdb[13]=0x00;
   cdb[14]=0x00;
   cdb[15]=0x00;

   usb_cbw_cmd(dev_usb_no, 0, cdb, 0x00, data_length);
   usb_WriteNData(dev_usb_no, data, data_length);

   return 0;
}

int it8951_dev_usb::display(int width, int height, int startx, int starty,
                            int mode)
{
   uint8_t cdb[16];
   _TDRAW_UPD_ARG_DATA stDrawUpdArgData;
   unsigned int *data=(unsigned int*)&stDrawUpdArgData;
   unsigned int data_length;

   memset(cdb, 0x00, sizeof(cdb));
   cdb[ 0]=0xFE;
   cdb[ 1]=0x00;
   cdb[ 2]=0x00;
   cdb[ 3]=0x00;
   cdb[ 4]=0x00;
   cdb[ 5]=0x00;
   cdb[ 6]=IT8951_USB_OP_DPY_AREA;
   cdb[ 7]=0x00;
   cdb[ 8]=0x00;
   cdb[ 9]=0x00;
   cdb[10]=0x00;
   cdb[11]=0x00;
   cdb[12]=0x00;
   cdb[13]=0x00;
   cdb[14]=0x00;
   cdb[15]=0x00;

   stDrawUpdArgData.iMemAddr       =systemInfo.uiImageBufBase;
   stDrawUpdArgData.iWavMode       =mode;
   stDrawUpdArgData.iPosX          =startx;
   stDrawUpdArgData.iPosY          =starty;
   stDrawUpdArgData.iWidth         =width;
   stDrawUpdArgData.iHeight        =height;
   stDrawUpdArgData.iEnWaitDpyReady=0;
   data_length=sizeof(_TDRAW_UPD_ARG_DATA);

   EndianLittleBig(data, data_length);
//   display_buffer_hex((unsigned char*)data, data_length);

   usb_cbw_cmd(dev_usb_no, 0, cdb, 0x00, 28);
   usb_WriteNData(dev_usb_no, (unsigned char*)data, 28);

   return 0;
}

int it8951_dev_usb::writeReg(int mode)
{
   uint8_t cdb[16];
   unsigned int ulRegAddr;
   unsigned int data;
   uint8_t data_[4];

   ulRegAddr=0x18001138;

   memset(cdb, 0x00, sizeof(cdb));
   cdb[ 0]=0xFE;
   cdb[ 1]=0x00;
   cdb[ 2]=(ulRegAddr>>24)&0xFF;
   cdb[ 3]=(ulRegAddr>>16)&0xFF;
   cdb[ 4]=(ulRegAddr>> 8)&0xFF;
   cdb[ 5]=(ulRegAddr    )&0xFF;
   cdb[ 6]=IT8951_USB_OP_WRITE_REG;
   cdb[ 7]=0x00;
   cdb[ 8]=0x04;
   cdb[ 9]=0x00;
   cdb[10]=0x00;
   cdb[11]=0x00;
   cdb[12]=0x00;
   cdb[13]=0x00;
   cdb[14]=0x00;
   cdb[15]=0x00;

   if(mode==1)
      data=0x00080000;
   else
      data=0x00000000;

   data_[0]=(data>>24)&0xFF;
   data_[1]=(data>>16)&0xFF;
   data_[2]=(data>> 8)&0xFF;
   data_[3]=(data    )&0xFF;

//   display_buffer_hex((unsigned char*)data_, sizeof(data));

   usb_cbw_cmd(dev_usb_no, 0, cdb, 0x00, sizeof(data_));
   usb_WriteNData(dev_usb_no, (unsigned char*)data_, sizeof(data_));
}

int it8951_dev_usb::setPower(int mode)
{
   uint8_t cdb[16];

   // mode: 0/OFF, 1/ON
   memset(cdb, 0x00, sizeof(cdb));
   cdb[ 0]=0xFE;
   cdb[ 1]=0x00;
   cdb[ 2]=0x00;
   cdb[ 3]=0x00;
   cdb[ 4]=0x00;
   cdb[ 5]=0x00;
   cdb[ 6]=IT8951_USB_OP_PMIC_CTRL;
   cdb[ 7]=0x00;
   cdb[ 8]=0x00;
   cdb[ 9]=0x00;
   cdb[10]=0x01;
   if(mode==1)
      cdb[11]=0x01;
   else
      cdb[11]=0x00;
   cdb[12]=0x00;
   cdb[13]=0x00;
   cdb[14]=0x00;
   cdb[15]=0x00;

   usb_cbw_cmd(dev_usb_no, 0, cdb, 0x00, 0);
}

int it8951_dev_usb::getWidth()
{
//   return systemInfo.uiWidth;
   return this->width;
}

int it8951_dev_usb::getHeight()
{
//   return systemInfo.uiHeight;
   return this->height;
}

int it8951_dev_usb::getIntID()
{
//   return systemInfo.uiMultiPanelIndex;
   return this->intID;
}

int it8951_dev_usb::getExtID()
{
   return this->extID;
}
