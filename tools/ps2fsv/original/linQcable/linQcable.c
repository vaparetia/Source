#define _linQcable_c_
#include <linux/linQcable.h>

#ifndef _DEBUG_
#define _DEBUG_
#endif /* _DEBUG_ */

/* #define DISABLE_IRQ */

#ifdef _DEBUG_
#define DBG(args...)  printk(args)
#else
#define DBG(args...)
#endif /* _DEBUG_ */

#define LNQ_IOCTL

MODULE_AUTHOR("Yoshihito Kira, ykira@fsinet.or.jp");
MODULE_DESCRIPTION("USB LinQ CABLE Driver");
static __s32 vendor = -1, product = -1;
MODULE_PARM(vendor, "i");
MODULE_PARM_DESC(vendor, "User specified USB idVendor");

MODULE_PARM(product, "i");
MODULE_PARM_DESC(product, "User specified USB idProduct");

/* =========================================================================
 *                          Module Macro
 * ========================================================================= */
/*
 * Vendor ID の定義
 */
#define VENDOR_UNKNOWN 0x067b  /* LinQ CABLEvendor */

/* =========================================================================
 *                          Module variables
 * ========================================================================= */
#define DEVICE_ENTRY  2   /* ドライバが対応している Vendor/Product の組合せ */

static const struct { __u16 vendor; __u16 product; } device_list[] = {
  { VENDOR_UNKNOWN, 0x0000 },   /* USB LinQ CABLE */
  { VENDOR_UNKNOWN, 0x0001 },   /* ELECOM UC_NBB  */
};

static struct lnQ_usb_data * p_lnQ_table[ LNQ_MAX_MNR ] = { NULL, /* ... */ };


/* =========================================================================
 *                               functons
 * ========================================================================= */

/*
 * 割り込み処理関数(?)
 */
static void lnQ_irq(struct urb *urb)
{
  struct lnQ_usb_data * lnQ = urb->context;
  unsigned char *data = &(lnQ->status);

  data += 0;  /* Keep gcc from complaining about unused var 
		 …つまりはオプティマイズ対策らしい */

  if(urb->status) return;

  /* DBG("lnQ_irq(%d): data:%x\n", lnQ->lnQ_minor, *data); */
  return;
}


/*
 * 検出処理関数
 */
static void * lnQ_probe(struct usb_device * dev, unsigned int ifnum)
{
  struct lnQ_usb_data * lnQ;   /* データ交換用構造体領域のポインタ
				* データ交換用構造体は、各ドライバ
				* 独自のものを用いる(らしい)          */
  struct usb_interface_descriptor *interface;
  struct usb_endpoint_descriptor *endpoint;

  kdev_t lnQ_minor;
  int i, ep_cnt;
  char valid_device = 0;
  char have_bulk_in, have_bulk_out, have_intr;

  DBG("Vendor:Product = 0x%04x:0x%04x\n", vendor, product);
  /*
   * idVendor と idProduct をリスト (device_list[]) にあるものと比較し,
   * 該当するものがあれば valid_device の値を 1 にする
   */
  valid_device = 0;
  for(i = 0; i < DEVICE_ENTRY; i++)
    if((dev->descriptor.idVendor == device_list[i].vendor) &&
       (dev->descriptor.idProduct == device_list[i].product))
      {
	valid_device = 1;
	break;
      }

  /*
   * valid_device の値が 0, つまりリスト中に挙げられた対応デバイスで
   * ない場合は,検出に失敗したものとする。
   */
  if(!valid_device) return NULL;

  vendor = dev->descriptor.idVendor;
  product = dev->descriptor.idProduct;

#ifdef _DEBUG_
  /* ここでは、デバイスの descriptor 情報をコンソールに出力 */
  printk("-- USB Device ---\n"
	 "bLength:            0x%02x\n"	 "bDescriptorType:    0x%02x\n"
	 "bcdUSB:             0x%04x\n"	 "bDeviceClass:       0x%02x\n"
	 "bDeviceSubClass:    0x%02x\n"	 "bDeviceProtocol:    0x%02x\n"
	 "bMaxPacketSize0:    0x%02x\n"	 "idVendor:           0x%04x\n"
	 "idProduct:          0x%04x\n"	 "bcdDevice:          0x%04x\n"
	 "iManufacturer:      0x%02x\n"	 "iProduct:           0x%02x\n"
	 "iSerialNumber:      0x%02x\n"	 "bNumConfigurations: 0x%02x\n",
	 dev->descriptor.bLength,	  dev->descriptor.bDescriptorType,
	 dev->descriptor.bcdUSB,	  dev->descriptor.bDeviceClass,
	 dev->descriptor.bDeviceSubClass, dev->descriptor.bDeviceProtocol,
	 dev->descriptor.bMaxPacketSize0, dev->descriptor.idVendor,
	 dev->descriptor.idProduct,	  dev->descriptor.bcdDevice,
	 dev->descriptor.iManufacturer,	  dev->descriptor.iProduct,
	 dev->descriptor.iSerialNumber,	  dev->descriptor.bNumConfigurations);
#endif /* _DEBUG_ */


  
  /* =======================================================================
   * 検出に成功した場合は,デバイスの初期化を行う
   * ======================================================================= */

  /* バルク転送に必要なエンドポイントの情報を取得し,初期化を行う */

  if(dev->descriptor.bNumConfigurations != 1)
    {
      info("lnQ_probe: Only one device configuration is supported.");
      return NULL;
    }
  if(dev->config[0].bNumInterfaces != 1)
    {
      info("lnQ_probe: Only one device interface is supported.");
      return NULL;
    }

  interface = dev->config[0].interface[ifnum].altsetting;
  endpoint = interface[ifnum].endpoint;

  if(interface->bNumEndpoints != 3)
    {
      info("lnQ_probe: Only three endpoints supported.");
      return NULL;
    }

  ep_cnt = have_bulk_in = have_bulk_out = have_intr = 0;

  while(ep_cnt < interface->bNumEndpoints)
    {
      if(!have_bulk_in && IS_EP_BULK_IN(endpoint[ep_cnt]))
	{
	  ep_cnt++;
	  have_bulk_in = ep_cnt;
	  continue;
	}

      if(!have_bulk_out && IS_EP_BULK_OUT(endpoint[ep_cnt]))
	{
	  ep_cnt++;
	  have_bulk_out = ep_cnt;
	  continue;
	}

      if(!have_intr && IS_EP_INTR(endpoint[ep_cnt]))
	{
	  ep_cnt++;
	  have_intr = ep_cnt;
	  continue;
	}
      info("lnQ_probe: Undetected endpoint. Notify the maintener.");

      return NULL;
    }
  switch(interface->bNumEndpoints)
    {
    case 2:
      if(!have_bulk_in || !have_bulk_out)
	{
	  info("lnQ_probe: Two bulk endpoints reqwuired.");
	  return NULL;
	}
      break;
    case 3:
      if(!have_bulk_in || !have_bulk_out || !have_intr)
	{
	  info("lnQ_probe: Two bulk endpoints and one interrupt endpoint required.");
	  return NULL;
	}
      break;
    default:
      info("lnQ_probe: Endpoint determination failed. Notify the mainteiner.");
      return NULL;
    }
  
  
  for(lnQ_minor = 0; lnQ_minor < LNQ_MAX_MNR; lnQ_minor++)
    if(!p_lnQ_table[lnQ_minor]) break;

  if(p_lnQ_table[lnQ_minor])
    {
      err("lnQ_probe: No more minor devices remaining.");
      return NULL;
    }

  
  /* データ交換構造体領域を確保 */
  if(!(lnQ = kmalloc(sizeof(struct lnQ_usb_data), GFP_KERNEL)))
    {
      err("lnQ_probe: Out of memory.");
      return NULL;
    }
  memset(lnQ, 0, sizeof(struct lnQ_usb_data));

#ifndef DISABLE_IRQ
  if(have_intr)
    {
      DBG("lnQ_probe(%d): Configuring IRQ handler for intr EP:%d\n", lnQ_minor, have_intr);
      FILL_INT_URB(&lnQ->lnQ_irq, dev,
		   usb_rcvintpipe(dev, have_intr),
		   &lnQ->status, 1,lnQ_irq, lnQ, 1);

      if(usb_submit_urb(&lnQ->lnQ_irq))
	{
	  err("lnQ_probe(%d): Unable to allocate INT URB.", lnQ_minor);
	  kfree(lnQ);
	  return NULL;
	}
    }
#endif /* DISABLE_IRQ */

  if(!(lnQ->obuf = (char *)kmalloc(OBUF_SIZE, GFP_KERNEL)))
    {
      err("lnQ_probe(%d): Not enough memory for the output buffer.", lnQ_minor);
      kfree(lnQ);
      return NULL;
    }
  if(!(lnQ->ibuf = (char *)kmalloc(IBUF_SIZE, GFP_KERNEL)))
    {
      err("lnQ_probe(%d): Not enough memory for the input buffer.", lnQ_minor);
      kfree(lnQ->obuf);
      kfree(lnQ);
      return NULL;
    }
  
  lnQ->bulk_in_ep  = have_bulk_in;
  lnQ->bulk_out_ep = have_bulk_out;
  lnQ->intr_ep     = have_intr;
  lnQ->present     = 1;
  lnQ->lnQ_dev     = dev;
  lnQ->lnQ_minor   = lnQ_minor;
  lnQ->isopen      = 0;

  DBG("linQ CABLE Driver is registered.\n");
  
  return p_lnQ_table[lnQ_minor] = lnQ;
}


/*
 * 接続切断処理関数
 */
static void lnQ_disconnect(struct usb_device * dev, void * ptr)
{
  struct lnQ_usb_data * lnQ = (struct lnQ_usb_data *)ptr;

#ifndef DISABLE_IRQ
  if(lnQ->intr_ep)
    {
      DBG("lnQ_disconnect(%d): Unlinking IRQ URB\n", lnQ->lnQ_minor);
      usb_unlink_urb(&lnQ->lnQ_irq);
    }
#endif /* DISABLE_IRQ */

  usb_driver_release_interface(&lnQ_driver,
			       &lnQ->lnQ_dev
			       ->actconfig->interface[lnQ->ifnum]);
  kfree(lnQ->ibuf);
  kfree(lnQ->obuf);
  DBG("lnQ_disconnect: De-allocating minor: %d\n", lnQ->lnQ_minor);
  p_lnQ_table[lnQ->lnQ_minor] = NULL;
  kfree(lnQ);
  DBG("Disconnect USB device.\n");
}

#ifdef LNQ_IOCTL

/*
 * デバイスの所在を突き止める
 */
static struct lnQ_port_info * lnQ_search_device(struct usb_device *dev,
						struct lnQ_port_info *info)
{
  struct usb_device * usbdev;
  int level;     /* 接続階層 */
  int i;

  info->devnum = dev->devnum;         /* 当該デバイスのデバイス番号 */
  info->busnum = dev->bus->busnum;    /* USBバス番号(通常1)         */
  info->parent = dev->parent->devnum; /* 親となる USB hub のデバイス番号 */

  /* port 番号を求める */
  for(i = 0; i < dev->parent->maxchild; i++)
    if(dev->parent->children[i] == dev)
      {
	info->port = i;
	break;
      }

  /* USB デバイスの接続階層レベルを求める */
  level = 0;
  usbdev = dev;
  while(usbdev->parent)
    {
      level ++;
      usbdev = usbdev->parent;
    }
  info->level = level;

  return info;
}

static int lnQ_ioctl(struct inode *inode, struct file *file,
		     unsigned int cmd, unsigned long arg)
{
  struct usb_device *dev;
  struct lnQ_usb_data *lnQ;
  int result = 0;

  kdev_t lnQ_minor;
  lnQ_minor = USB_LNQ_MINOR(inode);
  lnQ = file->private_data;

  if(!p_lnQ_table[lnQ_minor])
    {
      err("lnQ_ioctl(%d): invalid lnQ_minor", lnQ_minor);
      return -ENODEV;
    }

  dev = p_lnQ_table[ lnQ_minor ]->lnQ_dev;

  switch(cmd)
    {
    case LNQ_IOCTL_CLEANUP:  /* エンドポイントに溜っているデータをクリアする */
      {
	
      }
      break;

    case LNQ_IOCTL_GETPORT:  /* 機器が接続されているポートの取得 */
      {
	struct lnQ_port_info inf;

	lnQ_search_device(dev, &inf);  /* ポート情報を得る */
	if(copy_to_user((void *)arg, &inf, sizeof(struct lnQ_port_info)))
	  return -EFAULT;
      }
      break;

    default:
      return -ENOIOCTLCMD;
    }

  return 0;
}
#endif /* LNQ_IOCTL */

/*
 * デバイスの読み込み(すなわち LNQ 側からのデータ受信)
 */
static ssize_t lnQ_read(struct file * file, char * buffer,
			size_t count, loff_t *ppos)
{
  struct lnQ_usb_data * lnQ;
  struct usb_device *dev;
  ssize_t bytes_read;   /* overall count of bytes_read */
  ssize_t ret;
  kdev_t  lnQ_minor;
  int partial;   /* Number of bytes successfully read */
  int this_read; /* Max number of bytes to read */
  int result;
  char *ibuf;

  lnQ = file->private_data;
  lnQ_minor = lnQ->lnQ_minor;
  ibuf = lnQ->ibuf;
  dev = lnQ->lnQ_dev;

  DBG("lnQ_read(%d): count = %d\n", lnQ_minor, count);

  bytes_read = 0;
  ret = 0;

  while(count)
    {
      if(signal_pending(current))
	{
	  ret = -EINTR;
	  break;
	}
      this_read = (count >= IBUF_SIZE) ? IBUF_SIZE : count;

      result = usb_bulk_msg(dev, usb_rcvbulkpipe(dev, lnQ->bulk_in_ep),
			    ibuf, this_read, &partial, 120*HZ);
      DBG("read stats(%d): result:%d this_read:%d partial:%d\n", lnQ_minor, result, this_read, partial);

      if(result == USB_ST_TIMEOUT)
	{
	  warn("lnQ_read(%d): NAK recieved", lnQ_minor);
	  ret = -ETIME;
	  break;
	}
      else if((result < 0) && (result != USB_ST_DATAUNDERRUN))
	{
	  warn("lnQ_read(%d): funky result: %d. Prease notify the maintainer.",
	       lnQ_minor, (int)result);
	  ret = -EIO;
	  break;	  
	}
      if(partial)
	{
	  if(copy_to_user(buffer, ibuf, this_read))
	    {
	      ret = -EFAULT;
	      break;
	    }
	  count -= partial;
	  bytes_read += partial;
	  buffer += partial;
	}
      else
	{
	  ret = 0;
	  break;
	}
    }
  DBG("lnQ_read(%d): finished.\n", lnQ_minor);

  return ret ? ret : bytes_read;
}

/*
 * デバイスの書き込み(すなわち LNQ 側へのデータ送出)
 */
static ssize_t lnQ_write(struct file * file, const char * buffer,
			 size_t count, loff_t *ppos)
{
  struct lnQ_usb_data * lnQ;
  struct usb_device *dev;

  ssize_t bytes_written = 0;
  ssize_t ret = 0;

  kdev_t lnQ_minor;

  int this_write;    /* Number of bytes to write             */
  int partial;       /* Number of bytes successfully written */
  int result = 0;

  char * obuf;

  lnQ = file->private_data;
  lnQ_minor = lnQ->lnQ_minor;

  obuf = lnQ->obuf;
  dev = lnQ->lnQ_dev;
  while(count > 0)
    {
      if(signal_pending(current)) {
	ret = -EINTR;
	break;
      }

      this_write = (count >= OBUF_SIZE) ? OBUF_SIZE : count;
      if(copy_from_user(lnQ->obuf, buffer, this_write))
	{
	  ret = -EFAULT;
	  break;
	}
      result = usb_bulk_msg(dev, usb_sndbulkpipe(dev, lnQ->bulk_out_ep),
			    obuf, this_write, &partial, 60 * HZ);

      DBG("write stats(%d): result: %d this_write:%d partial:%d\n", lnQ_minor, result, this_write, partial);

      if(result == USB_ST_TIMEOUT)  /* NAK -- shouldn't happen */
	{
	  warn("lnQ_write: NAK recieved.");
	  ret = -ETIME;
	  break;
	}
      else if(result < 0) /* We should not get any I/O errors */
	{
	  warn("lnQ_write(%d): funky result: %d. Prease norify the maintainer.", lnQ_minor, result);
	  ret = -EIO;
	  break;
	}
      if(partial != this_write)
	{
	  ret = -EIO;
	  break;
	}
      if(partial)  /* Data written */
	{
	  buffer += partial;
	  count -= partial;
	  bytes_written += partial;
	}
      else         /* No data written */
	{
	  ret = 0;
	  bytes_written = 0;
	  break;
	}
    }
  mdelay(5);  /* This seems to help with SANE queries */
  DBG("lnQ_write(%d): finished.\n", lnQ_minor);
  return ret ? ret : bytes_written;
}


/*
 * LNQ 送受信デバイスのオープン
 */
static int lnQ_open(struct inode * inode, struct file * file)
{
  struct lnQ_usb_data * lnQ;
  struct usb_device *dev;
  kdev_t lnQ_minor;

  lnQ_minor = USB_LNQ_MINOR(inode);

  if(!p_lnQ_table[lnQ_minor])
    {
      err("lnQ_open(%d): invalid lnQ_minor", lnQ_minor);
      return -ENODEV;
    }

  lnQ = p_lnQ_table[lnQ_minor];

  dev = lnQ->lnQ_dev;

  if(!dev)          return -ENODEV;
  if(!lnQ->present) return -ENODEV;
  if(lnQ->isopen)   return -EBUSY;

  lnQ->isopen = 1;

  file->private_data = lnQ;

  MOD_INC_USE_COUNT;

  return 0;  
}

/*
 * LNQ 送受信デバイスのクローズ
 */
static int lnQ_close(struct inode * inode, struct file * file)
{
  struct lnQ_usb_data * lnQ;
  kdev_t lnQ_minor;

  lnQ_minor = USB_LNQ_MINOR(inode);

  if(!p_lnQ_table[ lnQ_minor ])
    {
      err("lnQ_close(%d): inbvalid lnQ_minor", lnQ_minor);
      return -ENODEV;
    }
  lnQ = p_lnQ_table[ lnQ_minor ];
  lnQ->isopen = 0;

  file->private_data = NULL;

  MOD_DEC_USE_COUNT;

  return 0;
}





static struct
file_operations usb_lnQ_fops = {
  read:     lnQ_read,
  write:    lnQ_write,
#ifdef LNQ_IOCTL
  ioctl:    lnQ_ioctl,
#endif /* LNQ_IOCTL */
  open:     lnQ_open,
  release:  lnQ_close,
};

/*
 * USB driver structure.
 */
static struct usb_driver lnQ_driver = {
  name:        LNQ_NAME,
  probe:       lnQ_probe,
  disconnect:  lnQ_disconnect,
  driver_list: {NULL, NULL},
  fops:        &usb_lnQ_fops,
  minor:       LNQ_BASE_MNR
};

void __exit usb_lnQ_exit(void)
{
  usb_deregister(&lnQ_driver);
}

int __init usb_lnQ_init(void)
{
  if(usb_register(&lnQ_driver) < 0)  return -1;
  
  info("USB linQ CABLE Driver registered.");
  return 0;
}



module_init(usb_lnQ_init);
module_exit(usb_lnQ_exit);
