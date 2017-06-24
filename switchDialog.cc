#include "switchDialog.h"

#include <iostream>
#include <wx/checklst.h>
#include <vector>

using namespace std;

switchDialog::switchDialog(Tab *parent, wxWindowID id, const wxString &title, const wxString &message, names *_nmz, devices *_dmz)
: wxDialog(parent, id, title, wxDefaultPosition, wxSize(300, 300), wxCAPTION | wxSYSTEM_MENU) {

    CentreOnParent();

    nmz = _nmz;
    dmz = _dmz;
    par = parent;
    
    // Create the text control with the message in it
    wxStaticText* text = new wxStaticText(this, wxID_ANY, _("Click to turn on/off:"));

    devlink devlist = dmz -> getDeviceList();
    
    if (devlist != NULL){
        do{
            if (devlist -> kind == aswitch) 
	          { 
			if (devlist -> swstate == high) checked.push_back(1);
			else checked.push_back(0);
		         wxString s;
		         s += nmz -> getName(devlist->id);
		         switchtable.push_back(s);
            }
          devlist = devlist -> next;
        } while(devlist != NULL);
    }

    listbox = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, switchtable.size(),&switchtable[0], 0, wxDefaultValidator, "");
    for(int x = 0; x< switchtable.size(); x++)
    {
	if (checked[x] == 1) listbox -> Check(x,true);
     }
     Bind(wxEVT_CHECKLISTBOX, &switchDialog::OnListBox, this, listbox -> GetId());
     Bind(wxEVT_LISTBOX_DCLICK, &switchDialog::OnDClickListBox,this,listbox->GetId());
    

    // Create the button
    wxButton *okButton = new wxButton(this, wxID_ANY, _("OK"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &switchDialog::OnOk, 
            this, okButton->GetId());

    // Apply the sizer
    wxBoxSizer *vBox = new wxBoxSizer(wxVERTICAL);
    vBox->Add(text, 0, wxALL, 15);
    vBox->Add(listbox, 1, wxEXPAND | wxLEFT | wxRIGHT, 15);
    vBox->Add(okButton, 0,  wxALIGN_CENTER | wxALL, 15);

    SetSizer(vBox);
}

switchDialog::~switchDialog() {

}

void switchDialog::OnOk(wxCommandEvent& WXUNUSED(pEvent)) {
    EndModal(wxID_OK); 
    Destroy();
}

void switchDialog::OnListBox(wxCommandEvent &event) {
   bool ok;
   int n = event.GetInt();
   name sid = nmz -> cvtname(switchtable[n].ToStdString());
   if(checked[n] == 1)
   {
      dmz -> setswitch(sid,low,ok);   
   }
   else if(checked[n] == 0)
   {
     dmz -> setswitch(sid,high,ok);
   }
}

void switchDialog::OnDClickListBox(wxCommandEvent &event)
{  // need to change to do the more natural thing
   OnListBox(event);
   bool status = listbox -> IsChecked(event.GetInt());
   listbox->Check(event.GetInt(), !status);
}
