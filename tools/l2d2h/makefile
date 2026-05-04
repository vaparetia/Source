# copyするだけ。

TARGET = l2d2h

ifeq ("$(OSTYPE)","linux")
INSTALL_PATH = /u/develop/mj001b/tools/bin/linux
else
INSTALL_PATH = /u/develop/mj001b/exports/tools/bin/mgs2xbox
endif

install :
	chmod +x $(TARGET)
	cp $(TARGET) $(INSTALL_PATH)
