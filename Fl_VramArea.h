#ifndef VRAMGRID_H
#define VRAMGRID_H

#include <FL/Fl_Group.H>

class Fl_VramArea : public Fl_Group {
private:
	
	int _buffer_w;
	int _buffer_h;
	int _buffer_order;
	int _buffer_res;
	
	int _zoom;
	
protected:
	
	int handle(int e);
		
public:
	
	Fl_VramArea(int x, int y, int w, int h, const char *l = 0);
	
	void draw();
	
	void SetZoom(int val);
	
	void SetBufferRes(int res);
	
	int GetBufferRes() {
		return _buffer_res;
	}
	
	void SetBufferOrder(int order);
	
	int GetBufferOrder() {
		return _buffer_order;
	}
	
	int GetZoom() {
		return _zoom;
	}
	
};

#endif /* VRAMGRID_H */

