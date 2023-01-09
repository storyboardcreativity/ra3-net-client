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
SOURCE_FILES := src/main.cpp
SOURCE_FILES += src/terminal_printing_system.cpp
SOURCE_FILES += src/printing_tools.cpp
SOURCE_FILES += src/gamespy.cpp
SOURCE_FILES += src/fesl.cpp
SOURCE_FILES += src/connection/peerchat.cpp
SOURCE_FILES += src/connection/gpcm.cpp
SOURCE_FILES += src/connection/master_server.cpp
SOURCE_FILES += src/ra3_engine_tools/ra3_engine_tools.cpp
SOURCE_FILES += src/ra3_engine_tools/md5.cpp
SOURCE_FILES += src/connection/connection.cpp
SOURCE_FILES += src/auth_info.cpp

# C files
SOURCE_FILES += src/peerchat_encryption.c
SOURCE_FILES += src/connection/enctypex_decoder.c

# DirtySDKEAWebKit
SOURCE_FILES += src/DirtySDKEAWebKit/cryptarc4.c
SOURCE_FILES += src/DirtySDKEAWebKit/dirtylibps3.c
SOURCE_FILES += src/DirtySDKEAWebKit/dirtynetps3.c
SOURCE_FILES += src/DirtySDKEAWebKit/protossl.c
SOURCE_FILES += src/DirtySDKEAWebKit/platformsocketapi.c
SOURCE_FILES += src/DirtySDKEAWebKit/socketapicallbacks.c
SOURCE_FILES += src/DirtySDKEAWebKit/dirtymem.c
SOURCE_FILES += src/DirtySDKEAWebKit/cryptmd5.c
SOURCE_FILES += src/DirtySDKEAWebKit/cryptsha1.c
SOURCE_FILES += src/DirtySDKEAWebKit/cryptmd2.c
SOURCE_FILES += src/DirtySDKEAWebKit/cryptrsa.c
SOURCE_FILES += src/DirtySDKEAWebKit/lobbybase64.c
SOURCE_FILES += src/DirtySDKEAWebKit/dirtynet.c
SOURCE_FILES += src/DirtySDKEAWebKit/string_tools.c

all:
	$(TOOLCHAIN)$(CC) $(CFL) $(ARCH) $(SOURCE_FILES) $(LIBS) $(INCLUDES) -o $(OUT_EXECUTABLE_NAME)

clear:
	rm -f *.o
	rm -f $(OUT_EXECUTABLE_NAME)