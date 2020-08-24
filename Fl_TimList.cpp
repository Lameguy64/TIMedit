#include <Fl/fl_draw.H>
#include <Fl/Fl_Widget.H>
#include <Fl/Enumerations.H>
#include "Fl_TimList.h"

#define HEADER_FONTFACE FL_HELVETICA_BOLD
#define HEADER_FONTSIZE 12
#define ROW_FONTFACE    FL_HELVETICA
#define ROW_FONTSIZE    12

#define TABLE_ROW_HEIGHT	16

static const char *table_headings[] = {
	"File",
	"Directory",
	"Size",
	"Mode"
};


Fl_TimList::Fl_TimList(int X, int Y, int W, int H, const char *l)
	: Fl_Table_Row(X, Y, W, H, l) {
	
	type(SELECT_SINGLE);
	col_header(1);
	col_resize(1);
	cols(4);
	rows(0);
	
	col_header_height(TABLE_ROW_HEIGHT);
	
	// Workaround of what might be a bug in FLTK where Fl_Table_Row's box style
	// won't apply until a resize
	//box(FL_DOWN_FRAME);
	//resize(X, Y, W, H);
	
}

Fl_TimList::~Fl_TimList() {
	
	for( size_t i=0; i<list_items.size(); i++ ) {
		
		for( int j=0; j<4; j++ ) {
			
			if( list_items[i].text[j] )
				free(list_items[i].text[j]);
			
		}
	}
	
}

extern std::string user_name;
extern std::string replace_name;

void Fl_TimList::add_item(TimItem* item, int deprecate) {
	
	_item itm;
	int i,tw,th;
	
	memset(&itm, 0, sizeof(_item));
	
	// Set name of item
	itm.text[0] = strrchr((char*)item->file.c_str(), '/');
	if( !itm.text[0] ) {

		itm.text[0] = strrchr((char*)item->file.c_str(), '\\');

		if( !itm.text[0] ) {
			
			itm.text[0] = strdup(item->file.c_str());
			
		} else {
			
			itm.text[0] = strdup(itm.text[0]+1);
			
		}

	} else {
		
		itm.text[0] = strdup(itm.text[0]+1);
		
	}
	
	// Set directory of item
	std::string path_temp = item->file;			
	size_t pos = path_temp.find_last_of("/");

	if( pos == std::string::npos )
		pos = path_temp.find_last_of("\\");

	if( pos != std::string::npos )
		path_temp.erase(pos);

	if( deprecate )
	{
		while( (i = path_temp.find(user_name)) != std::string::npos )
		{
			path_temp.replace(i, user_name.size(), replace_name);
		}
	}
	
	itm.text[1] = strdup(path_temp.c_str());
	
	// Set image dimensions
	item->tim.GetDimensions(tw, th);

	path_temp = std::to_string(tw) + "x" + std::to_string(th);
	itm.text[2] = strdup(path_temp.c_str());
	
	// Set color depth
	switch(item->tim.GetPmode()) {
		case TimImage::PMODE_4:
			path_temp = "4-bit ";
			break;
		case TimImage::PMODE_8:
			path_temp = "8-bit ";
			break;
		case TimImage::PMODE_16:
			path_temp = "16-bit";
			break;
		case TimImage::PMODE_24:
			path_temp = "24-bit";
			break;
	}

	if( ( item->tim.GetPmode() < TimImage::PMODE_16 )
	&& item->tim.HasClut() ) {

		path_temp += "CLUT";

	}

	itm.text[3] = strdup(path_temp.c_str());
	
	// Set a reference to the specified item
	itm.item = item;
	
	list_items.push_back(itm);
	
	rows(list_items.size());
	row_height(list_items.size()-1, TABLE_ROW_HEIGHT);
	
	autowidth(20);
	
}

TimItem* Fl_TimList::get_item(int row) {

	if( row < 0 )
		return NULL;
	
	if( row >= rows() )
		return NULL;
	
	return list_items[row].item;
}


void Fl_TimList::del_item(TimItem *item) {
	
	size_t index = -1;
	
	for( size_t i=0; i<list_items.size(); i++ ) {
		
		if( list_items[i].item == item ) {
			index = i;
			break;
		}
		
	}
	
	if( index < 0 )
		return;
	
	for( int i=0; i<4; i++ )
		free(list_items[index].text[i]);
	
	list_items.erase(list_items.begin()+index);
	
	rows(list_items.size());
	
}

void Fl_TimList::clear() {

	for( size_t R=0; R<list_items.size(); R++ ) {	
		for( int C=0; C<cols(); C++ ) {
			
			if( list_items[R].text[C] )
				free(list_items[R].text[C]);
			
		}
	}
	
	list_items.clear();
	
	rows(0);
	
}

void Fl_TimList::autowidth(int pad) {
	
	for(size_t R=0; R<list_items.size(); R++) {
		
		for(int C=0; C<cols(); C++) {

			int tw,th;
			fl_measure(list_items[R].text[C], tw, th, 0);

			if( (tw+pad) > col_width(C) ) {
				
				col_width(C, tw+pad);
				
			}

		}
		
	}
	
}

void Fl_TimList::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {
    
	/*const char *s = nullptr; 
	
	if( R < (int)_row_items.size() && C < (int)_row_items[R].cols.size() ) {
		s = _row_items[R].cols[C].c_str(); 
	}*/
	
	char temp[64];
	const char *text = "Item";
	std::string path_temp;
	size_t pos;
	int tw,th;
	
	
    switch( context ) {
        case CONTEXT_COL_HEADER:
			
            fl_push_clip(X, Y, W, H);
			
			fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, FL_BACKGROUND_COLOR);
			
            if( C < 9 ) {
				
				fl_font(HEADER_FONTFACE, HEADER_FONTSIZE);
				fl_color(FL_BLACK);
				fl_draw(table_headings[C], X+2, Y, W, H, FL_ALIGN_LEFT, 0, 0);
				
				// Draw sort arrow
				/*if( C == _sort_lastcol ) {
					draw_sort_arrow( X, Y, W, H );
				}*/
				
			}
			
            fl_pop_clip();
			
            return; 
			
        case CONTEXT_CELL:
			
			if( R < list_items.size() ) {
				text = list_items[R].text[C];
			}
			
            fl_push_clip(X,Y,W,H);
			
			// Bg color
			Fl_Color bgcolor = row_selected( R ) ? selection_color() : FL_WHITE;
			//Fl_Color bgcolor = FL_WHITE;
			
			fl_color(bgcolor);
			fl_rectf(X, Y, W, H);
			fl_font(ROW_FONTFACE, ROW_FONTSIZE);
			fl_color(FL_BLACK );
			
			fl_color(fl_contrast( FL_WHITE, bgcolor));
			
			fl_draw(text, X+2,Y,W,H, FL_ALIGN_LEFT);
			
            // Border
            fl_color(FL_LIGHT2);
			fl_rect(X, Y, W, H);
            
            fl_pop_clip();
			
            return;
        
    }
	
}
