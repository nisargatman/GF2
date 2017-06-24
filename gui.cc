#include "gui.h"
#include "wx_icon.xpm"
#include "../icons/openIcon.xpm"
#include "../icons/saveIcon.xpm"
#include "../icons/infoIcon.xpm"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <wx/process.h>
#include <wx/artprov.h>
#include <wx/wfstream.h>

#include "parser.h"
#include "scanner.h"
#include "tab.h"
#include "errorDialog.h"
#include "trace.h"

using namespace std;

// MyFrame 

// Event handling
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
  EVT_MENU(MY_OPENFILE_ID, MyFrame::OnOpenFile)
  EVT_MENU(MY_EXPORT_OUTPUT_ID, MyFrame::OnExportOutput)
  EVT_MENU(wxID_EXIT, MyFrame::OnExit)
  EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
  EVT_TOOL(OPEN_TOOLBAR_ID, MyFrame::OnOpenFile)
  EVT_TOOL(EXPORT_TOOLBAR_ID, MyFrame::OnExportOutput)
  EVT_TOOL(INFO_TOOLBAR_ID, MyFrame::OnAbout)
  EVT_TOOL(EXIT_TOOLBAR_ID, MyFrame::OnExit)
  EVT_AUINOTEBOOK_PAGE_CHANGED(NOTEBOOK_ID, MyFrame::OnChangePage)
END_EVENT_TABLE()

  
MyFrame::MyFrame(wxWindow *parent, const wxString& title, const wxPoint& pos, const wxSize& size, long style): wxFrame(parent, wxID_ANY, title, pos, size, style)
{
  SetIcon(wxIcon(wx_icon));

  // create a new menu with the following entries: Open File, Export Output, About, Quit
  wxMenu *fileMenu = new wxMenu;
  fileMenu -> Append(MY_OPENFILE_ID, _("&Open File"));
  fileMenu -> Append(MY_EXPORT_OUTPUT_ID, _("&Export Output"));
  fileMenu->Append(wxID_ABOUT, _("&About"));
  fileMenu->Append(wxID_EXIT, _("&Quit"));

  // append the menus to the menu bar
  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, _("&File"));

  SetMenuBar(menuBar);

  // define a toolbar with custom icons
  wxToolBar *toolBar = CreateToolBar();
  toolBar->AddTool(OPEN_TOOLBAR_ID, _("Open file"), wxBitmap(openIcon), _("Open a new definition file"));
  toolBar->AddTool(EXPORT_TOOLBAR_ID, _("Export output"), wxBitmap(saveIcon), _("Export current output to a file"));
  toolBar->AddTool(INFO_TOOLBAR_ID, _("Info"), wxBitmap(infoIcon), _("Info"));
  toolBar->SetToolBitmapSize(wxSize(24,24));
  toolBar->Realize();

  const wxSize& sz = wxSize(720,450);
  SetMinSize(sz); // set minimum size of the frame

  notebook = new wxAuiNotebook(this, NOTEBOOK_ID); // functionality to add tabs to view new files
                                                   // allows viewing multiple def files simulations in the same overarching window
}

bool MyFrame::OpenFile(std::string fname) {
    // Initialise everything needed for parsing
    names* nmz = new names();
    network* netz = new network(nmz);
    devices* dmz = new devices(nmz, netz);
    monitor* mmz = new monitor(nmz, netz);

    // Initialise the scanner
    Scanner* smz;
    try {
        smz = new Scanner(nmz, fname.c_str());
    }
    catch(exception& e) {
        // Opening the file failed
        wxMessageDialog* msg = new wxMessageDialog(this, e.what(), _("Error"));
        msg->ShowModal();
        return false;
    }

    ostringstream stream;
    Parser* pmz = new Parser(netz, dmz, mmz, smz, stream);

    // Parse
    bool ok = pmz->readin();
    delete pmz;
    delete smz;

    if (ok) {
        // Add a new tab if everything is ok
        string basename = getBasename(fname);
        AddTab(basename, fname, nmz, dmz, mmz);
    }
    else {
        // Display an error
        ErrorDialog* dlg = new ErrorDialog(this, wxID_ANY, _("Could not open file"), stream.str());
        dlg->ShowModal();
    }
    return ok;
}

void MyFrame::AddTab(string title, string tooltip, names *names_mod, devices *devices_mod, monitor *monitor_mod) {

    // Create a new tab
    Tab* newTab = new Tab(notebook, wxID_ANY, names_mod, devices_mod, monitor_mod);
    notebook->AddPage(newTab, title, true);

    // Set the tooltip
    size_t pageIdx = notebook->GetPageIndex(notebook->GetCurrentPage());
    notebook->SetPageToolTip(pageIdx, tooltip);
}

void MyFrame::OnExit(wxCommandEvent &event) // Event handler for the exit menu item
{
  Close(true);
}

void MyFrame::OnAbout(wxCommandEvent &event) // Event handler for the about menu item
{
  wxMessageDialog about(this, _("Logic Circuit Simulator\nTeam 3\nMay 2016"), _("About Logsim"), wxICON_INFORMATION | wxOK);
  about.ShowModal();
}

void MyFrame::OnOpenFile(wxCommandEvent &WXUNUSED(event)) // Open a new simulation from a different def file in a new tab
{
    wxFileDialog openFileDialog(this, _("Open .txt file"), "", "",wxFileSelectorDefaultWildcardStr, wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL) return; // the user changed their mind
    // Get the filename to open
    wxFileInputStream input_stream(openFileDialog.GetPath());
    string fname = string(openFileDialog.GetPath().mb_str());
    OpenFile(fname);
}

void MyFrame::OnExportOutput(wxCommandEvent &event)
{
  // get the current tab. Although Export Output has been defined in the frame for display, it should belong to the tab class.
  // So here we get the current tab and call a function from the tab class to do the heavy lifting
  Tab* current = dynamic_cast<Tab*>(notebook->GetCurrentPage());
  if (!current) {
    wxMessageDialog* msg = new wxMessageDialog(this, _("Could not export data because no files are open."), _("Error")); // cannot export any output as simulation has not been run!
    msg->ShowModal();
  }
  else
    current -> ExportOutput();
}

void MyFrame::OnChangePage(wxAuiNotebookEvent &event)
{
    notebook->ChangeSelection(event.GetSelection());
}
