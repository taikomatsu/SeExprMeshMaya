
# Maya2013 supports officially gcc 4.1.2, but its version is not supported OpenMP.
# If you want to use gcc 4.1.2, you have to remove "-fopenmp" from CFLAGS below.

#C++ = g++412
C++ = g++ #default version of fedorq14: 4.5.1

CFLAGS = -DBits64_ -m64 -DUNIX -D_BOOL -DLINUX -DFUNCPROTO -D_GNU_SOURCE -DLINUX_64 -fPIC \
         -fno-strict-aliasing -DREQUIRE_IOSTREAM -Wno-deprecated -O3 -Wall \
         -Wno-multichar -Wno-comment -Wno-sign-compare -funsigned-char \
         -Wno-reorder -fno-gnu-keywords -ftemplate-depth-25 -pthread -fopenmp

C++FLAGS = $(CFLAGS) -Wno-deprecated -fno-gnu-keywords

# Maya installed path and version
MAYA_LOCATION = /usr/autodesk/maya2013-x64
MAYA_VERSION = 2013

# SeExpr binary path where you installed
SEEXPR_LOCATION = /usr/local

INCLUDES = -I$(MAYA_LOCATION)/include -I$(SEEXPR_LOCATION)/include
LFLAGS   = -Wl,-Bsymbolic -shared
LIBS     = -L$(MAYA_LOCATION)/lib -ldl -lOpenMaya -lOpenMayaAnim -lFoundation
EXT      = so

ALL_CPP_FILES = SeExprMeshCmd.cpp SeExprMeshNode.cpp ClosestPointFunc.cpp pluginMain.cpp
OBJS = SeExprMeshCmd.o SeExprMeshNode.o ClosestPointFunc.o pluginMain.o $(SEEXPR_LOCATION)/lib64/libSeExpr.a

OUT_DIR = ../build/$(MAYA_VERSION)

### You have to change here where you want ###
INSTALL_DIR = /home/username/foo/bar

TARGET = SeExprMesh.$(EXT)

all: build $(TARGET)

$(TARGET): $(OBJS)
	$(C++) -o $(OUT_DIR)/$@ $(INCLUDES) $(C++FLAGS) $(OBJS) $(LFLAGS) $(LIBS)
	chmod -x $(OUT_DIR)/$@

preprocess: seExprMeshNode.cpp
	$(C++) -E -o $<_prepro.cpp $(INCLUDES) $(C++FLAGS) $<

depend:
	makedepend -- $(C++FLAGS) -- $(ALL_CPP_FILES)

build: $(ALL_CPP_FILES)
	mkdir -p $(OUT_DIR)
.PHONY: build

install: installAE
	mkdir -p $(INSTALL_DIR)/plug-ins
	cp -f $(OUT_DIR)/$(TARGET) $(INSTALL_DIR)/plug-ins
.PHONY: install

installAE:
	mkdir -p $(INSTALL_DIR)/scripts
	cp -f ../scripts/AEseExprMeshTemplate.mel $(INSTALL_DIR)/scripts
.PHONY: installAE

clean:
	rm -f *.o
.PHONY: clean

.cpp.o:
	$(C++) -o $@ -c $(INCLUDES) $(C++FLAGS) $<


