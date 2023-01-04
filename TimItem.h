#ifndef TIMITEM_H
#define TIMITEM_H

#include <string>
#include <tinyxml2.h>
#include "Fl_TimImage.h"
#include "TimImage.h"
#include "ImportImage.h"

class TimItem {
public:
	
	TimItem();
	virtual ~TimItem();

	void OutputXML(tinyxml2::XMLDocument *doc, tinyxml2::XMLElement *element, 
		const char *base_dir);
	
	void ParseXML(const tinyxml2::XMLElement *tim_element);
	
	std::string file;
	std::string src_file;
	
	Fl_TimImage	*ctrl;
	Fl_TimImage	*clut_ctrl;
	
	TimImage	tim;
	int			imported;
	
	int			group;
	
	ImportParams import_params;
};

#endif /* TIMITEM_H */

