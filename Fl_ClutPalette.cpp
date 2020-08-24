//Created on September 27, 2019, 10:26 AM

#include <Fl/Fl.H>
#include <Fl/fl_draw.H>
#include "Fl_ClutPalette.h"

Fl_ClutPalette::Fl_ClutPalette(int X, int Y, int W, int H, const char *L)
	: Fl_Widget(X, Y, W, H, L)
{
	
	_focus_select = 0;
	_selected = 0;
	
	_edit_image = NULL;
	_edit_clut = 0;
	
}

Fl_ClutPalette::~Fl_ClutPalette()
{
}

int Fl_ClutPalette::handle(int e)
{
	
	switch(e)
	{
		case FL_PUSH:
			if( ( Fl::event_x() >= x()+2 ) && ( Fl::event_y() >= y()+2 ) )
			{
				if( ( Fl::event_x() < x()+w()-2 ) && ( Fl::event_y() < y()+h()-2 ) )
				{
					int nval = ((Fl::event_x()-x()-2)>>4)
						+(((Fl::event_y()-y()-2)>>4)<<4);
					
					int cw,ch;
					_edit_image->GetClutDimensions(cw, ch);
							
					if( nval < cw )
					{
						if( ( when() == FL_WHEN_CHANGED ) && ( nval != _selected ) )
						{
							_selected = nval;
							/*if( callback() )
							{
								callback()(this, user_data());
							}*/
						}
						if( callback() )
						{
							callback()(this, user_data());
						}

						_selected = nval;
						_focus_select = _selected;
					}
				}
			}
			Fl::focus(this);
			redraw();
			break;
			
		case FL_KEYDOWN:
			
			if( *Fl::event_text() == ' ' )
			{
				if( ( when() == FL_WHEN_CHANGED ) && ( _selected != _focus_select ) )
				{
					if( callback() )
					{
						_selected = _focus_select;
						callback()(this, user_data());
					}
				}
				_selected = _focus_select;
				redraw();
				return 1;
			}
			
			if( Fl::event_key() == FL_Up )
			{
				if( (_focus_select>>4) > 0 )
				{
					_focus_select -= 16;
					redraw();
					return 1;
				}
			}
			
			if( Fl::event_key() == FL_Down )
			{
				if( (_focus_select>>4) < 15 )
				{
					_focus_select += 16;
					redraw();
					return 1;
				}
			}
			
			if( Fl::event_key() == FL_Left )
			{
				if( (_focus_select%16) > 0 )
				{
					_focus_select -= 1;
					redraw();
					return 1;
				}
			}
			
			if( Fl::event_key() == FL_Right )
			{
				if( (_focus_select%16) < 15 )
				{
					_focus_select += 1;
					redraw();
					return 1;
				}
			}
			
			break;
			
		case FL_FOCUS:
		case FL_UNFOCUS:
			if( Fl::visible_focus() )
			{
				redraw();
				return 1;
			}
			else
			{
				return 0;
			}
			
	}
	
	return 0;
}

void Fl_ClutPalette::draw()
{
	
	fl_push_clip(x(), y(), w(), h());
	
	fl_draw_box(FL_DOWN_BOX, x(), y(), w(), h(), FL_GRAY);
	
	fl_pop_clip();
	
	if( _edit_image == NULL )
	{
		return;
	}
	
	fl_push_clip(x()+2, y()+2, w()-4, h()-4);

	for( int cy=0; cy<16; cy++ )
	{
		for( int cx=0; cx<16; cx++ )
		{
			TIM_PIX_16 ccol = _edit_image->ClutPixel(cx+(cy<<4), _edit_clut);
			Fl_Color col = fl_rgb_color(ccol.r<<3, ccol.g<<3, ccol.b<<3);
			
			int ccx = x()+2+(cx<<4);
			int ccy = y()+2+(cy<<4);
			
			if( _selected == (cx+(cy<<4)) )
			{
				fl_draw_box(FL_THIN_DOWN_BOX, ccx, ccy, 16, 16, col);
			}
			else
			{
				fl_draw_box(FL_THIN_UP_BOX, ccx, ccy, 16, 16, col);
			}
			
			if( ccol.i )
			{
				fl_color(col^0xffffff00);
				fl_polygon(ccx+1, ccy+1, ccx+1, ccy+9, ccx+9, ccy+1);
			}
			
			if( ( Fl::focus() == this ) && ( _focus_select == (cx+(cy<<4)) ) )
			{
				Fl_Color oldcol = color();
				color(col);
				draw_focus(FL_THIN_DOWN_BOX, ccx, ccy, 15, 15);
				color(oldcol);
			}
			
			/*
			fl_push_clip(ccx+1, ccy+1, 14, 14);
			
			fl_color(fl_contrast(col, color()));
			for( int i=2; i<32; i+=4 )
			{
				fl_line(ccx+1, ccy+1+i, ccx+1+i, ccy+1);
			}
			
			fl_pop_clip();
			*/
		}
		
		if( _edit_image->GetPmode() == TimImage::PMODE_4 )
			break;
		
	}
	
	fl_pop_clip();
	
}

void Fl_ClutPalette::SetImage(TimImage* tim)
{
	_edit_image = tim;
	redraw();
}

void Fl_ClutPalette::SetEditClut(int clut)
{
	_edit_clut = clut;
	redraw();
}

void Fl_ClutPalette::SetIndex(int index)
{
	_selected = index;
	redraw();
}