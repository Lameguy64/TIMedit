#ifndef FL_TIMIMAGE_H
#define FL_TIMIMAGE_H

#include <FL/Fl_Box.H>
#include "TimImage.h"

#define MUL_FACTOR	33694

class Fl_TimImage : public Fl_Box {
private:
	
	static void draw_cb(void *u, int p_x, int p_y, int p_w, unsigned char *data);
	static void draw_normal_cb(void *u, int p_x, int p_y, int p_w, unsigned char *data);
	
	TimImage *image;
	int is_clut;
	int _zoom;
	int overlap;
	
	int _blend;
	int _blend_mode;
	
	int _draw_mode;
	int _no_clip;
	int _draw_clut;
	int _pick;
	int _index_picked;
	
protected:
	
	int handle(int e);
	void draw();
	
public:
	
	Fl_TimImage(int x, int y, int w, int h, const char* l = 0);	
	
	void position(int X, int Y);
	
	void SetImage(TimImage *im, int clut = 0);
	
	void SetBlendMode(int mode);
	
	void SetBlend(int blend);
	
	void SetZoom(int val);
	
	void NormalDraw(int mode);
	
	void PickMode(int m)
	{
		_pick = m;
	}
	
	void SetOverlap(int v) {
		overlap = v;
	}
	
	TimImage *GetImage() {
		return image;
	}
	
	int GetZoom() {
		return _zoom;
	}
	
	int clut() {
		return is_clut;
	}
	
	void NoBorderClip(int noclip)
	{
		_no_clip = noclip;
	}
	
	void SetDrawClut(int clut)
	{
		_draw_clut = clut;
	}
	
	int GetLastPickedIndex()
	{
		return _index_picked;
	}
};

void SetSnap(bool image, bool clut, bool grid);

#endif /* FL_TIMIMAGE_H */