#ifndef gui_h
#define gui_h

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/aui/auibook.h>
#include "names.h"
#include "devices.h"
#include "monitor.h"

enum { 
  MY_SPINCNTRL_ID = wxID_HIGHEST + 1,
  MY_TEXTCTRL_ID,
  MY_RUN_SPIN_ID,
  MY_OPENFILE_ID,
  MY_EXPORT_OUTPUT_ID,
  MY_RUN_BUTTON_ID,
  MY_CONT_BUTTON_ID,
  MY_CONT_SPIN_ID,
  MY_SWITCH_BUTTON_ID,
  MY_MONITOR_BUTTON_ID,
  OPEN_TOOLBAR_ID,
  EXPORT_TOOLBAR_ID,
  INFO_TOOLBAR_ID,
  EXIT_TOOLBAR_ID,
  NOTEBOOK_ID,
  MONITOR_LIST_ID,
  MY_TIDEL_BUTTON_ID,
  MY_TIDUP_BUTTON_ID,
  MY_TIDWN_BUTTON_ID
}; // widget identifiers


/// Main wxWidgets frame containing the GUI
class MyFrame: public wxFrame
{
    public:
    MyFrame(wxWindow *parent, const wxString& title, const wxPoint& pos, const wxSize& size, long style = wxDEFAULT_FRAME_STYLE); // constructor
    bool OpenFile(std::string fname);

private:
    wxAuiNotebook* notebook;
    void AddTab(std::string title, std::string tooltip, names *names_mod, devices *devices_mod, monitor *monitor_mod);
    void OnExit(wxCommandEvent& event);     // event handler for exit menu item
    void OnAbout(wxCommandEvent& event);    // event handler for about menu item
    void OnOpenFile(wxCommandEvent& WXUNUSED(event)); // event handler for the open file item
    void OnExportOutput(wxCommandEvent& event);
    void OnChangePage(wxAuiNotebookEvent &event);
    DECLARE_EVENT_TABLE()
};

#endif /* gui_h */
