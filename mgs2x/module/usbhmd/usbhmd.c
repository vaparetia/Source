/*
	usbkbd.c
		USB キーボードデータ取得ドライバ
		2000/04/05 K.Uehara
	$Id: usbhmd.c,v 1.1 2002/05/13 02:18:22 usr01475 Exp $
*/

#include <stdio.h>
#include <kernel.h>
#include <sif.h>
#include <sifcmd.h>

#include <usb.h>
#include <usbd.h>

#include "jsifman.h"
#include "usbhmd.h"

ModuleInfo Module = { "KCEJ_USB_HMD", 0x0101 };

typedef struct _unit {
	int number, c_pipe, d_pipe, payload, ifnum, as, count;
	u_char data[0];
} UNIT;

/* ---------------------------------------------------------------------- */
/*
	EE側に送信
*/

static void send_ee( UNIT *unit )
{
#if 0
	sceSifCmdSRData sysreg;
	u_char *p;

	p = unit->data;

	sysreg.rno = SIF_SYSREG_USBKBD;
	sysreg.value = p[ 0 ] | ( p[ 2 ] << 8 ) | ( p[ 3 ] << 16 ) | ( p[ 4 ] << 24 );
	sceSifSendCmd( SIF_CMDC_SET_SREG, &sysreg, sizeof( sysreg ), 0, 0, 0 );
#else
	sceSifCmdSRData sysreg[ 2 ];
	u_char *p;

	p = unit->data;

	sysreg[ 0 ].rno = SIF_SYSREG_HMD;
	sysreg[ 0 ].value = ( ( p[2] | ( p[3] << 8 ) ) << 16 ) | ( p[0] | ( p[1] << 8) );
	sceSifSendCmd( SIF_CMDC_SET_SREG, &sysreg[ 0 ], sizeof( sysreg ), 0, 0, 0 );
	sysreg[ 1 ].rno = SIF_SYSREG_HMD2;
	sysreg[ 1 ].value = ( ( p[6] | ( p[7] << 8 ) ) << 16 ) | ( p[4] | ( p[5] << 8) );
	sceSifSendCmd( SIF_CMDC_SET_SREG, &sysreg[ 1 ], sizeof( sysreg ), 0, 0, 0 );
#endif
}

#define TRUE	1

static int next_unit_number = 0;

static UNIT *unit_alloc(int payload, int ifnum, int as)
{
	UNIT *p;

	if(NULL != (p = AllocSysMemory(0, sizeof(UNIT) + payload, NULL))){
		p->number = next_unit_number ++;
		p->payload = payload;
		p->ifnum = ifnum;
		p->as = as;
		p->count = 0;
		memset( p->data, 0, p->payload );
	}
	return(p);
}

static void unit_free(UNIT *p)
{
	FreeSysMemory(p);
}

#define err(p, f, r)	if(r) printf("usbmouse%d: %s -> 0x%x\n", \
				(p)->number, (f), (r))

static void data_transfer(UNIT *unit);

static void data_transfer_done(int result, int count, void *arg)
{
	UNIT *unit = arg;

	err(unit, "sceUsbdInterruptTransfer", result);
	if(result == sceUsbd_NOERR){
#if 1
		{
			unsigned char *p;
			int i;

			p = unit->data;

			for( i = 0; i < count; i++ ){
				printf( "%02X ", p[ i ] );
			}
			printf( "\n" );
		}
#endif
		send_ee( unit );
	}

	data_transfer(unit);
}

static void data_transfer(UNIT *unit)
{
	int r;

	r = sceUsbdInterruptTransfer(unit->d_pipe,
		unit->data, unit->payload, data_transfer_done, unit);
	err(unit, "sceUsbdInterruptTransfer", r);
}

static void set_interface_done(int result, int count, void *arg)
{
	UNIT *unit = arg;

	err(unit, "sceUsbdSetInterface", result);
	data_transfer(unit);
}

static void set_config_done(int result, int count, void *arg)
{
	UNIT *unit = arg;
	int r;

	err(unit, "sceUsbdSetConfiguration", result);
	r = sceUsbdSetInterface(unit->c_pipe, unit->ifnum, unit->as,
		set_interface_done, unit);
	err(unit, "sceUsbdSetInterface", r);
}

static int usb_attach(int dev_id)
{
	UsbConfigurationDescriptor *cdesc;
	UsbInterfaceDescriptor *idesc;
	UsbEndpointDescriptor *edesc;
	UNIT *unit;
	int payload, r;

	if(NULL == (cdesc = sceUsbdScanStaticDescriptor(dev_id, NULL,
													USB_DESCRIPTOR_TYPE_CONFIGURATION))){
		return(-1);
	}

	if(NULL == (idesc = sceUsbdScanStaticDescriptor(dev_id, cdesc,
													USB_DESCRIPTOR_TYPE_INTERFACE))){
		return(-1);
	}

	if(NULL == (edesc = sceUsbdScanStaticDescriptor(dev_id, idesc,
													USB_DESCRIPTOR_TYPE_ENDPOINT))){
		return(-1);
	}
#if 0
	if(cdesc->bNumInterfaces != 2){
		return(-1);
	}
	if(idesc->bNumEndpoints != 1){
		return(-1);
	}
#endif

	if((edesc->bEndpointAddress & USB_ENDPOINT_DIRECTION_BITS)
	   != USB_ENDPOINT_DIRECTION_IN){
		return(-1);
	}
	if((edesc->bmAttribute & USB_ENDPOINT_TRANSFER_TYPE_BITS)
	   != USB_ENDPOINT_TRANSFER_TYPE_INTERRUPT){
		return(-1);
	}
	payload = edesc->wMaxPacketSize0 | (edesc->wMaxPacketSize1 << 8);

	if(NULL == (unit = unit_alloc(payload, idesc->bInterfaceNumber,
								  idesc->bAlternateSetting))){
		return(-1);
	}
	if(0 > (unit->c_pipe = sceUsbdOpenPipe(dev_id, NULL))){
		return(-1);
	}
	if(0 > (unit->d_pipe = sceUsbdOpenPipe(dev_id, edesc))){
		return(unit_free(unit), -1);
	}
	sceUsbdSetPrivateData(dev_id, unit);

	if(sceUsbd_NOERR != (r = sceUsbdSetConfiguration(unit->c_pipe,
			cdesc->bConfigurationValue, set_config_done, unit))){
		err(unit, "sceUsbdSetConfiguration", r);
		return(-1);
	}

	printf("usbhmd %d: attached\n", unit->number);

	return(0);
}

static int usb_detach(int dev_id)
{
	UNIT *unit;

	if(NULL == ( unit = sceUsbdGetPrivateData(dev_id) ) ){
		/* 自分が確保したデータ領域がない */
		return(-1);
	}

	printf("usbhmd %d: detached\n", unit->number);
	memset( unit->data, 0, unit->payload );
	send_ee( unit );
	unit_free(unit);

	return(0);
}

static int usb_probe(int dev_id)
{
	UsbDeviceDescriptor *ddesc;
	UsbInterfaceDescriptor *idesc;

	if( NULL == ( ddesc = sceUsbdScanStaticDescriptor( dev_id, NULL,
													USB_DESCRIPTOR_TYPE_DEVICE ) ) ){
		return 0;
	}
	if( NULL == ( idesc = sceUsbdScanStaticDescriptor( dev_id, ddesc,
													USB_DESCRIPTOR_TYPE_INTERFACE ) ) ){
		return 0;
	}

	if( ddesc->bDeviceClass != 0 || ddesc->bNumConfigurations != 1 ){
		return 0;
	}
	if( idesc->bInterfaceClass != 3
	   || idesc->bInterfaceSubClass != 0
	   || idesc->bInterfaceProtocol != 0 ){
		return 0;
	}
	Kprintf( "usb HMD detect\n" );

	return(1);
}

static sceUsbdLddOps usb_ops = {
	NULL, NULL,
	"usbhmd",
	usb_probe,
	usb_attach,
	usb_detach,
};

int usbhmd_start(void)
{
	int r;

	printf( "USB HMD Driver ver.0.01\n" );

	/* USBドライバに登録して常駐終了 */
	if( ( r = sceUsbdRegisterLdd( &usb_ops ) ) != sceUsbd_NOERR ){
		printf("ERR: sceUsbdRegisterLdd -> 0x%x\n", r);
		return NO_RESIDENT_END;
	}

	return RESIDENT_END;
}
