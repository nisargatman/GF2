#pragma once

#include <wx/glcanvas.h>
#include <wx/radiobut.h>

#include "names.h"
#include "devices.h"
#include "monitor.h"
#include "gui.h"            // wx

class TraceItem;
class TraceDisplay;

/// Canvas uset to draw a single trace using the monitor number from the parent TraceItem
class MyGLCanvas: public wxGLCanvas
{
 public:
  MyGLCanvas(TraceItem *parent, wxWindowID id = wxID_ANY, monitor* monitor_mod = NULL, names* names_mod = NULL,
	     const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0,
	     const wxString& name = "MyGLCanvas", const wxPalette &palette=wxNullPalette); ///< constructor
  void Render(int signaldisplayed, int cycles = -1); ///< function to draw canvas contents
  void Render();
  void changePan(int dx);
 private:
  wxGLContext *context;              ///< OpenGL rendering context
  bool init;                         ///< has the OpenGL context been initialised?
  int pan_x;                         ///< the current x pan
  double zoom;                       ///< the current zoom
  int signal;                        ///< which signal is being displayed
  int cyclesdisplayed;               ///< how many simulation cycles have been displayed
  float cycleWidth;
  monitor *mmz;                      ///< pointer to monitor class, used to extract signal traces
  names *nmz;                        ///< pointer to names class, used to extract signal names
  TraceItem* par;

  void InitGL();                     ///< function to initialise OpenGL context
  void OnSize(wxSizeEvent& event);   ///< event handler for when canvas is resized
  void OnPaint(wxPaintEvent& event); ///< event handler for when canvas is exposed
  void OnMouse(wxMouseEvent& event); ///< event handler for when mouse is exposed
  
  DECLARE_EVENT_TABLE()
  
};

/// Contains pointers to a canvas, some buttons, labels and sizer all used to display one signal output and control it
class TraceItem: public wxWindow {
    friend TraceDisplay;
public:
    ///constructor, takes usual + monitor index and minimum height
    TraceItem(TraceDisplay *parent, wxWindowID id, names *names_mod, devices *devices_mod, monitor *monitor_mod, int monnum, int minHeight);
    ~TraceItem();
    ///Render contained canvas
    void Render(int cycles);
    ///Recalculate monnum. Necessary if a monitor is removed
    void UpdateMon();
    ///Getstring representing device
    wxString getStr();
    
    name mondev;
    name monout;
    TraceDisplay *par;
    
private:

    wxBoxSizer *trace_sizer;
    wxBoxSizer *tag_sizer;

    MyGLCanvas *canvas;
    
    names *nmz;
    devices *dmz;
    monitor *mmz;

    int monitorNum;
    
    void OnDelButton(wxCommandEvent &event);
    void OnDwnButton(wxCommandEvent &event);
    void OnUpButton(wxCommandEvent &event);
    
    DECLARE_EVENT_TABLE()
    
};
