TARGET		= timedit
RCFILE		= timedit.rc
INSTALL		= /usr/bin

CFILES		= $(notdir $(wildcard *.c))
CPPFILES	= $(notdir $(wildcard *.cpp))
CXXFILES	= $(notdir $(wildcard *.cxx))
AFILES		= $(notdir $(wildcard *.s))

IMAGES		= timedit.png
OFILES		= $(addprefix build/,$(CPPFILES:.cpp=.o) $(CXXFILES:.cxx=.o) $(IMAGES:.png=.o))

LIBS		= -lfreeimage -ltinyxml2 -lfltk_images -lfltk

CFLAGS := -std=c++17

ifeq "$(CONF)" "debug"
CFLAGS		+= -g -DDEBUG
CXXFLAGS	= $(CFLAGS)
AFLAGS		=
else
CFLAGS		+= -O2
CXXFLAGS	= $(CFLAGS)
AFLAGS		=
endif

ifeq "$(OS)" "Windows_NT"
LIBS		+= -lcomctl32 -lcomdlg32 -lgdi32 -lole32 -luuid
LIBDIRS		= -LC:\fltk-1.3.4-1\lib -LC:\tinyxml2 -LC:\freeimage
INCLUDE		= -IC:\fltk-1.3.4-1 -IC:\tinyxml2 -IC:\freeimage
CFLAGS		+= -DWIN32 -mwindows
WINRES		= $(addprefix build/,$(RCFILE:.rc=.res))
endif

CC		= gcc
CXX		= g++

all: $(OFILES) $(WINRES)
	$(CXX) $(CXXFLAGS) $(OFILES) $(LIBDIRS) $(LIBS) $(WINRES) -o $(TARGET)

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
