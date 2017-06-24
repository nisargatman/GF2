#include <stdexcept>
#include "logsim.h"
#include "userint.h"
#include "gui.h"
#include "names.h"
#include "devices.h"
#include "monitor.h"
#include "network.h"
#include "parser.h"
#include <wx/intl.h>

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

IMPLEMENT_APP(MyApp)

void printUsage(wxString filename) {
    wcout << "Usage:   " << filename << " [-h | -t filename | -c filename | filename]" << endl; 
    wcout << "  -h    Show this message and exit" << endl;
    wcout << "  -t    Run text interface, filename required" << endl;
    wcout << "  -c    Check if the file parses and exit, filename required" << endl;
    wcout << "Available language are: Spanish (es_ES), Hindi (hi_IN), French (fr_FR) and English (Default)" << endl;
}

// Language 
void initLanguageSupport(long language)
{
    wxLocale* MyLocale = new wxLocale(language, wxLOCALE_LOAD_DEFAULT);
    if(MyLocale->IsAvailable(language)) 
    {
      MyLocale->AddCatalogLookupPathPrefix("lang");
      if(!MyLocale->AddCatalog("logsim")) cerr << "AddCatalog failed\n";
      if(! MyLocale->IsOk()) cerr << "selected language is wrong" << endl;
    }
    else 
    {
        cout << "The selected language is not supported by your system." << "Try installing support for this language." << endl;
    }
}


bool MyApp::OnInit()
  // This function is automatically called when the application starts
{
    if (argc == 2 && argv[1] == "-h") {
        printUsage(argv[0]);
        return false;
    }
    else if (argc > 3) {
        printUsage(argv[0]);
        return false;
    }
    else if (argc == 3) {
        if (argv[1] != "-t" && argv[1] != "-c") {
            printUsage(argv[0]);
            return false;
        }

	  
        // Parse the file
        names* nmz = new names();
        network* netz = new network(nmz);
        devices* dmz = new devices(nmz, netz);
        monitor* mmz = new monitor(nmz, netz);
        Scanner* smz;
        try {
            smz = new Scanner(nmz, wxString(argv[2]).mb_str());
        }
        catch (const invalid_argument& e) {
            cout << e.what() << endl;
            return false;
        }
        Parser* pmz = new Parser(netz, dmz, mmz, smz);

        bool ok = pmz->readin();
        delete pmz; delete smz; // Clean up
        if (!ok)
            return false;

        if (argv[1] == "-t") {
            // Construct the text-based interface
            userint umz(nmz, dmz, mmz); // Probably memory leak inside
            umz.userinterface();
        }
        else {
            // Just checking the file
            wcout << "  All OK" << endl;
            delete mmz; delete dmz; delete netz; delete nmz;
        }
        return false;
    }
    else if (argc == 2 && argv[1][0] == '-') {
        printUsage(argv[0]);
        return false;
    }
    else { // Run in the graphical mode
        // glutInit cannot cope with Unicode command line arguments, so we pass
        // it some fake ASCII ones instead
        char **tmp1; int tmp2 = 0; glutInit(&tmp2, tmp1);

	char* language = getenv("LC_ALL");
        if (!language) {}
        else if (strcmp("es_ES",language) == 0) initLanguageSupport(wxLANGUAGE_SPANISH);
        else if (strcmp("hi_IN",language) == 0) initLanguageSupport(wxLANGUAGE_HINDI);
        else if (strcmp("fr_FR",language) == 0) initLanguageSupport(wxLANGUAGE_FRENCH);
             else {cout << "Language not supported by application. Using English" << endl;} 

        // Construct the gui
        MyFrame *frame = new MyFrame(NULL, _("Logic simulator"), 
                                     wxDefaultPosition,  wxSize(800, 600));
        frame->Show(true);

        // Try to open the file if one was supplied
        if (argc == 2 && !frame->OpenFile(string(argv[1]))) {
            return false;
        }
        return true;
    }
}
