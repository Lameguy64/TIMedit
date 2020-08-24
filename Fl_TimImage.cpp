#include <Fl/Fl.H>
#include <Fl/fl_draw.H>
#include "Fl_TimImage.h"
#include <vector>
#include "TimItem.h"

extern std::vector<TimItem*> ctx_items;
extern void *img_selected;

static bool snap_images = false;
static bool snap_cluts = false;
static bool snap_grid = false;

static int inwidget(int x, int y, Fl_Widget *w) {
	
	int x1,y1;
	int x2,y2;
	
	x1 = w->x();
	y1 = w->y();
	x2 = w->x()+(w->w()-1);
	y2 = w->y()+(w->h()-1);
	
	if( ( x >= x1 ) && ( x <= x2 ) ) {
		if( ( y >= y1 ) && ( y <= y2 ) ) {
			return 1;
		}
	}
	
	return 0;
	
}

void Fl_TimImage::draw_cb(void *u, int p_x, int p_y, int p_w, unsigned char *data) {

	if( !u ) { 

		for( int x=p_x; x<(p_x+p_w); x++ ) {

			if( (((x>>2)+(p_y>>2))&0x1) == 0 ) {
				data[0] = 0x7f;
				data[1] = 0x7f;
				data[2] = 0x7f;
			} else {
				data[0] = 0x4f;
				data[1] = 0x4f;
				data[2] = 0x4f;
			}

			data += 3;

		}

		return;

	}

	Fl_TimImage *w = (Fl_TimImage*)u;
	
	if( w->image )
	{
		if( w->image->GetPmode() != TimImage::PMODE_24 ) {

			TIM_PIX_16	pix;

			for( int x=p_x; x<(p_x+p_w); x++ ) {

				if( w->is_clut ) {

					pix = w->image->ClutPixel(x/w->_zoom, p_y/w->_zoom);

				} else {

					pix = w->image->ImagePixel(
						(x/w->_zoom)<<(2-w->image->GetPmode()), 
						p_y/w->_zoom,
						w->_draw_clut);

				}

				if( pix.r+pix.g+pix.b+pix.i ) {

					data[0] = (pix.r*MUL_FACTOR)>>12;
					data[1] = (pix.g*MUL_FACTOR)>>12;
					data[2] = (pix.b*MUL_FACTOR)>>12;

					if( !w->clut() ) {

						int bcol;
						int t;

						if( pix.i )
						{

							if( (((x>>2)+(p_y>>2))&0x1) == 0 ) {
								bcol = 0x7f;
							} else {
								bcol = 0x4f;
							}

							if( w->_blend ) {
								switch( w->_blend_mode ) {
									case 0:	// 50%
										data[0] = (bcol+data[0])>>1;
										data[1] = (bcol+data[1])>>1;
										data[2] = (bcol+data[2])>>1;
										break;
									case 1: // 100% + 100%
										for( int i=0; i<3; i++ ) {	
											t = bcol+data[i];
											if( t > 255 )
												t = 255;
											data[i] = t;
										}
										break;
									case 2: // 100% - 100%
										for( int i=0; i<3; i++ ) {	
											t = bcol-data[i];
											if( t < 0 )
												t = 0;
											data[i] = t;
										}
										break;
									case 3: // 100% + 25%
										for( int i=0; i<3; i++ ) {	
											t = bcol+(data[i]*0.25f);
											if( t > 255 )
												t = 255;
											data[i] = t;
										}
										break;
								}

							}
						}

					}

				} else {

					if( (((x>>2)+(p_y>>2))&0x1) == 0 ) {
						data[0] = 0x7f;
						data[1] = 0x7f;
						data[2] = 0x7f;
					} else {
						data[0] = 0x4f;
						data[1] = 0x4f;
						data[2] = 0x4f;
					}

				}

				if( w->overlap ) {

					Fl_Group *p = w->parent();

					for(int i=0; i<p->children(); i++) {

						if( p->child(i) == w )
							continue;
						if( !p->child(i)->visible() )
							continue;
						
						if( inwidget(x+w->x(), p_y+w->y(), p->child(i)) ) {
							data[0] = 0xff;
							data[1] = 0x0;
							data[2] = 0x0;
							break;
						}

					}

				}

				data += 3;

			}

		} else {

			TIM_PIX_24 pix;

			for( int x=p_x; x<(p_x+p_w); x++ ) {

				pix = w->image->ImagePixel24((int)(x/1.5f)/w->_zoom, p_y/w->_zoom);

				data[0] = pix.r;
				data[1] = pix.g;
				data[2] = pix.b;

				if( w->overlap ) {

					Fl_Group *p = w->parent();

					for(int i=0; i<p->children(); i++) {

						if( p->child(i) == w )
							continue;

						if( inwidget(x+w->x(), p_y+w->y(), p->child(i)) ) {
							data[0] = 0xff;
							data[1] = 0x0;
							data[2] = 0xff;
							break;
						}

					}

				}

				data += 3;

			}

		}
		
	}
	else
	{
		for( int x=p_x; x<(p_x+p_w); x++ )
		{

			if( (((x>>2)+(p_y>>2))&0x1) == 0 )
			{
				data[0] = 0x7f;
				data[1] = 0x7f;
				data[2] = 0x7f;
			}
			else
			{
				data[0] = 0x4f;
				data[1] = 0x4f;
				data[2] = 0x4f;
			}

			data += 3;

		}

	}

}

void Fl_TimImage::draw_normal_cb(void *u, int p_x, int p_y, int p_w, unsigned char *data) {

	if( !u ) { 

		for( int x=p_x; x<(p_x+p_w); x++ ) {

			if( (((x>>2)+(p_y>>2))&0x1) == 0 ) {
				data[0] = 0x7f;
				data[1] = 0x7f;
				data[2] = 0x7f;
			} else {
				data[0] = 0x4f;
				data[1] = 0x4f;
				data[2] = 0x4f;
			}

			data += 3;

		}

		return;

	}

	Fl_TimImage *w = (Fl_TimImage*)u;
	
	if( w->image )
	{
		if( w->image->GetPmode() != TimImage::PMODE_24 ) {

			TIM_PIX_16	pix;

			for( int x=p_x; x<(p_x+p_w); x++ ) {

				int bcol;
				
				if( w->is_clut ) {

					pix = w->image->ClutPixel(x/w->_zoom, p_y/w->_zoom);

				} else {

					pix = w->image->ImagePixel(
						(x/w->_zoom), p_y/w->_zoom, w->_draw_clut);

				}
				
				if( (((x>>2)+(p_y>>2))&0x1) == 0 )
				{
					bcol = 0x7f;
				}
				else
				{
					bcol = 0x4f;
				}

				if( pix.i+pix.r+pix.g+pix.b )
				{
					data[0] = (pix.r*MUL_FACTOR)>>12;
					data[1] = (pix.g*MUL_FACTOR)>>12;
					data[2] = (pix.b*MUL_FACTOR)>>12;

					if( !w->clut() ) {

						int t;
						
						if( pix.i )
						{

							if( w->_blend ) {
								
								switch( w->_blend_mode ) {
									case 0:	// 50%
										data[0] = (bcol+data[0])>>1;
										data[1] = (bcol+data[1])>>1;
										data[2] = (bcol+data[2])>>1;
										break;
									case 1: // 100% + 100%
										for( int i=0; i<3; i++ ) {	
											t = bcol+data[i];
											if( t > 255 )
												t = 255;
											data[i] = t;
										}
										break;
									case 2: // 100% - 100%
										for( int i=0; i<3; i++ ) {	
											t = bcol-data[i];
											if( t < 0 )
												t = 0;
											data[i] = t;
										}
										break;
									case 3: // 100% + 25%
										for( int i=0; i<3; i++ ) {	
											t = bcol+(data[i]*0.25f);
											if( t > 255 )
												t = 255;
											data[i] = t;
										}
										break;
								}

							}
							
						}

					}

				}
				else
				{

					if( (((x>>2)+(p_y>>2))&0x1) == 0 )
					{
						data[0] = 0x7f;
						data[1] = 0x7f;
						data[2] = 0x7f;
					} else {
						data[0] = 0x4f;
						data[1] = 0x4f;
						data[2] = 0x4f;
					}

				}
				
				if( ( w->_blend ) && ( w->_blend_mode == 4 ) )
				{
					if( pix.i )
					{
						data[0] = 0xFF;
						data[1] = 0xFF;
						data[2] = 0x0;
					}
					else
					{
						data[0] = bcol;
						data[1] = bcol;
						data[2] = bcol;
					}
				}

				if( w->overlap ) {

					Fl_Group *p = w->parent();

					for(int i=0; i<p->children(); i++) {

						if( p->child(i) == w )
							continue;

						if( inwidget(x+w->x(), p_y+w->y(), p->child(i)) ) {
							data[0] = 0xff;
							data[1] = 0x0;
							data[2] = 0x0;
							break;
						}

					}

				}

				data += 3;

			}

		} else {

			TIM_PIX_24 pix;

			for( int x=p_x; x<(p_x+p_w); x++ ) {

				pix = w->image->ImagePixel24((int)x/w->_zoom, p_y/w->_zoom);

				data[0] = pix.r;
				data[1] = pix.g;
				data[2] = pix.b;

				if( w->overlap ) {

					Fl_Group *p = w->parent();

					for(int i=0; i<p->children(); i++) {

						if( p->child(i) == w )
							continue;

						if( inwidget(x+w->x(), p_y+w->y(), p->child(i)) ) {
							data[0] = 0xff;
							data[1] = 0x0;
							data[2] = 0xff;
							break;
						}

					}

				}

				data += 3;

			}

		}
		
	}
	else
	{
		for( int x=p_x; x<(p_x+p_w); x++ )
		{

			if( (((x>>2)+(p_y>>2))&0x1) == 0 )
			{
				data[0] = 0x7f;
				data[1] = 0x7f;
				data[2] = 0x7f;
			}
			else
			{
				data[0] = 0x4f;
				data[1] = 0x4f;
				data[2] = 0x4f;
			}

			data += 3;

		}

	}

}

int Fl_TimImage::handle(int e) {

	int ox,oy;
	int nx,ny;

	static int click_offset[2] = { 0, 0 };
	int ret = Fl_Box::handle( e );
	Fl_Group *p = parent();

	switch( e ) {
		case FL_PUSH:
			
			if( (Fl::event_button1()) || (Fl::event_button3()) )
			{
				if( !_pick )
				{
					click_offset[0] = x() - Fl::event_x();
					click_offset[1] = y() - Fl::event_y();

					if( !_draw_mode ) {

						if( img_selected ) {

							((Fl_TimImage*)img_selected)->redraw();

						}

						img_selected = this;
#ifdef DEBUG
						printf("Selected Image=%p\n", img_selected);
#endif /* DEBUG */
					}
					Fl::focus(this);

					// Trick to bring the widget front
					/*p->remove(this);
					p->add(this);
					p->damage(FL_DAMAGE_OVERLAY, x(), y(), w(), h());*/

					if( callback() )
					{
						callback()(this, user_data());
					}

					if( Fl::event_button1() )
						return 1;
				}
				else
				{
					ox = (Fl::event_x()-x())/_zoom;
					oy = (Fl::event_y()-y())/_zoom;
					
					_index_picked = image->PixelIndex(ox, oy);
					
					if( callback() )
					{
						callback()(this, user_data());
					}
					
					_pick = 0;
					fl_cursor(FL_CURSOR_DEFAULT);
					break;
				}
			}
			break;

		case FL_RELEASE:
			
			if( Fl::event_button3() )
			{
				if( callback() )
				{
					callback()(this, user_data());
				}
			}
			return 1;

		case FL_DRAG:
			ox = x(); oy = y();
			nx = click_offset[0]+Fl::event_x();
			ny = click_offset[1]+Fl::event_y();

			if( ( !_no_clip ) && ( Fl::event_button1() ) ) {
				
				if( nx < p->x() )
					nx = p->x();
				if( ny < p->y() )
					ny = p->y();

				if( (nx+w()) > (p->x()+p->w()) )
					nx = (p->x()+p->w())-w();
				if( (ny+h()) > (p->y()+p->h()) )
					ny = (p->y()+p->h())-h();

				// Snapping logic
				if( snap_images || snap_cluts ) {

					for( int i=0; i<parent()->children(); i++ ) {

						Fl_TimImage *ch = (Fl_TimImage*)parent()->child(i);

						if( ch == this )
							continue;
						if( !ch->visible() )
							continue;

						// Don't snap CLUTs to other CLUTs
						if( clut() && ch->clut() )
							continue;

						if( !snap_images && !ch->clut() )
							continue;

						if( !snap_cluts && ch->clut() )
							continue;

						int cx1 = ch->x();
						int cy1 = ch->y();
						int cx2 = (ch->w()-1)+cx1;
						int cy2 = (ch->h()-1)+cy1;
						int x_snapped = false, y_snapped = false;

						if( ( nx+(w()-1) >= cx1 ) && ( nx <= cx2 ) ) {

							// Snap on top edges
							if( ( ny+(h()-1) >= cy1-4 ) && 
							( ny+(h()-1) <= cy1+4 ) ) {
								ny = (cy1+_zoom)-(h()+1);
								y_snapped = true;
							}

							// Snap on bottom edges
							if( ( ny >= cy2-4 ) && ( ny <= cy2+4 ) ) {
								ny = cy2+_zoom;
								y_snapped = true;
							}

						}

						if( ( y_snapped ) && 
						( nx+(w()-1) >= cx1 ) && ( nx <= cx2 ) ) {

							// Snap on left
							if( ( nx >= cx1-4 ) && ( nx <= cx1+4 ) ) 
								nx = cx1+(_zoom-1);

							// Snap on right
							if( ( nx+(w()-1) >= cx2-4 ) &&
							( nx+(w()-1) <= cx2+4 ) )
								nx = (cx2+_zoom)-w();

						}

						if( ( ny+(h()-1) >= cy1 ) && ( ny <= cy2 ) ) {

							// Snap on left edges
							if( ( nx+(w()-1) >= cx1-4 ) &&
							( nx+(w()-1) <= cx1+4 ) ) {
								nx = (cx1+_zoom)-(w()+1);
								x_snapped = true;
							}

							// Snap on right edges
							if( ( nx >= cx2-4 ) && ( nx <= cx2+4 ) ) {
								nx = cx2+_zoom;
								x_snapped = true;
							}

						}

						if( ( x_snapped ) &&
						( ny+(h()-1) >= cy1 ) && ( ny <= cy2 ) ) {

							// Snap on top
							if( ( ny >= cy1-4 ) && ( ny <= cy1+4 ) )
								ny = cy1+(_zoom-1);

							// Snap on bottom
							if( ( ny+(h()-1) >= cy2-4 ) &&
							( ny+(h()-1) <= cy2+4 ) )
								ny = (cy2+_zoom)-h();

						}

					}

				}

				// Grid snapping logic
				if( snap_grid ) {

					for( int i=0; i<=(1024*_zoom); i+=(64*_zoom) ) {

						int gx = p->x()+i;

						if( ( nx >= gx-4 ) && ( nx <= gx+4 ) ) 
							nx = gx;

						if( ( nx+(w()) >= gx-4 ) && ( nx+(w()) <= gx+4 ) ) 
							nx = gx-w();

					}

					for( int i=0; i<=(512*_zoom); i+=(256*_zoom) ) {

						int gy = p->y()+i;

						if( ( ny >= gy-4 ) && ( ny <= gy+4 ) ) 
							ny = gy;

						if( ( ny+(h()-1) >= gy-4 ) && ( ny+(h()-1) <= gy+4 ) ) 
							ny = gy-h();

					}

				}
				
			}
			
			position(nx, ny);

			redraw();
			if( !_no_clip ) {
				
				p->damage(FL_DAMAGE_SCROLL, ox, oy, w(), h());
				
			} else {
				
				p->redraw();
				
			}
			
			image->modified = true;
			
			if( callback() )
				callback()(this, user_data());

			return 1;
			
		case FL_ENTER:
			if( _pick )
			{
				fl_cursor(FL_CURSOR_CROSS);
			}
			return 1;
			
		case FL_LEAVE:
			if( _pick )
			{
				fl_cursor(FL_CURSOR_DEFAULT);
			}
			return 1;
	}

	return ret;

}
	
Fl_TimImage::Fl_TimImage(int x, int y, int w, int h, const char* l) 
	: Fl_Box( x, y, w, h, l ) {

	box( FL_SHADOW_BOX );
	color( FL_BLUE );

	image = NULL;
	_zoom = 1;
	overlap = 0;
	_blend = 0;
	
	_draw_mode = 0;
	_no_clip = 0;
	_draw_clut = 0;

	_pick = 0;
	_index_picked = -1;
	
}

void Fl_TimImage::draw() {

	Fl_Widget *p = parent();

	fl_push_clip(p->x(), p->y(), p->w(), p->h());

	if( _draw_mode == 0 )
	{
		fl_draw_image(draw_cb, this, x(), y(), w(), h());
	}
	else
	{
		fl_draw_image(draw_normal_cb, this, x(), y(), w(), h());
	}
	
	if( img_selected == this ) {
	
		fl_draw_box(FL_BORDER_FRAME, x(), y(), w(), h(), FL_WHITE);
	
	}
	
	fl_pop_clip();

}

void Fl_TimImage::SetImage(TimImage *im, int clut) {

	int tw,th;

	image = im;
	is_clut = clut;
	
	if( _draw_mode == 0 )
	{
		if( !clut ) {

			im->GetDimensionsVRAM(tw, th);

		} else {

			im->GetClutDimensions(tw, th);

		}
	}
	else
	{
		im->GetDimensions(tw, th);
	}
	
	size( tw, th );
	
}

void Fl_TimImage::SetZoom(int val) {

	int tw,th;
		
	_zoom = val;
	
	if( !is_clut ) {
		
		if( !_draw_mode )
		{
			image->GetDimensionsVRAM(tw, th);
		}
		else
		{
			image->GetDimensions(tw, th);
		}
		
	} else {
		
		image->GetClutDimensions(tw, th);
		
	}
	
	size( tw*_zoom, th*_zoom );
	
}

void Fl_TimImage::position(int X, int Y) {
	
	int p_x = parent()->x();
	int p_y = parent()->y();
	int real_x,real_y;
	
	parent()->damage(FL_DAMAGE_OVERLAY, x(), y(), w(), h());

	if( !_draw_mode )
	{
		real_x = (X-p_x)/_zoom;
		real_y = (Y-p_y)/_zoom;

		if( !clut() ) {
			image->SetPosition(real_x, real_y);
		} else {
			image->SetClutPosition(real_x, real_y);
		}
		
		Fl_Box::position(p_x+(real_x*_zoom), p_y+(real_y*_zoom));
	}
	else
	{
		Fl_Box::position(X, Y);
	}
	
	redraw();
	
}

void SetSnap(bool image, bool clut, bool grid) {

	snap_images = image;
	snap_cluts = clut;
	snap_grid = grid;
	
}

void Fl_TimImage::SetBlend(int blend) {

	_blend = blend;
	redraw();
	
}

void Fl_TimImage::SetBlendMode(int mode) {
	
	_blend_mode = mode;
	
	redraw();
	
}

void Fl_TimImage::NormalDraw(int mode)
{
	_draw_mode = mode;

	if( image )
	{
		int w,h;
		if( mode )
		{
			image->GetDimensions(w, h);
		}
		else
		{
			image->GetDimensionsVRAM(w, h);
		}
		size(w*_zoom, h*_zoom);
	}
}