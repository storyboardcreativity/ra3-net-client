CC := gcc
CFL := -ggdb -fPIC
TOOLCHAIN :=
OUT_EXECUTABLE_NAME := ra3-net-client
ARCH :=
LIBS := -lpthread -lstdc++ -lcurl -lcurlpp
SOURCE_FILES := main.cpp terminal_printing_system.cpp printing_tools.cpp gamespy.cpp

all:
	$(TOOLCHAIN)$(CC) $(CFL) $(ARCH) $(SOURCE_FILES) $(LIBS) -o $(OUT_EXECUTABLE_NAME)

clear:
	rm -f *.o
	rm -f $(OUT_EXECUTABLE_NAME)