#include <string>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_message.H>
#include <FreeImage.h>
#include "mainui.h"
#include "importui.h"
#include "ImportImage.h"
#include "Fl_TimImage.h"
	
extern MainUI					*ui;
extern std::filesystem::path ctx_project;
extern std::vector<TimItem*>	ctx_items;

void RegisterTimItem(TimItem* item, int refresh = 0);


ImportUI		*import_ui;
std::filesystem::path import_image;
ImportParams	import_params;
ImportImage		*importer;
TimItem			*import_item;
TimItem			*reimport_item;

static int import_created;
static int pick_state;

extern void* img_selected;

static int select_image()
{
	Fl_Native_File_Chooser chooser;
	
	chooser.title("Open Project");
	chooser.filter(
		"Portable Network Graphics\t*.png\n"
		"Bitmap Image\t*.bmp\n"
		"JPEG Image\t*.jpg,*.jpeg\n"
		"Graphics Interchange Format\t*.gif\n");

	chooser.type(chooser.BROWSE_FILE);
	
	if( chooser.show() != 0 )
		return 0;
	
	import_image = chooser.filename();
	
	return 1;
}

static int update_tim()
{
	fl_message_title("Error Converting Image");
	
	switch( importer->Convert(&import_item->tim, &import_params) )
	{
		case ImportImage::CONVERT_UNSUPPORT:
			fl_message("Unsupported target parameters.");
			return 1;
		case ImportImage::CONVERT_MANYCOLS:
			fl_message("Too many colors for target color depth.\nTry a different quantizer.");
			return 1;
		case ImportImage::CONVERT_MISSING:
			fl_message("Missing colors, quantizer bugged.");
			return 1;
		case ImportImage::CONVERT_QUANTERR:
			fl_message("Quantization error, possibly too few colors in image.");
			return 1;
	}
	
	return 0;
}

static void SetControls(ImportParams* params)
{
	// Set selected color depth
	switch( params->target_bpp )
	{
		case 4:
			import_ui->depthChoice->value(0);
			break;
		case 8:
			import_ui->depthChoice->value(1);
			break;
		case 16:
		case 24:
		case 32:
			import_ui->depthChoice->value(2);
			break;
		default:
			import_ui->depthChoice->value(0);
	}
	
	// Set quantizer mode
	switch( params->quant_mode )
	{
		case 0:
			import_ui->quantizeMode->value(0);
			break;
		case 1:
			import_ui->quantizeMode->value(1);
			break;
		case 2:
			import_ui->quantizeMode->value(2);
			break;
		default:
			import_ui->quantizeMode->value(0);
	}
	
	// Set toggle options
	import_ui->ditherToggle->value(params->dithering);
	import_ui->stpNonBlack->value(params->color_stp);
	import_ui->stpBlack->value(params->black_stp);
	
	switch( params->transp_mode )
	{
		case 0:
			import_ui->stpNormal->set();
			break;
		case 1:
			import_ui->stpAlpha->set();
			import_ui->alphaCtrls->activate();
			break;
		case 2:
			import_ui->colorKey->set();
			import_ui->colorKeyCtrls->activate();
			break;
	}
	
	// Color adjust parameters
	if( params->color_adjust )
	{
		import_ui->colorAdjToggle->set();
		
		import_ui->generalAdj[0]->value(params->adj_red);
		import_ui->generalAdj[1]->value(params->adj_green);
		import_ui->generalAdj[2]->value(params->adj_blue);
		
		import_ui->blendAdj[0]->value(params->blend_red);
		import_ui->blendAdj[1]->value(params->blend_grn);
		import_ui->blendAdj[2]->value(params->blend_blu);
		
		import_ui->invertChan[0]->value(params->inv_red);
		import_ui->invertChan[1]->value(params->inv_grn);
		import_ui->invertChan[2]->value(params->inv_blu);
	}
	else
	{
		import_ui->colorAdjToggle->clear();
	}
}

void cb_ImportOkayAlt(Fl_Return_Button* w, void* u);

void do_import(const std::filesystem::path &filename, ImportParams *params)
{
	fl_message_title("Error Importing");
	
	importer = new ImportImage();
	
	if( importer->LoadSource(filename) )
	{
		fl_message("Source image not found, or error loading source image.");
		return;
	}
	
	import_item = new TimItem;
	
	if( params )
	{
		// Use specified parameters
		import_params = *params;
	}
	else
	{
		// Set default parameters
		import_params.target_bpp = importer->GetSourceDepth();

		if( import_params.target_bpp > 16 )
			import_params.target_bpp = 16;

		import_params.color_stp		= 0;
		import_params.black_stp		= 0;
		import_params.alpha_thresh	= 127;
		import_params.transp_mode	= TRANSP_BLACK;
		import_params.dithering		= false;
		import_params.quant_mode	= 0;
		import_params.colorkey		= 0;

		import_params.color_adjust	= 0;
		import_params.adj_red		= 1.0f;
		import_params.adj_green		= 1.0f;
		import_params.adj_blue		= 1.0f;

		import_params.blend_red		= 1.0f;
		import_params.blend_grn		= 1.0f;
		import_params.blend_blu		= 1.0f;
	}
	
	int p_w,p_h;
	if( !update_tim() )
	{
		import_item->tim.GetDimensions(p_w, p_h);
	}
	else
	{
		p_w = 64;
		p_h = 64;
	}
	
	pick_state = 0;
	import_created = 0;
	
	// Create import dialog and assign image to preview
	import_ui = new ImportUI();
	
	import_item->ctrl = new Fl_TimImage(
		(import_ui->preview->x()+(import_ui->preview->w()>>1))-(p_w>>1),
		(import_ui->preview->y()+(import_ui->preview->h()>>1))-(p_h>>1),
		p_w, p_h);

	if( params )
	{
		import_ui->okayButton->callback((Fl_Callback*)cb_ImportOkayAlt);
	}
	import_ui->preview->add(import_item->ctrl);
	import_ui->srcDepth->value(importer->GetSourceDepth());
	
	import_item->ctrl->SetImage(&import_item->tim);
	import_item->ctrl->user_data(import_item);
	
	import_item->ctrl->redraw();
	import_item->ctrl->NormalDraw(1);
	import_item->ctrl->NoBorderClip(1);
	
	import_ui->preview->add(import_item->ctrl);
	import_ui->vmWidth->value(p_w);
	
	SetControls(&import_params);
	
	import_ui->show();
	
	while( import_ui->shown() )
	{
		Fl::wait();
	}
	
	import_ui->preview->remove(import_item->ctrl);
	delete import_ui;
	
	Fl::wait();
	
	if( !import_created )
		delete import_item;
	
	delete importer;
}

void cb_ImportImage(Fl_Menu_*, void*)
{
	import_image.clear();
	if( !select_image() )
	{
		return;
	}
	
	fl_message_title( "Error Importing" );
	
	if( import_image.empty() )
	{
		fl_message("No source image file specified.");
		return;
	}
	
	reimport_item = NULL;
	do_import(import_image, NULL);
}

void cb_ReimportTim(Fl_Menu_*, long u)
{
	if( !img_selected )
		return;
	
	Fl_TimImage *tim_ctl = (Fl_TimImage*)img_selected;
	TimImage *tim_img = tim_ctl->GetImage();
	TimItem *tim;
	
	{
		int found = -1;
		for( size_t i=0; i<ctx_items.size(); i++ )
		{
			if( tim_img == &ctx_items[i]->tim )
			{
				found = i;
				break;
			}
		}
		
		if( found < 0 )
			return;
		
		tim = ctx_items[found];
	}
	
	if( !tim->imported )
	{
		fl_message_title( "Not Imported TIM" );
		fl_message( "The TIM file's source image was not imported." );
		return;
	}
	
	fl_message_title( "Error Importing" );
	
	std::filesystem::path abs_file;
	
	if( tim->src_file.is_absolute() )
	{
		abs_file = tim->src_file;
	}
	else
	{
		abs_file = ctx_project.parent_path();
		abs_file /= tim->src_file;
	}
	abs_file = abs_file.lexically_normal();
	
	if( !u )
	{
		if( abs_file.empty() )
		{
			fl_message("Selected TIM has no source image associated.\n");
			return;
		}
		
		importer = new ImportImage();

#ifdef DEBUG
		printf( "Attempt to import: %s\n", abs_file.c_str() );
#endif /* DEBUG */

		if (importer->LoadSource(abs_file))
		{
			fl_message("Source image not found or cannot be loaded.");
			delete importer;
			return;
		}

		if( importer->Convert( tim_img, &tim->import_params ) 
			!= ImportImage::CONVERT_OK )
		{
			delete importer;
			return;
		}

		RegisterTimItem(tim, 1);

		delete importer;
	}
	else
	{
		reimport_item = tim;
		do_import( abs_file.c_str(), &tim->import_params );
	}
}

void cb_ImportDepthChoice(Fl_Choice*, void*)
{
	switch( import_ui->depthChoice->value() )
	{
		case 0:
			import_params.target_bpp = 4;
			if( import_ui->quantizeMode->value() == 0 )
			{
				import_ui->ditherToggle->activate();
			}
			else
			{
				import_ui->ditherToggle->deactivate();
			}
			import_ui->quantizeMode->activate();
			break;
		case 1:
			import_params.target_bpp = 8;
			if( import_ui->quantizeMode->value() == 0 )
			{
				import_ui->ditherToggle->activate();
			}
			else
			{
				import_ui->ditherToggle->deactivate();
			}
			import_ui->quantizeMode->activate();
			break;
		case 2:
			import_params.target_bpp = 16;
			import_ui->ditherToggle->activate();
			import_ui->quantizeMode->deactivate();
			break;
		case 3:
			import_params.target_bpp = 24;
			import_ui->ditherToggle->deactivate();
			import_ui->quantizeMode->deactivate();
			break;
	}
	
	if( import_params.target_bpp == importer->GetSourceDepth() )
	{
		if( importer->GetSourceDepth() < 16 )
		{
			import_ui->ditherToggle->deactivate();
		}
		import_ui->quantizeMode->deactivate();
	}
	
	if( !update_tim() )
	{
		import_item->ctrl->redraw();
		int p_w,p_h;
		import_item->tim.GetDimensionsVRAM(p_w, p_h);
		import_ui->vmWidth->value(p_w);
	}
	
}

void cb_ImportDither(Fl_Check_Button*, void*)
{
	import_params.dithering = import_ui->ditherToggle->value();
	if( !update_tim() )
	{
		import_item->ctrl->redraw();
		int p_w,p_h;
		import_item->tim.GetDimensionsVRAM(p_w, p_h);
		import_ui->vmWidth->value(p_w);
	}
}

void cb_ImportZoom(Fl_Spinner*, void*)
{
	int zoom = import_ui->zoomValue->value();
	int p_w,p_h;
	
	import_item->tim.GetDimensions(p_w, p_h);
	import_item->ctrl->SetZoom(zoom);
	
	import_item->ctrl->position(
		(import_ui->preview->x()+(import_ui->preview->w()>>1))-((p_w*zoom)>>1),
		(import_ui->preview->y()+(import_ui->preview->h()>>1))-((p_h*zoom)>>1) );
	
	import_ui->preview->redraw();
}

void cb_ImportCancel(Fl_Button*, void*)
{
	import_ui->hide();
}

void cb_ImportQuantChoice(Fl_Choice*, void*)
{
	import_params.quant_mode = import_ui->quantizeMode->value();
	
	if( import_params.quant_mode > 0 )
	{
		import_ui->ditherToggle->deactivate();
	}
	else
	{
		import_ui->ditherToggle->activate();
	}
	
	if( !update_tim() )
	{
		import_item->ctrl->redraw();
		int p_w,p_h;
		import_item->tim.GetDimensionsVRAM(p_w, p_h);
		import_ui->vmWidth->value(p_w);
	}
}

void cb_ImportUpdate(Fl_Check_Button*, void*)
{
	import_params.color_stp = import_ui->stpNonBlack->value();
	import_params.black_stp = import_ui->stpBlack->value();
	
	if( import_ui->stpAlpha->value() )
	{
		import_params.transp_mode = TRANSP_ALPHA;
		import_ui->alphaCtrls->activate();
	}
	else
	{
		import_params.transp_mode = TRANSP_BLACK;
		import_ui->alphaCtrls->deactivate();
	}
	
	import_params.alpha_thresh	= import_ui->alphaValue->value();
	import_params.stp_thresh	= import_ui->stpAlphaValue->value();
		
	if( import_ui->colorKey->value() )
	{
		import_params.transp_mode = TRANSP_COLORKEY;
		import_ui->colorKeyCtrls->activate();
	}
	else
	{
		import_ui->colorKeyCtrls->deactivate();
	}
	
	import_ui->colorKeyBox->color(fl_rgb_color(
		import_params.colorkey&0xff,
		(import_params.colorkey>>8)&0xff,
		(import_params.colorkey>>16)&0xff));
	
	if( !update_tim() )
	{
		import_item->ctrl->redraw();
	}
}

void cb_ImportUpdateV(Fl_Value_Input *w, void *u)
{
	cb_ImportUpdate((Fl_Check_Button*)w, u);
}

void cb_ImportDrawMode(Fl_Choice*, void*)
{
	if( import_ui->drawMode->value() == 0 )
	{
		import_item->ctrl->SetBlend(0);
		return;
	}
	
	import_item->ctrl->SetBlendMode(import_ui->drawMode->value()-1);
	import_item->ctrl->SetBlend(1);
	
}

void cb_ImportSetColorKey(Fl_Button*, void*)
{
	uchar key_r,key_g,key_b;
	
	if( import_params.target_bpp < 16 )
		return;
	
	key_r = import_params.colorkey&0xff;
	key_g = (import_params.colorkey>>8)&0xff;
	key_b = (import_params.colorkey>>16)&0xff;
	
	if( fl_color_chooser("Color Key", key_r, key_g, key_b, 1) )
	{
		import_ui->colorKeyBox->color(fl_rgb_color(key_r, key_g, key_b));
		import_ui->colorKeyBox->redraw();
		import_params.colorkey = key_r|(key_g<<8)|(key_b<<16);
		
		if( !update_tim() )
		{
			import_item->ctrl->redraw();
		}
	}
	
}

void cb_ImportColorPickCtrl(Fl_Widget*, void*)
{
	int pick_x,pick_y;
	RGBQUAD pix_rgb;
	TIM_PIX_16 pix_tim;
	
	pick_x = (Fl::event_x()-import_item->ctrl->x())/import_item->ctrl->GetZoom();
	pick_y = (Fl::event_y()-import_item->ctrl->y())/import_item->ctrl->GetZoom();
	
	switch(import_item->tim.GetPmode())
	{
		case TimImage::PMODE_4:
		case TimImage::PMODE_8:
			import_params.colorkey = 
				import_item->tim.PixelIndex(pick_x, pick_y)<<24;
			
			pix_tim = import_item->tim.ClutPixel(import_params.colorkey>>24, 0);
			
			pix_rgb.rgbRed		= (pix_tim.r*MUL_FACTOR)>>12;
			pix_rgb.rgbGreen	= (pix_tim.g*MUL_FACTOR)>>12;
			pix_rgb.rgbBlue		= (pix_tim.b*MUL_FACTOR)>>12;
			
			break;
			
		case TimImage::PMODE_16:
		case TimImage::PMODE_24:
			pix_rgb = importer->GetRGBcolor(pick_x, pick_y);
			import_params.colorkey = (
				(pix_rgb.rgbRed)|
				(pix_rgb.rgbGreen<<8)|
				(pix_rgb.rgbBlue<<16));
			
			break;
	}
	
	import_ui->colorKeyBox->color(fl_rgb_color(
		pix_rgb.rgbRed, pix_rgb.rgbGreen, pix_rgb.rgbBlue));
	import_ui->colorKeyBox->redraw();
	
	import_ui->pickButton->value(0);
	import_item->ctrl->callback((Fl_Callback_p)NULL);
	pick_state = 0;
	
	if( !update_tim() )
	{
		import_item->ctrl->redraw();
	}
}

void cb_ImportColorPick(Fl_Button*, void*)
{
	import_item->ctrl->PickMode(1);
	import_item->ctrl->callback(cb_ImportColorPickCtrl);
	pick_state = 1;
}

void cb_ImportOkay(Fl_Return_Button*, void*)
{
	Fl_Native_File_Chooser chooser;
	
	chooser.title("Save Imported TIM");
	chooser.filter("TIM Image\t*.tim\n");

	chooser.type(chooser.BROWSE_SAVE_FILE);
	chooser.options(chooser.SAVEAS_CONFIRM);
	
	if( chooser.show() != 0 )
		return;
	
	import_item->file = chooser.filename();
	
	if (!import_item->file.has_extension()) {
		import_item->file += (".tim");
	}
	
#ifdef DEBUG
	printf("Export save: %s\n", import_item->file.c_str());
#endif /* DEBUG */

	// Set default TIM coords
	import_item->tim.SetPosition(640, 0);
	import_item->tim.SetClutPosition(0, 480);
	
	fl_message_title("Error writing TIM");
	switch( import_item->tim.SaveTim(import_item->file) )
	{
		case TimImage::ERR_NO_IMAGE:
			fl_message("No image to write.");
			return;
		case TimImage::ERR_CANT_WRITE:
			fl_message("Cannot create or write to file.");
			return;
	}
	
	import_ui->preview->remove(import_item->ctrl);
	
	import_item->src_file		= import_image;
	import_item->imported		= 1;
	import_item->import_params	= import_params;
	import_item->group			= ui->groupList->value();
	
	import_created = 1;
	
	RegisterTimItem(import_item);
	import_ui->hide();
	
}

void cb_ImportOkayAlt(Fl_Return_Button*, void*)
{
	reimport_item->tim.Copy(&import_item->tim, 0);
	
	reimport_item->ctrl->redraw();
	
	if( reimport_item->clut_ctrl )
		reimport_item->ctrl->redraw();
	
	reimport_item->import_params = import_params;
	reimport_item->tim.modified = true;
	import_ui->hide();
	
	import_created = 1;
}

void cb_ColorAdjToggle(Fl_Check_Button*, void*)
{
	import_params.color_adjust = import_ui->colorAdjToggle->value();
	
	if( !update_tim() )
		import_item->ctrl->redraw();
}

void cb_GeneralColAdj(Fl_Value_Slider*, long slider)
{
	switch(slider)
	{
		case 0:
			import_params.adj_red = import_ui->generalAdj[0]->value();
			break;
		case 1:
			import_params.adj_green = import_ui->generalAdj[1]->value();
			break;
		case 2:
			import_params.adj_blue = import_ui->generalAdj[2]->value();
			break;
	}
	
	if( import_params.color_adjust )
		if( !update_tim() )
			import_item->ctrl->redraw();
}

void cb_BlendColAdj(Fl_Value_Slider*, long slider)
{
	switch(slider)
	{
		case 0:
			import_params.blend_red = import_ui->blendAdj[0]->value();
			break;
		case 1:
			import_params.blend_grn = import_ui->blendAdj[1]->value();
			break;
		case 2:
			import_params.blend_blu = import_ui->blendAdj[2]->value();
			break;
	}
	
	if( import_params.color_adjust )
		if( !update_tim() )
			import_item->ctrl->redraw();
}

void cb_ImportChanInv(Fl_Check_Button*, void*)
{
	import_params.inv_red = import_ui->invertChan[0]->value();
	import_params.inv_grn = import_ui->invertChan[1]->value();
	import_params.inv_blu = import_ui->invertChan[2]->value();
	
	if( import_params.color_adjust )
		if( !update_tim() )
			import_item->ctrl->redraw();
}
