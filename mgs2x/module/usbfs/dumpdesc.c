/*
	dumpdesc.c
		usb デスクリプタダンプサブルーチン

		$Id: dumpdesc.c,v 1.2 2000/06/07 03:34:20 usr01475 Exp $
*/

#include <stdio.h>
#include <kernel.h>
#include <memory.h>
#include <sif.h>
#include <sifcmd.h>
#include <sifrpc.h>
#include <string.h>

#include <usb.h>
#include <usbd.h>

/*
	デスクプリタのダンプを行なう
	デバッグ用
*/

void dump_usb_desc( int dev_id )
{
	void *desc;

	desc = NULL;
	printf( "\n\n---- DUMP static Descriptor ----\n" );
	while( ( desc = sceUsbdScanStaticDescriptor(dev_id, desc, 0 ) ) != NULL ){
		struct {
			u_char bLength;
			u_char bDescriptorType;
		} *header;
		header = ( typeof( header ) )desc;
		switch( header->bDescriptorType ){
		  case USB_DESCRIPTOR_TYPE_DEVICE:
			{
				UsbDeviceDescriptor *ddesc = desc;
				printf( "--Device--\n" );
				printf( "bLength 0x%02X\n", ddesc->bLength );
				printf( "bDescriptorType 0x%02X\n", ddesc->bDescriptorType );
				printf( "bcdUSB 0x%02X\n", ddesc->bcdUSB );
				printf( "bDeviceClass 0x%02X\n", ddesc->bDeviceClass );
				printf( "bDeviceSubClass 0x%02X\n", ddesc->bDeviceSubClass );
				printf( "bDeviceProtocol 0x%02X\n", ddesc->bDeviceProtocol );
				printf( "bMaxPacketSize0 0x%02X\n", ddesc->bMaxPacketSize0 );
				printf( "idVendor 0x%02X\n", ddesc->idVendor );
				printf( "idProduct 0x%02X\n", ddesc->idProduct );
				printf( "bcdDevice 0x%02X\n", ddesc->bcdDevice );
				printf( "iManufacturer 0x%02X\n", ddesc->iManufacturer );
				printf( "iProduct 0x%02X\n", ddesc->iProduct );
				printf( "iSerialNumber 0x%02X\n", ddesc->iSerialNumber );
				printf( "bNumConfigurations 0x%02X\n", ddesc->bNumConfigurations );
			}
			break;
		  case USB_DESCRIPTOR_TYPE_CONFIGURATION:
			{
				UsbConfigurationDescriptor *cdesc = desc;
				printf( "---ConfigDesc---\n" );
				printf( "bLength 0x%02X\n", cdesc->bLength );
				printf( "bDecsriptorType 0x%02X\n", cdesc->bDescriptorType );
				printf( "wTotalLength0 0x%02X\n", cdesc->wTotalLength0 );
				printf( "wTotalLength1 0x%02X\n", cdesc->wTotalLength1 );
				printf( "bNumInterfaces 0x%02X\n", cdesc->bNumInterfaces );
				printf( "bConfigurationValue 0x%02X\n", cdesc->bConfigurationValue );
				printf( "iConfiguration 0x%02X\n", cdesc->iConfiguration );
				printf( "bmAttribute 0x%02X\n", cdesc->bmAttribute );
				printf( "MaxPower 0x%02X\n", cdesc->MaxPower );
			}
			break;
		  case USB_DESCRIPTOR_TYPE_STRING:
			{
				UsbStringDescriptor *sdesc = desc;

				printf( "---StringDesc---\n" );
				printf( "bDescriptorType 0x%02X\n", sdesc->bDescriptorType );
				printf( "bString %s\n", sdesc->bString );
			}
			break;
		  case USB_DESCRIPTOR_TYPE_INTERFACE:
			{
				UsbInterfaceDescriptor *idesc = desc;
				printf( "--Interface--\n" );
				printf( "bLength 0x%02X\n", idesc->bLength );
				printf( "bDescriptorType 0x%02X\n", idesc->bDescriptorType );
				printf( "bInterfaceNumber 0x%02X\n", idesc->bInterfaceNumber );
				printf( "bAlternateSetting 0x%02X\n", idesc->bAlternateSetting );
				printf( "bNumEndpoints 0x%02X\n", idesc->bNumEndpoints );
				printf( "bInterfaceClass 0x%02X\n", idesc->bInterfaceClass );
				printf( "bInterfaceSubClass 0x%02X\n", idesc->bInterfaceSubClass );
				printf( "bInterfaceProtocol 0x%02X\n", idesc->bInterfaceProtocol );
				printf( "iInterface 0x%02X\n", idesc->iInterface );
			}
			break;
		  case USB_DESCRIPTOR_TYPE_ENDPOINT:
			{
				UsbEndpointDescriptor *edesc = desc;

				printf( "ENDPOINT ADR 0x%02X ATR 0x%02X SIZE 0x%02X INTR 0x%02X\n"
						, edesc->bEndpointAddress
						, edesc->bmAttribute
						, edesc->wMaxPacketSize0 | ( edesc->wMaxPacketSize1 << 8 )
						, edesc->bInterval );
			}
			break;
		  default:
			{
				if( header->bDescriptorType == 0x21 ){
					struct {
						u_char bLength;
						u_char bDescriptorType;
						u_char bcdUSB;
						u_char bcdUSB1;
						u_char bContryCode;
						u_char bNumDescriptors;
						u_char bDescriptorType2;
						u_char wDescriptorLength0;
						u_char wDescriptorLength1;
					} *hid;
					hid = ( typeof( hid ) )header;
					printf( "--HID--\n" );
					printf( "bLength 0x%02X\n", hid->bLength );
					printf( "bDescriptorType 0x%02X\n", hid->bDescriptorType );
					printf( "bcdUSB 0x%02X\n", hid->bcdUSB );
					printf( "bContryCode 0x%02X\n", hid->bContryCode );
					printf( "bNumDescriptors 0x%02X\n", hid->bNumDescriptors );
					printf( "bDescriptorType2 0x%02X\n", hid->bDescriptorType2 );
					printf( "wDescriptorLength0 0x%02X\n", hid->wDescriptorLength0 );
					printf( "wDescriptorLength1 0x%02X\n", hid->wDescriptorLength1 );
				} else {
					printf( "--Unknown--\n" );
					printf( "bLength 0x%02X\n", header->bLength );
					printf( "bDescriptorType 0x%02X\n", header->bDescriptorType );
					{
						int	i;
						u_char *p;
						p = ( u_char * )( header + 1 );
						for( i = 2; i < header->bLength; i++ ){
							printf( "%02X ", *p );
							p++;
						}
						printf( "\n" );
					}
				}
				break;
			}
		}
	}
	printf( "----- DUMP END -----\n" );
}

