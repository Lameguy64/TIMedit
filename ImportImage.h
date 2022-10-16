#ifndef IMPORTIMAGE_H
#define IMPORTIMAGE_H

#include "TimImage.h"
#include <FreeImage.h>

#define TRANSP_BLACK	0
#define TRANSP_ALPHA	1
#define TRANSP_COLORKEY	2

typedef struct ImportParams
{
	int target_bpp;		// Target color depth
	int dithering;		// Do dithering
	int quant_mode;		// Palette quantization mode (0 - Simple, 1 - Xiaolin Wu, 2 - NeuQuant)
	int color_stp;		// Set STP on non black pixels
	int black_stp;		// Set STP on black (makes them opaque)
	
	int transp_mode;
	int alpha_thresh;	// Alpha transparency threshold (alpha < this = transparent)
	int stp_thresh;		// Stp transparency threshold (alpha < this = stp enabled)
	int colorkey;
	
	int color_adjust;
	float adj_red;
	float adj_green;
	float adj_blue;
	
	float blend_red;
	float blend_grn;
	float blend_blu;
	
	int inv_red;
	int inv_grn;
	int inv_blu;
} ImportParams;
	
class ImportImage {
private:
	
	FIBITMAP	*image;
	TIM_PIX_16	*palette;
	int			palette_cols;
	
	TIM_PIX_16 ConvertPixel(BYTE *pix, int has_alpha, ImportParams *params, int x, int y);
	
	int ConvertRGB(TimImage *image, ImportParams *params);
	int ConvertNative(TimImage *tim, ImportParams *params);
	
	int SimpleQuantize(FIBITMAP *img, TIM_PIX_16 *palbuff, ImportParams *params);
	int ColorIndex8(FIBITMAP *img, unsigned char *buff, ImportParams* params);
	int ColorIndex4(FIBITMAP *img, unsigned char *buff, ImportParams* params);
	
	void ProcessTransparency(TIM_PIX_16 *pix, int alpha, ImportParams *params);
	int ApplyAdjust(int val, int chan, ImportParams *params);
	
public:
	
	enum {
		CONVERT_OK = 0,		// Conversion ok
		CONVERT_NOSRC,		// No source image loaded
		CONVERT_MANYCOLS,	// Too many colors for target depth
		CONVERT_UNSUPPORT,	// Unsupported target depth
		CONVERT_MISSING,	// Missing colors
		CONVERT_QUANTERR
	} CONVERT;
	
	ImportImage();
	virtual ~ImportImage();
	
	int LoadSource(const std::filesystem::path &filename);

	int Convert(TimImage *image, ImportParams *params);
	
	int GetSourceDepth()
	{
		return FreeImage_GetBPP(image);
	}
	
	RGBQUAD GetRGBcolor(int x, int y);
	/*int ColorDepth();
	char *GetRow(int row);
	int SimpleQuantize();
	
	TIM_PIX_16 ConvertPixelDithered(BYTE *pix, int x, int y);
	
	void Convert16bit(TimImage *image);
	
	FIBITMAP *Image()
	{
		return image;
	}
	*/
	
};

#endif /* IMPORTIMAGE_H */

