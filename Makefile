TARGET		= timedit$(EXEEXT)
RCFILE		= timedit.rc
PREFIX		?= /usr/local
bindir		?= $(PREFIX)/bin
DESTDIR		?=
CONF		?= release
FLTK_CONFIG	?= fltk-config
PKG_CONFIG	?= pkg-config
WINDRES		?= windres
PYTHON		?= python3

ifndef PLATFORM
ifeq "$(OS)" "Windows_NT"
PLATFORM	= Windows
else
PLATFORM	:= $(shell uname -s)
endif
endif

BUILDDIR	= build/$(PLATFORM)/$(CONF)

CFILES		= $(notdir $(wildcard *.c))
CPPFILES	= $(notdir $(wildcard *.cpp))
CXXFILES	= $(notdir $(wildcard *.cxx))

IMAGES		= timedit.png
OFILES		= $(addprefix $(BUILDDIR)/,$(CFILES:.c=.o) $(CPPFILES:.cpp=.o) $(CXXFILES:.cxx=.o) $(IMAGES:.png=.o))

FLTK_CXXFLAGS	?= $(shell $(FLTK_CONFIG) --cxxflags)
FLTK_LIBS	?= $(shell $(FLTK_CONFIG) --use-images --ldflags)
FREEIMAGE_CFLAGS	?= $(shell $(PKG_CONFIG) --cflags freeimage 2>/dev/null)
FREEIMAGE_LIBS	?= $(shell $(PKG_CONFIG) --libs freeimage 2>/dev/null || echo -lfreeimage)
TINYXML2_CFLAGS	?= $(shell $(PKG_CONFIG) --cflags tinyxml2 2>/dev/null)
TINYXML2_LIBS	?= $(shell $(PKG_CONFIG) --libs tinyxml2 2>/dev/null || echo -ltinyxml2)
FLTK_CXXFLAGS	:= $(FLTK_CXXFLAGS)
INCLUDE		:= $(FREEIMAGE_CFLAGS) $(TINYXML2_CFLAGS)
LIBS		:= $(FREEIMAGE_LIBS) $(TINYXML2_LIBS) $(FLTK_LIBS)

ifeq "$(CONF)" "debug"
BUILD_CFLAGS	= -g -O0
BUILD_CPPFLAGS	= -DDEBUG
else
BUILD_CFLAGS	= -O2
endif

ifeq "$(PLATFORM)" "Windows"
EXEEXT		= .exe
WINRES		= $(addprefix $(BUILDDIR)/,$(RCFILE:.rc=.res))
BUILD_CPPFLAGS	+= -DWIN32
ifneq "$(CONF)" "debug"
BUILD_LDFLAGS	= -mwindows
endif
endif

ifeq "$(origin CXX)" "default"
CXX		= c++
endif

.PHONY: all clean install
.SECONDARY: $(addprefix $(BUILDDIR)/,$(IMAGES:.png=.cpp))

all: $(BUILDDIR)/$(TARGET)
	@cmp -s "$<" "$(TARGET)" || cp -p "$<" "$(TARGET)"

$(BUILDDIR)/$(TARGET): $(OFILES) $(WINRES)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(BUILD_LDFLAGS) $(OFILES) $(WINRES) $(LIBS) $(LDLIBS) -o "$@"

clean:
	rm -Rf build "$(TARGET)" timedit timedit.exe

$(BUILDDIR)/%.o: %.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(BUILD_CPPFLAGS) $(CPPFLAGS) $(BUILD_CFLAGS) $(INCLUDE) $(CFLAGS) -MMD -MP -c "$<" -o "$@"

$(BUILDDIR)/%.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(BUILD_CPPFLAGS) $(CPPFLAGS) $(BUILD_CFLAGS) $(FLTK_CXXFLAGS) $(INCLUDE) -std=c++11 $(CXXFLAGS) -MMD -MP -c "$<" -o "$@"
	
$(BUILDDIR)/%.o: %.cxx Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(BUILD_CPPFLAGS) $(CPPFLAGS) $(BUILD_CFLAGS) $(FLTK_CXXFLAGS) $(INCLUDE) -std=c++11 $(CXXFLAGS) -MMD -MP -c "$<" -o "$@"

$(BUILDDIR)/%.o: $(BUILDDIR)/%.cpp Makefile
	$(CXX) $(BUILD_CPPFLAGS) $(CPPFLAGS) $(BUILD_CFLAGS) -std=c++11 $(CXXFLAGS) -MMD -MP -c "$<" -o "$@"

$(BUILDDIR)/%.cpp: icons/%.png embed_icon.py
	@mkdir -p $(dir $@)
	$(PYTHON) embed_icon.py "$<" "$@"
	
$(BUILDDIR)/%.res: %.rc icons/timedit.ico Makefile
	@mkdir -p $(dir $@)
	$(WINDRES) "$<" -O coff -o "$@"
	
install: all
	install -d "$(DESTDIR)$(bindir)"
	install -m 755 "$(TARGET)" "$(DESTDIR)$(bindir)/$(TARGET)"

-include $(OFILES:.o=.d)
