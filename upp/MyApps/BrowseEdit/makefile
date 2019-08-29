# To run this makefile:
# 1. Open command line terminal (cmd.exe)
# 2. Change directory to this folder
# 3. Execute: set PATH=<upp>\bin\mingw64\32\bin
# 4. To compile, execute: mingw32-make
# 5. To clean, execute: mingw32-make clean

#-------------------------------------------------

# library output file
LIB_OUT_FILE = ..\BrowseOrEdit\BrowseEdit.a

OBJECT_FILES = BrowseEdit.o \
               Editor.o \
               Tool.o \
               EncodeHTML.o \
               libcef.o \
               HTML2QTF.o \
               parsehtml.o \
               avl.o

# path to the U++ source folder
UPPSRC = ..\..\uppsrc

#-------------------------------------------------

# compilers
CC = gcc
CXX = g++

# archiver
AR = ar

# compiler flags
CC_FLAGS = -Wall -pedantic $(CFLAGS)
CXX_FLAGS = -Wall -I.. $(CXXFLAGS)

# archiver flags
AR_FLAGS = -crs #$(ARFLAGS)

#-------------------------------------------------

make: $(OBJECT_FILES)
	$(AR) $(AR_FLAGS) $(LIB_OUT_FILE) *.o

# remove all created files
clean:
	del *.o $(LIB_OUT_FILE)

#-------------------------------------------------

BrowseEdit.o: BrowseEdit.cpp *.h
	$(CXX) $(CXX_FLAGS) -I$(UPPSRC) -Wno-parentheses -c -o $@ $<

Editor.o: Editor.cpp *.h
	$(CXX) $(CXX_FLAGS) -I$(UPPSRC) -Wno-parentheses -c -o $@ $<

Tool.o: Tool.cpp *.h
	$(CXX) $(CXX_FLAGS) -I$(UPPSRC) -Wno-parentheses -c -o $@ $<

EncodeHTML.o: EncodeHTML.cpp *.h
	$(CXX) $(CXX_FLAGS) -I$(UPPSRC) -Wno-parentheses -c -o $@ $<


libcef.o: libcef.cpp *.h
	$(CXX) $(CXX_FLAGS) -pedantic -c -o $@ $<

HTML2QTF.o: HTML2QTF.cpp *.h
	$(CXX) $(CXX_FLAGS) -pedantic -c -o $@ $<


parsehtml.o: parsehtml.c *.h
	$(CC) $(CC_FLAGS) -c -o $@ $<

avl.o: avl.c *.h
	$(CC) $(CC_FLAGS) -c -o $@ $<
