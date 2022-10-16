#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "TimImage.h"
#include "Fl_TimImage.h"
#include <FreeImage.h>

#ifdef DEBUG
#include <iostream>
#endif

typedef struct {
	unsigned int id;	// 0x10
	struct {
		unsigned int pmode:3;
		unsigned int clut:1;
		unsigned int padding:24;
	} flags;
} TIM_HEADER;

typedef struct {
	unsigned int len;
    unsigned short x,y;
	unsigned short w,h;
} TIM_BLOCK;

TimImage::TimImage() {
	
	im_pixels = NULL;
	im_x = 0; im_y = 0;
	im_w = 0; im_h = 0;
	im_pmode = PMODE_4;
	
	cl_pixels = NULL;
	cl_x = 0; cl_y = 0;
	cl_w = 0; cl_h = 0;
	
	modified = 0;
	blendmode = 0;
	
}

TimImage::~TimImage() {
	
	if( im_pixels )
		free( im_pixels );
	
	if( cl_pixels )
		free( cl_pixels );
	
}

TimImage::TIM_ERR TimImage::LoadTim(const std::filesystem::path &filename) {
	
	TIM_HEADER	head;
	TIM_BLOCK	block;
	FILE *fp;
	//int sz;
	
	fp = fopen(filename.c_str(), "rb");
	
	if( !fp )
		return ERR_NOT_FOUND;
	
	if( fread( &head, 1, sizeof(TIM_HEADER), fp ) != sizeof(TIM_HEADER) ) {
		fclose( fp );
		return( ERR_CANT_READ );
	}
	
	if( head.id != 0x10 ) {
		fclose( fp );
		return( ERR_INVALID );
	}
	
	if( fread( &block, 1, sizeof(TIM_BLOCK), fp ) != sizeof(TIM_BLOCK) ) {
		fclose( fp );
		return( ERR_CANT_READ );
	}
	
	if( im_pixels ) {
		free( im_pixels );
		im_pixels = NULL;
	}
	
	if( cl_pixels ) {
		free( cl_pixels );
		cl_pixels = NULL;
	}
	
	if( head.flags.clut ) {
		
		//sz = (block.w*block.h)<<1;
		block.len -= 12;
		cl_pixels = (TIM_PIX_16*)malloc(block.len);
		
		if( fread( cl_pixels, 1, block.len, fp ) != block.len ) {
			free( cl_pixels );
			cl_pixels = NULL;
			fclose( fp );
			return( ERR_CANT_READ );
		}
		
		cl_x = block.x;	cl_y = block.y;
		cl_w = block.w;	cl_h = block.h;
		
		if( fread( &block, 1, sizeof(TIM_BLOCK), fp ) != sizeof(TIM_BLOCK) ) {
			free( cl_pixels );
			cl_pixels = NULL;
			fclose( fp );
			return( ERR_CANT_READ );
		}
		
	}
	
	//sz = (block.w*block.h)<<1;
	block.len -= 12;
	im_pixels = malloc(block.len);
	
	if( fread( im_pixels, 1, block.len, fp ) != block.len ) {
		free( im_pixels );
		im_pixels = NULL;
		if( cl_pixels ) {
			free( cl_pixels );
			cl_pixels = NULL;
		}
		fclose( fp );
		return( ERR_CANT_READ );
	}
	
	im_pmode = head.flags.pmode;
	im_x = block.x;	im_y = block.y;
	im_w = block.w;	im_h = block.h;
	if( im_pmode != PMODE_24 ) {
	
		im_iw = im_w<<(2-im_pmode);
	
	} else {
		
		im_iw = im_w/1.5f;
		
	}
	
#ifdef DEBUG
	std::cout << "Opened TIM " << filename << std::endl;
	std::cout << "  PMODE = " << im_pmode << std::endl;
	std::cout << "  SIZE  = " << im_w << "x" << im_h << std::endl;
#endif
	
	fclose( fp );
	
	return( ERR_OK );
	
}

TimImage::TIM_ERR TimImage::SaveTim(const std::filesystem::path &filename) {
	
	TIM_HEADER	head;
	TIM_BLOCK	block;
	FILE *fp;
	size_t len;
	
	fp = fopen(filename.c_str(), "wb");
	
	if( !fp ) {
		return ERR_CANT_WRITE;
	}
	
	// Write header
	head.id				= 0x10;
	head.flags.clut		= cl_pixels?1:0;
	head.flags.pmode	= im_pmode;
	
	if( fwrite(&head, 1, sizeof(TIM_HEADER), fp) != sizeof(TIM_HEADER) ) {
		fclose(fp);
		return ERR_CANT_WRITE;
	}
	
	// Write CLUT data block
	if( cl_pixels ) {
		
		block.x		= cl_x;
		block.y		= cl_y;
		block.w		= cl_w;
		block.h		= cl_h;
		block.len	= (2*(cl_w*cl_h))+12;
		
		if( fwrite(&block, 1, sizeof(TIM_BLOCK), fp) != sizeof(TIM_BLOCK) ) {
			fclose(fp);
			return ERR_CANT_WRITE;
		}
		
		len = 2*(cl_w*cl_h);
		
		if( fwrite(cl_pixels, 1, len, fp) != len ) {
			fclose(fp);
			return ERR_CANT_WRITE;
		}
		
	}
	
	// Write pixel data block
	if( im_pixels ) {
		
		block.x		= im_x;
		block.y		= im_y;
		block.w		= im_w;
		block.h		= im_h;
		block.len	= (4*(((2*(im_w*im_h))+3)/4))+12;
		
		if( fwrite(&block, 1, sizeof(TIM_BLOCK), fp) != sizeof(TIM_BLOCK) ) {
			fclose(fp);
			return ERR_CANT_WRITE;
		}
		
		len = 2*(im_w*im_h);
		
		if( fwrite(im_pixels, 1, len, fp) != len ) {
			fclose(fp);
			return ERR_CANT_WRITE;
		}
		
		// Write padding
		len = (block.len-12)-len;
		
		if( len > 0 ) {
			for (size_t i=0; i<len; i++) {
				fputc(0, fp);
			}
		}
		
	}
	
	fclose(fp);
	
	modified = 0;
	
	return ERR_OK;
	
}

TIM_PIX_24 TimImage::ImagePixel24(int x, int y) {
	
	TIM_PIX_24 pix = { 0 };
	
	if( (x < 0) || (x >= im_iw) )
		return pix;
	
	if( (y < 0) || (y >= im_h) )
		return pix;
	
	if( im_pmode != PMODE_24 )
		return pix;
	
	pix = ((TIM_PIX_24*)im_pixels)[x+(im_iw*y)];
	
	return pix;
	
}

TIM_PIX_16 TimImage::ImagePixel(int x, int y, int clut_row) {
	
	TIM_PIX_16 pix = { 0 };
	int v;
	
	if( (x < 0) || (x >= im_iw) )
		return pix;
	
	if( (y < 0) || (y >= im_h) )
		return pix;
	
	switch( im_pmode ) {
		case PMODE_4:
			if( cl_pixels ) {
				pix = cl_pixels[(((unsigned char*)im_pixels)[(x>>1)+((im_iw>>1)*y)]>>(4*(x&0x1))&0xf)+(16*clut_row)];
			} else {
				// Fallback in case no CLUT is present
				v = ((unsigned char*)im_pixels)[(x>>1)+((im_iw>>1)*y)]&(0xf<<(4*(x&0x1)))+(16*clut_row)<<1;
				pix.r = v; pix.g = v; pix.b = v; pix.i = 1;
			}
			break;
		case PMODE_8:
			if( cl_pixels ) {
				pix = cl_pixels[((unsigned char*)im_pixels)[x+(im_iw*y)]+(256*clut_row)];
			} else {
				// Fallback in case no CLUT is present
				v = ((unsigned char*)im_pixels)[x+(im_iw*y)]>>3;
				pix.r = v; pix.g = v; pix.b = v; pix.i = 1;
			}
			break;
		case PMODE_16:
			pix = ((TIM_PIX_16*)im_pixels)[x+(im_w*y)];
			break;
	}
	
	return pix;
	
}

int TimImage::PixelIndex(int x, int y)
{
	
	if( im_pmode > 1 )
		return -1;
	
	switch( im_pmode ) {
		case PMODE_4:
			return (((unsigned char*)im_pixels)[(x>>1)+((im_iw>>1)*y)]>>(4*(x&0x1)))&0xf;
		case PMODE_8:
			return ((unsigned char*)im_pixels)[x+(im_iw*y)];
	}
	
	return 0;
}

TIM_PIX_16 TimImage::ClutPixel(int x, int y) {
	
	TIM_PIX_16 pix = { 0 };
	
	if( (x < 0) || (x >= cl_w) )
		return pix;
	
	if( (y < 0) || (y >= cl_h) )
		return pix;

	pix = cl_pixels[x+(cl_w*y)];
	
	return pix;
	
}

void TimImage::SetClutPixel(int x, int y, short r, short g, short b, short i)
{
	TIM_PIX_16 pix;
	
	if( (x < 0) || (x >= cl_w) )
		return;
	
	if( (y < 0) || (y >= cl_h) )
		return;
	
	pix.r = r;
	pix.g = g;
	pix.b = b;
	pix.i = i;
	
	cl_pixels[x+(cl_w*y)] = pix;
}

void TimImage::Copy(TimImage *src, int copy_pos)
{
	
	int len = (src->im_w*src->im_h)<<1;
	
	// Copy pixels
	if( im_pixels )
		free(im_pixels);
	
	im_pixels = malloc(len);
	memcpy(im_pixels, src->im_pixels, len);
	
	im_pmode = src->im_pmode;
	im_w = src->im_w;
	im_h = src->im_h;
	im_iw = src->im_iw;
	
	if( copy_pos )
	{
		im_x = src->im_x;
		im_y = src->im_y;
	}
	
	if( cl_pixels )
	{
		free(cl_pixels);
		cl_pixels = NULL;
	}
	
	// Copy CLUT data
	if( !src->cl_pixels )
		return;
		
	len = (src->cl_w*src->cl_h)<<1;
	
	cl_pixels = (TIM_PIX_16*)malloc(len);
	memcpy(cl_pixels, src->cl_pixels, len);
	
	if( copy_pos )
	{
		cl_x = src->cl_x;
		cl_y = src->cl_y;
	}

	cl_w = src->cl_w;
	cl_h = src->cl_h;
	
}

int TimImage::AddClutSlot(int nslots)
{
	
	cl_pixels = (TIM_PIX_16*)realloc(cl_pixels, (cl_w*(cl_h+nslots))<<1);
	
	for( int i=0; i<nslots; i++ )
	{
		memcpy(&cl_pixels[cl_w*(cl_h+i)], &cl_pixels[cl_w*(cl_h-1)], 2*cl_w);
	}
	
	cl_h += nslots;
	
	return cl_h-1;
}

void TimImage::DeleteClut(int slot)
{
	if( cl_h < 1 )
		return;
	
	memmove(
		cl_pixels+(cl_w*slot), 
		cl_pixels+(cl_w*(slot+1)),
		(cl_w*(cl_h-slot))<<1);
	
	cl_h--;
	cl_pixels = (TIM_PIX_16*)realloc(cl_pixels, (cl_w*cl_h)<<1);
	
}

void TimImage::SetImageData(void *pixels, int w, int h, int pmode)
{
	if( im_pixels )
	{
		free(im_pixels);
	}
	
	im_pmode	= pmode;
	im_pixels	= pixels;
	im_iw		= w;
	im_h		= h;

	if( im_pmode != PMODE_24 ) {
		im_w = w>>(2-im_pmode);
	} else {
		im_w = w+(w>>1);
	}
}

void TimImage::SetClutData(void *colors, int w, int h)
{
	if( cl_pixels )
	{
		free(cl_pixels);
	}
	
	cl_pixels = (TIM_PIX_16*)colors;
	cl_w = w;
	cl_h = h;
}
