// Created on September 27, 2019, 10:26 AM

#ifndef FL_CLUTPALETTE_H
#define FL_CLUTPALETTE_H

#include <Fl/Fl_Widget.H>
#include "TimImage.h"

class Fl_ClutPalette : public Fl_Widget {
public:
	
	Fl_ClutPalette(int X, int Y, int W, int H, const char *L);
	virtual ~Fl_ClutPalette();
	
	void end()
	{
		
	}
	
	int selected()
	{
		return _selected;
	}
	
	void SetImage(TimImage *tim);
	
	void SetIndex(int index);
	void SetEditClut(int clut);
	int GetEditClut()
	{
		return _edit_clut;
	}
	
protected:
		
	int handle(int e);
	void draw();

private:
	
	int _selected;
	int _focus_select;
	
	TimImage *_edit_image;
	int _edit_clut;
	
};

#endif /* FL_CLUTPALETTE_H */

