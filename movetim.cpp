#include <stdio.h>

#include <FL/Fl_Browser.H>
#include <FL/Fl_Return_Button.H>
#include <FL/fl_message.H>

#include "mainui.h"


extern MainUI *ui;
extern void *img_selected;


static Fl_Double_Window *dialog;
static Fl_Choice *group_list;
	
static Fl_Return_Button *ok_button;
static Fl_Button *cancel_button;


TimItem* GetSelectedTimItem();
void SetVisibleGroup(int group);


void cb_MoveTimOkay(Fl_Return_Button*, void*)
{
	TimItem *tim;
	
	tim = GetSelectedTimItem();
	if( !tim )
		return;
	
	tim->group = group_list->value();
	
	SetVisibleGroup(ui->groupList->value());
	
	dialog->hide();
}

void cb_MoveTimCancel(Fl_Button*, void*)
{
	dialog->hide();
}

void cb_MoveTimToGroup(Fl_Menu_*, void*)
{
	if( !img_selected )
	{
		fl_message_title("No TIM Selected");
		fl_message("You must select a TIM you want to move to a group.");
		return;
	}
	
	// Dialog setup
	dialog = new Fl_Double_Window(300, 58, "Move TIM To...");
	group_list = new Fl_Choice(8, 8, 284, 20);
	
	ok_button = new Fl_Return_Button(8, 32, 80, 20, "Ok");
	ok_button->labelsize(12);
	ok_button->callback((Fl_Callback*)cb_MoveTimOkay);
	
	cancel_button = new Fl_Button(96, 32, 80, 20, "Cancel");
	cancel_button->labelsize(12);
	cancel_button->callback((Fl_Callback*)cb_MoveTimCancel);
	
	dialog->set_modal();
	dialog->end();
	
	// Add group items
	group_list->menu(ui->groupList->menu());
	
	// Show dialog
	dialog->show();
	
	while( dialog->shown() )
	{
		Fl::wait();
	}
	
	delete dialog;
}
