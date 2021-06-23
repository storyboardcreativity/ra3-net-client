CC := gcc
CFL := -ggdb -fPIC -Wmultichar -Wno-multichar -Wno-literal-suffix
TOOLCHAIN :=
OUT_EXECUTABLE_NAME := ra3-net-client
ARCH :=
LIBS := -lpthread -lstdc++ -lcurl -lcurlpp
INCLUDES := -Inana/include

# Link Nana GUI
LIBS += -Lnana/build/bin -lnana -lm -lX11 -lXft -lfontconfig -lstdc++fs

# C++ files
SOURCE_FILES := main.cpp
SOURCE_FILES += terminal_printing_system.cpp
SOURCE_FILES += printing_tools.cpp
SOURCE_FILES += gamespy.cpp
SOURCE_FILES += fesl.cpp
SOURCE_FILES += connection/peerchat.cpp
SOURCE_FILES += connection/gpcm.cpp
SOURCE_FILES += ra3_engine_tools/ra3_engine_tools.cpp
SOURCE_FILES += ra3_engine_tools/md5.cpp
SOURCE_FILES += connection/connection.cpp
SOURCE_FILES += auth_info.cpp

# C files
SOURCE_FILES += peerchat_encryption.c

# DirtySDKEAWebKit
SOURCE_FILES += DirtySDKEAWebKit/cryptarc4.c
SOURCE_FILES += DirtySDKEAWebKit/dirtylibps3.c
SOURCE_FILES += DirtySDKEAWebKit/dirtynetps3.c
SOURCE_FILES += DirtySDKEAWebKit/protossl.c
SOURCE_FILES += DirtySDKEAWebKit/platformsocketapi.c
SOURCE_FILES += DirtySDKEAWebKit/socketapicallbacks.c
SOURCE_FILES += DirtySDKEAWebKit/dirtymem.c
SOURCE_FILES += DirtySDKEAWebKit/cryptmd5.c
SOURCE_FILES += DirtySDKEAWebKit/cryptsha1.c
SOURCE_FILES += DirtySDKEAWebKit/cryptmd2.c
SOURCE_FILES += DirtySDKEAWebKit/cryptrsa.c
SOURCE_FILES += DirtySDKEAWebKit/lobbybase64.c
SOURCE_FILES += DirtySDKEAWebKit/dirtynet.c
SOURCE_FILES += DirtySDKEAWebKit/string_tools.c

all:
	$(TOOLCHAIN)$(CC) $(CFL) $(ARCH) $(SOURCE_FILES) $(LIBS) $(INCLUDES) -o $(OUT_EXECUTABLE_NAME)

clear:
	rm -f *.o
	rm -f $(OUT_EXECUTABLE_NAME)