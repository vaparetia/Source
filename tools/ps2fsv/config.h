#ifndef _config_h_
#define _config_h_

#define DEFAULT_DEVICE  "/dev/usblinq0"

#define DEFAULT_ROOT    "/usr/local/ps2"

#define MAX_PS2_FILES   15   /* PlayStation2 側のリクエストで
				オープンできるファイルの最大数 */

#define CONFIG_BASE     getenv("HOME")
#define CONFIG_FILE     ".linQ"




#endif /* _config_h_ */
