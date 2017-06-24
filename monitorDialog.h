#pragma once

#include <wx/wx.h>
#include <wx/checklst.h>
#include <vector>

#include "names.h"
#include "devices.h"
#include "tab.h"

/// Dialog box for selecting monitored outputs
class monitorDialog: public wxDialog {
public:
    monitorDialog(Tab *parent, wxWindowID id, const wxString &title, const wxString &message, names *_nmz, devices *_dmz, monitor *_mmz);
    ~monitorDialog();
    wxCheckListBox *listbox;
private:
    void OnOk(wxCommandEvent& WXUNUSED(pEvent));
    void OnListBox(wxCommandEvent &event);
    void OnDClickListBox(wxCommandEvent &event);
    names* nmz;
    devices* dmz;
    monitor* mmz;
    Tab *par;

    std::vector<bool> disp;    
    
};

