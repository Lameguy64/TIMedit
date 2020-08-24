// Created on September 26, 2019, 11:58 AM

#include <Fl/fl_message.H>
#include "mainui.h"
#include "paletteui.h"
#include "TimImage.h"

#define MUL_FACTOR	33694

extern MainUI *ui;

static PaletteUI *palette_ui;
static TimImage *edit_image;
static Fl_TimImage *img_prev;

extern void *img_selected;

static void center_preview()
{
	img_prev->position(
		((palette_ui->preview->x()+(palette_ui->preview->w()>>1))
		- (img_prev->w()>>1)), 
		((palette_ui->preview->y()+(palette_ui->preview->h()>>1))
		- (img_prev->h()>>1)));
}

static void update_edit()
{
	TIM_PIX_16 col = edit_image->ClutPixel(palette_ui->palette->selected(), 
		palette_ui->palette->GetEditClut());
	
	palette_ui->redSlider->value(col.r);
	palette_ui->greenSlider->value(col.g);
	palette_ui->blueSlider->value(col.b);
	palette_ui->stpToggle->value(col.i);

	palette_ui->colorBox->color(fl_rgb_color(
		(col.r*MUL_FACTOR)>>12,
		(col.g*MUL_FACTOR)>>12,
		(col.b*MUL_FACTOR)>>12));
	
	palette_ui->colorBox->redraw();
}

void cb_ClutOkay(Fl_Return_Button *w, void *u) {
	
	TimImage *edit = ((Fl_TimImage*)img_selected)->GetImage();
	
	edit->Copy(edit_image);
	edit->modified = 1;
	
	ui->vramArea->redraw();
	
	palette_ui->hide();
	
}

void cb_ClutCancel(Fl_Button *w, void *u) {
	
	palette_ui->hide();
	
}

void cb_ClutPickMode(Fl_Button *w, void *u)
{
	
	img_prev->PickMode(w->value());
	
}

void cb_ClutRevert(Fl_Button *w, void *u)
{	
	TimImage *edit = ((Fl_TimImage*)img_selected)->GetImage();
	
	edit_image->Copy(edit);
	
	palette_ui->clutIndex->maximum(edit_image->GetNumCluts()-1);
	palette_ui->clutIndex->value(0);
	
	palette_ui->palette->SetEditClut(0);
	img_prev->SetDrawClut(0);
	
	palette_ui->palette->redraw();
	palette_ui->preview->redraw();
}

void cb_ClutSliderChange(Fl_Value_Slider *w, void *u)
{
	int i = palette_ui->palette->selected();
	
	palette_ui->colorBox->color(fl_rgb_color(
		(((int)palette_ui->redSlider->value())*MUL_FACTOR)>>12,
		(((int)palette_ui->greenSlider->value())*MUL_FACTOR)>>12,
		(((int)palette_ui->blueSlider->value())*MUL_FACTOR)>>12));
	
	palette_ui->colorBox->redraw();
	
	edit_image->SetClutPixel(i, palette_ui->palette->GetEditClut(), 
		palette_ui->redSlider->value(),
		palette_ui->greenSlider->value(),
		palette_ui->blueSlider->value(),
		palette_ui->stpToggle->value());
	
	palette_ui->palette->redraw();
	img_prev->redraw();
}

void cb_ClutSlotValue(Fl_Spinner *w, void *u)
{
	int i = w->value();
	palette_ui->clutIndex->value(i);
	palette_ui->palette->SetEditClut(i);
	img_prev->SetDrawClut(i);
	img_prev->redraw();
	
	update_edit();
}

void cb_ClutZoom(Fl_Spinner *w, void *u)
{
	int z = w->value();
	
	img_prev->SetZoom(z);
	center_preview();
	
	palette_ui->preview->redraw();
}

void cb_ClutNewSlot(Fl_Button *w, void *u)
{
	int i = edit_image->AddClutSlot(1);
	
	palette_ui->clutIndex->maximum(edit_image->GetNumCluts()-1);
	palette_ui->clutIndex->value(i);
	palette_ui->palette->SetEditClut(i);
	img_prev->SetDrawClut(i);
	img_prev->redraw();
}

void cb_ClutDeleteSlot(Fl_Button *w, void *u)
{
	if( palette_ui->clutIndex->value() < 1 )
	{
		fl_message_title("Cannot delete CLUT");
		fl_message("Last CLUT cannot be deleted.");
		return;
	}
	
	int i,cw,ch;
	
	edit_image->DeleteClut(palette_ui->clutIndex->value());
	
	edit_image->GetClutDimensions(cw, ch);
	
	if( palette_ui->clutIndex->value() >= ch )
	{
		palette_ui->clutIndex->value(ch-1);
	}
	
	palette_ui->clutIndex->maximum(ch-1);
	i = palette_ui->clutIndex->value();
	palette_ui->palette->SetEditClut(i);
	img_prev->SetDrawClut(i);
	img_prev->redraw();
	
	update_edit();
}

void cb_ClutPrevDrawMode(Fl_Menu_ *w, long u)
{
	if( u < 0 )
	{
		img_prev->SetBlend(0);
		return;
	}
	
	img_prev->SetBlend(1);
	img_prev->SetBlendMode(u);
}

void cb_ClutStpToggle(Fl_Check_Button *w, void *u)
{
	int i = palette_ui->palette->selected();
	
	edit_image->SetClutPixel(i, palette_ui->palette->GetEditClut(), 
		palette_ui->redSlider->value(),
		palette_ui->greenSlider->value(),
		palette_ui->blueSlider->value(),
		palette_ui->stpToggle->value());
	
	palette_ui->palette->redraw();
	img_prev->redraw();
}

void cb_PaletteClick(Fl_ClutPalette *w, void *u)
{
	TIM_PIX_16 col = edit_image->ClutPixel(w->selected(), w->GetEditClut());
	int i = palette_ui->palette->selected();
	
	if( Fl::event_clicks() )
	{
		col.i = !col.i;
		edit_image->SetClutPixel(i, palette_ui->palette->GetEditClut(), 
			col.r, col.g, col.b, col.i);
		img_prev->redraw();
	}
	
	update_edit();
}

void cb_ClutPreview(Fl_TimImage *w, void *u)
{
	if( palette_ui->pickToggle->value() )
	{
		palette_ui->palette->SetIndex(img_prev->GetLastPickedIndex());
		update_edit();
		palette_ui->pickToggle->clear();
	}
}

void cb_EditClut(Fl_Menu_ *w, void *u)
{	
	if( !img_selected )
	{
		fl_message_title("Cannot edit CLUT");
		fl_message("You need to select a TIM with CLUT first.");
		return;
	}
	
#ifdef DEBUG
	printf("Image selected=%p\n", img_selected);
#endif /* DEBUG */

	TimImage *edit = ((Fl_TimImage*)img_selected)->GetImage();
	
	if( !edit )
	{
		fl_message_title("Cannot edit CLUT");
		fl_message("You need to select a TIM with CLUT first.");
		return;
	}
	
	if( !edit->HasClut() )
	{
		fl_message_title("Cannot edit CLUT");
		fl_message("Selected TIM has no CLUT data.");
		return;
	}
	
	// Copy image for editing
	edit_image = new TimImage();
	edit_image->Copy(edit);
	
	// Create UI
	palette_ui = new PaletteUI();
	
	cb_PaletteClick(palette_ui->palette, NULL);
	
	palette_ui->palette->SetImage(edit_image);
	palette_ui->clutIndex->maximum(edit_image->GetNumCluts()-1);
	
	// Create preview
	int tw,th;
	edit_image->GetDimensions(tw, th);
	
	img_prev = new Fl_TimImage(
		palette_ui->preview->x(), palette_ui->preview->y(), 
		64, 64);
	
	img_prev->NormalDraw(1);
	img_prev->NoBorderClip(1);
	img_prev->SetImage(edit_image);
	img_prev->callback((Fl_Callback_p)cb_ClutPreview);
	palette_ui->preview->add(img_prev);	
	
	center_preview();
	
	palette_ui->show();
	
	while(palette_ui->shown())
	{
		Fl::wait();
	}
	
	palette_ui->preview->remove(img_prev);
	Fl::wait();
	
	delete palette_ui;
	delete edit_image;
	delete img_prev;
}
