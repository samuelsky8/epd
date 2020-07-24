#include <stdint.h>

// Command Block Wrapper (CBW)
typedef struct command_block_wrapper{
   uint8_t  dCBWSignature[4];
   uint32_t dCBWTag;
   uint32_t dCBWDataTransferLength;
   uint8_t  bmCBWFlags;
   uint8_t  bCBWLUN;
   uint8_t  bCBWCBLength;
   uint8_t  CBWCB[16];
}Command_Block_Wrapper;

// Command Status Wrapper (CSW)
typedef struct command_status_wrapper{
   uint8_t  dCSWSignature[4];
   uint32_t dCSWTag;
   uint32_t dCSWDataResidue;
   uint8_t  bCSWStatus;
}Command_Status_Wrapper;

extern int tcon_no;

int usb_init(void);
int dev_usb_open(int);
void dev_usb_close(int);
void usb_close(void);
int usb_ReadNData(int, unsigned char*, int);
int usb_WriteNData(int, unsigned char* data, int data_length);
int usb_cbw_cmd(int, uint8_t lun, uint8_t *cdb, uint8_t direction, int data_length);

