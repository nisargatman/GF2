#pragma once

#include <wx/wx.h>
#include <wx/checklst.h>
#include <vector>

#include "names.h"
#include "devices.h"
#include "tab.h"

/// Dialog box for setting switch states
class switchDialog: public wxDialog {
public:
    switchDialog(Tab *parent, wxWindowID id, const wxString &title, const wxString &message, names *
      _nmz, devices *_dmz);
    ~switchDialog();
    wxCheckListBox *listbox;
private:
    void OnOk(wxCommandEvent& WXUNUSED(pEvent));
    void OnListBox(wxCommandEvent &event);
    void OnDClickListBox(wxCommandEvent &event);
    names* nmz;
    devices* dmz;
    Tab *par;
    std::vector<wxString> switchtable;
    std::vector<int> checked;
};
