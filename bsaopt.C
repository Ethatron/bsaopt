/* Version: MPL 1.1/LGPL 3.0
 *
 * "The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is BSAopt.
 *
 * The Initial Developer of the Original Code is
 * Ethatron <niels@paradice-insight.us>. Portions created by The Initial
 * Developer are Copyright (C) 2011 The Initial Developer.
 * All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Library General Public License Version 3 license (the
 * "LGPL License"), in which case the provisions of LGPL License are
 * applicable instead of those above. If you wish to allow use of your
 * version of this file only under the terms of the LGPL License and not
 * to allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the LGPL License.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under either the MPL or the LGPL License."
 */

#include <string.h>
#include <algorithm>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <vector>

using namespace std;

#define	_CRT_SECURE_NO_WARNINGS
#define	_CRT_NONSTDC_NO_DEPRECATE

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <sys/types.h>

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <knownfolders.h> // for KnownFolder APIs/datatypes/function headers

/*
 * Find the first occurrence of find in s, ignore case.
 */
char *stristr(const char *s, const char *find) {
  char c, sc;
  size_t len;

  if ((c = *find++) != 0) {
    c = tolower((unsigned char)c);
    len = strlen(find);
    do {
      do {
	if ((sc = *s++) == 0)
	  return (NULL);
      } while ((char)tolower((unsigned char)sc) != c);
    } while (strnicmp(s, find, len) != 0);

    s--;
  }

  return ((char *)s);
}

BOOL CreateDirectoryRecursive(const char *pathname) {
  size_t len, l;
  char buf[1024];
  BOOL ret = FALSE;

  len = strlen(pathname);
  strcpy(buf, pathname);

  /* iterate down */
  while (!(ret = CreateDirectory(buf, NULL))) {
    char *sep = strrchr(buf, '\\');
    if (!sep)
      break;
    *sep = '\0';
  }

  /* back up again */
  while ((l = strlen(buf)) != len) {
    char *sep = buf + l;
    *sep = '\\';
    if (!(ret = CreateDirectory(buf, NULL)))
      break;
  }

  return ret;
}

#define simulation  0
bool skipexisting = false;
bool skipnewer = false;
bool skiphashcheck = false;
bool skipbroken = false;
bool processhidden = false;
bool verbose = false;
bool dropextras = true;
bool optimizequick = false;

map<string, string> duplicates;

int virtualbsabytes = 0;
int virtualbsafiles = 0;

size_t compressedinbytes = 0;
size_t compresseddtbytes = 0;
size_t compressedoubytes = 0;

size_t processedinbytes;
size_t processedoubytes;

#define nfoprintf(f, format, ...) { if (logfile) { fprintf(logfile, format, ## __VA_ARGS__); } }
#define errprintf(f, format, ...) { if (logfile) { fprintf(logfile, format, ## __VA_ARGS__); } }

FILE *logfile = NULL;
void summary(FILE *out, const char *ou, int processedfiles) {
  if (1) {
    fprintf(out, "\n\nFinal report \"%s\":\n", ou);
    fprintf(out, " processed files: %d\n", processedfiles);

    if (processedoubytes || processedinbytes)
      fprintf(out, " i/o delta: %d bytes\n", processedoubytes - processedinbytes);
    if (compressedinbytes || compressedoubytes)
      fprintf(out, " z delta: %d bytes\n", compressedoubytes - compressedinbytes);
    if (virtualbsabytes || virtualbsafiles)
      fprintf(out, " bsa virtuals: -%d bytes (%d files)\n", virtualbsabytes, virtualbsafiles);
  }

  if (1) {
    if (duplicates.size() > 0) {
      fprintf(out, "\nduplicates files:\n");
      map<string, string>::iterator n;
      for (n = duplicates.begin(); n != duplicates.end(); n++)
	fprintf(out, "%s == %s\n", n->first.data(), n->second.data());
    }
  }

  fflush(out);
}

void InitProgress(const char *patterna, const char *patternb, int dne, int rng);
void SetProgress(const char *str, int dne);
void SetProgress(size_t din, size_t dou);
void SetTopic(const char *topic);
void SetReport(const char *status, size_t din, size_t dou);
void SetReport(const char *status, size_t din, size_t dou, int dpl);
bool RequestFeedback(const char *question);

#include "bsaopt-io.C"

extern LPSTR selected_string;
extern long AskInput();
extern long AskOutput();

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
//#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <msvc/wx/setup.h>

#include <wx/propgrid/propgrid.h>
#include <wx/filename.h>
#include <wx/regex.h>
#include <wx/fontutil.h>

#pragma comment(lib,"Comctl32")
#pragma comment(lib,"Rpcrt4")

#include "BSAopt_Window.h"

// ----------------------------------------------------------------------------

DWORD __stdcall ConversionStart(LPVOID lp);

// ----------------------------------------------------------------------------
class ioio;
class iost;
typedef	map<string, ioio> iomap;
typedef	map<string, iost> stmap;

class ioio {
public:
  ioio() { selected = true; skip = iex = oex = false; }

  bool skip, selected;

  struct ioinfo in; bool iex; 
  struct ioinfo ou; bool oex; 

#if 0
  static bool compare(
    const iomap::iterator d1,
    const iomap::iterator d2
  ) {
    return (d1->second.in.io_time < d2->second.in.io_time);
  }
#endif
};

class iost {
public:
  iost() { imsk = omsk /*= icnt = ocnt*/ = 0; }

  int imsk; //int icnt; 
  int omsk; //int ocnt; 

#if 0
  static bool compare(
    const iomap::iterator d1,
    const iomap::iterator d2
    ) {
      return (d1->second.in.io_time < d2->second.in.io_time);
  }
#endif
};

class ioTreeItemData : public wxTreeItemData {
public:
  ioTreeItemData(string &fp) : wxTreeItemData() {fullpath = fp;}
  string fullpath;
};

// ----------------------------------------------------------------------------
class BSAoptPrg; class BSAoptPrg *prg;
class BSAoptPrg : public wxProgress {

private:
  const char *lastpa;
  const char *lastpb;

public:
  /* all executed by Async-thread */
  void StartProgress(int rng) {
    Wait();

    BOTask->SetRange(rng);

//  Update();
  }

  void InitProgress(const char *patterna, const char *patternb, int dne, int rng) {
    Wait();

    char tmp[256];

    if (patterna) { sprintf(tmp, lastpa = patterna, "..."); BOSubject->SetLabel(tmp); }
    if (patternb) { sprintf(tmp, lastpb = patternb, "..."); BOReport ->SetLabel(tmp); }

    BOTask->SetValue(dne);

    BOEfficiency->SetRange(rng);
    BOEfficiency->SetValue(rng);

//  Update();
  }

  int range1, value1;
  int range2, value2;

  void SetProgress(const char *str, int dne) {
    Wait();

    char tmp[256];

    if (lastpa) { sprintf(tmp, lastpa, str  ); BOSubject->SetLabel(tmp); }
//  if (lastpb) { sprintf(tmp, lastpb, "..."); BOReport ->SetLabel(tmp); }

    BOTask->SetValue(dne);
  }

  void SetProgress(size_t din, size_t dou) {
    Wait();

    int rng = (din ? ((unsigned __int64)dou * 0xFFFF) / din : 0);

    BOEfficiency->SetRange(0xFFFF);
    BOEfficiency->SetValue(rng);
  }

  void SetTopic(const char *topic) {
    BOSubject->SetLabel(topic);
  }

  void SetReport(const char *status, size_t din, size_t dou, int dpl = -1) {
    char tmp[256], tmpi[256], tmpo[256];

    /**/ if (din > 1000 * 1000 * 1000)
      sprintf(tmpi, "%d.%03d.%03d.%03d", (int)(((din / 1000) / 1000) / 1000), (int)(((din / 1000) / 1000) % 1000), (int)((din / 1000) % 1000), (int)(din % 1000));
    else if (din > 1000 * 1000)
      sprintf(tmpi, "%d.%03d.%03d", (int)((din / 1000) / 1000), (int)((din / 1000) % 1000), (int)(din % 1000));
    else if (din > 1000)
      sprintf(tmpi, "%d.%03d", (int)(din / 1000), (din % 1000));
    else 
      sprintf(tmpi, "%d", (int)(din % 1000));

    /**/ if (dou > 1000 * 1000 * 1000)
      sprintf(tmpo, "%d.%03d.%03d.%03d", (int)(((dou / 1000) / 1000) / 1000), (int)(((dou / 1000) / 1000) % 1000), (int)((dou / 1000) % 1000), (int)(dou % 1000));
    else if (dou > 1000 * 1000)
      sprintf(tmpo, "%d.%03d.%03d", (int)((dou / 1000) / 1000), (int)((dou / 1000) % 1000), (int)(dou % 1000));
    else if (dou > 1000)
      sprintf(tmpo, "%d.%03d", (int)(dou / 1000), (dou % 1000));
    else 
      sprintf(tmpo, "%d", (int)(dou % 1000));

    if (dpl < 0)
      sprintf(tmp, status, tmpi, tmpo);
    else
      sprintf(tmp, status, tmpi, tmpo, dpl);

    BOReport->SetLabel(tmp);
  }

  void PollProgress() {
    Wait();
  }

private:
  bool paused, quit;
  time_t tinit, tlast, tnow;

public:
  /* all executed by Progress-thread */
  virtual void PauseProgress(wxCommandEvent& event) {
    if (!paused) {
      Block();

      paused = true;
      BOPause->SetLabel("Unpause");
    }
    else {
      Unblock();

      paused = false;
      BOPause->SetLabel("Pause");
    }
  }

  virtual void AbortProgress(wxCloseEvent& event) {
    if (event.CanVeto())
      event.Veto();

    Abort();
  }

  virtual void AbortProgress(wxCommandEvent& event) {
    Abort();
  }

  virtual void IdleProgress(wxIdleEvent& event) {
    tnow = time(NULL);

    long tpass = (long)(tnow - tinit);
    if (tpass != tlast) {
      tlast = tpass;

      float f1 = (float)(BOTask->GetValue()) / (BOTask->GetRange());
      int tfinal = (int)floor((float)tpass / f1);
      int trem = tfinal - tlast;

      char buf[256];

      sprintf(buf, "%02d:%02ds", tpass / 60, tpass % 60);
      BORunning->SetLabel(buf);

      if (tfinal != 0x80000000) {
	sprintf(buf, "%02d:%02ds", trem / 60, trem % 60);
	BORemaining->SetLabel(buf);
	sprintf(buf, "%02d:%02ds", tfinal / 60, tfinal % 60);
	BOTotal->SetLabel(buf);
      }
      else {
	BORemaining->SetLabel("00:00s");
	BOTotal->SetLabel(buf);
      }
    }

//  Sleep(500);
//  event.RequestMore();
  }

private:
  HANDLE evt, end;
  HANDLE async;

  /* called from Progress-thread */
  void Abort() {
    quit = true;
    SetEvent(evt);
  }

  /* called from Async-thread */
  void Wait() {
    WaitForSingleObject(evt, INFINITE);

    /* signal abortion (inside Async-thread) */
    if (quit)
      throw runtime_error("ExitThread");
  }

  void Block() {
    ResetEvent(evt);
  }

  void Unblock() {
    SetEvent(evt);
  }

public:
  wxEventType evtLeave;
  int idLeave;
  int retCode;

  class LeaveEvent: public wxCommandEvent {
public:
    LeaveEvent(int id, const wxEventType& event_type) : wxCommandEvent(event_type, id) {}
    LeaveEvent(const LeaveEvent& event) : wxCommandEvent(event) {}

    wxEvent* Clone() const { return new LeaveEvent(*this); }
  };

  typedef void (wxEvtHandler::*LeaveEventFunction)(LeaveEvent &);
  
  /* called from outside-thread */
  int Enter(LPTHREAD_START_ROUTINE routine) {
    if ((async = CreateThread(NULL, 0, routine, this, 0, NULL)) == INVALID_HANDLE_VALUE)
      return 0;
    SetThreadPriority(async, THREAD_PRIORITY_BELOW_NORMAL);

    return ShowModal();
  }

  /* called from Async-thread */
  void Leave(int rc) {
    retCode = rc;

    LeaveEvent event(idLeave, evtLeave);
    wxPostEvent(this, event);

    /* wait for the progress-dialog to recognize our message */
    WaitForSingleObject(end, INFINITE);
  }

  /* called from Progress-thread */
  void Leave(LeaveEvent &) {
    /* signal that we recognize the message */
    SetEvent(end);

    WaitForSingleObject(async, INFINITE);
    CloseHandle(async);

    EndModal(retCode);
  }

  BSAoptPrg::BSAoptPrg(wxWindow *parent) : wxProgress(parent) {
    evtLeave = wxNewEventType();
    idLeave = wxNewId();

    /* Connect to event handler that will make us close */
    Connect(wxID_ANY, evtLeave,
      (wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)wxStaticCastEvent(LeaveEventFunction, &BSAoptPrg::Leave),
      NULL,
      this);

    tinit = time(NULL);
    paused = false;
    quit = false;
    prg = this;

    evt = CreateEvent(
      NULL,		  // default security attributes
      TRUE,		  // manual reset
      TRUE,		  // initially set
      NULL		  // unnamed mutex
    );

    end = CreateEvent(
      NULL,		  // default security attributes
      TRUE,		  // manual reset
      FALSE,		  // initially not set
      NULL		  // unnamed mutex
    );

    SetSize(600, 265);
  }

  BSAoptPrg::~BSAoptPrg() {
    CloseHandle(evt);
    CloseHandle(end);
  }
};

// ----------------------------------------------------------------------------
class BSAoptGUI; class BSAoptGUI *gui;
class BSAoptGUI : public wxBSAopt
{
  friend class BSAoptApp;

public:
  int iarchive, oarchive;
  int iactives, oactives;
  int iprogres, oprogres;
  iomap fdirectory;
  stmap ddirectory;
  vector<iomap::iterator> ldirectory;
  wxRegEx filter;
  string currentpath;
  bool warmup;

private:
  // Installed Path
  char IPath[1024];
  char OPath[1024];
  HKEY Settings;
  BSAoptPrg *prog;

  /* ---------------------------------------------------------------------------- */
  void ResetHButtons() {
    /* enable button */
    BOConvert->Enable(
      (BOInText ->GetValue() != "") &&
      (BOOutText->GetValue() != "")
    );
  }

  /* ---------------------------------------------------------------------------- */
  void ResetCFileList() {
    iomap::iterator walk = fdirectory.begin();
    while (walk != fdirectory.end()) {
      walk->second.selected = true;
      walk++;
    }

    ResetHButtons();
  }

  void ClearCFileList() {
    iomap::iterator walk = fdirectory.begin();
    while (walk != fdirectory.end()) {
      walk->second.selected = false;
      walk++;
    }

    ResetHButtons();
  }

  void LoadCFileList() {
#if 0
    int num = OSFileList->GetCount();
    for (int n = 0; n < num; n++) {
      const char *p = OSFileList->GetItem(n)->GetName().data();
      char s[32] = ""; DWORD sl = 32;

      RegGetValue(Settings, "Plugins", p, RRF_RT_REG_SZ, NULL, s, &sl);

      if (s[0] != '\0')
	OSFileList->Check(n, s[0] == '1');
    }
#endif
  }

  void SaveCFileList() {
#if 0
    int num = OSFileList->GetCount();
    for (int n = 0; n < num; n++) {
      const char *p = OSFileList->GetItem(n)->GetName().data();
      const char *s = "0";
      if (OSFileList->IsChecked(n))
	s = "1";

      RegSetKeyValue(Settings, "Plugins", p, RRF_RT_REG_SZ, s, (DWORD)strlen(s) + 1);
    }
#endif
  }

  /* ---------------------------------------------------------------------------- */
  void FilterDirectory(wxTreeItemId root, const wxString& sFilterFor) {
    wxTreeItemIdValue cookie;
    wxTreeItemId item = BOArchiveTree->GetFirstChild(root, cookie);
    wxTreeItemId child;

    while (item.IsOk()) {
      wxString sData = BOArchiveTree->GetItemText(item);
      if (sFilterFor.CompareTo(sData) == 0) {
	;
      }

      if (BOArchiveTree->ItemHasChildren(item)) {
	FilterDirectory(item, sFilterFor);
      }

      item = BOArchiveTree->GetNextChild(root, cookie);
    }
  }

  void RefreshDirectory(wxTreeItemId root, int lvl = 0) {
    wxTreeItemIdValue cookie;
    wxTreeItemId item = BOArchiveTree->GetFirstChild(root, cookie);
    wxTreeItemId child;

    while (item.IsOk()) {
//    wxString sData = BOArchiveTree->GetItemText(item);
      if (lvl == 0) {
	BOArchiveTree->Expand(item);
//	BOArchiveTree->SelectItem(item);
      }

      if (BOArchiveTree->ItemHasChildren(item)) {
	RefreshDirectory(item, lvl + 1);
      }

      item = BOArchiveTree->GetNextChild(root, cookie);
    }
  }

  wxTreeItemId FindItem(wxTreeItemId root, wxString name) {
    wxTreeItemIdValue cookie;
    wxTreeItemId item = BOArchiveTree->GetFirstChild(root, cookie);
    wxTreeItemId child;

    while (item.IsOk()) {
      wxString sData = BOArchiveTree->GetItemText(item);
      if (!sData.CmpNoCase(name))
	return item;

      item = BOArchiveTree->GetNextChild(root, cookie);
    }

    return item;
  }

  wxTreeItemId ScanEntry(const wxTreeItemId &parent, const char *name, const char *dle) {
    ioTreeItemData *dt = new ioTreeItemData(string(name));

    wxTreeItemId entry = FindItem(parent, dle);
    if (!entry.IsOk())
      entry = BOArchiveTree->AppendItem(parent, dle, -1, -1, (wxTreeItemData *)dt);

    return entry;
  }

#define OEX 1
#define IEX 2
#define CNT 4
  int Scan(const char *name, const char *locl, const wxTreeItemId &parent, int lvl, bool o) {
    size_t ilen = (name ? strlen(name) : 0);
    size_t llen = (locl ? strlen(locl) : 0);
    int msk = 0;

    struct ioinfo info;
    if (!iostat(name, &info)) {
      /* input: directory */
      if (info.io_type & IO_DIRECTORY) {
	const char *dle;
	char tmp[256];
	class iost *stat = &ddirectory[locl];

	/* strip trailing slash */
	if (!(dle = strrchr(name, '/')))
	  if (!(dle = strrchr(name, '\\')))
	    dle = name - 1;

	dle += 1;

	/* no BSAs inside BSAs! */
	if (!o && oarchive && isext(locl, "bsa"))
	  return msk;

	sprintf(tmp, "Skimming %s \"%s\" ...", o ? "output" : "input", dle);
	BOStatusBar->SetStatusText(wxT(tmp), 0);

	/* the in and output directories are parallel-roots */
	wxTreeItemId entry = (!lvl ? parent : ScanEntry(parent, locl, dle));
	
	/* walk */
	struct iodir *dir;
	if ((dir = ioopendir(name))) {
	  struct iodirent *et;

	  while ((et = ioreaddir(dir))) {
	    if (!strcmp(et->name, ".") ||
		!strcmp(et->name, ".."))
	      continue;

	    char *nname = (char *)malloc(ilen + 1 + et->namelength + 1);
	    char *lname = (char *)malloc(llen + 1 + et->namelength + 1);

	    strcpy(nname, name);
	    strcpy(lname, locl);

	    strcat(nname, "\\");
	    strcat(lname, "\\");

	    strcat(nname, et->name);
	    strcat(lname, et->name);

	    strlwr(lname);

	    /* recurse */
	    int lmsk = Scan(nname, lname, entry, lvl + 1, o);

	    /* break down the field */
	    msk = ((lmsk & ~3) + (msk & ~3)) | (lmsk & 3) | (msk & 3);

	    free(nname);
	    free(lname);
	  }

	  ioclosedir(dir);

	  /* register */
	  if (o) stat->omsk = msk;
	  else   stat->imsk = msk;

	  /* combine */
	  msk = stat->omsk | stat->imsk;

	  /* assign the color */
	  wxColour col(0, 0, 0, 255);
	  if (!(msk & IEX))
	    col = wxColour(140, 140, 140, 255);
	  else if (!(msk & OEX))
	    col = wxColour(0, 60, 0, 255);

	  BOArchiveTree->SetItemTextColour(entry, col);
//	  BOArchiveTree->SetItemText(entry, col);
	}
	else {
	  sprintf(rerror, "Can't open the directory \"%s\"\n", name);
	  throw runtime_error(rerror);
	}
      }
      /* input: file */
      else {
	const char *fle;
	struct iodir *dir;
	bool dealloc = false;
	bool docopy = true;
	bool iszero = !info.io_size;

	/* strip trailing slash */
	if (!(fle = strrchr(name, '/')))
	  if (!(fle = strrchr(name, '\\')))
	    fle = name - 1;

	fle += 1;
	if (stristr(fle, "thumbs.db"))
	  return 0;

	bool firsttime = (fdirectory.count(locl) == 0);
	class ioio *nfo = &fdirectory[locl];
	if (dropextras) {
	  /* none of this stuff inside BSAs! */
	  if (!o && oarchive && firsttime) {
	    if (isext(fle, "esp")) nfo->selected = false;
	    if (isext(fle, "esm")) nfo->selected = false;
	    if (isext(fle, "pdf")) nfo->selected = false;
	    if (isext(fle, "psd")) nfo->selected = false;
	  }

//	  if (!docopy)
//	    fprintf(stderr, "filtered \"%s\"\n", fle);
	}

//	nfo->selected = newio;
	nfo->skip = docopy;
	if (o) {
	  nfo->oex = true;
	  nfo->ou = info;

	  oactives++;
	}
	else {
	  nfo->iex = true;
	  nfo->in = info;

	  iactives += (nfo->selected ? 1 : 0);
	}

	msk |= (nfo->oex ? OEX : 0) | (nfo->iex ? IEX : 0);
	msk += CNT;
      }
    }

    return msk;
  }

  void DirectoryFromFiles(int io = 0) {
    if (warmup)
      return;

    skipexisting  = BOSettings->FindChildItem(wxID_SKIPE, NULL)->IsChecked();
    skipnewer     = BOSettings->FindChildItem(wxID_SKIPN, NULL)->IsChecked();
    skiphashcheck = BOSettings->FindChildItem(wxID_SKIPC, NULL)->IsChecked();
    skipbroken    = BOSettings->FindChildItem(wxID_SKIPB, NULL)->IsChecked();
    processhidden = BOSettings->FindChildItem(wxID_SKIPH, NULL)->IsChecked();

    wxBusyCursor busy;
    wxTreeItemId root;

    iarchive = isarchive(BOInText ->GetValue().data());
    oarchive = isarchive(BOOutText->GetValue().data());

    if (!io) {
      iactives =
      oactives = 0;

      fdirectory.clear();
    }
    else if (io == 1) {
      iactives = 0;

      iomap newmap;
      iomap::iterator walk = fdirectory.begin();
      while (walk != fdirectory.end()) {
	if (walk->second.oex) {
	  ioio *m = &newmap[walk->first];

	  m->skip     = walk->second.skip;
	  m->selected = walk->second.selected;

	  m->oex = walk->second.oex;
	  m->ou  = walk->second.ou;
	}

	walk++;
      }

      fdirectory = newmap;
    }
    else if (io == 2) {
      oactives = 0;

      iomap newmap;
      iomap::iterator walk = fdirectory.begin();
      while (walk != fdirectory.end()) {
	if (walk->second.iex) {
	  ioio *m = &newmap[walk->first];

	  m->skip     = walk->second.skip;
	  m->selected = walk->second.selected;

	  m->iex = walk->second.iex;
	  m->in  = walk->second.in;
	}

	walk++;
      }

      fdirectory = newmap;
    }

    BOArchiveList->Clear();
    BOArchiveTree->DeleteAllItems(); root =
    BOArchiveTree->AddRoot("\\");

    try {
//    if (!io || (io == 1)) {
	BOStatusBar->SetStatusText(wxT("Skimming input ..."), 0);
	Scan(BOInText->GetValue().data(), "", root, 0, false);
//    }

//    if (!io || (io == 2)) {
	BOStatusBar->SetStatusText(wxT("Skimming output ..."), 0);
	Scan(BOOutText->GetValue().data(), "", root, 0, true);
//    }
    }
    catch(exception &e) {
      if (strcmp(e.what(), "ExitThread")) {
	wxMessageDialog d(this, e.what(), "BSAopt error");
	d.ShowModal();
      }
    }

    RefreshDirectory(root);

    /* preselect root (will trigger ChangedItem()) */
    BOArchiveTree->Expand(root);
    BOArchiveTree->SelectItem(root, true);

    BOStatusBar->SetStatusText(wxT("Ready"), 0);
  }

  void ChangeFilter(wxCommandEvent& event) {
//  filter = event.GetString();
  }

  void ApplyFilter(wxCommandEvent& event) {
    /* . -> \.
     * * -> .*
     */
    wxString spat = BOFilter->GetValue();
    wxString wildcard = spat;
    wildcard.Replace(".", "\\.", true);
    wildcard.Replace("*", ".*", true);

    if (!filter.Compile(wildcard))
      filter.Compile("");
    else
      RegSetKeyValue(Settings, NULL, "Filter", RRF_RT_REG_SZ, spat.data(), spat.length() + 1);

    /* refresh file-list */
    ChangeTreeItem(currentpath.data());
  }

  void ResetFileList(wxCommandEvent& event) {
//  ResetCFileList();

    /* refresh file-list */
    ChangeTreeItem(currentpath.data(), 1);
  }

  void ClearFileList(wxCommandEvent& event) {
//  ClearCFileList();

    /* refresh file-list */
    ChangeTreeItem(currentpath.data(), 2);
  }

  void LoadFileList(wxCommandEvent& event) {
    LoadCFileList();
  }

  void SaveFileList(wxCommandEvent& event) {
    SaveCFileList();
  }

  void ChangeSkipExisting(wxCommandEvent& event) {
    RegSetKeyValue(Settings, "Skip", "Existing", RRF_RT_REG_SZ, event.IsChecked() ? "1" : "0", 2);

    /* refresh file-list */
    ChangeTreeItem(currentpath.data());
  }

  void ChangeSkipNewer(wxCommandEvent& event) {
    RegSetKeyValue(Settings, "Skip", "Newer", RRF_RT_REG_SZ, event.IsChecked() ? "1" : "0", 2);

    /* refresh file-list */
    ChangeTreeItem(currentpath.data());
  }

  void ChangeSkipHidden(wxCommandEvent& event) {
    RegSetKeyValue(Settings, "Skip", "Hidden", RRF_RT_REG_SZ, event.IsChecked() ? "1" : "0", 2);

    /* refresh file-list */
    ChangeTreeItem(currentpath.data());
  }

  void ChangeSkipHash(wxCommandEvent& event) {
    RegSetKeyValue(Settings, "Skip", "Hash", RRF_RT_REG_SZ, event.IsChecked() ? "1" : "0", 2);
  }

  void ChangeSkipBroken(wxCommandEvent& event) {
    RegSetKeyValue(Settings, "Skip", "Broken", RRF_RT_REG_SZ, event.IsChecked() ? "1" : "0", 2);
  }

  void ChangeUnselectExtras(wxCommandEvent& event) {
    RegSetKeyValue(Settings, "Unselect", "Extras", RRF_RT_REG_SZ, event.IsChecked() ? "1" : "0", 2);
  }

  void ChangeLogFile(wxCommandEvent& event) {
    RegSetKeyValue(Settings, NULL, "Logging", RRF_RT_REG_SZ, event.IsChecked() ? "1" : "0", 2);
  }

  void ChangeRecursion(wxCommandEvent& event) {
    RegSetKeyValue(Settings, NULL, "Show Recursive", RRF_RT_REG_SZ, event.IsChecked() ? "1" : "0", 2);

    /* refresh file-list */
    ChangeTreeItem(currentpath.data());
  }

  void ChangeTreeItem(wxTreeEvent& event) {
    wxTreeItemId item = event.GetItem();
    ioTreeItemData *iod = (ioTreeItemData *)BOArchiveTree->GetItemData(item);
    currentpath = (iod ? iod->fullpath : "");

    /* refresh file-list */
    ChangeTreeItem(currentpath.data());
  }

  void ChangeTreeItem(const char *basedir, int op = 0) {
    wxBusyCursor wait;
//  BOStatusBar->SetStatusText(wxT("Running ..."), 0);

    if (op != 0) {
      size_t n = ldirectory.size();
      while (n-- > 0) {
	iomap::iterator walk = ldirectory[n];

	/* apply operation locally */
	bool selected = 
	  op == 2 ? false :
	  op == 1 ? true  : walk->second.selected;

	/**/ if ( selected && !walk->second.selected)
	  iactives++;
	else if (!selected &&  walk->second.selected)
	  iactives--;

	walk->second.selected = selected;

	/* change the item */
	BOArchiveList->Check(n, walk->second.selected && walk->second.iex);
      }

      return;
    }

    wxFont fnt = BOArchiveList->GetFont();
    const wxNativeFontInfo *fni = fnt.GetNativeFontInfo();
    wxNativeFontInfo sni = *fni; sni.lf.lfStrikeOut = TRUE;
    wxFont str = fnt; str.SetNativeFontInfo(sni);

    skipexisting  = BOSettings->FindChildItem(wxID_SKIPE, NULL)->IsChecked();
    skipnewer     = BOSettings->FindChildItem(wxID_SKIPN, NULL)->IsChecked();
    skiphashcheck = BOSettings->FindChildItem(wxID_SKIPC, NULL)->IsChecked();
    skipbroken    = BOSettings->FindChildItem(wxID_SKIPB, NULL)->IsChecked();
    processhidden = BOSettings->FindChildItem(wxID_SKIPH, NULL)->IsChecked();

    BOArchiveList->Clear();
/*  If we don't want root selectable do this: if (!basedir[0]) return; */

    BOStatusBar->SetStatusText(wxT("Refreshing file-list ..."), 0);
    ldirectory.clear();
    iomap::iterator walk = fdirectory.begin();
    while (walk != fdirectory.end()) {
      const char *fname = walk->first.data();

      if ((fname == stristr(fname, basedir))) {
	const char *fbase = fname + strlen(basedir) + 1;
	/* prevent matching similarly named subdirectories */
	if (fbase[-1] == '\\')

	/* recursive or direct file */
	if ((BORecursive->GetValue() || !strchr(fbase, '\\')) && filter.Matches(fbase)) {
	  /* apply operation locally */
	  bool selected = 
	    op == 2 ? false :
	    op == 1 ? true  : walk->second.selected;

	  /**/ if ( selected && !walk->second.selected)
	    iactives++;
	  else if (!selected &&  walk->second.selected)
	    iactives--;
	  
	  walk->second.selected = selected;

	  /* add the item */
	  int n =
	  BOArchiveList->Append(fbase);
	  BOArchiveList->Check(n, walk->second.selected && walk->second.iex);
	  wxOwnerDrawn *id = 
	  BOArchiveList->GetItem(n);

	  /* client-data substitute */
	  if (n >= ldirectory.size())
	    ldirectory.resize(n + 1);
	  ldirectory[n] = walk;

	  /* mark destination-only */
	  id->SetCheckable(walk->second.iex);
	  wxColour col(0, 0, 0, 255);
	  if (!walk->second.iex)
	    col = wxColour(140, 140, 140, 255);
	  else if (!walk->second.oex)
	    col = wxColour(0, 60, 0, 255);

	  /* mark no-overwrite */
	  if (walk->second.iex && walk->second.oex) {
	    if (skipexisting)
	      id->SetFont(str);
	    else if (skipnewer && (walk->second.in.io_time <= walk->second.ou.io_time))
	      id->SetFont(str);
	  }

	  id->SetTextColour(col);
	}
      }

      walk++;
    }

    BOStatusBar->SetStatusText(wxT("Ready"), 0);
  }

  void ChangeSelectedFiles(wxCommandEvent& event) {
    int n = event.GetSelection();
    iomap::iterator f = ldirectory[n];
    /* event.IsChecked() doesn't work here */
    bool selected = BOArchiveList->IsChecked(n);

    /**/ if ( selected && !f->second.selected)
      iactives++;
    else if (!selected &&  f->second.selected)
      iactives--;

    f->second.selected = selected;
  }

  /* ---------------------------------------------------------------------------- */
  void TypedIn(wxCommandEvent& event) {
    wxString ph = event.GetString();
//  BOInText->SetValue(ph);

    if (ph.IsNull()) 
      return;

    /* does it exist? */
    if (GetFileAttributes(ph.data()) == INVALID_FILE_ATTRIBUTES)
      return;

    BrowseIn(ph);
  }

  void BrowseIn(wxCommandEvent& event) {
    HRESULT hr = AskInput();
    if (!SUCCEEDED(hr))
      return;

    wxString ph = selected_string;
    BOInText->SetValue(ph);

    if (!ph.IsNull()) {
    }

    BrowseIn(ph);
  }

  void BrowseIn(wxString ph) {
    BOConvert->Enable(FALSE);

    strcpy(IPath, ph.data());

//  ResetCFileList();
    ResetHButtons();

    RegSetKeyValue(Settings, NULL, "Input Location", RRF_RT_REG_SZ, IPath, (DWORD)strlen(IPath) + 1);

    if (ph.IsNull())
      return;

    DirectoryFromFiles(1);
    ResetHButtons();
  }

  /* ---------------------------------------------------------------------------- */
  void TypedOut(wxCommandEvent& event) {
    wxString ph = event.GetString();
//  BOOutText->SetValue(ph);

    if (ph.IsNull()) 
      return;

    BrowseOut(ph);
  }

  void BrowseOut(wxCommandEvent& event) {
    HRESULT hr = AskOutput();
    if (!SUCCEEDED(hr))
      return;

    wxString ph = selected_string;
    BOOutText->SetValue(ph);

    if (!ph.IsNull()) {
    }

    BrowseOut(ph);
  }

  void BrowseOut(wxString ph) {
    BOConvert->Enable(FALSE);

    strcpy(OPath, ph.data());

//  ResetCFileList();
    ResetHButtons();

    RegSetKeyValue(Settings, NULL, "Output Location", RRF_RT_REG_SZ, OPath, (DWORD)strlen(OPath) + 1);

    if (ph.IsNull())
      return;

    /* does it exist? */
    if (GetFileAttributes(ph.data()) == INVALID_FILE_ATTRIBUTES)
      return;

    DirectoryFromFiles(2);
    ResetHButtons();
  }

  /* ---------------------------------------------------------------------------- */
  void ConversionStart(wxCommandEvent& event) {
    BOStatusBar->SetStatusText(wxT("Running ..."), 0);
    wxBusyCursor wait;
    prog = new BSAoptPrg(this);
    int ret = prog->Enter(::ConversionStart);
    delete prog;

    /* wait for the asynchronous processes ... */
    ioflush();
    free_arc();

    BOStatusBar->SetStatusText(wxT("Ready"), 0);
    prog = NULL;
    if (ret != 666)
      return;

    /* reflect the changes made to the output */
    DirectoryFromFiles(2);
  }

public:
  void Process(const char *inname, const char *ouname, const char *lcname) {
    size_t ilen = (inname ? strlen(inname) : 0);
    size_t olen = (ouname ? strlen(ouname) : ilen);
    size_t llen = (lcname ? strlen(lcname) : 0);

    bool rm = false;
    struct ioinfo iinfo, oinfo;
    if (!iostat(inname, &iinfo)) {
      /* input: directory */
      if (iinfo.io_type & IO_DIRECTORY) {
	/* don't create output in simulation-mode */
	if (ouname && !simulation && iostat(ouname, &oinfo)) {
	  rm = true;

	  if (iomkdir(ouname)) {
	    sprintf(rerror, "Can't create the directory \"%s\"\n", ouname);
	    throw runtime_error(rerror);
	  }

	  if (iostat(ouname, &oinfo)) {
	    sprintf(rerror, "Can't find the directory \"%s\"\n", ouname);
	    throw runtime_error(rerror);
	  }
	}

	if ((oinfo.io_type & IO_DIRECTORY) || simulation) {
	  struct iodir *dir;

	  /* no BSAs inside BSAs! */
	  if (oarchive && isext(lcname, "bsa"))
	    return;

	  if ((dir = ioopendir(inname))) {
	    struct iodirent *et;

	    while ((et = ioreaddir(dir))) {
	      if (!strcmp(et->name, ".") ||
		  !strcmp(et->name, ".."))
		continue;

	      char *ninname = (char *)malloc(ilen + 1 + et->namelength + 1);
	      char *nouname = (char *)malloc(olen + 1 + et->namelength + 1);
	      char *nlcname = (char *)malloc(olen + 1 + et->namelength + 1);

	      strcpy(ninname, inname);
	      strcpy(nouname, ouname ? ouname : inname);
	      strcpy(nlcname, lcname);

	      strcat(ninname, "\\");
	      strcat(nouname, "\\");
	      strcat(nlcname, "\\");

	      strcat(ninname, et->name);
	      strcat(nouname, et->name);
	      strcat(nlcname, et->name);

	      strlwr(nlcname);

	      Process(ninname, nouname, nlcname);

	      free(ninname);
	      free(nouname);
	      free(nlcname);
	    }

	    ioclosedir(dir);

	    /* don't create output in simulation-mode */
	    if (ouname && !simulation && (dir = ioopendir(ouname))) {
	      struct iodirent *et;
	      int num = 0;

	      while ((et = ioreaddir(dir))) {
		if (!strcmp(et->name, ".") ||
		    !strcmp(et->name, ".."))
		  continue;

		/* ioopen/fopen is synchronous, only close is
		 * asynchronous, which means even non-yet-written
		 * files exist in the folder and we don't need to wait for
		 * completion to check if a folder is empty/non-empty
		 *
		 * no lock/synchronization necessary
		 */
		num++;
	      }

	      ioclosedir(dir);

	      if (!num && rm)
		iormdir(ouname);
	    }
	  }
	  else {
	    sprintf(rerror, "Can't open the directory \"%s\"\n", inname);
	    throw runtime_error(rerror);
	  }
	}
	else {
	  sprintf(rerror, "Destination \"%s\" isn't a directory\n", ouname);
	  throw runtime_error(rerror);
	}
      }
      /* input: file */
      else {
	const char *fle;
	struct iodir *dir;
	bool dealloc = false;
	bool docopy = true;
	bool iszero = !iinfo.io_size;

	/* strip trailing slash */
	if (!(fle = strrchr(inname, '/')))
	  if (!(fle = strrchr(inname, '\\')))
	    fle = inname - 1;

	fle += 1;
	if (stristr(fle, "thumbs.db"))
	  return;

	/* skip unselected ones (iactive is altered by the selection) */
	if (!fdirectory[lcname].selected)
	  return;

	if (!docopy)
	  prog->SetProgress(lcname, iprogres++);
	else
	  prog->SetProgress(lcname, iprogres++);

	/* if there is no destination, clone the source */
	if (ouname && !iostat(ouname, &oinfo)) {
	  if (oinfo.io_type & IO_DIRECTORY) {
	    char *nouname = (char *)malloc(olen + 1 + strlen(fle) + 1);

	    /* if we push a single specific file into a BSA
	     * we have to open/close it to flush the BSA to disk
	     */
	    dir = ioopendir(ouname);

	    strcpy(nouname, ouname);
	    strcat(nouname, "\\");
	    strcat(nouname, fle);

	    ouname = nouname;
	    dealloc = true;
	  }
	  else if (skipexisting)
	    return;
	  else if (skipnewer)
	    if (iinfo.io_time <= oinfo.io_time)
	      return;
	}

	/* passthrough-mode, don't create output in simulation-mode */
	if (docopy && ouname && !simulation) {
	  /* action required */
	  if (stricmp(inname, ouname)) {
	    nfoprintf(stderr, "copying \"%s\"\n", lcname);

	    /* TODO: nowait asynchronous */
	    while (1) {
	      try {
		iocp(inname, ouname);
	      }
	      catch(exception &e) {
		if (strcmp(e.what(), "ExitThread")) {
		  errprintf(stdout, e.what());
		  if (skipbroken)
		    break;

		  char buf[256]; sprintf(buf, "Failed to copy file \"%s\". Retry?", lcname);
		  wxMessageDialog d(this, buf, "BSAopt error", wxYES_NO | wxCANCEL | wxCENTRE);
		  int res = d.ShowModal();
		  if (res == wxID_YES)
		    continue;
		  if (res == wxID_NO)
		    break;
		}

		throw runtime_error("ExitThread");
	      }

	      break;
	    };

	    /* progress */
	    processedinbytes += iinfo.io_size;
	    prog->SetReport("Efficiency: %s to %s bytes", 
	      processedinbytes, 
	      processedinbytes - compresseddtbytes - virtualbsabytes
	    );
	    prog->SetProgress(
	      processedinbytes, 
	      processedinbytes - compresseddtbytes - virtualbsabytes
	    );
	  }
	}

	if (dealloc) {
	  ioclosedir(dir);
	  free((void *)ouname);
	}
      }
    }
  }

  void ConversionStart() {
    skipexisting  = BOSettings->FindChildItem(wxID_SKIPE, NULL)->IsChecked();
    skipnewer     = BOSettings->FindChildItem(wxID_SKIPN, NULL)->IsChecked();
    skiphashcheck = BOSettings->FindChildItem(wxID_SKIPC, NULL)->IsChecked();
    skipbroken    = BOSettings->FindChildItem(wxID_SKIPB, NULL)->IsChecked();
    processhidden = BOSettings->FindChildItem(wxID_SKIPH, NULL)->IsChecked();
    dropextras    = BOSettings->FindChildItem(wxID_SKIPX, NULL)->IsChecked();
    verbose = false;

    gameversion = -1;
    /**/ if (BOGame->FindChildItem(wxID_OBLIVON, NULL)->IsChecked())
      gameversion = OB_BSAHEADER_VERSION, RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "1", 2);
    else if (BOGame->FindChildItem(wxID_FALLOUT, NULL)->IsChecked())
      gameversion = F3_BSAHEADER_VERSION, RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "2", 2);
    else if (BOGame->FindChildItem(wxID_SKYRIM , NULL)->IsChecked())
      gameversion = SK_BSAHEADER_VERSION, RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "3", 2);
    else
					  RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "0", 2);

    srchbestbsa = false;
    thresholdbsa = true;
    /**/ if (BOCompression->FindChildItem(wxID_BOZ0, NULL)->IsChecked())
      compresslevel = 0;
    else if (BOCompression->FindChildItem(wxID_BOZ1, NULL)->IsChecked())
      compresslevel = 1;
    else if (BOCompression->FindChildItem(wxID_BOZ2, NULL)->IsChecked())
      compresslevel = 2;
    else if (BOCompression->FindChildItem(wxID_BOZ3, NULL)->IsChecked())
      compresslevel = 3;
    else if (BOCompression->FindChildItem(wxID_BOZ4, NULL)->IsChecked())
      compresslevel = 4;
    else if (BOCompression->FindChildItem(wxID_BOZ5, NULL)->IsChecked())
      compresslevel = 5;
    else if (BOCompression->FindChildItem(wxID_BOZ6, NULL)->IsChecked())
      compresslevel = 6;
    else if (BOCompression->FindChildItem(wxID_BOZ7, NULL)->IsChecked())
      compresslevel = 7;
    else if (BOCompression->FindChildItem(wxID_BOZ8, NULL)->IsChecked())
      compresslevel = 8;
    else if (BOCompression->FindChildItem(wxID_BOZ9, NULL)->IsChecked())
      compresslevel = 9;
    else if (BOCompression->FindChildItem(wxID_BOZ10Q, NULL)->IsChecked())
      compresslevel = 9, srchbestbsa = true, optimizequick = true;
    else if (BOCompression->FindChildItem(wxID_BOZ10T, NULL)->IsChecked())
      compresslevel = 9, srchbestbsa = true, optimizequick = false;
    /**/ if (BOCompression->FindChildItem(wxID_FORCE , NULL)->IsChecked())
      thresholdbsa = false;

    char cl[] = "0"; cl[0] = '0' + compresslevel;
    RegSetKeyValue(Settings, "Compression", "Level", RRF_RT_REG_SZ, cl, 2);
    RegSetKeyValue(Settings, "Compression", "Search", RRF_RT_REG_SZ, srchbestbsa ? "1" : "0", 2);
    RegSetKeyValue(Settings, "Compression", "Quick", RRF_RT_REG_SZ, optimizequick ? "1" : "0", 2);
    RegSetKeyValue(Settings, "Compression", "Force", RRF_RT_REG_SZ, thresholdbsa ? "0" : "1", 2);

    virtualbsabytes = 0;
    virtualbsafiles = 0;

    compressedinbytes = 0;
    compresseddtbytes = 0;
    compressedoubytes = 0;

    processedinbytes = 0;
    processedoubytes = 0;

    /* count again */
    iactives = 0;
    iomap::iterator walk = fdirectory.begin();
    while (walk != fdirectory.end()) {
      if (walk->second.iex)
	iactives += walk->second.selected ? 1 : 0;

      walk++;
    }

    /* initialize progress-bar(s) */
    prog->StartProgress(iactives + 1); iprogres = 0;
    prog->InitProgress("Copying file \"%s\":", "Efficiency:", 0, 1);

    /* if we want a log, open it */
    duplicates.clear();
    if (BOSettings->FindChildItem(wxID_LOGF, NULL)->IsChecked()) {
      wxString ph = BOOutText->GetValue();
      wxFileName log(ph); log.ClearExt(); log.SetExt("log");
      logfile = fopen(log.GetFullPath().data(), "wb");
    }

    try {
      Process(BOInText->GetValue().data(), BOOutText->GetValue().data(), "");
    }
    catch(exception &e) {
      if (strcmp(e.what(), "ExitThread")) {
	errprintf(stdout, e.what());

	wxMessageDialog d(prog, e.what(), "BSAopt error");
	d.ShowModal();
      }
      else {
	errprintf(stdout, "Canceled ...");
      }

      /* if we wanted a log, close it */
      if (logfile)
	fclose(logfile);

      prog->Leave(0);
      return;
    }

    /* if we wanted a log, close it */
    if (logfile) {
      summary(logfile, BOOutText->GetValue().data(), iactives);
      fclose(logfile);
    }

    prog->Leave(666);
  }

  bool RequestFeedback(const char *question) {
    wxMessageDialog d(prg ? (wxWindow *)prg : (wxWindow *)this, question, "BSAopt", wxOK | wxCANCEL | wxCENTRE);
    int ret = d.ShowModal();
    if (ret == wxID_CANCEL)
      return false;
    return true;
  }

public:
  BSAoptGUI::BSAoptGUI(const wxString& title)
    : wxBSAopt(NULL, wxID_ANY, title) {
    gui = this;
    warmup = true;

    Settings = 0;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Bethesda Softworks\\BSAopt", 0, KEY_READ | KEY_WRITE | KEY_WOW64_32KEY, &Settings) == ERROR_SUCCESS) {
    }
    else if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\Bethesda Softworks\\BSAopt", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE | KEY_WOW64_32KEY, NULL, &Settings, NULL) == ERROR_SUCCESS) {
    }
    
    char TS[1024]; DWORD TSL = 1023;
    TS[0] = 0; RegGetValue(Settings, NULL, "Filter", RRF_RT_REG_SZ, NULL, TS, &TSL);
    if (TS[0]) BOFilter->SetValue(TS); TSL = 1023;

    /* . -> \.
     * * -> .*
     */
    wxString spat = BOFilter->GetValue();
    wxString wildcard = spat;
    wildcard.Replace(".", "\\.", true);
    wildcard.Replace("*", ".*", true);

    if (!filter.Compile(wildcard))
      filter.Compile("");

    TS[0] = 0; RegGetValue(Settings, "Skip", "Existing", RRF_RT_REG_SZ, NULL, TS, &TSL);
    if (TS[0]) BOSettings->FindChildItem(wxID_SKIPE, NULL)->Check(TS[0] == '1'); TSL = 1023;
    TS[0] = 0; RegGetValue(Settings, "Skip", "Newer", RRF_RT_REG_SZ, NULL, TS, &TSL);
    if (TS[0]) BOSettings->FindChildItem(wxID_SKIPN, NULL)->Check(TS[0] == '1'); TSL = 1023;
    TS[0] = 0; RegGetValue(Settings, "Skip", "Hidden", RRF_RT_REG_SZ, NULL, TS, &TSL);
    if (TS[0]) BOSettings->FindChildItem(wxID_SKIPH, NULL)->Check(TS[0] == '1'); TSL = 1023;
    TS[0] = 0; RegGetValue(Settings, "Skip", "Hash", RRF_RT_REG_SZ, NULL, TS, &TSL);
    if (TS[0]) BOSettings->FindChildItem(wxID_SKIPC, NULL)->Check(TS[0] == '1'); TSL = 1023; skiphashcheck = (TS[0] == '1');
    TS[0] = 0; RegGetValue(Settings, "Skip", "Broken", RRF_RT_REG_SZ, NULL, TS, &TSL);
    if (TS[0]) BOSettings->FindChildItem(wxID_SKIPB, NULL)->Check(TS[0] == '1'); TSL = 1023;
    TS[0] = 0; RegGetValue(Settings, "Unselect", "Extras", RRF_RT_REG_SZ, NULL, TS, &TSL);
    if (TS[0]) BOSettings->FindChildItem(wxID_SKIPX, NULL)->Check(TS[0] == '1'); TSL = 1023;
    TS[0] = 0; RegGetValue(Settings, NULL, "Logging", RRF_RT_REG_SZ, NULL, TS, &TSL);
    if (TS[0]) BOSettings->FindChildItem(wxID_LOGF, NULL)->Check(TS[0] == '1'); TSL = 1023; dropextras = (TS[0] == '1');

    TS[0] = 0; RegGetValue(Settings, NULL, "Show Recursive", RRF_RT_REG_SZ, NULL, TS, &TSL);
    if (TS[0]) BORecursive->SetValue(TS[0] == '1'); TSL = 1023;
    TS[0] = 0; RegGetValue(Settings, NULL, "Game", RRF_RT_REG_SZ, NULL, TS, &TSL);
    switch (TS[0]) {
      case '0': BOGame->FindChildItem(wxID_AUTO   , NULL)->Check(true); break;
      case '1': BOGame->FindChildItem(wxID_OBLIVON, NULL)->Check(true); break;
      case '2': BOGame->FindChildItem(wxID_FALLOUT, NULL)->Check(true); break;
      case '3': BOGame->FindChildItem(wxID_SKYRIM , NULL)->Check(true); break;
    } TSL = 1023;
    TS[0] = 0; RegGetValue(Settings, "Compression", "Force", RRF_RT_REG_SZ, NULL, TS, &TSL);
    if (TS[0]) BOCompression->FindChildItem(wxID_FORCE, NULL)->Check(TS[0] == '1'); TSL = 1023;
    TS[0] = 0; RegGetValue(Settings, "Compression", "Level", RRF_RT_REG_SZ, NULL, TS, &TSL);
    switch (TS[0]) {
      case '0': BOCompression->FindChildItem(wxID_BOZ0, NULL)->Check(true); break;
      case '1': BOCompression->FindChildItem(wxID_BOZ1, NULL)->Check(true); break;
      case '2': BOCompression->FindChildItem(wxID_BOZ2, NULL)->Check(true); break;
      case '3': BOCompression->FindChildItem(wxID_BOZ3, NULL)->Check(true); break;
      case '4': BOCompression->FindChildItem(wxID_BOZ4, NULL)->Check(true); break;
      case '5': BOCompression->FindChildItem(wxID_BOZ5, NULL)->Check(true); break;
      case '6': BOCompression->FindChildItem(wxID_BOZ6, NULL)->Check(true); break;
      case '7': BOCompression->FindChildItem(wxID_BOZ7, NULL)->Check(true); break;
      case '8': BOCompression->FindChildItem(wxID_BOZ8, NULL)->Check(true); break;
      case '9': BOCompression->FindChildItem(wxID_BOZ9, NULL)->Check(true); TSL = 1023;
	TS[0] = 0; RegGetValue(Settings, "Compression", "Search", RRF_RT_REG_SZ, NULL, TS, &TSL);
	if (TS[0]) BOCompression->FindChildItem(wxID_BOZ10T, NULL)->Check(TS[0] == '1'); TSL = 1023;
	TS[0] = 0; RegGetValue(Settings, "Compression", "Quick", RRF_RT_REG_SZ, NULL, TS, &TSL);
	if (TS[0]) BOCompression->FindChildItem(wxID_BOZ10Q, NULL)->Check(TS[0] == '1'); TSL = 1023;
	break;
    } TSL = 1023;
    TS[0] = 0; RegGetValue(Settings, NULL, "Input Location", RRF_RT_REG_SZ, NULL, TS, &TSL);
    if (TS[0]) BOInText->SetValue(TS); TSL = 1023;
    TS[0] = 0; RegGetValue(Settings, NULL, "Output Location", RRF_RT_REG_SZ, NULL, TS, &TSL);
    if (TS[0]) BOOutText->SetValue(TS); TSL = 1023;

    warmup = false;

//  BrowseIn(BOInText->GetValue());
//  BrowseOut(BOOutText->GetValue());

    ResetHButtons();
  }

  BSAoptGUI::~BSAoptGUI() {
    if (Settings)
      RegCloseKey(Settings);
  }
};

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// Define a new application type, each program should derive a class from wxApp
class BSAoptApp : public wxApp
{
  BSAoptGUI *frame;

public:
  // override base class virtuals
  // ----------------------------

  // this one is called on application startup and is a good place for the app
  // initialization (doing it here and not in the ctor allows to have an error
  // return: if OnInit() returns false, the application terminates)
  virtual bool OnInit();
  virtual int OnExit();
};

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. BSAoptApp and
// not wxApp)
IMPLEMENT_APP_NO_MAIN(BSAoptApp)

extern "C" int WINAPI WinMain(HINSTANCE hInstance,
			      HINSTANCE hPrevInstance,
			      wxCmdLineArgType lpCmdLine,
			      int nCmdShow)
{
  ioinit();

  int ret = wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

  ioexit();

  return ret;
}

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool BSAoptApp::OnInit()
{
  // call the base class initialization method, currently it only parses a
  // few common command-line options but it could be do more in the future
  if ( !wxApp::OnInit() )
    return false;

  // create the main application window
  frame = new BSAoptGUI(_T("BSAopt"));

  // and show it (the frames, unlike simple controls, are not shown when
  // created initially)
  frame->Show(true);
//frame->Refresh();
//frame->Update();
  frame->DirectoryFromFiles(0);

  // success: wxApp::OnRun() will be called which will enter the main message
  // loop and the application will run. If we returned false here, the
  // application would exit immediately.
  return true;
}

int BSAoptApp::OnExit()
{
  return wxApp::OnExit();
}

/* --------------------------------------------------------------
 */

void InitProgress(const char *patterna, const char *patternb, int dne, int rng) {
  if (prg)
    prg->InitProgress(patterna, patternb, dne, rng);
}

void SetProgress(const char *str, int dne) {
  if (prg)
    prg->SetProgress(str, dne);
}

void SetProgress(size_t din, size_t dou) {
  if (prg)
    prg->SetProgress(din, dou);
}

void SetTopic(const char *topic) {
  if (prg)
    prg->SetTopic(topic);
}

void SetReport(const char *status, size_t din, size_t dou) {
  if (prg)
    prg->SetReport(status, din, dou);
}

void SetReport(const char *status, size_t din, size_t dou, int dpl) {
  if (prg)
    prg->SetReport(status, din, dou, dpl);
}

bool RequestFeedback(const char *question) {
  if (gui)
    return gui->RequestFeedback(question);
  return false;
}

DWORD __stdcall ConversionStart(LPVOID lp) {
  if (gui)
    gui->ConversionStart();
  return 0;
}
