TARGET		= timedit
RCFILE		= timedit.rc
INSTALL		= /usr/bin

CFILES		= $(notdir $(wildcard *.c))
CPPFILES	= $(notdir $(wildcard *.cpp))
CXXFILES	= $(notdir $(wildcard *.cxx))
AFILES		= $(notdir $(wildcard *.s))

IMAGES		= timedit.png
OFILES		= $(addprefix build/,$(CPPFILES:.cpp=.o) $(CXXFILES:.cxx=.o) $(IMAGES:.png=.o))

# Libraries common to all platforms
LIBS		= -ltinyxml2 -lfltk_images -lfltk

ifeq "$(CONF)" "debug"
CFLAGS		= -g
CXXFLAGS	= $(CFLAGS)
AFLAGS		=
else
ifeq "$(OS)" "Windows_NT"
CFLAGS		= -mwindows -O2
else
CFLAGS		= -O2
endif
CXXFLAGS	= $(CFLAGS)
AFLAGS		=
endif

WINRES		= $(addprefix build/,$(RCFILE:.rc=.res))

ifeq "$(OS)" "Windows_NT"
LIBS		+= -lFreeImage -lcomctl32 -lcomdlg32 -lgdi32 -lole32 -luuid
LIBDIRS		= -LC:\fltk-1.3.4-1\lib -LC:\tinyxml2 -LC:\freeimage
INCLUDE		= -IC:\fltk-1.3.4-1 -IC:\tinyxml2 -IC:\freeimage
CFLAGS		+= -DWIN32
else
# This library is capitalized differently on Windows and Linux
# TODO: A check for mingw not running on Windows is needed here for cross-compilation to Windows to work
LIBS		+= -lfreeimage
endif

CC		= gcc
CXX		= g++
AS		= as

ifeq "$(OS)" "Windows_NT"
all: $(OFILES) $(WINRES)
	$(CXX) $(CXXFLAGS) $(OFILES) $(LIBDIRS) $(LIBS) $(WINRES) -o $(TARGET)
else
all: $(OFILES)
	$(CXX) $(CXXFLAGS) $(OFILES) $(LIBDIRS) $(LIBS) -o $(TARGET)
endif

clean:
	rm -Rf build $(TARGET)

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

build/%.o: %.cxx
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

build/%.o: icons/%.png
	@mkdir -p $(dir $@)
	ld -r -b binary -o $@ $<

build/%.res: %.rc
	windres $< -O coff $@

install:
	cp -p $(TARGET) $(INSTALL)/$(TARGET)
