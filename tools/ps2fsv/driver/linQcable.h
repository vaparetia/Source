#ifndef _linQcable_h_
#define _linQcable_h_
/*
 * include files
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#ifdef __KERNEL__
#include <asm/uaccess.h>
#endif /* __KERNEL__ */
#include <linux/init.h>
#include <linux/malloc.h>
#include <linux/delay.h>
#include <linux/ioctl.h>

#include <linux/usb.h>


/* =========================================================================
 *                           Macro
 * ========================================================================= */

#define LNQ_MAX_MNR 8
#define LNQ_BASE_MNR 66

#define LNQ_NAME   "usb_linQcable"
#define LNQ_MIN_ENDPOINTS 2
#define LNQ_MAX_ENDPOINTS 3

#define IS_EP_BULK(ep)  ((ep).bmAttributes == USB_ENDPOINT_XFER_BULK ? 1 : 0)
#define IS_EP_BULK_IN(ep) (IS_EP_BULK(ep) && ((ep).bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)
#define IS_EP_BULK_OUT(ep) (IS_EP_BULK(ep) && ((ep).bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT)
#define IS_EP_INTR(ep) ((ep).bmAttributes == USB_ENDPOINT_XFER_INT ? 1 : 0)

#define USB_LNQ_MINOR(X) (MINOR((X)->i_rdev) - LNQ_BASE_MNR)


#define OBUF_SIZE (32*1024)
#define IBUF_SIZE (32*1024)

/* =========================================================================
 *                             Type defines
 * ========================================================================= */
#ifdef __KERNEL__
struct lnQ_usb_data {
  struct usb_device * lnQ_dev;
  struct urb          lnQ_irq_urb;
  unsigned int        ifnum;   /* interface number */
  kdev_t              lnQ_minor;
  unsigned char		  statbuf[ 8 ];
  unsigned char       type;
  unsigned char       status;  /* status by interrupt */
  char                isopen;  /* Not zero if the device is open */
  char                present; /* Not zero if device is present  */
  char bulk_in_ep, bulk_out_ep, intr_ep; /* Endpoint assignments */
  char *obuf, *ibuf;
};

extern struct list_head usb_driver_list;
extern struct list_head usb_bus_list;

static struct usb_driver lnQ_driver;

#endif __KERNEL__

struct lnQ_port_info {
  int busnum;   /* USB bus           */
  int level;    /* connection level  */
  int parent;   /* Parent HUB device */
  int port;     /* port              */
  int devnum;   /* device number     */
};



/* =========================================================================
 *                           IOCTL function
 * ========================================================================= */
#define LNQ_IOCTL_SYNCSTAT _IO('U', 0x10)
#define LNQ_IOCTL_GETPORT _IOR('U', 0x11, struct lnQ_port_info)
#define LNQ_IOCTL_CHECK		_IOR( 'U', 0x12, int )

#endif /* _linQcable_h_ */
