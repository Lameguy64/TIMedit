TARGET		= timedit
INSTALL		= /usr/local/bin

CFILES		= $(notdir $(wildcard *.c))
CPPFILES	= $(notdir $(wildcard *.cpp))
CXXFILES	= $(notdir $(wildcard *.cxx))
OFILES		= $(addprefix build/,$(CPPFILES:.cpp=.o) $(CXXFILES:.cxx=.o) $(IMAGES:.png=.o))

LIBS		= -lfreeimage -ltinyxml2 -lfltk_images -lfltk

ifeq "$(CONF)" "debug"
CFLAGS		= -g
CXXFLAGS	= $(CFLAGS)
AFLAGS		=
else
CFLAGS		= -O2
CXXFLAGS	= $(CFLAGS)
AFLAGS		=
endif

WINRES		= $(addprefix build/,$(RCFILE:.rc=.res))

ifeq "$(OS)" "Windows_NT"
	LIBS		+= -lcomctl32 -lcomdlg32 -lgdi32 -lole32 -luuid
	LIBDIRS		= -LC:\fltk-1.3.4-1\lib -LC:\tinyxml2 -LC:\freeimage
	INCLUDE		= -IC:\fltk-1.3.4-1 -IC:\tinyxml2 -IC:\freeimage
	CFLAGS		+= -mwindows -DWIN32
endif

CC		= gcc
CXX		= g++
AS		= as

all: $(OFILES)
	$(CXX) $(CXXFLAGS) $(OFILES) $(LIBDIRS) $(LIBS) -o $(TARGET)

clean:
	rm -rf build $(TARGET)

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@
	
build/%.o: %.cxx
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

build/%.o:
	@mkdir -p $(dir $@)
	ld -r -b binary -o $@ $<
	
install:
	cp -p $(TARGET) $(INSTALL)/$(TARGET)
