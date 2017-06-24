#include "monitorDialog.h"

#include <iostream>
#include <wx/checklst.h>
#include <vector>

using namespace std;

monitorDialog::monitorDialog(Tab *parent, wxWindowID id, const wxString &title, const wxString &message, names *_nmz, devices *_dmz, monitor *_mmz)
: wxDialog(parent, id, title, wxDefaultPosition, wxSize(300, 300), wxCAPTION | wxSYSTEM_MENU) {

    CentreOnParent();

	  nmz = _nmz;
	  dmz = _dmz;
	  mmz = _mmz;
	  par = parent;
	  
    // Create the text control with the message in it
    wxStaticText* text = new wxStaticText(this, wxID_ANY, _("Click to enable/disable:"));

    // Get Monitor list from tab->tracedisplay
    vector<wxString> montable = par->getMonList();
    
    //disp.assign(montable.size(), false);

    listbox = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,montable.size(),&montable[0], 0, wxDefaultValidator, "");
     
    //Populate checkbox
    for (int i=0; i < par->getMonCount(); ++i)
    {
      disp.push_back(par->getMon(i));
      listbox -> Check(i,disp[i]);
    }
    
    Bind(wxEVT_CHECKLISTBOX, &monitorDialog::OnListBox, this, listbox -> GetId());
    Bind(wxEVT_LISTBOX_DCLICK, &monitorDialog::OnDClickListBox,this,listbox->GetId());

    // Create the button
    wxButton *okButton = new wxButton(this, wxID_ANY, _("OK"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &monitorDialog::OnOk, this, okButton->GetId());

    // Apply the sizer
    wxBoxSizer *vBox = new wxBoxSizer(wxVERTICAL);
    vBox->Add(text, 0, wxALL, 15);
    vBox->Add(listbox, 1, wxEXPAND | wxLEFT | wxRIGHT, 15);
    vBox->Add(okButton, 0,  wxALIGN_CENTER | wxALL, 15);

    SetSizer(vBox);
}

monitorDialog::~monitorDialog() {

}

void monitorDialog::OnOk(wxCommandEvent& WXUNUSED(pEvent)) {
    par->setMonList(disp);
    EndModal(wxID_OK);
    Destroy();
}

void monitorDialog::OnListBox(wxCommandEvent &event) {
    disp[event.GetInt()] = !disp[event.GetInt()];
    //cout << event.GetInt() << " " << disp[event.GetInt()] << endl;
}

void monitorDialog::OnDClickListBox(wxCommandEvent &event) {
     OnListBox(event);
     bool status = listbox -> IsChecked(event.GetInt());
     listbox->Check(event.GetInt(), !status);
}





