#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include "ImportImage.h"
#include "TimImage.h"

ImportImage::ImportImage()
{
	image = NULL;
	
	palette = NULL;
	palette_cols = 0;
	
}

ImportImage::~ImportImage()
{
	if( image )
	{
		FreeImage_Unload(image);
	}
	if( palette )
	{
		free(palette);
	}
}

int ImportImage::LoadSource(const std::filesystem::path &filename)
{
#ifdef DEBUG
	printf("[DEBUG] ImportImage::LoadSource: %s\n", filename.c_str());
#endif
	// Check if file exists
	if( access(filename.c_str(), F_OK) == -1 )
	{
		fprintf(stderr, "[ERROR] File not found: %s\n", filename.c_str());
		return 1;
	}

	// Determine format of input file
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filename.c_str(), 0);

	if( fif == FIF_UNKNOWN )
	{
		fif = FreeImage_GetFIFFromFilename(filename.c_str());

		if( !FreeImage_FIFSupportsReading(fif) )
		{
			fprintf(stderr, "[ERROR] Unknown/unsupported image format: %s\n", filename.c_str());
			return 1;
		}
	}

	// Load the input image
	image = FreeImage_Load(fif, filename.c_str(), 0);

	if( image == NULL )
	{
		fprintf(stderr, "[ERROR] Cannot load specified image file: %s\n", filename.c_str());
		return 1;
	}

	// Some checks to make sure that the image is really valid
	if( !FreeImage_HasPixels(image) ) {
		fprintf(stderr, "[ERROR] Source image has no pixel data... Somehow: %s\n", filename.c_str());
		FreeImage_Unload(image);
		image = NULL;
		return 1;
	}
	
	return 0;
}

int ImportImage::Convert(TimImage *tim, ImportParams *params)
{
	if( image == NULL )
	{
		return CONVERT_NOSRC;
	}
	
	if( ( FreeImage_GetBPP(image) >= 16 ) || ( params->transp_mode == TRANSP_ALPHA ) )
	{
		return ConvertRGB(tim, params);
	}
	else
	{
		// If target depth is not equal to source depth
		if( params->target_bpp != FreeImage_GetBPP(image) )
		{
			return ConvertRGB(tim, params);
		}
		
		return ConvertNative(tim, params);
		
	}
	
	return CONVERT_OK;
}

int ImportImage::ConvertRGB(TimImage *tim, ImportParams *params)
{
	int w,h,ret;
	FIBITMAP *img_temp = NULL;
	int has_alpha = false;
	int temp_present = false;
	TIM_PIX_16 palette_temp[256];
	
	if( FreeImage_GetBPP(image) < 24 )
	{
		img_temp = FreeImage_ConvertTo32Bits(image);
		temp_present = true;
		has_alpha = true;
	}
	else
	{
		img_temp = image;
		if( FreeImage_GetBPP(image) > 24 )
		{
			has_alpha = true;
		}
	}
	
	w = FreeImage_GetWidth(img_temp);
	h = FreeImage_GetHeight(img_temp);
	
	switch( params->target_bpp )
	{
		case 4:
		{
			unsigned char *buff_temp;
			TIM_PIX_16* palette_new;
			
			if( palette )
			{
				free(palette);
				palette = NULL;
				palette_cols = 0;
			}
			
			if( params->quant_mode == 0 )
			{
				int col_count = SimpleQuantize(img_temp, palette_temp, params);
				if( col_count == 0 )
				{
					return CONVERT_MANYCOLS;
				}
				
				if( col_count > 16 )
				{
					return CONVERT_MANYCOLS;
				}

				// Copy palette result
				palette = (TIM_PIX_16*)malloc(sizeof(TIM_PIX_16)*16);
				palette_cols = col_count;

				memset(palette, 0, sizeof(TIM_PIX_16)*16);
				memcpy(palette, palette_temp, sizeof(TIM_PIX_16)*col_count);

				// Convert image to 4-bit color index
				buff_temp = (unsigned char*)malloc((w*h)/2);

				if( ColorIndex4(img_temp, buff_temp, params) )
				{
					if( palette )
					{
						free(palette);
						palette = NULL;
						palette_cols = 0;
					}
					free(buff_temp);
					return CONVERT_MISSING;
				}

				if( params->transp_mode == TRANSP_COLORKEY )
				{
					int idx = params->colorkey>>24;
					if( idx < palette_cols )
					{
						palette[idx].r = 0;
						palette[idx].g = 0;
						palette[idx].b = 0;
						palette[idx].i = 0;
					}
				}
				
				palette_new = (TIM_PIX_16*)malloc(sizeof(TIM_PIX_16)*16);
				memcpy(palette_new, palette, sizeof(TIM_PIX_16)*16);

				tim->SetImageData(buff_temp, w, h, 0);
				tim->SetClutData(palette_new, 16, 1);
			}
			else
			{
				BYTE *src_pix;
				unsigned char *buff_temp,*b;
				TIM_PIX_16 *palette_new;
				FIBITMAP *quant_temp;
				RGBQUAD *fi_pal;
				int t;
				
				if( params->quant_mode == 1 )
				{
					quant_temp = FreeImage_ColorQuantizeEx(
						img_temp, FIQ_WUQUANT, 16, 0, NULL);
				}
				else
				{
					quant_temp = FreeImage_ColorQuantizeEx(
						img_temp, FIQ_NNQUANT, 16, 0, NULL);
				}
				
				if( quant_temp == NULL )
				{
					return CONVERT_QUANTERR;
				}
				
				fi_pal = FreeImage_GetPalette(quant_temp);
				
				t = params->dithering;
				params->dithering = 0;
				
				for(int i=0; i<16; i++)
				{
					palette_temp[i] = ConvertPixel((BYTE*)&fi_pal[i], 
						has_alpha, params, 0, 0);
				}
				
				params->dithering = t;
				
				if( params->transp_mode == TRANSP_COLORKEY )
				{
					int idx = params->colorkey>>24;
					if( idx < 16 )
					{
						palette_temp[idx].r = 0;
						palette_temp[idx].g = 0;
						palette_temp[idx].b = 0;
						palette_temp[idx].i = 0;
					}
				}
				
				palette_new = (TIM_PIX_16*)malloc(sizeof(TIM_PIX_16)*16);
				memcpy(palette_new, palette_temp, sizeof(TIM_PIX_16)*16);
				
				buff_temp = (unsigned char*)malloc((w*h)/2);
				b = buff_temp;
				for(int i=0; i<h; i++)
				{
					src_pix = FreeImage_GetScanLine(quant_temp, (h-1)-i);
					for(int j=0; j<w; j++)
					{
						if( (j&1) == 0 )
						{
							*b = *src_pix;
						}
						else
						{
							*b = *b|((*src_pix&0xF)<<4);
							b++;
						}
						src_pix++;
					}
				}
				
				tim->SetImageData(buff_temp, w, h, 0);
				tim->SetClutData(palette_new, 16, 1);
				
				FreeImage_Unload(quant_temp);
			}
			break;
		}
		case 8:
		{
			unsigned char *buff_temp;
			TIM_PIX_16* palette_new;
			
			if( palette )
			{
				free(palette);
				palette = NULL;
				palette_cols = 0;
			}
			
			if( params->quant_mode == 0 )
			{
				int col_count = SimpleQuantize(img_temp, palette_temp, params);
				if( col_count == 0 )
				{
					return CONVERT_MANYCOLS;
				}

				// Copy palette result
				palette = (TIM_PIX_16*)malloc(sizeof(TIM_PIX_16)*256);
				palette_cols = col_count;

				memset(palette, 0, sizeof(TIM_PIX_16)*256);
				memcpy(palette, palette_temp, sizeof(TIM_PIX_16)*col_count);

				// Convert image to 8-bit color index
				buff_temp = (unsigned char*)malloc(w*h);

				if( ColorIndex8(img_temp, buff_temp, params) )
				{
					if( palette )
					{
						free(palette);
						palette = NULL;
						palette_cols = 0;
					}
					free(buff_temp);
					return CONVERT_MISSING;
				}
				
				if( params->transp_mode == TRANSP_COLORKEY )
				{
					int idx = params->colorkey>>24;
					if( idx < palette_cols )
					{
						palette[idx].r = 0;
						palette[idx].g = 0;
						palette[idx].b = 0;
						palette[idx].i = 0;
					}
				}

				palette_new = (TIM_PIX_16*)malloc(sizeof(TIM_PIX_16)*256);
				memcpy(palette_new, palette, sizeof(TIM_PIX_16)*256);

				tim->SetImageData(buff_temp, w, h, 1);
				tim->SetClutData(palette_new, 256, 1);
			}
			else
			{
				BYTE *src_pix;
				unsigned char *buff_temp;
				TIM_PIX_16 *palette_new;
				FIBITMAP *quant_temp;
				RGBQUAD *fi_pal;
				int t;
				
				if( params->quant_mode == 1 )
				{
					quant_temp = FreeImage_ColorQuantize(img_temp, FIQ_WUQUANT);
				}
				else
				{
					quant_temp = FreeImage_ColorQuantize(img_temp, FIQ_NNQUANT);
				}
				
				if( quant_temp == NULL )
				{
					return CONVERT_QUANTERR;
				}
				
				fi_pal = FreeImage_GetPalette(quant_temp);
				
				t = params->dithering;
				params->dithering = 0;
				
				for(int i=0; i<256; i++)
				{
					palette_temp[i] = ConvertPixel((BYTE*)&fi_pal[i], has_alpha,
						params, 0, 0);
				}
				
				params->dithering = t;
				
				if( params->transp_mode == TRANSP_COLORKEY )
				{
					int idx = params->colorkey>>24;
					if( idx < 256 )
					{
						palette_temp[idx].r = 0;
						palette_temp[idx].g = 0;
						palette_temp[idx].b = 0;
						palette_temp[idx].i = 0;
					}
				}
				
				palette_new = (TIM_PIX_16*)malloc(sizeof(TIM_PIX_16)*256);
				memcpy(palette_new, palette_temp, sizeof(TIM_PIX_16)*256);
				
				buff_temp = (unsigned char*)malloc(w*h);
				for(int i=0; i<h; i++)
				{
					src_pix = FreeImage_GetScanLine(quant_temp, (h-1)-i);
					memcpy(buff_temp+(w*i), src_pix, w);
				}
				
				tim->SetImageData(buff_temp, w, h, 1);
				tim->SetClutData(palette_new, 256, 1);
				
				FreeImage_Unload(quant_temp);
			}
			
			break;
		}
		case 16:
		{
			BYTE		*src_pix;
			TIM_PIX_16	*dst_buff;
			TIM_PIX_16	*dst_pix;
			
			dst_buff = (TIM_PIX_16*)malloc(2*(w*h));
			dst_pix = dst_buff;
			
			for(int i=0; i<h; i++)
			{
				src_pix = FreeImage_GetScanLine(img_temp, (h-1)-i);
				for(int x=0; x<w; x++)
				{
					
					*dst_pix = ConvertPixel(src_pix, has_alpha, params, x, i);
					
					if( params->transp_mode == TRANSP_COLORKEY )
					{
						if( ( src_pix[0] == (params->colorkey&0xff) )
							&& ( src_pix[1] == ((params->colorkey>>8)&0xff) )
							&& ( src_pix[2] == ((params->colorkey>>16)&0xff) ) )
						{
							dst_pix->r = 0;
							dst_pix->g = 0;
							dst_pix->b = 0;
							dst_pix->i = 0;
						}		
					}
					
					dst_pix++;
					
					if( has_alpha )
						src_pix += 4;
					else
						src_pix += 3;

				}
			}
			
			tim->SetImageData(dst_buff, w, h, TimImage::PMODE_16);
			ret = CONVERT_OK;
			break;
		}
		case 24:
		{
			BYTE		*src_pix;
			TIM_PIX_24	*dst_buff;
			TIM_PIX_24	*dst_pix;
			
			dst_buff = (TIM_PIX_24*)malloc(3*((w+(w>>1))*h));
			dst_pix = dst_buff;
			
			for(int i=0; i<h; i++)
			{
				src_pix = FreeImage_GetScanLine(img_temp, (h-1)-i);
				for(int x=0; x<w; x++)
				{
					dst_pix->r = ApplyAdjust(src_pix[2], 0, params);
					dst_pix->g = ApplyAdjust(src_pix[1], 1, params);
					dst_pix->b = ApplyAdjust(src_pix[0], 2, params);
					dst_pix++;
					
					if( has_alpha )
						src_pix += 4;
					else
						src_pix += 3;
					
				}
			}
	
			tim->SetImageData(dst_buff, w, h, TimImage::PMODE_24);
			ret = CONVERT_OK;
			break;
		}
		default:
			ret = CONVERT_UNSUPPORT;
	}
	
	if( temp_present )
	{
		FreeImage_Unload(img_temp);
	}
	
	return ret;
}

TIM_PIX_16 ImportImage::ConvertPixel(BYTE *pix, int has_alpha, ImportParams *params, int x, int y)
{
	char dither_mtx[4][4] = {
		-4,  0, -3,  1,
		 2, -2,  3, -1,
		-3,  1, -4,  0,
		 3, -1,  2, -2
	};
	
	int r,g,b;
	TIM_PIX_16 pixel;
	
	r = pix[2];
	g = pix[1];
	b = pix[0];

	if( has_alpha )
	{
		r = (r*pix[3])>>8;
		g = (g*pix[3])>>8;
		b = (b*pix[3])>>8;
	}
	
	if( params->color_adjust )
	{
		r *= params->adj_red;
		g *= params->adj_green;
		b *= params->adj_blue;
	}
		
	if( params->dithering )
	{	
		r += dither_mtx[y&0x3][x&0x3];
		g += dither_mtx[y&0x3][x&0x3];
		b += dither_mtx[y&0x3][x&0x3];
	}
	
	if( r<0 )
		r= 0;
	if( r>255 )
		r = 255;

	if( g<0 )
		g= 0;
	if( g>255 )
		g = 255;

	if( b<0 )
		b= 0;
	if( b>255 )
		b = 255;
	
	pixel.r = r>>3;
	pixel.g = g>>3;
	pixel.b = b>>3;
	
	pixel.i = 0;
	
	if( has_alpha )
		ProcessTransparency(&pixel, pix[3], params);
	else
		ProcessTransparency(&pixel, -1, params);
	
	return pixel;
}

int ImportImage::ColorIndex8(FIBITMAP *img, unsigned char *buff, ImportParams* params)
{
	int			found;
	int			p_w,p_h,p_bpp;
	BYTE		*src_pix;
	TIM_PIX_16	ccol;
	
	p_w = FreeImage_GetWidth(img);
	p_h = FreeImage_GetHeight(img);
	p_bpp = FreeImage_GetBPP(img);
	
	for(int i=0; i<p_h; i++)
	{
		src_pix = FreeImage_GetScanLine(img, (p_h-1)-i);
		
		for(int x=0; x<p_w; x++)
		{
			if( p_bpp == 32 )
				ccol = ConvertPixel(src_pix, 1, params, x, i);
			else
				ccol = ConvertPixel(src_pix, 0, params, x, i);
			
			found = 0;
			for(int j=0; j<palette_cols; j++)
			{
				if( *((short*)&ccol) == *((short*)&palette[j]) )
				{
					*buff = j;
					found = 1;
					break;
				}
			}
			
			if( !found )
			{
				return 1;
			}
			
			if( p_bpp == 32 )
				src_pix += 4;
			else
				src_pix += 3;
			
			buff++;
		}
	}
	
	return 0;
}

int ImportImage::ColorIndex4(FIBITMAP *img, unsigned char *buff, ImportParams* params)
{
	int			found;
	int			p_w,p_h,p_bpp;
	BYTE		*src_pix;
	TIM_PIX_16	ccol;
	
	p_w = FreeImage_GetWidth(img);
	p_h = FreeImage_GetHeight(img);
	p_bpp = FreeImage_GetBPP(img);
	
	for(int i=0; i<p_h; i++)
	{
		src_pix = FreeImage_GetScanLine(img, (p_h-1)-i);
		
		for(int x=0; x<p_w; x++)
		{
			if( p_bpp == 32 )
				ccol = ConvertPixel(src_pix, 1, params, x, i);
			else
				ccol = ConvertPixel(src_pix, 0, params, x, i);
			
			found = 0;
			for(int j=0; j<palette_cols; j++)
			{
				if( *((short*)&ccol) == *((short*)&palette[j]) )
				{
					if( (x&1) == 0 )
					{
						*buff = j&0xF;
					}
					else
					{
						*buff = *buff|(j&0xF)<<4;
						buff++;
					}
					found = 1;
					break;
				}
			}
			
			if( !found )
			{
				return 1;
			}
			
			if( p_bpp == 32 )
				src_pix += 4;
			else
				src_pix += 3;
			
		}
	}
	
	return 0;
}

int ImportImage::SimpleQuantize(FIBITMAP *img, TIM_PIX_16* palbuff, ImportParams* params)
{
	int			ncols = 0,found;
	int			p_w,p_h,p_bpp;
	BYTE		*src_pix;
	TIM_PIX_16	ccol;
	
	p_w = FreeImage_GetWidth(img);
	p_h = FreeImage_GetHeight(img);
	p_bpp = FreeImage_GetBPP(img);
	
	for(int i=0; i<p_h; i++)
	{
		src_pix = FreeImage_GetScanLine(img, (p_h-1)-i);
		
		for(int x=0; x<p_w; x++)
		{
			if( p_bpp == 32 )
				ccol = ConvertPixel(src_pix, 1, params, x, i);
			else
				ccol = ConvertPixel(src_pix, 0, params, x, i);
			
			found = false;
			for(int j=0; j<ncols; j++)
			{
				if( *((short*)&ccol) == *((short*)&palbuff[j]) )
				{
					found = true;
					break;
				}
			}
			
			if( !found )
			{
				if( ncols > 255 )
				{
					return 0;
				}
				palbuff[ncols] = ccol;
				ncols++;
			}
			
			if( p_bpp == 32 )
				src_pix += 4;
			else
				src_pix += 3;

		}
	}
	return ncols;
}

int ImportImage::ApplyAdjust(int val, int chan, ImportParams *params)
{
	if( !params->color_adjust )
		return val;
	
	switch(chan)
	{
		case 0:
			val *= params->adj_red;
			break;
		case 1:
			val *= params->adj_green;
			break;
		case 2:
			val *= params->adj_blue;
			break;
	}
	
	if( val < 0 )
		val = 0;
	if( val > 255 )
		val = 255;
	
	return val;
}

int ImportImage::ConvertNative(TimImage *tim, ImportParams *params)
{
	BYTE *dst_buff,*dst_pix;
	TIM_PIX_16 *pal_temp;
	RGBQUAD *pal_src;
	int w,h;
	
	w = FreeImage_GetWidth(image);
	h = FreeImage_GetHeight(image);
	FreeImage_SetTransparent(image, 1);
	
	if( FreeImage_GetBPP(image) == 8 )
	{
		// Copy palette data
		pal_src = FreeImage_GetPalette(image);
				
		pal_temp = (TIM_PIX_16*)malloc(sizeof(TIM_PIX_16)*256);
		
		for( int i=0; i<256; i++ )
		{
			pal_temp[i].r = ApplyAdjust(pal_src[i].rgbRed, 0, params)>>3;
			pal_temp[i].g = ApplyAdjust(pal_src[i].rgbGreen, 1, params)>>3;
			pal_temp[i].b = ApplyAdjust(pal_src[i].rgbBlue, 2, params)>>3;
			pal_temp[i].i = 0;
			
			ProcessTransparency(&pal_temp[i], pal_src[i].rgbReserved, params);
		}
		
		if( params->transp_mode == TRANSP_COLORKEY )
		{
			int idx = params->colorkey>>24;
			if( idx < 256 )
			{
				pal_temp[idx].r = 0;
				pal_temp[idx].g = 0;
				pal_temp[idx].b = 0;
				pal_temp[idx].i = 0;
			}
		}
		
		tim->SetClutData(pal_temp, 256, 1);
		
		// Copy pixel data
		dst_buff = (BYTE*)malloc(w*h);
		
		for( int y=0; y<h; y++ )
		{
			BYTE *line_pix = FreeImage_GetScanLine(image, (h-1)-y);
			memcpy(dst_buff+(w*y), line_pix, w);
		}
		
		tim->SetImageData(dst_buff, w, h, TimImage::PMODE_8);
		
	}
	else if( FreeImage_GetBPP(image) == 4 )
	{
		// Copy palette data
		pal_src = FreeImage_GetPalette(image);
				
		pal_temp = (TIM_PIX_16*)malloc(sizeof(TIM_PIX_16)*16);
		
		for( int i=0; i<16; i++ )
		{
			pal_temp[i].r = ApplyAdjust(pal_src[i].rgbRed, 0, params)>>3;
			pal_temp[i].g = ApplyAdjust(pal_src[i].rgbGreen, 1, params)>>3;
			pal_temp[i].b = ApplyAdjust(pal_src[i].rgbBlue, 2, params)>>3;
			pal_temp[i].i = 0;
			
			ProcessTransparency(&pal_temp[i], pal_src[i].rgbReserved, params);
		}
		
		if( params->transp_mode == TRANSP_COLORKEY )
		{
			int idx = params->colorkey>>24;
			if( idx < 16 )
			{
				pal_temp[idx].r = 0;
				pal_temp[idx].g = 0;
				pal_temp[idx].b = 0;
				pal_temp[idx].i = 0;
			}
		}
		
		tim->SetClutData(pal_temp, 16, 1);
		
		dst_buff = (BYTE*)malloc((w*h)>>1);
		
		dst_pix = dst_buff;
		for( int y=0; y<h; y++ )
		{
			BYTE *line_pix = FreeImage_GetScanLine(image, (h-1)-y);
			
			// Annoying nibble swapping because M$ DIB format
			for( int x=0; x<(w>>1); x++ )
			{
				int i = line_pix[x];
				*dst_pix = ((i&0xf)<<4)|((i>>4)&0xf);
				dst_pix++;
			}
		}
		
		tim->SetImageData(dst_buff, w, h, TimImage::PMODE_4);
		
	}
	
	
	return 0;
}

void ImportImage::ProcessTransparency(TIM_PIX_16 *pix, int alpha, ImportParams *params)
{
	if( params->color_stp )
	{
		if( (pix->r+pix->g+pix->b) > 0 )
		{
			pix->i = 1;
		}
	}
	
	if( params->black_stp )
	{
		if( (pix->r+pix->g+pix->b) == 0 )
		{
			pix->i = 1;
		}
	}
	
	if( ( alpha >= 0 ) && ( params->transp_mode == TRANSP_ALPHA ) )
	{
		pix->i = 0;
		
		if( alpha <= params->stp_thresh )
		{
			pix->i = 1;
		}
		
		if( alpha < params->alpha_thresh )
		{
			pix->r = 0;
			pix->g = 0;
			pix->b = 0;
			pix->i = 0;
		}
		
	}
	
	if( ( params->color_adjust ) && ( pix->i ) )
	{
		int r,g,b;
		
		r = pix->r;
		g = pix->g;
		b = pix->b;
		
		r = (r*params->blend_red);
		g = (g*params->blend_grn);
		b = (b*params->blend_blu);
		
		if( r > 31 )
			r = 31;
		if( g > 31 )
			g = 31;
		if( b > 31 )
			b = 31;
		
		if( params->inv_red )
			r = 31-r;
		if( params->inv_grn )
			g = 31-g;
		if( params->inv_blu )
			b = 31-b;
		
		pix->r = r;
		pix->g = g;
		pix->b = b;
	}
}

RGBQUAD ImportImage::GetRGBcolor(int x, int y)
{
	RGBQUAD pix_rgb;
	
	int h = FreeImage_GetHeight(image)-1;
	
	if( FreeImage_GetBPP(image) < 16 )
	{
		BYTE idx;
		FreeImage_GetPixelIndex(image, x, h-y, &idx);
		RGBQUAD *pal = FreeImage_GetPalette(image);
		return pal[idx];
	}
	
	FreeImage_GetPixelColor(image, x, h-y, &pix_rgb);
	
	return pix_rgb;
}
