#ifndef logsim_h
#define logsim_h

#include <wx/intl.h>
#include <wx/wx.h>
#include "wx/file.h"
#include "wx/log.h"

/// wxWidgets App
class MyApp: public wxApp
{
 public:
  bool OnInit(); // automatically called when the application starts
};

#endif /* logsim_h */
