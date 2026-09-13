#ifndef FL_TIMLIST_H
#define FL_TIMLIST_H

#include <vector>
#include <FL/Fl_Table_Row.H>

#include "TimItem.h"

class Fl_TimList : public Fl_Table_Row {
public:
	
	Fl_TimList(int X, int Y, int W, int H, const char *l = 0);
	virtual ~Fl_TimList();

	void add_item(TimItem *item, int deprecate = 0);
	TimItem *get_item(int row);
	void del_item(TimItem *item);
	
	void autowidth(int pad);
	void clear();
	
protected:
		
	void draw_cell(TableContext context, int R, int C, 
		int X, int Y, int W, int H);
	
private:
	
	typedef struct _item {
		char	*text[4];
		TimItem	*item;
	} _item;
	
	std::vector<_item> list_items;
	
};

#endif /* FL_TIMLIST_H */

