#include <iostream>
#include "trace.h"
#include <math.h>

#include "tab.h"              // TraceDisplay

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

using namespace std;


// MyGLCanvas ////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MyGLCanvas, wxGLCanvas)
  EVT_SIZE(MyGLCanvas::OnSize)
  EVT_PAINT(MyGLCanvas::OnPaint)
  EVT_MOTION(MyGLCanvas::OnMouse)
  EVT_LEFT_DOWN(MyGLCanvas::OnMouse)
END_EVENT_TABLE()
  
int wxglcanvas_attrib_list[5] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

MyGLCanvas::MyGLCanvas(TraceItem *parent, wxWindowID id, monitor* monitor_mod, names* names_mod, const wxPoint& pos, 
		       const wxSize& size, long style, const wxString& name, const wxPalette& palette):
  wxGLCanvas(parent, id, wxglcanvas_attrib_list, pos, size, style, name, palette)
  // Constructor - initialises private variables
{
  context = new wxGLContext(this);
  mmz = monitor_mod;
  nmz = names_mod;
  par = parent;
  init = false;
  pan_x = 0;
  zoom = 1.0;
  cyclesdisplayed = -1;
  cycleWidth = 30.0;
}

void MyGLCanvas::changePan(int dx) {
    pan_x += dx;
    if (pan_x > 0)
        pan_x = 0;

    int w, h;
    GetClientSize(&w, &h);

    int sigWidth = cyclesdisplayed * cycleWidth;
    int delta = w - sigWidth - pan_x - 20;

    if (delta > 0) {
        if (sigWidth > w)
            pan_x = w - sigWidth - 20;
        else
            pan_x = 0;
    }
    init = false;

}

void MyGLCanvas::OnMouse(wxMouseEvent& event)
  // Event handler for mouse events inside the GL canvas
{
    static int last_x;
    int dx;

    if (event.ButtonDown()) {
        last_x = event.m_x;
    }
    if (event.Dragging()) {
        dx = event.m_x - last_x;
        last_x = event.m_x;
        par->par->changePanAll(dx);
    }
    par->par->GetEventHandler()->ProcessEvent(event);
}

void MyGLCanvas::Render() {
    Render(signal, -1);
}

void MyGLCanvas::Render(int signaldisplayed, int cycles)
  // Draws canvas contents - the following example writes the string "example text" onto the canvas
  // and draws a signal trace. The trace is artificial if the simulator has not yet been run.
  // When the simulator is run, the number of cycles is passed as a parameter and the first monitor
  // trace is displayed.
{

  signal = signaldisplayed;
  float y;
  unsigned int i;
  asignal s;

  if (cycles >= 0) cyclesdisplayed = cycles;

  SetCurrent(*context);
  if (!init) {
    InitGL();
    init = true;
  }
  glClear(GL_COLOR_BUFFER_BIT);

  int n = mmz->moncount();
  float lo = 15;
  float hi = this->GetSize().y - 15; // get current size of screen and size signal trace accordingly

    wxString cyclenum; // temporary string used to display characters on canvas for axes display
    // 
    glColor3f(0.6,0.6,0.6); // set color to grey
    glLineWidth(0.1); // make sure the axes is thin
    glBegin(GL_LINE_STRIP);
    for (int k = 0; k<cyclesdisplayed; k++)
    {
       glVertex2f(cycleWidth*k + 10.0 , lo);
       glVertex2f(cycleWidth*k + 10 + cycleWidth , lo); // draw a large line equal to the size of a cycle with some scaling as the horizontal axis
       glVertex2f(cycleWidth*k + 10 + cycleWidth , 1.3*lo); // draw a small marker on the axis  
    }
     glEnd();

    for (int k = 0;k<cyclesdisplayed;k++)
    {
       if (remainder(k,10) == 0.0) // change to display the last cycle run
       {
	       glRasterPos2f(cycleWidth*k + 5, 0.1*lo);
	       cyclenum = to_string(k);
	       for (int a = 0;a<cyclenum.Len(); a++){
	       glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, cyclenum[a]);}
        }
        else if (k == cyclesdisplayed-1)
        {
              glRasterPos2f(cycleWidth*k + 5 + cycleWidth, 0.1*lo); // offset the last display on axis to show last number as multiple of 10
	      cyclenum = to_string(k+1);
	      for (int a = 0;a<cyclenum.Len(); a++){
	      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, cyclenum[a]);}
        }
     }
  
  if ((cyclesdisplayed >= 0) && (signal >= 0) && (signal < n)) { // draw the appropriately sized signal on canvas
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(2.0);
    glBegin(GL_LINE_STRIP);
    for (i = 0; i < cyclesdisplayed; i++) {
      if (mmz->getsignaltrace(signal, i, s)) {
        if (s == low) y = lo;
        if (s == high) y = hi;
        glVertex2f(cycleWidth*i + 10.0, y); 
        glVertex2f(cycleWidth*i + 10.0 + cycleWidth, y);
      }
    }
    glEnd();
    
  }
  
  /*/ Debug text on the canvas
  wxString example_text(to_string(GetId()));
  cout<<"id: "<<GetId()<< ",  signaldisplayed: "<<signaldisplayed<<endl;
  glColor3f(0.0, 0.0, 1.0);
  glRasterPos2f(10, 2);
  for (i = 0; i < example_text.Len(); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, example_text[i]);
  */

  glFlush();
  SwapBuffers();
}

void MyGLCanvas::InitGL()
  // Function to initialise the GL context
{
  int w, h;

  GetClientSize(&w, &h);
  SetCurrent(*context);
  glDrawBuffer(GL_BACK);
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glViewport(0, 0, (GLint) w, (GLint) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, w, 0, h, -1, 1); 
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslated(pan_x, 0.0, 0.0);
  glScaled(zoom, 1, 1);
}

void MyGLCanvas::OnPaint(wxPaintEvent& event)
  // Event handler for when the canvas is exposed
{
  int w, h;

  wxPaintDC dc(this); // required for correct refreshing under MS windows
  GetClientSize(&w, &h);
  Render(signal);
}

void MyGLCanvas::OnSize(wxSizeEvent& event)
  // Event handler for when the canvas is resized
{
  init = false;; // this will force the viewport and projection matrices to be reconfigured on the next paint
  changePan(0);
}


// TraceItem ////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(TraceItem, wxWindow)
  EVT_BUTTON(MY_TIDEL_BUTTON_ID, TraceItem::OnDelButton)
  EVT_BUTTON(MY_TIDUP_BUTTON_ID, TraceItem::OnUpButton)
  EVT_BUTTON(MY_TIDWN_BUTTON_ID, TraceItem::OnDwnButton)
END_EVENT_TABLE()

TraceItem::TraceItem(TraceDisplay *parent, wxWindowID id, names *names_mod, devices *devices_mod, monitor *monitor_mod, int monnum, int minHeight)
:wxWindow(parent, id, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER) {
      
    nmz = names_mod;
    dmz = devices_mod;
    mmz = monitor_mod;
    par = parent;
    monitorNum=monnum;
    
    int width = 90;
    trace_sizer = new wxBoxSizer(wxHORIZONTAL);
    tag_sizer = new wxBoxSizer(wxVERTICAL);
    tag_sizer->SetMinSize(width, 30);
    wxBoxSizer *tagbutton_sizer = new wxBoxSizer(wxHORIZONTAL);
    
    // add a 'x' button to remove the display from the frame
    tagbutton_sizer -> Add(new wxButton(this, MY_TIDEL_BUTTON_ID, "x", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), 0, wxALIGN_LEFT | wxALL, 1);
    tagbutton_sizer -> Add(new wxButton(this, MY_TIDUP_BUTTON_ID, wxString::FromUTF8("↑"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), 0, wxALIGN_LEFT | wxALL, 1);
    tagbutton_sizer -> Add(new wxButton(this, MY_TIDWN_BUTTON_ID, wxString::FromUTF8("↓"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), 0, wxALIGN_LEFT | wxALL, 1);
        
    tag_sizer -> Add(tagbutton_sizer, 0, wxALIGN_LEFT | wxALL, 1);

    // Add the label with the name of the device
    mmz -> getmonname(monnum, mondev, monout);
    wxStaticText* label = new wxStaticText(this, wxID_ANY, getStr(), wxDefaultPosition, wxSize(width, 20), wxST_ELLIPSIZE_END);
    wxFont font = parent->GetFont();
    label->SetFont(font.Bold());
    tag_sizer -> Add(label, 1, wxALL, 5);

    // Add the label with the device type
    string kind = dmz->getDeviceType(mondev);
    wxStaticText* kindLabel = new wxStaticText(this, wxID_ANY, "(" + kind + ")", wxDefaultPosition, wxSize(width, 20), wxST_ELLIPSIZE_END);
    kindLabel->SetFont(font.Smaller());
    tag_sizer -> Add(kindLabel, 1, wxALL, 5);
    //tag_sizer -> Add(new wxRadioButton(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,0,wxDefaultValidator,""));
    
    trace_sizer-> Add(tag_sizer, 0,  wxALL, 0);
    
    canvas = new MyGLCanvas(this, wxID_ANY, mmz, nmz);
    
    trace_sizer -> Add(canvas, 1, wxEXPAND | wxALL, 7) -> SetMinSize(10,minHeight);
    
    this -> SetBackgroundColour(wxColour(220, 220, 220));
    this -> SetSizer(trace_sizer);
    SetMaxSize(wxSize(-1, 120));
}

TraceItem::~TraceItem() {
}

void TraceItem::Render(int cycles) {
  canvas -> Render(monitorNum, cycles);
}

void TraceItem::UpdateMon() {
    int const n = mmz->moncount();
    for(int j=0; j<n; j++) // loop over the number of signals to find new monnum
    {
        name dev;
        name out;
        mmz -> getmonname(j, dev, out);
        if (mondev==dev && monout==out)
        {
            monitorNum=j;
        }
    }
}

wxString TraceItem::getStr() {
  wxString s;
  s += wxString(nmz->getName(mondev));
  if (monout != blankname)
  {
    s += ".";
    s += wxString(nmz->getName(monout));
  }
  return s;
}

void TraceItem::OnDelButton(wxCommandEvent &event) {
  par->RemoveTrace(this);
}

void TraceItem::OnUpButton(wxCommandEvent &event) {
  par->MvTrace(this,-1);
}

void TraceItem::OnDwnButton(wxCommandEvent &event) {
  par->MvTrace(this,1);
}


