#include <FL/Fl.H>
#include "TimItem.h"

TimItem::TimItem()
{	
	ctrl		= NULL;
	clut_ctrl	= NULL;
	imported	= 0;
	group		= 0;
	
	memset(&import_params, 0, sizeof(ImportParams));
	
	import_params.adj_red	= 1.0f;
	import_params.adj_green	= 1.0f;
	import_params.adj_blue	= 1.0f;
	
	import_params.blend_red = 1.0f;
	import_params.blend_grn = 1.0f;
	import_params.blend_blu = 1.0f;
}

TimItem::~TimItem()
{	
	if( ctrl )
		Fl::delete_widget(ctrl);
	
	if( clut_ctrl )
		Fl::delete_widget(clut_ctrl);	
}

void TimItem::OutputXML(tinyxml2::XMLDocument *doc, 
	tinyxml2::XMLElement *element, const std::filesystem::path &base_dir)
{
	tinyxml2::XMLElement *o,*oo,*base;
	char temp[16];
	
	base = doc->NewElement("tim");
	
	const std::filesystem::path save_name = file.lexically_relative(base_dir);
	base->SetAttribute("file", save_name.c_str());
	
	if( imported )
	{
		base->SetAttribute("source", save_name.c_str());
		o = doc->NewElement("import_parameters");
		
		oo = doc->NewElement("target_bpp");
		sprintf(temp, "%d", import_params.target_bpp);
		oo->SetText(temp);
		o->InsertEndChild(oo);
		
		oo = doc->NewElement("dithering");
		sprintf(temp, "%d", import_params.dithering);
		oo->SetText(temp);
		o->InsertEndChild(oo);
		
		oo = doc->NewElement("quantize_mode");
		sprintf(temp, "%d", import_params.quant_mode);
		oo->SetText(temp);
		o->InsertEndChild(oo);
		
		oo = doc->NewElement("set_color_stp");
		sprintf(temp, "%d", import_params.color_stp);
		oo->SetText(temp);
		o->InsertEndChild(oo);
		
		oo = doc->NewElement("set_black_stp");
		sprintf(temp, "%d", import_params.black_stp);
		oo->SetText(temp);
		o->InsertEndChild(oo);
		
		oo = doc->NewElement("transparency_mode");
		sprintf(temp, "%d", import_params.transp_mode);
		oo->SetText(temp);
		o->InsertEndChild(oo);
		
		if( import_params.transp_mode == TRANSP_ALPHA )
		{
			oo = doc->NewElement("alpha_threshold");
			sprintf(temp, "%d", import_params.alpha_thresh);
			oo->SetText(temp);
			o->InsertEndChild(oo);

			oo = doc->NewElement("alpha_stp_threshold");
			sprintf(temp, "%d", import_params.stp_thresh);
			oo->SetText(temp);
			o->InsertEndChild(oo);
		}
		else if( import_params.transp_mode == TRANSP_COLORKEY )
		{
			oo = doc->NewElement("color_key");
			sprintf(temp, "0x%x", import_params.colorkey);
			oo->SetText(temp);
			o->InsertEndChild(oo);
		}
		
		if( import_params.color_adjust )
		{
			oo = doc->NewElement("color_adjust");
			oo->SetAttribute("red", import_params.adj_red);
			oo->SetAttribute("grn", import_params.adj_green);
			oo->SetAttribute("blu", import_params.adj_blue);
			
			oo->SetAttribute("stp_red", import_params.blend_red);
			oo->SetAttribute("stp_grn", import_params.blend_grn);
			oo->SetAttribute("stp_blu", import_params.blend_blu);
			
			oo->SetAttribute("inv_red", import_params.inv_red);
			oo->SetAttribute("inv_grn", import_params.inv_grn);
			oo->SetAttribute("inv_blu", import_params.inv_blu);
			
			o->InsertEndChild(oo);
		}
		
		base->InsertEndChild(o);
	}
	
	element->InsertEndChild(base);
}

void TimItem::ParseXML(tinyxml2::XMLElement* tim_element)
{
	tinyxml2::XMLElement *o,*oo;
	
	if( !tim_element->FindAttribute("source") )
		return;
	
	if( !(o = tim_element->FirstChildElement("import_parameters")) )
		return;
	
	if( (oo = o->FirstChildElement("target_bpp")) )
		import_params.target_bpp	= oo->IntText(16);
	if( (oo = o->FirstChildElement("dithering")) )
		import_params.dithering		= oo->IntText();
	if( (oo = o->FirstChildElement("quantize_mode")) )
		import_params.quant_mode	= oo->IntText();
	if( (oo = o->FirstChildElement("set_color_stp")) )
		import_params.color_stp		= oo->IntText();
	if( (oo = o->FirstChildElement("set_black_stp")) )
		import_params.black_stp		= oo->IntText();
	if( (oo = o->FirstChildElement("transparency_mode")) )
		import_params.transp_mode	= oo->IntText();
	
	if( import_params.transp_mode == TRANSP_ALPHA )
	{
		if( (oo = o->FirstChildElement("alpha_threshold")) )
			import_params.alpha_thresh	= oo->IntText();
		if( (oo = o->FirstChildElement("alpha_stp_threshold")) )
			import_params.stp_thresh	= oo->IntText();
	}
	else if( import_params.transp_mode == TRANSP_COLORKEY )
	{
		if( (oo = o->FirstChildElement("color_key")) )
		{
			sscanf(oo->GetText(), "%08x", &import_params.colorkey);
		}
	}
	
	if( (oo = o->FirstChildElement("color_adjust")) )
	{
		import_params.color_adjust = 1;
		import_params.adj_red	= oo->FloatAttribute("red", 1.f);
		import_params.adj_green	= oo->FloatAttribute("grn", 1.f);
		import_params.adj_blue	= oo->FloatAttribute("blu", 1.f);
		
		import_params.blend_red	= oo->FloatAttribute("stp_red", 1.f);
		import_params.blend_grn	= oo->FloatAttribute("stp_grn", 1.f);
		import_params.blend_blu	= oo->FloatAttribute("stp_blu", 1.f);
		
		import_params.inv_red = oo->IntAttribute("inv_red", 0);
		import_params.inv_grn = oo->IntAttribute("inv_grn", 0);
		import_params.inv_blu = oo->IntAttribute("inv_blu", 0);
	}
	
	src_file = tim_element->Attribute("source");
	imported = 1;
	
	// debug
#ifdef DEBUG
	printf("Import params for %s\n", tim_element->Attribute("file"));
	printf("  Target bpp : %d\n", import_params.target_bpp);
	printf("  Dithering  : %d\n", import_params.dithering);
	printf("  Quantizer  : %d\n", import_params.quant_mode);
	printf("  Color STP  : %d\n", import_params.color_stp);
	printf("  Black STP  : %d\n", import_params.black_stp);
	printf("  Transp Mode: %d\n", import_params.transp_mode);
	printf("  Alpha Thr. : %d\n", import_params.alpha_thresh);
	printf("  Alpha Stp. : %d\n\n", import_params.stp_thresh);
	printf("  Color Key  : %x\n", import_params.colorkey);
#endif /* DEBUG */
}
