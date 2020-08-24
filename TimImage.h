#ifndef TIMIMAGE_H
#define TIMIMAGE_H

#include <FreeImage.h>

typedef struct {
	unsigned short	r:5;
	unsigned short	g:5;
	unsigned short	b:5;
	unsigned short	i:1;
} TIM_PIX_16;

#pragma pack(push, 1)
typedef struct {
	unsigned char	r;
	unsigned char	g;
	unsigned char	b;
} TIM_PIX_24;
#pragma pack(pop)

class TimImage {
private:
	
	// TIM image parameters
	void *im_pixels;
	int im_pmode;
	int im_iw;
	int im_x,im_y;
	int im_w,im_h;
	
	// CLUT image paramters
	TIM_PIX_16 *cl_pixels;
	int cl_x,cl_y;
	int cl_w,cl_h;
	
public:
	
	enum TIM_ERR {
		ERR_OK,
		ERR_NOT_FOUND,
		ERR_CANT_READ,
		ERR_INVALID,
		ERR_NO_IMAGE,
		ERR_CANT_WRITE
	};
	
	enum PMODE {
		PMODE_4,
		PMODE_8,
		PMODE_16,
		PMODE_24
	};
	
	typedef struct IMPORT_PARAMS
	{
		int color_depth;
		bool set_stp;
		bool set_stp_black;
		bool use_alpha;
		int alpha_threshold;
		int stp_threshold;
		short image_x,image_y;
		short clut_x,clut_y;
	} IMPORT_PARAMS;
	
	enum IMPORT_ERR {
		IMPORT_ERR_OK,
		IMPORT_ERR_CANT_RGB32,
	};
	
	TimImage();
	virtual ~TimImage();
	
	TIM_ERR LoadTim(const char *filename);
	TIM_ERR SaveTim(const char *filename);
	
	TIM_PIX_24 ImagePixel24(int x, int y);
	
	TIM_PIX_16 ImagePixel(int x, int y, int clut_row = 0);
	TIM_PIX_16 ClutPixel(int x, int y);
	int PixelIndex(int x, int y);
	
	void SetClutPixel(int x, int y, short r, short g, short b, short i);
	
	void Copy(TimImage *src, int copy_pos = 1);
	
	int HasClut() {
		
		if( cl_pixels )
			return 1;
		
		return 0;
	}
	
	int GetPmode() {
		return im_pmode;
	}
	
	void GetClutPosition(int &x, int &y) {
		x = cl_x;
		y = cl_y;
	}
	
	void SetClutPosition(int x, int y) {
		cl_x = x;
		cl_y = y;
	}
	
	void GetClutDimensions(int &w, int &h) {
		w = cl_w;
		h = cl_h;
	}
	
	void GetDimensionsVRAM(int &w, int &h) {
		w = im_w;
		h = im_h;
	}
	
	void GetDimensions(int &w, int &h) {
		w = im_iw;
		h = im_h;
	}
	
	void GetPosition(int &x, int &y) {
		x = im_x;
		y = im_y;
	}
	
	void SetPosition(int x, int y) {
		im_x = x;
		im_y = y;
	}
	
	int GetNumCluts() {
		return cl_h;
	}
	
	void SetImageData(void *pixels, int w, int h, int pmode);
	void SetClutData(void *colors, int w, int h);
	
	int AddClutSlot(int nslots);
	void DeleteClut(int slot);
	
	int modified;
	int blendmode;
	
};

#endif /* TIMIMAGE_H */

