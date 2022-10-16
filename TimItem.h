#ifndef TIMITEM_H
#define TIMITEM_H

#include <filesystem>
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
		const std::filesystem::path &base_dir);
	
	void ParseXML(tinyxml2::XMLElement *tim_element);
	
	std::filesystem::path file;
	std::filesystem::path src_file;
	
	Fl_TimImage	*ctrl;
	Fl_TimImage	*clut_ctrl;
	
	TimImage	tim;
	int			imported;
	
	int			group;
	
	ImportParams import_params;
};

#endif /* TIMITEM_H */

