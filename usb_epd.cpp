#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "libusb-1.0/libusb.h"
#include "include/usb_epd.h"
#include "include/tool.h"

#define ERR_EXIT(errcode) do { perr("   %s\n", libusb_strerror((enum libusb_error)errcode)); return -1; } while (0)
#define CALL_CHECK_CLOSE(fcall, hdl) do { int _r=fcall; if (_r < 0) { libusb_close(hdl); ERR_EXIT(_r); } } while (0)

#define RETRY_MAX      5
#define INQUIRY_LENGTH 0x28
#define TCON_USB_MAX    100

const struct libusb_version* version;
libusb_device_handle *tcon_handles[TCON_USB_MAX];
libusb_device *tcon_dev[TCON_USB_MAX];
int tcon_no=0;
uint8_t _endpoint_in=0x00, _endpoint_out=0x00;
int _nb_ifaces;

int usb_cbw_cmd(int dev_no, uint8_t lun, uint8_t *cdb, uint8_t direction,
                int data_length)
{
   static uint32_t tag = 1;
   uint8_t cdb_len;
   int i, r, size;
   struct command_block_wrapper cbw;
   libusb_device_handle *_handle;

   _handle=tcon_handles[dev_no];

   if(cdb==NULL) return -1;

   cdb_len=16;
   if((cdb_len==0)||(cdb_len >sizeof(cbw.CBWCB))){
      perr("send_mass_storage_command: don't know how to handle this command (%02X, length %d)\n",
           cdb[0], cdb_len);
      return -1;
   }

   memset(&cbw, 0, sizeof(cbw));
   cbw.dCBWSignature[0]      ='U';
   cbw.dCBWSignature[1]      ='S';
   cbw.dCBWSignature[2]      ='B';
   cbw.dCBWSignature[3]      ='C';
//   *ret_tag = tag;
//   cbw.dCBWTag = tag++;
   cbw.dCBWDataTransferLength=data_length;
   cbw.bmCBWFlags            =direction;
   cbw.bCBWLUN               =lun;
   // Subclass is 1 or 6 => cdb_len
   cbw.bCBWCBLength          =cdb_len;
   memcpy(cbw.CBWCB, cdb, cdb_len);

   i=0;
   do{
      // The transfer length must always be exactly 31 bytes.
      r=libusb_bulk_transfer(_handle, _endpoint_out, (unsigned char*)&cbw, 31,
                             &size, 5);
      if(r==LIBUSB_ERROR_PIPE){
         libusb_clear_halt(_handle, _endpoint_out);
      }
      i++;
   }while((r==LIBUSB_ERROR_PIPE)&&(i<RETRY_MAX));
   if(r!= LIBUSB_SUCCESS){
      perr("   send_mass_storage_command: %s\n", libusb_strerror((enum libusb_error)r));
      return -1;
   }

//   printf("\t\t\tsent %d CDB bytes\n", cdb_len);
   return 0;
}

static int getDeviceReady(int dev_no, uint8_t endpoint)
{
   int i, r, size;
   struct command_status_wrapper csw;
   libusb_device_handle *_handle;

   _handle=tcon_handles[dev_no];

   // The device is allowed to STALL this transfer. If it does, you have to
   // clear the stall and try again.
   i=0;
   do{
      r=libusb_bulk_transfer(_handle, endpoint, (unsigned char*)&csw, 13,
                             &size, 500);
      switch(r){
         case LIBUSB_ERROR_TIMEOUT:
            printf("Error1: TIMEOUT!!\n");
            exit(1);
         case LIBUSB_ERROR_PIPE:
            printf("Error1: PIPE!!\n");
            exit(1);
         case LIBUSB_ERROR_OVERFLOW:
            printf("Error1: OVERFLOW!!\n");
            exit(1);
         case LIBUSB_ERROR_NO_DEVICE:
            printf("Error1: NO DEVICE!!\n");
            exit(1);
         case LIBUSB_ERROR_BUSY:
            printf("Error1: BUSY!!\n");
            exit(1);
         default:
            break;
      }

      if(r==LIBUSB_ERROR_PIPE){
         libusb_clear_halt(_handle, endpoint);
      }
      i++;
   }while((r==LIBUSB_ERROR_PIPE)&&(i<RETRY_MAX));
   if(r!=LIBUSB_SUCCESS){
      perr("   get_mass_storage_status: %s\n",
           libusb_strerror((enum libusb_error)r));
//      return -1;
      exit(1);
   }
   if(size!=13){
      perr("   get_mass_storage_status: received %d bytes (expected 13)\n", size);
      return -1;
   }

   // For this test, we ignore the dCSWSignature check for validity...
//   printf("\t\t\tMass Storage Status: %02X (%s)\n", csw.bCSWStatus, csw.bCSWStatus?"FAILED":"Success");

   if(csw.bCSWStatus){
      // REQUEST SENSE is appropriate only if bCSWStatus is 1, meaning that the
      // command failed somehow.  Larger values (2 in particular) mean that
      // the command couldn't be understood.
      if(csw.bCSWStatus==1)
         return -2;	// request Get Sense
      else
         return -1;
   }

   // In theory we also should check dCSWDataResidue.  But lots of devices
   // set it wrongly.
   return 0;
}

int usb_ReadNData(int dev_no, unsigned char* data, int data_length)
{
   uint8_t buffer[256];
   int i, r, size;
   libusb_device_handle *_handle;

   _handle=tcon_handles[dev_no];

   memset(buffer, 0x00, sizeof(buffer));
   i=0;
   do{
      r=libusb_bulk_transfer(_handle, _endpoint_in, (unsigned char*)&buffer,
                             data_length, &size, 500);
      if(r==LIBUSB_ERROR_PIPE){
         libusb_clear_halt(_handle, _endpoint_in);
      }
      i++;
   }while((r==LIBUSB_ERROR_PIPE)&&(i<RETRY_MAX));
//   printf("\t\t\tReceived %d bytes\n", size);

   memcpy(data, buffer, data_length);
//   display_buffer_hex(buffer, data_length);

   getDeviceReady(dev_no, _endpoint_in);
   return 0;
}

int usb_WriteNData(int dev_no, unsigned char* data, int data_length)
{
   int i, r, size;
   libusb_device_handle *_handle;

   _handle=tcon_handles[dev_no];

   i=0;
   do{
//      display_buffer_hex(data, 256);

      r=libusb_bulk_transfer(_handle, _endpoint_out, data, data_length,
                             &size, 0);

      if(r==LIBUSB_ERROR_PIPE){
         libusb_clear_halt(_handle, _endpoint_out);
      }
      switch(r){
         case LIBUSB_ERROR_TIMEOUT:
            printf("Error: TIMEOUT!!\n");
            exit(1);
         case LIBUSB_ERROR_PIPE:
            printf("Error: PIPE!!\n");
            exit(1);
         case LIBUSB_ERROR_OVERFLOW:
            printf("Error: OVERFLOW!!\n");
            exit(1);
         case LIBUSB_ERROR_NO_DEVICE:
            printf("Error: NO DEVICE!!\n");
            exit(1);
         case LIBUSB_ERROR_BUSY:
            printf("Error: BUSY!!\n");
            exit(1);
         default:
            break;
      }

      i++;
   }while((r==LIBUSB_ERROR_PIPE)&&(i<RETRY_MAX));

   getDeviceReady(dev_no, _endpoint_in);
   return 0;
}

int dev_usb_open(int dev_no)
{
   libusb_device *_dev;
   libusb_device_handle *_handle;
//   uint8_t bus, port_path[8];
   struct libusb_config_descriptor *conf_desc;
   const struct libusb_endpoint_descriptor *endpoint;
   int i, j, k, r;
   int iface, first_iface = -1;
   struct libusb_device_descriptor dev_desc;
   const char* speed_name[5] = { "Unknown", "1.5 Mbit/s (USB LowSpeed)", "12 Mbit/s (USB FullSpeed)",
		"480 Mbit/s (USB HighSpeed)", "5000 Mbit/s (USB SuperSpeed)"};
   char string[128];
   uint8_t string_index[3];	// indexes of the string descriptors

   _dev=tcon_dev[dev_no];
   r=libusb_open(_dev, &_handle);
   tcon_handles[dev_no]=_handle;

   CALL_CHECK_CLOSE(libusb_get_device_descriptor(_dev, &dev_desc), _handle);
   // Copy the string descriptors for easier parsing
   string_index[0]=dev_desc.iManufacturer;
   string_index[1]=dev_desc.iProduct;
   string_index[2]=dev_desc.iSerialNumber;

   CALL_CHECK_CLOSE(libusb_get_config_descriptor(_dev, 0, &conf_desc), _handle);
   _nb_ifaces=conf_desc->bNumInterfaces;
   if(_nb_ifaces>0)
      first_iface=conf_desc->interface[0].altsetting[0].bInterfaceNumber;
   for(i=0; i<_nb_ifaces; i++){
      for(j=0; j<conf_desc->interface[i].num_altsetting; j++){
         for(k=0; k<conf_desc->interface[i].altsetting[j].bNumEndpoints; k++){
            struct libusb_ss_endpoint_companion_descriptor *ep_comp=NULL;
            endpoint = &conf_desc->interface[i].altsetting[j].endpoint[k];

            // Use the first interrupt or bulk IN/OUT endpoints as default for testing
            if((endpoint->bmAttributes&LIBUSB_TRANSFER_TYPE_MASK)&
               (LIBUSB_TRANSFER_TYPE_BULK|LIBUSB_TRANSFER_TYPE_INTERRUPT)){
               if(endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN) {
                  if(!_endpoint_in)
                     _endpoint_in=endpoint->bEndpointAddress;
               }
               else{
                  if(!_endpoint_out)
                     _endpoint_out=endpoint->bEndpointAddress;
               }
            }
            libusb_get_ss_endpoint_companion_descriptor(NULL, endpoint,
                                                        &ep_comp);
            if(ep_comp){
               libusb_free_ss_endpoint_companion_descriptor(ep_comp);
            }
         }
      }
   }
   libusb_free_config_descriptor(conf_desc);

   libusb_set_auto_detach_kernel_driver(_handle, 1);
   for(iface=0; iface<_nb_ifaces; iface++){
      r=libusb_claim_interface(_handle, iface);
      if(r!=LIBUSB_SUCCESS) {
         perr("Interface Failed!!\n");
      }
   }

   return 0;
}

int usb_init()
{
   int i, j, r;
   libusb_device **_devs;
   libusb_device *_idev;
   struct libusb_device_descriptor dev_desc;

   version=libusb_get_version();

   r=libusb_init(NULL);
   if(r<0) return r;

   r=libusb_get_device_list(NULL, &_devs);
   if(r<0) return r;

   i=0;
   j=0;
   while((_idev=_devs[i++])!=NULL){
      r=libusb_get_device_descriptor(_idev, &dev_desc);
      if(r<0) return r;

      if(dev_desc.idVendor==0x048d){
         tcon_dev[j]=_idev;
         dev_usb_open(j);
         j++;
      }
   }

   tcon_no=j;
   printf("Total %d USB Device Found\n", tcon_no);

   return tcon_no;
}

void dev_usb_close(int dev_no)
{
   libusb_device_handle *_handle;
   int iface;

   _handle=tcon_handles[dev_no];

   for(iface=0; iface<_nb_ifaces; iface++) {
      libusb_release_interface(_handle, iface);
   }

   libusb_close(_handle);
}

void usb_close()
{
   libusb_exit(NULL);
}
