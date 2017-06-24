#pragma once

#include <vector>
#include <wx/spinctrl.h>
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <fstream>

#include "gui.h"            // wx
#include "trace.h"
#include "names.h"

class Tab;
class TraceDisplay;

/// Scrolled window displaying all the traceItems
class TraceDisplay: public wxScrolledWindow {
public:
    
    ///Constructor and destructor, minimum_height is the height of the canvas
    TraceDisplay(Tab *parent, wxWindowID id, names *names_mod, devices *devices_mod, monitor *monitor_mod, int minimum_Height);
    ~TraceDisplay();
    
    ///Render the canvases
    void Render(int cycles = -1);
    ///Pan the canvases
    void changePanAll(int dx);
    
    ///get wxString vector of possible monitor points' names for displaying in a list
    std::vector<wxString> getMonList();
    ///get wxString vector of traces' names for displaying in a list
    std::vector<wxString> getTraceList();
    ///set monItems private based on checkbox output
    void setMonList(std::vector<bool> list);
    ///get monItem shown value
    bool getMon(int index);
    ///get number of possible monitor points
    int getMonCount();
    
    ///Add a traceItem to the window
    bool AddTrace(name dev, name outp);
    ///Remove traceItem, overloaded
    void RemoveTrace(int index);
    void RemoveTrace(TraceItem* titem);
    void RemoveTrace(name devname, name outp);
    ///Reorder a trace in the display, dist is ditance up.
    void MvTrace(TraceItem* titem, int dist);
private:

    wxBoxSizer *tracelist_sizer;
    
    std::vector<TraceItem*> traceItems;
    
    struct monItem {
      name dev;
      name out; 
      bool shown;
    };
    std::vector<monItem> monItems;
    
    names *nmz;
    devices *dmz;
    monitor *mmz;
    Tab *par;
    
    int minHeight;
    
    void checkremoveMon(int traceindex);
    void addMon(name devname, name outpname);
    void appendTraceItem(int monNum);
    void updateTraces();
};

///Contains all the simulation controls, and a traceDisplay. Contructed for each opened file.
class Tab: public wxWindow {
public:
    Tab(wxWindow *parent, wxWindowID id, names *names_mod = NULL, devices *devices_mod = NULL, monitor *monitor_mod = NULL);
    ~Tab();
    int cyclescompleted;
    
    void Run();
    void ReRun();
    void Continue();
    void ZeroDevices();

    std::vector<wxString> getMonList();
    std::vector<wxString> getTraceList();
    void setMonList(std::vector<bool> list);
    bool getMon(int index);
    int getMonCount();
    
    void ExportOutput();

private:

    TraceDisplay *tdisplay;
    
    wxSpinCtrl *runSpin;
    wxSpinCtrl *continueSpin;

    monitor *mmz;
    names *nmz;
    devices *dmz;
    
    bool debug_mode;

    void runnetwork(int ncycles);
    void continuenetwork(int ncycles);
    
    void OnRunButton(wxCommandEvent& event);
    void OnContButton(wxCommandEvent& event);

    void OnViewCircuit(wxCommandEvent& event);
    void OnMonitorButton(wxCommandEvent& event);
    void OnSwitchButton(wxCommandEvent& event);

    DECLARE_EVENT_TABLE()
};

