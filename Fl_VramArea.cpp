#include <Fl/Fl.H>
#include <FL/fl_draw.H>
#include <Fl/Fl_Widget.H>
#include "Fl_VramArea.h"

extern void *img_selected;

Fl_VramArea::Fl_VramArea(int x, int y, int w, int h, const char *l)
	: Fl_Group( x, y, w, h, l ) {

	_zoom = 1;
	
	_buffer_w = 0;
	_buffer_h = 0;
	_buffer_order = 0;
	
	size( 1024, 512 );
	
}

int Fl_VramArea::handle(int e) {

	int ret = Fl_Group::handle( e );

	if( ret ) {
		return ret;
	}

	Fl_Widget *w;

	switch( e ) {
		case FL_PUSH:

			if( Fl::event_button1() ) {

				Fl::focus(this);
				
				if( img_selected ) {

					w = (Fl_Widget*)img_selected;
					img_selected = NULL;
#ifdef DEBUG
					printf("Image selected=%p\n", img_selected);
#endif /* DEBUG */
					w->redraw();

				}

				if( callback() )
					callback()(this, user_data());

			}

			break;
	}

	return 0;
	
}

void Fl_VramArea::draw() {
	
	/*if( _bare )
		return;*/
	
	int xx,yy;
	
	fl_draw_box( FL_BORDER_BOX, x(), y(), w(), h(), color() );
	fl_draw_box( FL_BORDER_FRAME, x(), y(), w(), h(), FL_BLACK );
	
	fl_push_clip( x(), y(), w(), h() );
	
	if( _buffer_w|_buffer_h ) {
		
		fl_draw_box(FL_FLAT_BOX, x(), y(), 
			_buffer_w*_zoom, _buffer_h*_zoom, fl_rgb_color(0, 128, 0));
		
		if( _buffer_order == 0 ){
			
			if( _buffer_h*2 <= 512 ) {

				fl_draw_box(FL_FLAT_BOX, x(), y()+(_buffer_h*_zoom), 
					_buffer_w*_zoom, _buffer_h*_zoom, fl_rgb_color(0, 96, 0));

			}
			
		} else {
			
			if( ( _buffer_h <= 256 ) && ( _buffer_w*2 <= 1024 ) ) {

				fl_draw_box(FL_FLAT_BOX, x()+(_buffer_w*_zoom), y(), 
					_buffer_w*_zoom, _buffer_h*_zoom, fl_rgb_color(0, 96, 0));

			}
			
		}
		
	}
	
	fl_color( FL_BLACK );
	fl_line( x(), y()+(256*_zoom), x()+w(), y()+(256*_zoom) );
	
	for( int i=(64*_zoom); i<(1024*_zoom); i+=(64*_zoom) ) {
		
		fl_line( x()+i, y(), x()+i, y()+h() );
		
	}
	
	draw_children();
	
	/*
	fl_draw_box( FL_BORDER_FRAME, 
		x()+64*tpage_x, y()+256*tpage_y, 
		256, 256, FL_YELLOW );
	*/
	
	fl_pop_clip();
	
	//Fl_Group::draw();
	
}

/*void Fl_VramArea::SetTPagePos(int tx, int ty) {
	
	damage( FL_DAMAGE_OVERLAY, x()+64*tpage_x, y()+256*tpage_y, 256, 256 );

	tpage_x = tx;
	tpage_y = ty;

	damage( FL_DAMAGE_OVERLAY, x()+64*tpage_x, y()+256*tpage_y, 256, 256 );

	if( callback() ) {
		callback()( this, user_data() );
	}
	
}*/

void Fl_VramArea::SetZoom(int val) {
	
	_zoom = val;
	
	resize(x(), y(), 1024*_zoom, 512*_zoom);
	
	parent()->redraw();
	
}

void Fl_VramArea::SetBufferRes(int res) {
	
	static short res_table[21][2] = {
		// None
		{ 0, 0 },
		// NTSC modes
		{ 256, 240 },
		{ 320, 240 },
		{ 384, 240 },
		{ 512, 240 },
		{ 640, 240 },
		{ 256, 480 },
		{ 320, 480 },
		{ 384, 480 },
		{ 512, 480 },
		{ 640, 480 },
		// PAL modes
		{ 256, 256 },
		{ 320, 256 },
		{ 384, 256 },
		{ 512, 256 },
		{ 640, 256 },
		{ 256, 512 },
		{ 320, 512 },
		{ 384, 512 },
		{ 512, 512 },
		{ 640, 512 }
	};
	
	_buffer_w = res_table[res][0];
	_buffer_h = res_table[res][1];
	_buffer_res = res;
	
	redraw();
	
}

void Fl_VramArea::SetBufferOrder(int order) {
	
	_buffer_order = order;
	redraw();
	
}