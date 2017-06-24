#include <iostream>
#include <cstdlib>      // std::exit
#include <algorithm>    // std::find
#include <vector>
#include <ctime>
#include <string>
#include "tab.h"
#include "switchDialog.h"
#include "monitorDialog.h"

using namespace std;


// Tab ////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(Tab, wxWindow) // Event table for events inside the tab
  EVT_BUTTON(MY_RUN_BUTTON_ID, Tab::OnRunButton)
  EVT_BUTTON(MY_CONT_BUTTON_ID, Tab::OnContButton)
  EVT_BUTTON(MY_SWITCH_BUTTON_ID, Tab::OnSwitchButton)
  EVT_BUTTON(MY_MONITOR_BUTTON_ID,Tab::OnMonitorButton)
END_EVENT_TABLE()

Tab::Tab(wxWindow *parent, wxWindowID id, names *names_mod, devices *devices_mod, monitor *monitor_mod)
:wxWindow(parent, id) {

    debug_mode=false;
    cyclescompleted = 0;
    nmz = names_mod;
    dmz = devices_mod;
    mmz = monitor_mod;
    if (nmz == NULL || dmz == NULL || mmz == NULL) {
        cout << _("Cannot operate GUI without names, devices and monitor classes") << endl;
        exit(1);
    }
    if (debug_mode)
      dmz -> debug(true);
    
    int const n = mmz->moncount(); // variable to store the number of signals being monitored.
                                   // Needed to draw canvases
    
    //Main Sizer
    wxBoxSizer *top_sizer = new wxBoxSizer(wxVERTICAL);
    
    tdisplay = new TraceDisplay(this, wxID_ANY, nmz, dmz, mmz, 60);
    
    top_sizer->Add(tdisplay, 1, wxEXPAND | wxALL, 0);

    wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);

    // Create the spinners
    runSpin = new wxSpinCtrl(this, MY_RUN_SPIN_ID, _("10"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT); // Spinner next to run button
    continueSpin = new wxSpinCtrl(this, MY_CONT_SPIN_ID, _("10"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT); // Spinner next to continue button
    runSpin->SetMaxSize(wxSize(120, -1));
    continueSpin->SetMaxSize(wxSize(120, -1)); // Set the sizes of the spinners
    runSpin->SetRange(0, maxcycles);
    continueSpin->SetRange(0, maxcycles); // Set the range of the spinners

    // Add the spinners to the sizers
    button_sizer -> Add(runSpin, 0, wxTOP | wxBOTTOM, 5);
    button_sizer -> Add(new wxButton(this, MY_RUN_BUTTON_ID, _("Run"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER), 0, wxTOP | wxBOTTOM, 5);
    button_sizer -> AddSpacer(35);
    button_sizer -> Add(continueSpin, 0, wxTOP | wxBOTTOM, 5);
    button_sizer -> Add(new wxButton(this, MY_CONT_BUTTON_ID, _("Continue"), wxDefaultPosition, wxDefaultSize), 0, wxTOP | wxBOTTOM, 5);

    button_sizer->AddSpacer(35);

    // Add the buttons
    button_sizer -> Add(new wxButton(this, MY_MONITOR_BUTTON_ID, _("Monitors"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER), 0, wxALIGN_RIGHT | wxALL, 5);
    button_sizer -> Add(new wxButton(this, MY_SWITCH_BUTTON_ID, _("Switches"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER), 0, wxALIGN_RIGHT | wxALL, 5);
  
    top_sizer->Add(button_sizer, 0, wxALIGN_CENTER);

    this->SetSizeHints(400,400);
    this->SetSizer(top_sizer);
}

Tab::~Tab() { // destructor for the Tab
    delete nmz;
    delete mmz;
    delete dmz;
}

void Tab::OnRunButton(wxCommandEvent &event) {
    Run();
}

void Tab::OnContButton(wxCommandEvent &event) {
    Continue();
}

void Tab::OnMonitorButton(wxCommandEvent &event)
{
   // Initialise a new dialog box that contains a list of monitor points: Instance of class monitorDialog
   monitorDialog *dialogbox = new monitorDialog(this,wxID_ANY, _("Monitor Points"), "", nmz,dmz,mmz);
   dialogbox -> ShowModal();
}

void Tab::OnSwitchButton(wxCommandEvent &event)
{
    // Initialise a new dialog box that contains a list of switches: Instance of class switchDialog
    switchDialog *dial = new switchDialog(this, wxID_ANY,_("Switches"), "", nmz,dmz);
    dial -> ShowModal();
}

void Tab::ZeroDevices() {
    // Resets the clocks, d-types, siggens and all outputs to 0
    // makes running a given circuit fully deterministic
    devlink devlist = dmz -> getDeviceList();
    
    if (devlist != NULL){
        do{
            devlist->olist->sig = low;
            if (devlist -> kind == aclock) { 
                devlist->counter = 0;
            }
            else if (devlist->kind == dtype) { 
                devlist->memory = low;
                devlist->counter = -1;
                devlist->olist->next->sig = high;  //QBAR
            }
            else if (devlist->kind == asiggen) {
                devlist->counter = 0;
            }
          devlist = devlist -> next;
        } while(devlist != NULL);
    }
}

void Tab::Run() // Run the simulation based on variables and settings set so far
{
    cyclescompleted = 0;
    mmz->resetmonitor();
    ZeroDevices();
    runnetwork(runSpin->GetValue());
    tdisplay -> Render(cyclescompleted);
}

void Tab::ReRun() // Re run the simulation based on variables and settings set so far
{ 
    int temp = cyclescompleted;
    cyclescompleted = 0;
    mmz->resetmonitor();
    ZeroDevices();
    runnetwork(temp);
    tdisplay -> Render(cyclescompleted);
}

void Tab::Continue() // Continue the simulation based on variables set so far after running the simulation
{
    continuenetwork(continueSpin->GetValue());
    tdisplay -> Render(cyclescompleted);
}

void Tab::ExportOutput()
{
	wxString directory = ""; // string used to get directory

	if (cyclescompleted == 0) // ask user if they want to export empty contents to the file
	{
	  wxMessageDialog *mdial = new wxMessageDialog(this,wxString(_("No cycles displayed. Simulation has not been run. Are you sure you want to export?")), wxString(""), wxYES_NO ,wxDefaultPosition);
	  if (mdial -> ShowModal() == wxID_NO) return;
	}
	  
	  wxDirDialog *dirdlg = new wxDirDialog(this,wxString(_("Choose directory")), wxEmptyString, wxDD_DEFAULT_STYLE,wxDefaultPosition, wxDefaultSize,wxString(""));
	  if (dirdlg->ShowModal() == wxID_CANCEL) return;
	  else {directory = dirdlg->GetPath();} // ask the user to choose a directory to save the output file
	  
	  time_t currentTime; // find date and time to name files when saving
	  struct tm *localTime;
	  time(&currentTime);
	  localTime = localtime( &currentTime );
	  int day = localTime -> tm_mday;
	  int month = localTime -> tm_mon + 1;
	  int year = localTime -> tm_year + 1900;
	  int hour = localTime -> tm_hour;
	  int min = localTime -> tm_min;
	  int sec = localTime -> tm_sec; 

	  string fname = directory.ToStdString()+"/logsim_"+to_string(day) + "_" + to_string(month) + "_" + to_string(month) + "_" + to_string(year) + "_" + to_string(hour) + "_" + to_string(min) + "_" + to_string(sec) + ".txt"; // create file name by appending all date and time information
	  ofstream output;
	  output.open(fname); // open a file in the selected directory 
	  
	if (!output.good()) {
	     wxMessageDialog* fail = new wxMessageDialog(this, _("Output export not successful"), _("Possible error")); //error in exporting to file
	     fail->ShowModal();
	     return;
	  } // display this message if the operation to open file was not successful
            // can assume that if file opened correctly then writing to it is succesful

	  float y; // variable to define 0s and 1s to be written
	  unsigned int i; // variable to loop over the current signal being read
	  asignal s; // variable of type signal that contains the signal trace to be read
	 
	  name mondev;
	  name monout; // variables defined to get name of monitor
	  int n = mmz->moncount(); // variable to store the number of monitors
	  string temp;
	  for (int j=0;j<n;j++) // populate output from monitor class
	  {
	    if ((cyclescompleted >= 0) && (j >= 0)) {
	       mmz -> getmonname(j, mondev, monout);
	       temp = nmz -> getName(mondev);
	       output << temp << " : "; // write monitored device name to file
	       for (i=0; i<cyclescompleted; i++) {
		  if (mmz->getsignaltrace(j, i, s)) {
		    if (s==low) y = 0;
		    if (s==high) y = 1;
		    output << y; // write to opened file
		   }
		}

	     }
	   output << endl;
	   output << endl; // write new lines to enhance presentation
	  }
	  output.close();
	  wxMessageDialog* success = new wxMessageDialog(this, _("Output Successfully Exported"), _("OK")); //error in exporting to file
	  success->ShowModal(); // if it has got this far then operation was succesful: notify user of the same
}

void Tab::runnetwork(int ncycles)
  // Function to run the network, derived from corresponding function in userint.cc
{
    bool ok = true;
    int n = ncycles;

    while ((n > 0) && ok) {
        dmz->executedevices(ok);
        if (ok) {
            n--;
            mmz->recordsignals();
        }
        else {
            wxMessageDialog* msg = new wxMessageDialog(this, _("The network is oscillating"), _("Error"));
            msg->ShowModal();
        }
    }
    if (ok)
        cyclescompleted = cyclescompleted + ncycles;
    else
        cyclescompleted = 0;
}

void Tab::continuenetwork(int ncycles) {
    if (cyclescompleted > 0 && ncycles > 0) {
        if ((ncycles + cyclescompleted) > maxcycles) {
            ncycles = maxcycles - cyclescompleted;
            wxMessageDialog* msg = new wxMessageDialog(this, _("Maximum allowable number of cycles was reached (") + to_string(maxcycles) + _("). Re-run the simulation."), _("Error"));
            msg->ShowModal();
        }
        if (ncycles > 0)
            runnetwork(ncycles);
    }
    else {
        wxMessageDialog* msg = new wxMessageDialog(this, _("Nothing to continue!"), _("Error"));
        msg->ShowModal();
    }
}

vector<wxString> Tab::getMonList()
{
    return tdisplay->getMonList();
}

vector<wxString> Tab::getTraceList()
{
    return tdisplay->getTraceList();
}

void Tab::setMonList(std::vector<bool> l)
{
    tdisplay->setMonList(l);
}

bool Tab::getMon(int index) {
    return tdisplay->getMon(index);
}

int Tab::getMonCount() {
    return tdisplay->getMonCount();
}


// TraceDisplay ////////////////////////////////////////////////////////////////////////////////////

TraceDisplay::TraceDisplay(Tab *parent, wxWindowID id, names *names_mod, devices *devices_mod, monitor *monitor_mod, int minimum_Height)
:wxScrolledWindow(parent, id) {

    nmz = names_mod;
    dmz = devices_mod;
    mmz = monitor_mod;
    par = parent;
    int const n = mmz->moncount();

    minHeight = minimum_Height;

 
    devlink devlist = dmz->getDeviceList();
    if(devlist != NULL)
    {
      do
      {
        outplink outlist = devlist->olist;
        if(outlist != NULL)
        {
          do
          {
            monItem m;
            m.dev = devlist->id;
            m.out = outlist->id;
            m.shown = false;
            monItems.push_back(m); 
            outlist = outlist->next;
          } while (outlist != NULL);
        }
        devlist = devlist->next;
      } while (devlist != NULL);
    }


    //sizer containing traces
    tracelist_sizer = new wxBoxSizer(wxVERTICAL);

    //testing loop to make many monitors, remove ASAP, stop being lazy, and make a test file with many monitors
    for (int testcount = 0; testcount<1 ; testcount++)
    {    
      for(int j=0; j<n; j++) // loop over the number of signals to display all
      {
        name mondev;
        name monout;
        mmz -> getmonname(j, mondev, monout);
        appendTraceItem(j);
        addMon(mondev, monout);
      }
    }
    
    this -> SetBackgroundColour(*wxWHITE);
    this -> SetSizer(tracelist_sizer);
    this -> FitInside(); // ask the sizer about the needed size
    this -> SetScrollRate(0, 10);
    
}

TraceDisplay::~TraceDisplay() {
}

void TraceDisplay::Render(int cycles) {
    for(std::vector<TraceItem*>::size_type i = 0; i != traceItems.size(); i++) {
      traceItems[i] -> Render(cycles);
    }
}

void TraceDisplay::changePanAll(int dx) {
    for (int i = 0; i != traceItems.size(); i++) {
        traceItems[i]->canvas->changePan(dx);
        traceItems[i]->canvas->Render();
    }
}

vector<wxString> TraceDisplay::getMonList() {
    vector<wxString> sv;
    for (int i = 0; i != monItems.size(); i++) {
        wxString s;
        s += wxString(nmz->getName(monItems[i].dev));
        if (monItems[i].out != blankname)
        {
          s += ".";
          s += wxString(nmz->getName(monItems[i].out));
        }
        sv.push_back(s);
    }
    return sv;
}

vector<wxString> TraceDisplay::getTraceList() {
    vector<wxString> s;
    for (int i = 0; i != traceItems.size(); i++) {
        s.push_back(traceItems[i]->getStr());
    }
    return s;
}

void TraceDisplay::setMonList(std::vector<bool> list)
{
    if (list.size() != monItems.size())
    {
      cout << "tracedisplay setmonlist" << endl;
      exit(1);
    }

    //Remove then add the monitors, to prevent reaching monitor limit prematurely
    for (int i = 0; i < monItems.size(); i++) {
        if(monItems[i].shown != list[i])
        {         
          if(list[i] == false)
          {
            RemoveTrace(monItems[i].dev, monItems[i].out);
          }
        }
    }
    
    bool addfailed = false;
    for (int i = 0; i < monItems.size(); i++) {
        if(monItems[i].shown != list[i])
        {
          if(list[i] == true and not addfailed)
          {
            if(AddTrace(monItems[i].dev, monItems[i].out))
              monItems[i].shown = true;
            else
              addfailed=true;
          }
        }
    }
}

bool TraceDisplay::getMon(int index) {
    return monItems[index].shown;
}

void TraceDisplay::checkremoveMon(int traceindex) {
    int si;
    for (int i = 0; i != monItems.size(); i++) {
        if (monItems[i].dev==traceItems[traceindex]->mondev && monItems[i].out==traceItems[traceindex]->monout) {
            si = i;
        }
    }
    bool unique = true;
    for (int j = 0; j != traceItems.size(); j++) {
        if ((traceItems[j]->mondev==monItems[si].dev) && (traceItems[j]->monout==monItems[si].out) && (j!=traceindex)) {
            unique = false;
        }
    }
    if (unique) monItems[si].shown = false;
}

int TraceDisplay::getMonCount() {
    return monItems.size();
}

void TraceDisplay::addMon(name devname, name outp){
    for (int i = 0; i != monItems.size(); i++) {
        if (monItems[i].dev==devname && monItems[i].out==outp) {
            monItems[i].shown = true;
        }
    }
}

void TraceDisplay::appendTraceItem(int monNum) {
    TraceItem *titem = new TraceItem(this, wxID_ANY, nmz, dmz, mmz, monNum, minHeight);
    tracelist_sizer -> Add(titem, 1, wxEXPAND | wxALL, 5);  //warning, settings here are also found in mvtrace
    traceItems.push_back(titem);
}

void TraceDisplay::updateTraces() {  
    for (auto tit : traceItems)
    {
        tit->UpdateMon();
    }
}

bool TraceDisplay::AddTrace(name dev, name outp) {
    bool ok;
    mmz->makemonitor (dev, outp, ok);
    int const n = mmz->moncount();
    if (!ok)
    {
      wxMessageDialog* monmsg = new wxMessageDialog(par, _("Maximum allowable number of monitors reached (") + to_string(maxmonitors) + _("). Remove some monitors first and try again."), _("Error"));
      monmsg->ShowModal();
      return false;
    }
    
    appendTraceItem(n-1);
    addMon(dev, outp);
    
    par -> ReRun();
    this -> FitInside();
    tracelist_sizer->Layout();
    
    return true;
}

void TraceDisplay::RemoveTrace(int index) {
    
    if (index<traceItems.size())
    {
      checkremoveMon(index);
      
      bool ok;
      mmz -> remmonitor (traceItems[index]->mondev, traceItems[index]->monout, ok);
      if (!ok)
      {
        cout << "traceitem remmonitor" << endl;
        exit(1);
      }
      
      tracelist_sizer -> Remove(index);
      traceItems[index]->Destroy();
      traceItems.erase(traceItems.begin()+index);
      
      updateTraces();
      
      par -> ReRun();
      this -> FitInside();
      tracelist_sizer->Layout();
    }
    else
    {
      cout << "tracedisplay removetrace1" << endl;
      exit(1);
    }
}

void TraceDisplay::RemoveTrace(TraceItem *titem) {
    auto p = std::find (traceItems.begin(), traceItems.end(), titem);
    if (p != traceItems.end())
    {
      int index = distance(traceItems.begin(),p);
      RemoveTrace(index);
    }
}

void TraceDisplay::RemoveTrace(name devname, name outp) {

    for (int i = 0; i < traceItems.size(); i++) {
        if (traceItems[i]->mondev==devname && traceItems[i]->monout==outp) {
            RemoveTrace(i);
        }
    }
}

void TraceDisplay::MvTrace(TraceItem *titem, int dist) {
    auto p = std::find (traceItems.begin(), traceItems.end(), titem);
    if (p != traceItems.end())
    {
      int index = distance(traceItems.begin(),p);
      tracelist_sizer -> Remove(index);
      traceItems.erase(p);
      
      index += dist;
      index = (index<0)?0:index;
      int max=tracelist_sizer->GetItemCount();
      index = (index>max)?max:index;
      
      tracelist_sizer -> Insert(index,titem, 1, wxEXPAND | wxALL, 5);
      traceItems.insert(traceItems.begin()+index, titem);
    }
    
    updateTraces();
    
    //par -> ReRun();
    this -> FitInside();
    tracelist_sizer->Layout();
}

