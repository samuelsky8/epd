#include <stdint.h>

// The maximum transfer size of 60K bytes for IT8951 USB
#define SPT_BUF_SIZE (60*1024)

#define IT8951_USB_OP_GET_SYS        0x80
#define IT8951_USB_OP_WRITE_REG      0x84
#define IT8951_USB_OP_DPY_AREA       0x94
#define IT8951_USB_OP_LD_IMG_AREA    0xA2
#define IT8951_USB_OP_PMIC_CTRL      0xA3
#define IT8951_USB_OP_FSET_TEMP      0xA4
#define IT8951_USB_OP_FAST_WRITE_MEM 0xA5

typedef struct _TRSP_SYSTEM_INFO_DATA
{
   unsigned int uiStandardCmdNo;
   unsigned int uiExtendCmdNo;
   unsigned int uiSignature;
   unsigned int uiVersion;
   unsigned int uiWidth;
   unsigned int uiHeight;
   unsigned int uiUpdateBufBase;
   unsigned int uiImageBufBase;
   unsigned int uiTemperatureNo;
   unsigned int uiModeNo;
   unsigned int uiFrameCount[8];
   unsigned int uiNumImgBuf;
   unsigned int uiWbfSFIAddr;
   unsigned int uiwaveforminfo; // low byte: A2 mode index
   unsigned int uiMultiPanelIndex; // High 2 byte for Y-axis, Low 2 byte for X-axis
   unsigned int uiWbfRunType;
   unsigned int uiWbfFPLotNumber;
   unsigned int uiReserved[3];
   void* lpCmdInfoDatas[1];
}_TRSP_SYSTEM_INFO_DATA;

typedef struct _LOAD_IMG_AREA
{
   int iAddress;
   int iX;
   int iY;
   int iW;
   int iH;
}_LOAD_IMG_AREA;

typedef struct _TDRAW_UPD_ARG_DATA
{
   int iMemAddr;
   int iWavMode;
   int iPosX;
   int iPosY;
   int iWidth;
   int iHeight;
   int iEnWaitDpyReady;
}_TDRAW_UPD_ARG_DATA;

typedef struct T_F_SET_BOARDID
{
   unsigned char ucSetBoardID;
   unsigned char ucSetBoardVal;
}T_F_SET_BOARDID;

class it8951_dev_usb
{
private:
   _TRSP_SYSTEM_INFO_DATA systemInfo;
   int dev_usb_no;
   int width;
   int height;
   int intID;
   int extID;

   uint8_t cImgBufBase[4];
   uint8_t cImgBufSize[4];

   int getSystemInfo(void);
   int fastWriteMem(unsigned int ulBufAddr, unsigned char* data, int data_length);

   int getColorInfo(void);
   void colorTransform(int, int, int, int);

   int readExtID(void);

public:
   it8951_dev_usb(int, int, int);
   ~it8951_dev_usb(void);

   int getWidth(void);
   int getHeight(void);
   int getIntID(void);
   int getExtID(void);

   int ldPtr2Buf(unsigned char* ptr, int width, int height,
                 int startx, int starty, int type);
   int ldBmp2Buf(unsigned char* img, int img_w, int img_h, int startx, int starty);
   int display(int width, int height, int startx, int starty, int mode);

   int setPower(int mode);
   int writeReg(int mode);
};
