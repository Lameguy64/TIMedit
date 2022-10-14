#include <stdio.h>
#include <sys/time.h>
#include <vector>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_message.H>
#include <FL/fl_draw.H>
#include <tinyxml2.h>
#include <FreeImage.h>

#include "Fl_TimImage.h"

#include "mainui.h"
#include "TimImage.h"
#include "TimItem.h"

#define VERSION "0.10a"

MainUI *ui;
void *img_selected = NULL;


std::string user_name;
std::string replace_name;

#define REPLACE_NAME_COUNT 6

const char *replace_name_list[] = {
	"Anobody",
	"Bumhole",
	"Chicken",
	"Loser",
	"Nancyboy",
	"Trash"
};


// Context stuff
int ctx_project_modified = false;
std::string ctx_project;
std::vector<TimItem*> ctx_items;

Fl_PNG_Image	*app_icon;

void ClearGroups()
{
	ui->groupList->clear();
	ui->groupList->add("Global", 0, NULL);
	ui->groupList->value(0);
}

TimItem* GetSelectedTimItem()
{
	Fl_TimImage *tim_ctl = (Fl_TimImage*)img_selected;
	TimImage *tim_img = tim_ctl->GetImage();
	TimItem *tim;
	
	if( !img_selected )
		return NULL;
	
	size_t found = -1;
	for( size_t i=0; i<ctx_items.size(); i++ )
	{
		if( tim_img == &ctx_items[i]->tim )
		{
			found = i;
			break;
		}
	}
		
	if( found < 0 )
		return NULL;
		
	return ctx_items[found];
}

void SetVisibleGroup(int group)
{
	size_t i;
	
	for( i=0; i<ctx_items.size(); i++ )
	{
		TimItem* tim = ctx_items[i];
		
		if(( tim->group > 0 ) && ( tim->group != group ))
		{
			tim->ctrl->hide();
			if( tim->clut_ctrl )
				tim->clut_ctrl->hide();
		}
		else
		{
			tim->ctrl->show();
			if( tim->clut_ctrl )
				tim->clut_ctrl->show();
		}
	}
}

std::string StripFileName(const char *file)
{
	std::string output;
	int i;
	
	for( i=strlen(file)-1; ((file[i]!='\\')&&(i>0)); i-- );
	
	if( i > 0 )
		output.append(file, i);
	
	return output;
}

std::string MakePathAbsolute(const char* relpath, const char* base, 
	int has_file = false)
{
	std::string output;
	size_t i,n_parents;
	size_t path_len;
	const char *c_ptr;
	
	// First two characters must be periods for it to be a relative path
	if( strncmp(relpath, "..", 2) )
	{
		// Return path as-is if absolute already
		if( strncmp(base+1, ":\\", 2) )
		{
			output = relpath;
			return output;
		}
	}
	
	// Count number of parents (strspn not reliable for this)
	c_ptr = relpath;
	n_parents = 0;
	while( (c_ptr = strstr(c_ptr, "..")) )
	{
		n_parents++;
		c_ptr += 2;
	}
	
	if( has_file )
		n_parents++;
	
	// No parent directories, return path as-is
	if( n_parents < 1 )
	{
		// Relative path assumed to be file name, append relative path to base
		output = base;
		output += "\\";
		output += relpath;
		return output;
	}
	
	// End of string
	path_len = strlen(base)-1;
	
	// Trim off directory names based on number of parents of relative path
	i = n_parents;
	while( i > 0 )
	{
		if( base[path_len] == '\\' )
			path_len--;
		
		while( (base[path_len] != '\\') && (path_len >= 0) )
			path_len--;
		
		if( path_len < 0 )
			break;
		
		i--;
	}
	
	if( path_len < 0 )
	{
		output = relpath;
		return output;
	}
	
	output.append(base, path_len);
	
	// Now trim off the relative part of the path name
	i = n_parents;
	c_ptr = relpath;
	while( i > 0 )
	{
		const char *c;
		
		if( ( c = strstr(c_ptr, "..") ) == nullptr )
		{
			break;
		}
		
		c_ptr = c + 2;
	}
	
	// Combine
	output += c_ptr;
	return output;
}

std::string MakePathRelative(const char* path, const char* base)
{
	int diff_begin;
	std::string output;
	
	diff_begin = 0;
	while( (path[diff_begin] != 0) && (base[diff_begin] != 0) )
	{
		// Check for beginning of difference
		if( tolower(path[diff_begin]) != tolower(base[diff_begin]) )
		{
			if( diff_begin < 2 )
			{
				output = path;
				return output;
			}
			
			// Snap to slash character of parent directory
			while( base[diff_begin] != '\\' )
				diff_begin--;
			
			// Count directories from base path
			for( int i=diff_begin; base[i]!=0; i++ )
			{
				if( base[i] == '\\' )
				{
					output += "..\\";
				}
			}
			
			output += (path+diff_begin+1);
			
			break;
		}
		
		diff_begin++;
	}
	
	// If file is local, simply trim off the file path
	if( output.empty() )
	{
		char *c = strrchr(path, '\\');
		if( c == nullptr )
			return output;
		output = c+1;
	}

	return output;
}

void RegisterTimItem(TimItem *item, int refresh = 0) {
	
	void cb_TimImage(Fl_TimImage *w, void *u);
	
	// Adds interactive UI widgets of TIM items
	
	int x,y;
	const char *item_name;
	
	
	// Strip path name for tooltip
	item_name = strrchr(item->file.c_str(), '/');
	if( !item_name ) {

		item_name = strrchr(item->file.c_str(), '\\');

		if( !item_name ) {
			
			item_name = item->file.c_str();
			
		} else {
			
			item_name++;
			
		}

	} else {
		
		item_name++;
		
	}
	
	
	if( refresh )
	{
		Fl::delete_widget( item->ctrl );
		
		if( item->clut_ctrl )
			Fl::delete_widget( item->clut_ctrl );
	}
	
	
	// Create the image control
	item->tim.GetPosition(x, y);
	
	item->ctrl = new Fl_TimImage(
		ui->vramArea->x()+(x*ui->vramArea->GetZoom()), 
		ui->vramArea->y()+(y*ui->vramArea->GetZoom()), 60, 60);
	
	ui->vramArea->add(item->ctrl);
	item->ctrl->SetImage(&item->tim);
	item->ctrl->user_data(item);
	item->ctrl->callback((Fl_Callback_p)cb_TimImage);
	item->ctrl->tooltip(item_name);
	item->ctrl->SetOverlap(ui->overlapToggle->value());
	item->ctrl->SetZoom(ui->vramArea->GetZoom());
	item->ctrl->SetBlendMode(item->tim.blendmode);
	item->ctrl->redraw();
	
	// Create the CLUT control
	if( item->tim.HasClut() ) {
		
		item->tim.GetClutPosition(x, y);

		item->clut_ctrl = new Fl_TimImage( 
			ui->vramArea->x()+(x*ui->vramArea->GetZoom()),
			ui->vramArea->y()+(y*ui->vramArea->GetZoom()), 60, 60);

		ui->vramArea->add(item->clut_ctrl);
		item->clut_ctrl->SetImage(&item->tim, true);
		item->clut_ctrl->user_data(item);
		item->clut_ctrl->callback((Fl_Callback_p)cb_TimImage);
		item->clut_ctrl->tooltip(item_name);
		item->clut_ctrl->SetOverlap(ui->overlapToggle->value());
		item->clut_ctrl->SetZoom(ui->vramArea->GetZoom());
		item->clut_ctrl->redraw();
		
	}
	
	// Add item to TIM item table
	if( !refresh )
	{
		ui->list->add_item(item);
		ctx_items.push_back(item);
	}
}

void NewProject()
{	
	ui->vramArea->SetBufferRes(0);
	ui->vramArea->SetBufferOrder(0);

	ui->buffer_res[0]->setonly();
	ui->bufferOrderToggle->clear();
	ui->list->clear();
	
	for( size_t i=0; i<ctx_items.size(); i++ )
	{
		delete ctx_items[i];
	}
	
	ctx_items.clear();
	ctx_project.clear();
	ctx_project_modified = false;
	ClearGroups();
	
	img_selected = NULL;
	
}

int LoadProject(const char *filename)
{	
	int i;
	std::string base_path;
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLElement *base,*o,*oo;
	
	base_path = StripFileName(filename);
	
	fl_message_title("Error loading project");
	if( doc.LoadFile(filename) != tinyxml2::XML_SUCCESS ) {
		fl_message("tinyxml2 error:\n%s", doc.ErrorStr());
		return 1;
	}
	
	base = doc.FirstChildElement("tim_project");
	
	if( !base ) {
		fl_message("Element not found:\ntim_project");
		return 1;
	}
	
	o = base->FirstChildElement("tim_group");
	if( !o ) {
		fl_message("Element not found:\ntim_group");
	}
	
	NewProject();
	
	o = base->FirstChildElement("properties");
	if( o ) {
		int i;
				
		i = o->IntAttribute("buffer_res");
		if( i < 0 )
			i = 0;
		if( i > 20 )
			i = 20;
		
		ui->buffer_res[i]->setonly();
		ui->vramArea->SetBufferRes(i);
		
		i = o->IntAttribute("buffer_order");
		ui->vramArea->SetBufferOrder(i);
		
		if( i )
			ui->bufferOrderToggle->check();
		else
			ui->bufferOrderToggle->uncheck();
	}
	
	o = base->FirstChildElement("tim_group");
	
	// Scan group names
	oo = o;
	while( oo )
	{
		if( strcasecmp( oo->Attribute("name"), "global" ) != 0 )
		{
			ui->groupList->add(oo->Attribute("name"));
		}
		oo = oo->NextSiblingElement("tim_group");
	}
	
	// Load the TIMs
	i = 0;
	while( o )
	{
		oo = o->FirstChildElement("tim");
		while( oo )
		{
			
			TimItem *item = new TimItem;
			int ret;
			
			if( oo->FindAttribute("file") == NULL ) {
				oo = oo->NextSiblingElement("tim");
				delete item;
				continue;
			}
			
			item->file = oo->Attribute("file");
			
			if( item->file.empty() )
			{
				oo = oo->NextSiblingElement("tim");
				delete item;
				continue;
			}
			
			item->file = MakePathAbsolute(item->file.c_str(), base_path.c_str());
			
			ret = item->tim.LoadTim(item->file.c_str());
			if( ret != TimImage::ERR_OK ) {
				
				switch(ret) {
					case TimImage::ERR_NOT_FOUND:
						fl_message("%s\nWas not found.", item->file.c_str());
						break;
					case TimImage::ERR_INVALID:
						fl_message("%s\nIs not a valid TIM file.", item->file.c_str());
						break;
					case TimImage::ERR_NO_IMAGE:
						fl_message("%s\nDoes not contain image data.", item->file.c_str());
						break;
					case TimImage::ERR_CANT_READ:
						fl_message("%s\nCannot be read.", item->file.c_str());
						break;
				}
				
				oo = oo->NextSiblingElement("tim");
				delete item;
				continue;
				
			}
			
			if( oo->FindAttribute("source") )
			{
				item->ParseXML(oo);
			}
			
#ifdef DEBUG
			std::string test;
			test = MakePathRelative(item->src_file.c_str(), base_path.c_str());
			printf("Relative path: %s\n", test.c_str());
			test = MakePathAbsolute(test.c_str(), base_path.c_str(), true);
			printf("Absolute path: %s\n", test.c_str());
#endif /* DEBUG */

			item->group = i;
			RegisterTimItem(item);
			
			oo = oo->NextSiblingElement("tim");
		}
		
		o = o->NextSiblingElement("tim_group");
		i++;
	}
	
	ctx_project = filename;
	SetVisibleGroup(0);
	
	return 0;
	
}

void SaveTims()
{
	for( size_t i=0; i<ctx_items.size(); i++ ) {

		TimItem *item = ctx_items[i];
		if( item->tim.modified ) {

			int ret = item->tim.SaveTim(item->file.c_str());

			fl_message_title("Save Error");
			switch( ret ) {
				case TimImage::ERR_CANT_WRITE:
					fl_message("Error saving file:\n%s", item->file.c_str());
					break;
			}

		}

	}
}

int SaveProject(const char *filename)
{
	
	std::string project_base;
	int i;
	const Fl_Menu_Item *m;
	
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLElement *base,*o,*oo;
	
	project_base = StripFileName(filename);
	
	base = doc.NewElement("tim_project");
	
	// Save project properties
	o = doc.NewElement("properties");
	o->SetAttribute("buffer_res", ui->vramArea->GetBufferRes());
	o->SetAttribute("buffer_order", ui->vramArea->GetBufferOrder());
	base->InsertEndChild(o);
	
	i = 0;
	m = ui->groupList->menu();
	while( m )
	{
		if( !m->label() )
			break;
		
		// Write group element
		o = doc.NewElement("tim_group");
		if( i == 0 )
			o->SetAttribute("name", "global");
		else
			o->SetAttribute("name", m->label());
		
		for( size_t j=0; j<ctx_items.size(); j++ )
		{	
			if( ctx_items[j]->group == i )
			{
				ctx_items[j]->OutputXML(&doc, o, project_base.c_str());
			}
		}
		
		// Insert group to base
		base->InsertEndChild(o);
		
		// Insert base to doc
		doc.InsertEndChild(base);
		
		m = m->next();
		i++;
	}
	
	if( doc.SaveFile(filename) != tinyxml2::XML_SUCCESS ) {
		fl_message_title("Save Error");
		fl_message("%s", doc.ErrorStr());
		return 1;
	}
	
	ctx_project_modified = false;
	
	int tims_modded = false;
	std::string tim_list;
	
	for( size_t i=0; i<ctx_items.size(); i++ ) {
		
		if( ctx_items[i]->tim.modified ) {
			tim_list += ctx_items[i]->file + "\n";
			tims_modded = true;
		}
			
	}
	
	if( tims_modded ) {
		
		fl_message_title("Save TIMs");
		if( fl_choice("Save modified TIMs?\n%s", "No", "Yes", 
			NULL, tim_list.c_str()) )
		{
			SaveTims();
		}
		
	}
	
	return 0;
	
}

int SaveProjectDialog(int save_as = 0) {
	
	if( ctx_items.size() == 0 ) {
		fl_message_title("Empty Project");
		fl_message("Cannot save an empty project.");
		return 1;
	}
	
	if( ( ctx_project.empty() ) || ( save_as ) ) {
		
		Fl_Native_File_Chooser chooser;

		chooser.title("Open Project");
		chooser.filter("TIM Project\t*.tpj");

		chooser.type(chooser.BROWSE_SAVE_FILE);
		chooser.options(chooser.SAVEAS_CONFIRM);

		if( chooser.show() != 0 )
			return 1;

		ctx_project = chooser.filename();

		size_t ext_pos = ctx_project.rfind("/");

		if( ext_pos == std::string::npos ) {
			ext_pos = ctx_project.rfind("\\");
		}

		ext_pos = ctx_project.find(".", ext_pos);

		if( ext_pos == std::string::npos ) {
			ctx_project += ".tpj";
		}

#ifdef DEBUG
		printf("Save name: %s\n", ctx_project.c_str());
#endif /* DEBUG */
		
	}
	
	return SaveProject(ctx_project.c_str());
	
}

int CheckSave(void) {
	
	int ret;
	int tims_modded = false;
	std::string tim_list;
	
	for( size_t i=0; i<ctx_items.size(); i++ ) {
		
		if( ctx_items[i]->tim.modified ) {
			tim_list += ctx_items[i]->file + "\n";
			tims_modded = true;
		}
			
	}
	
	if( (tims_modded) || (ctx_project_modified) ) {
		
		fl_message_title("Project Not Saved");
		ret = fl_choice("Current project has been modified, save?",
			"No", "Yes", "Cancel");
		
		if( ret == 0 )
			return 0;
		
		if( ret == 2 )
			return 1;
		
		return SaveProjectDialog();
		
	}
	
	return 0;
	
}

void cb_NewProject(Fl_Menu_ *w, void *u) {
	
	if( CheckSave() )
		return;
	
	NewProject();
	
}

void cb_OpenProject(Fl_Menu_ *w, void *u) {
	
	if( CheckSave() )
		return;
	
	Fl_Native_File_Chooser chooser;
	
	chooser.title("Save Project");
	chooser.filter("TIM Project\t*.tpj");

	chooser.type(chooser.BROWSE_FILE);
	
	if( chooser.show() )
		return;
	
	LoadProject(chooser.filename());
	
}

void cb_SaveProject(Fl_Menu_ *w, long u) {

	SaveProjectDialog(u);
	
}


void cb_ImagePosition(Fl_Spinner *w, void *u) {
	
	TimItem *item = (TimItem*)((Fl_TimImage*)img_selected)->user_data();
	
	item->ctrl->position( 
		ui->vramArea->x()+(ui->imageXpos->value()*item->ctrl->GetZoom()), 
		ui->vramArea->y()+(ui->imageYpos->value()*item->ctrl->GetZoom()));
	
}

void cb_ClutPosition(Fl_Spinner *w, void *u) {
	
	TimItem *item = (TimItem*)((Fl_TimImage*)img_selected)->user_data();
	
	item->clut_ctrl->position( 
		ui->vramArea->x()+(ui->clutXpos->value()*item->ctrl->GetZoom()), 
		ui->vramArea->y()+(ui->clutYpos->value()*item->ctrl->GetZoom()));	
	
}

void cb_VramArea(Fl_VramArea *w, void *u) {
	
	ui->timParams->deactivate();
	ui->clutParams->deactivate();
	
}

void cb_TimImage(Fl_TimImage *w, void *u) {

	// Click callback of TIM image/CLUT UI elements
	
	Fl_Group *p;
	TimImage *tim = w->GetImage();
	int real_x,real_y;
	
	p = w->parent();
	
	tim->GetPosition(real_x, real_y);
	
	ui->imageXpos->value(real_x);
	ui->imageYpos->value(real_y);
	
	if( tim->HasClut() ) {
		
		tim->GetClutPosition(real_x, real_y);
		
		ui->clutXpos->value(real_x);
		ui->clutYpos->value(real_y);
		
	} else {
		
		ui->clutXpos->value(0);
		ui->clutYpos->value(0);
		
	}
	
	if( ( Fl::event() == FL_PUSH ) || ((long)u) ) {
		
		int tw,th;

		tim->GetDimensionsVRAM( tw, th );
		
		ui->imageXpos->maximum( 1024-tw );
		ui->imageYpos->maximum( 512-th );

		if( tim->HasClut() ) {
			
			tim->GetClutDimensions( tw, th );

			ui->clutXpos->maximum( 1024-tw );
			ui->clutYpos->maximum( 512-th );
			ui->clutParams->activate();
			
		} else {
			
			ui->clutParams->deactivate();
			
		}
		
		ui->timParams->activate();
		
	}
	
	if( w->GetImage()->blendmode < 0 )
		w->GetImage()->blendmode = 0;
	
	if( w->GetImage()->blendmode > 3 )
		w->GetImage()->blendmode = 3;
	
	ui->blendMode[w->GetImage()->blendmode]->setonly();
	
	// Bring widget forward
	p->remove(w);
	p->add(w);
	p->damage(FL_DAMAGE_OVERLAY, w->x(), w->y(), w->w(), w->h());
	
	if( Fl::event_button3() )
	{
		Fl_Menu_Item pulldown[] =
		{
			{"Edit CLUT...", 0, 
			(Fl_Callback*)cb_EditClut, 0, 
			0, 0, FL_HELVETICA, 12},
			
			{"Edit Import Parameters...", 0,
			(Fl_Callback*)cb_ReimportTim, (void*)(1),
			FL_MENU_DIVIDER, 0, FL_HELVETICA, 12},
			
			{"Re-import TIM", 0, 
			(Fl_Callback*)cb_ReimportTim, 0, 
			0, 0, FL_HELVETICA, 12},
			
			{"Remove TIM", 0,
			(Fl_Callback*)delItem_cb, 0, 
			FL_MENU_DIVIDER, 0, FL_HELVETICA, 12},
			
			{"Move TIM to Group...", 0, 
			(Fl_Callback*)cb_MoveTimToGroup, 0, 
			0, 0, FL_HELVETICA, 12},
			
			{0}
		};
		
		const Fl_Menu_Item* item = pulldown->popup( Fl::event_x(), Fl::event_y() );
		
		if( item ) {
			if( item->callback() ) {
				item->callback()(w, item->user_data());
			}
		}
	}
}

int last_zoom=1;

void cb_ZoomValue(Fl_Spinner *w, void *u) {
	
	ui->vramArea->SetZoom(w->value());
	
	for( int i=0; i<ctx_items.size(); i++ ) {
		
		ctx_items[i]->ctrl->SetZoom(w->value());
		
		if( ctx_items[i]->clut_ctrl )
		{
			ctx_items[i]->clut_ctrl->SetZoom(w->value());
		}
		
	}
	
	int orig_x = (ui->vramArea->x()-(ui->fbArea->x()+2))/last_zoom;
	int orig_y = (ui->vramArea->y()-(ui->fbArea->y()+2))/last_zoom;
	
	last_zoom = w->value();
	
	orig_x *= last_zoom;
	orig_y *= last_zoom;
	
	if( ui->vramArea->w() < ui->fbArea->w()-4 ) {
		orig_x = 0;
	}
	if( ui->vramArea->h() < ui->fbArea->h()-4 ) {
		orig_y = 0;
	}

	ui->fbArea->scroll_to(-orig_x, -orig_y);
	
}

void cb_SnapOption(Fl_Check_Button *w, void *u) {
	
	SetSnap(
		ui->snapImages->value(),
		ui->snapCluts->value(),
		ui->snapGrid->value());

}

void addItem_cb(Fl_Button *w, void *u) {
	
	Fl_Native_File_Chooser chooser;
	
	TimItem *item;
	
	chooser.title( "Open TIM File" );
	chooser.filter( "PSX Texture Image File\t*.tim" );
	
	if( chooser.show() != 0 )
		return;		
	
	item = new TimItem();
	
	if( item->tim.LoadTim( chooser.filename() ) != TimImage::ERR_OK ) {
		delete item;
		fl_message_title( "Error Loading TIM" );
		fl_message( "Unable to load selected TIM file." );
		return;
	}
	
	item->file = chooser.filename();
	
	RegisterTimItem(item);
	ctx_project_modified = true;
	
}

void delItem_cb(Fl_Button *w, void *u) {
	
	for( int i=0; i<ctx_items.size(); i++ ) {
		
		if( ctx_items[i]->ctrl == img_selected ) {
			
			ui->vramArea->damage( FL_DAMAGE_OVERLAY, 
				ctx_items[i]->ctrl->x(), ctx_items[i]->ctrl->y(),
				ctx_items[i]->ctrl->w(), ctx_items[i]->ctrl->h() );
			
			ui->list->del_item(ctx_items[i]);
			
			// The class will also delete the associated widgets
			delete ctx_items[i];
			ctx_items.erase( ctx_items.begin()+i );
			
			img_selected = NULL;
			ctx_project_modified = true;
			
			break;
			
		}
		
	}
	
}

void cb_OverlapToggle(Fl_Check_Button *w, void *u) {
	
	for( size_t i=0; i<ctx_items.size(); i++ ) {
		
		ctx_items[i]->ctrl->SetOverlap(w->value());
		
		if( ctx_items[i]->clut_ctrl ) {
			ctx_items[i]->clut_ctrl->SetOverlap(w->value());
		}
		
	}
	
	ui->vramArea->redraw();
	
}

void cb_SetBufferRes(Fl_Menu_ *w, long u) {
	
	ui->vramArea->SetBufferRes(u);
	
	ctx_project_modified = true;
	
}

void cb_SetBufferOrder(Fl_Menu_ *w, void *u) {
	
	ui->vramArea->SetBufferOrder(ui->bufferOrderToggle->value());
	
	ctx_project_modified = true;
	
}

void cb_ImgBlendMode(Fl_Round_Button *w, long u) {
	
	TimItem *item = (TimItem*)((Fl_TimImage*)img_selected)->user_data();
	
	item->tim.blendmode = u;
	
	item->ctrl->SetBlendMode(u);
	
}

void cb_ToggleSemiTrans(Fl_Check_Button *w, void *u) {
	
	for( size_t i=0; i<ctx_items.size(); i++ ) {
		
		ctx_items[i]->ctrl->SetBlend(w->value());
		
	}
	
}

void cb_TimList(Fl_TimList *w, void *u)
{	
	if(( Fl::event() == FL_RELEASE ) 
		&& ( Fl::event_button() == FL_LEFT_MOUSE ))
	{	
		TimItem *item;
		int selected = -1;
		
		for(int i=0; i<w->rows(); i++)
		{
			if( w->row_selected(i) )
			{
				selected = i;
				break;
			}
		}
		
		if( selected >= 0 )
		{	
			Fl_TimImage *image = (Fl_TimImage*)img_selected;
			
			img_selected = w->get_item(selected)->ctrl;
#ifdef DEBUG
			printf("Image selected=%p\n", img_selected);
#endif /* DEBUG */

			if( image )
			{
				image->redraw();
			}
			
			image = w->get_item(selected)->ctrl;
			cb_TimImage(image, (void*)1);
			image->redraw();
			
			item = GetSelectedTimItem();
			if( item )
			{
				SetVisibleGroup(item->group);
			}
			
		}
		else
		{
			Fl_TimImage *image = (Fl_TimImage*)img_selected;
			
			img_selected = NULL;
			
			ui->timParams->deactivate();
			ui->clutParams->deactivate();
			ui->blendParams->deactivate();
			
			if( image )
			{
				image->redraw();
			}
		}
	}
	
}

void cb_GroupChoice(Fl_Choice *w, void *u)
{
#ifdef DEBUG
	printf("Active group has been switched to #%d.\n", w->value());
#endif /* DEBUG */
	SetVisibleGroup(w->value());
}

void cb_AddGroup(Fl_Menu_ *w, void *u)
{
	const char *group_name;
	
	fl_message_title("Add TIM Group");
	
	group_name = fl_input("Name of group to add:", "New Group 1");
	
	if( group_name == NULL )
		return;
	
	ui->groupList->add(group_name, 0, NULL);
}

void cb_RemoveGroup(Fl_Menu_ *w, void *u)
{
	int i,del_group;
	
	if( ui->groupList->value() == 0 )
	{
		fl_message_title("Invalid Group");
		fl_message("You cannot delete the global group.");
		return;
	}
	
	del_group = ui->groupList->value();
	
	for( i=0; i<ctx_items.size(); i++ )
	{
		if( ctx_items[i]->group == del_group )
			ctx_items[i]->group = 0;
		if( ctx_items[i]->group > del_group )
			ctx_items[i]->group--;
	}
	
	ui->groupList->value(0);
	ui->groupList->remove(del_group);
	SetVisibleGroup(0);
}

int inbox(int x, int y, int x1, int y1, int x2, int y2)
{
	if(( x >= x1 ) && ( x <= x2 ))
	{
		if(( y >= y1 ) && ( y <= y2 ))
		{
			return 1;
		}
	}
	return 0;
}

void SelectImage(TimItem *item)
{
	img_selected = item->ctrl;
	
	cb_TimImage(item->ctrl, (void*)1);
	item->ctrl->redraw();
	
	SetVisibleGroup(item->group);
}

void SelectClut(TimItem *item)
{
	if( !item->clut_ctrl )
		return;
	
	img_selected = item->clut_ctrl;
	
	cb_TimImage(item->clut_ctrl, (void*)1);
	item->clut_ctrl->redraw();
	
	SetVisibleGroup(item->group);
}

void cb_deprecatePaths(Fl_Check_Button *w, void *u)
{
	replace_name = replace_name_list[rand()%REPLACE_NAME_COUNT];
	
	ui->list->clear();
	
	for( size_t i=0; i<ctx_items.size(); i++ )
	{
		ui->list->add_item( ctx_items[i], w->value() );
	}
}

void cb_CheckOverlap(Fl_Menu_ *w, void *u)
{
	int i,j;
	
	int tx1,ty1;
	int tx2,ty2;
	
	int cx1,cy1;
	int cx2,cy2;
	
	int clut_overlap;
	
	fl_message_title("Overlap Check");
	
	for( i=0; i<ctx_items.size(); i++ )
	{
		// Test for image-image and image-CLUT overlaps
		
		ctx_items[i]->tim.GetPosition(tx1, ty1);
		ctx_items[i]->tim.GetDimensionsVRAM(tx2, ty2);
		tx2 += tx1-1;
		ty2 += ty1-1;
		
		for( j=0; j<ctx_items.size(); j++ )
		{
			if( ctx_items[j]->group > 0 )
			{
				if( ctx_items[i]->group != ctx_items[j]->group )
					continue;
			}
			
			if( ctx_items[j]->tim.HasClut() )
			{
				clut_overlap = 1;
				ctx_items[j]->tim.GetClutPosition(cx1, cy1);
				ctx_items[j]->tim.GetClutDimensions(cx2, cy2);
				cx2 += cx1-1;
				cy2 += cy1-1;
			
				if( inbox(tx1, ty1,
					cx1, cy1, cx2, cy2 ) )
					break;
				if( inbox(tx2, ty2,
					cx1, cy1, cx2, cy2 ) )
					break;
			}
			
			if( i == j )
				continue;
			
			clut_overlap = 0;
			ctx_items[j]->tim.GetPosition(cx1, cy1);
			ctx_items[j]->tim.GetDimensionsVRAM(cx2, cy2);
			cx2 += cx1-1;
			cy2 += cy1-1;
			
			if( inbox(tx1, ty1,
				cx1, cy1, cx2, cy2 ) )
				break;
			if( inbox(tx2, ty2,
				cx1, cy1, cx2, cy2 ) )
				break;
				
			if( !ctx_items[j]->tim.HasClut() )
				continue;
		}
		
		if( j < ctx_items.size() )
		{
			if( !clut_overlap )
			{
				fl_message("Image-Image overlap detected between\n%s and \n%s", 
					ctx_items[i]->file.c_str(),
					ctx_items[j]->file.c_str());
			}
			else
			{
				fl_message("Image-CLUT overlap detected between\n%s and \n%s", 
					ctx_items[i]->file.c_str(),
					ctx_items[j]->file.c_str());
			}
			SelectImage(ctx_items[i]);
			return;
		}
		
		// Test for CLUT-Image and CLUT-CLUT overlaps
		
		ctx_items[i]->tim.GetClutPosition(tx1, ty1);
		ctx_items[i]->tim.GetClutDimensions(tx2, ty2);
		tx2 += tx1-1;
		ty2 += ty1-1;
		
		for( j=0; j<ctx_items.size(); j++ )
		{
			clut_overlap = 0;
			ctx_items[j]->tim.GetPosition(cx1, cy1);
			ctx_items[j]->tim.GetDimensionsVRAM(cx2, cy2);
			cx2 += cx1-1;
			cy2 += cy1-1;
			
			if( inbox(tx1, ty1,
				cx1, cy1, cx2, cy2 ) )
				break;
			if( inbox(tx2, ty2,
				cx1, cy1, cx2, cy2 ) )
				break;
			
			if(( i == j ) || ( !ctx_items[j]->tim.HasClut() ))
				continue;			
			
			clut_overlap = 1;
			ctx_items[j]->tim.GetClutPosition(cx1, cy1);
			ctx_items[j]->tim.GetClutDimensions(cx2, cy2);
			cx2 += cx1-1;
			cy2 += cy1-1;
			
			if( inbox(tx1, ty1,
				cx1, cy1, cx2, cy2 ) )
				break;
			if( inbox(tx2, ty2,
				cx1, cy1, cx2, cy2 ) )
				break;
		}
		
		if( j < ctx_items.size() )
		{
			if( ctx_items[j]->group > 0 )
			{
				if( ctx_items[i]->group != ctx_items[j]->group )
					continue;
			}
			
			if( !clut_overlap )
			{
				fl_message("CLUT-Image overlap detected between\n%s and \n%s", 
					ctx_items[i]->file.c_str(),
					ctx_items[j]->file.c_str());
			}
			else
			{
				fl_message("CLUT-CLUT overlap detected between\n%s and \n%s", 
					ctx_items[i]->file.c_str(),
					ctx_items[j]->file.c_str());
			}
			SelectClut(ctx_items[i]);
			return;
		}
	}
	
	fl_message("No overlapping images and CLUTs detected.");
}

void cb_MainWindow(Fl_Double_Window *w, void *u)
{
	if( CheckSave() == 0 )
		w->hide();
}

void cb_Exit(Fl_Menu_ *w, void *u)
{
	if( CheckSave() == 0 )
		ui->hide();
}

void cb_About(Fl_Menu_ *w, void *u) {
	
	fl_message_title("About TIMedit");
	fl_message("TIMedit - PSX TIM conversion/editing tool\nBy Lameguy64");
}

extern char binary_icons_timedit_png_start[];
//extern unsigned int _binary_icons_timedit_png_size;

int main(int argc, char** argv)
{
	// For path name 'deprecation'
	{
		struct timeval t;
		
		gettimeofday(&t, nullptr);
		srand(t.tv_sec);
		user_name = getenv("USERNAME");
	}
	
	FreeImage_Initialise(false);
	
	ui = new MainUI;

	app_icon = new Fl_PNG_Image( NULL, 
		(unsigned char*)binary_icons_timedit_png_start, 
		400);
		
	ui->icon( app_icon );
	ui->label( "TIMedit " VERSION );
	ClearGroups();
	
	ui->show();
	
	int ret = Fl::run();
	
	delete ui;
	delete app_icon;
	
	for( int i=0; i<ctx_items.size(); i++ ) {
		delete ctx_items[i];
	}
	ctx_items.clear();
	
	FreeImage_DeInitialise();
	
	return ret;	
}