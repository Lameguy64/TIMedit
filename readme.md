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
build-essential(s) in your Linux distro) and the required libraries fltk,
tinyxml2 and Freeimage.

Under Windows, the makefile assumes the dependencies reside in the root of
your C drive. You may want to modify them if you're using newer library
versions or you prefer to locate them elsewhere.

### Windows (with MinGW + GNU make)
1. Download and unpack required libraries to the root of your C drive.
  * fltk
  * tinyxml2
  * freeimage
  FLTK and tinyxml2 libraries need to be built first before compiling.
2. Change current directory to the TIMedit source directory.
3. Modify variables in makefile when necessary.
4. Run "make".

### Linux (may work in a MSys2 environment)
1. Install the build-essential(s) package if not yet installed already.
2. Install development packages (usually suffixed with -dev) of the
  following libraries:
  * fltk
  * tinyxml2
  * freeimage
3. Change current directory to the TIMedit source directory.
4. Modify variables in makefile when necessary.
5. Run "make".


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