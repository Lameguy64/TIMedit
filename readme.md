# TIMedit

TIMedit is more or less the PSn00bSDK equivalent or modern replacement of
TIMTOOL for converting, editing, arranging and organizing texture images
in the PlayStation TIM image format.

Whilst tools for converting images to the TIM image format have been around
for awhile such as img2tim, there has not been one for graphically managing
TIM images, which would be difficult in larger projects without such a tool.
TIMedit fills that niche and improves upon many of the shortcomings found in
TIMTOOL.


## Features

* Import and convert image files into PlayStation TIM format with color,
  transparency mask and palette conversion options provided by the FreeImage
  library.

* 1:1 pixel perfect WYSWYG framebuffer area preview with zoom, overlap
  detection, semi-transparency preview and snap to Grid/Image/CLUT options.
  No annoying image scaling to put up with.

* CLUT editor with support for multiple CLUT entries, mask and
  semi-transparency blending preview.

* Supports 4-bit, 8-bit 16-bit and 24-bit TIM image files, but not support
  for multi-image TIM files (documentation of those files are somewhat
  lacking).

* Image grouping for easier management of TIM images.


## Compiling

TIMedit is built without the use of a build system front-end such as cmake or
ninja. All you need is GNU make (provided by msys2 in Windows or
build-essential(s) in your Linux distro), a C++11 compiler, Python 3, and
the required libraries fltk, tinyxml2 and Freeimage.

The makefile uses fltk-config to locate FLTK and pkg-config when available
for the other libraries. For custom installations, override FLTK_CONFIG,
FREEIMAGE_CFLAGS, FREEIMAGE_LIBS, TINYXML2_CFLAGS and TINYXML2_LIBS on the
make command line. CPPFLAGS, CXXFLAGS, LDFLAGS and LDLIBS are also supported.
Python 3 generates the embedded icon; override PYTHON if necessary.

### Windows (with MinGW + GNU make)
1. Install MinGW, GNU make, Python 3, pkg-config and the required libraries
   in an MSYS2 environment. Use libraries matching the compiler architecture.
  * fltk
  * tinyxml2
  * freeimage
2. Change current directory to the TIMedit source directory.
3. Ensure fltk-config and windres are on PATH, or override FLTK_CONFIG and WINDRES.
4. Run "make".

### Linux
1. Install GNU make, a C++ compiler, Python 3 and pkg-config.
2. Install development packages (usually suffixed with -dev) of the
  following libraries:
  * fltk
  * tinyxml2
  * freeimage
3. Change current directory to the TIMedit source directory.
4. Ensure fltk-config is on PATH, or override FLTK_CONFIG.
5. Run "make".

### macOS
1. Install the Xcode command line tools, GNU make, Python 3 and pkg-config.
2. Install FLTK, tinyxml2 and FreeImage for the same architecture as the compiler.
3. Change current directory to the TIMedit source directory.
4. Ensure fltk-config is on PATH. For libraries outside the default search
   paths, supply the include and library flags described above.
5. Run "make CXX=clang++" (or "gmake CXX=clang++" if GNU make is named gmake).

Run "make CONF=debug" for a debug build. Object files are separated by
platform and configuration under build; the executable is copied to the
source directory as timedit (timedit.exe on Windows). The macOS build
produces an executable, not an application bundle.

Run "make install PREFIX=/usr/local" to install, or add DESTDIR to stage an
installation. For cross-compilation, set PLATFORM to Windows, Linux or
Darwin and select the target CXX, FLTK_CONFIG, PKG_CONFIG and WINDRES tools.


## Known Issues

Palette conversion may be intermittent due to the way how Freeimage's
quantization algorithms work (except Simple), usually throwing too few color
errors when converting a high color count image to 4-bit or 8-bit color
depth. This may change with an internal image quantizer in the future.


## Possible features to be added in the future

The following lists some features I (Lameguy64) may add in future versions
of TIMedit, but didn't get around to implementing them as I wanted to get
this out as this project has been long overdue.

* GNU make style automatic TIM re-import via comparing file dates of TIM
  image and source image.
* Light image editing features for small image touch-ups.
* Preview TIM images on a PlayStation (via serial or parallel port link)
  for previewing artwork on a CRT television.


## Changelog

**Version 0.10a (08/24/2020)**
* Initial release.