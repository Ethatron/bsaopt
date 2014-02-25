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

#define	_CRT_SECURE_NO_WARNINGS
#define	_CRT_NONSTDC_NO_DEPRECATE

#include <string.h>
#include <algorithm>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <vector>

using namespace std;

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

#include <fcntl.h>
#include <io.h>

// maximum mumber of lines the output console should have
static const WORD MAX_CONSOLE_LINES = 500;

bool IsInsideConsole() {
  CONSOLE_SCREEN_BUFFER_INFO coninfo;

  AttachConsole(ATTACH_PARENT_PROCESS);
  if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo))
    return false;

  // if cursor position is (0,0) then we were launched in a separate console
  return ((coninfo.dwCursorPosition.X) || (coninfo.dwCursorPosition.Y));
}

bool RedirectIOToConsole() {
  int hConHandle;
  long lStdHandle;
  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  FILE *fp;
  
  // we are in a console already
  if (IsInsideConsole())
    AttachConsole(ATTACH_PARENT_PROCESS);
  // allocate a console for this app
  else
    AllocConsole();

  // set the screen buffer to be big enough to let us scroll text
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
  
  coninfo.dwSize.Y = MAX_CONSOLE_LINES;

  SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

  // redirect unbuffered STDOUT to the console
  lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

  if (hConHandle > 0) {
    fp = _fdopen( hConHandle, "w" );
    *stdout = *fp;
  }

  setvbuf( stdout, NULL, _IONBF, 0 );

  // redirect unbuffered STDIN to the console
  lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

  if (hConHandle > 0) {
    fp = _fdopen( hConHandle, "r" );
    *stdin = *fp;
  }

  setvbuf( stdin, NULL, _IONBF, 0 );

  // redirect unbuffered STDERR to the console
  lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

  if (hConHandle > 0) {
    fp = _fdopen( hConHandle, "w" );
    *stderr = *fp;
  }

  setvbuf(stderr, NULL, _IONBF, 0);

  // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
  // point to console as well
  ios::sync_with_stdio();

  return true;
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

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "BSAopt.h"

DWORD __stdcall ConversionStart(LPVOID lp);

#ifdef XPSUPPORT
// RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "0", 2);
LONG RegSetKeyValue(
  HKEY    hKey,
  LPCSTR  lpSubKey,
  LPCSTR  lpValueName,
  DWORD   dwType,
  LPCVOID lpData,
  DWORD   cbData
) {
  LONG res;
  if (lpSubKey) {
    if ((res = RegCreateKey(
      hKey,
      lpSubKey,
      &hKey
    )) != ERROR_SUCCESS)
      return res;
  }

  res = RegSetValueEx(
    hKey,
    lpValueName,
    0,
    dwType,
    (const BYTE *)lpData,
    cbData
  );
  
  if (lpSubKey)
    RegCloseKey(hKey);

  return res;
}

LONG RegGetValue(
  HKEY    hKey,
  LPCSTR  lpSubKey,
  LPCSTR  lpValue,
  DWORD   dwFlags,
  LPDWORD pdwType,
  PVOID   pvData,
  LPDWORD pcbData
) {
  LONG res;
  if (lpSubKey) {
    if ((res = RegOpenKey(
      hKey,
      lpSubKey,
      &hKey
    )) != ERROR_SUCCESS)
      return res;
  }

  res = RegQueryValueEx(
    hKey,
    lpValue,
    0,
    pdwType,
    (LPBYTE)pvData,
    pcbData
  );
  
  if (lpSubKey)
    RegCloseKey(hKey);

  return res;
}
#endif

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

class ioio;
class iost;
typedef	map<string, ioio> iomap;
typedef	map<string, iost> stmap;

class ioio {
public:
  ioio() { selected = true; skip = iex = oex = false; }

  bool skip, selected;

  struct io::info in; bool iex;
  struct io::info ou; bool oex;

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
  iost() { imsk = omsk = 0; icnt = ocnt = 0; isze = osze = 0; }

  int imsk; size_t icnt; size_t isze;
  int omsk; size_t ocnt; size_t osze;

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
  wxEventType evtProgress;
  int idProgress;

  class ProgressEvent: public wxCommandEvent {
public:
    ProgressEvent(int id, const wxEventType& event_type) : wxCommandEvent(event_type, id) { memset(&state, 0, sizeof(state)); }
    ProgressEvent(const ProgressEvent& event) : wxCommandEvent(event) { memcpy(&state, &event.state, sizeof(state)); }

    wxEvent* Clone() const { return new ProgressEvent(*this); }

public:
    struct {
      unsigned int mask;

      int taskR; int taskV;
      int effcR; int effcV;
      char subjS[256];
      char reptS[256];
    } state;

    void SetTaskRange(int r) { state.mask |= 1; state.taskR = r; }
    void SetTaskValue(int v) { state.mask |= 2; state.taskV = v; }
    void SetEffcRange(int r) { state.mask |= 4; state.effcR = r; }
    void SetEffcValue(int v) { state.mask |= 8; state.effcV = v; }
    void SetSubject(const char *s) { state.mask |= 16; strcpy(state.subjS, s); }
    void SetReport(const char *s) { state.mask |= 32; strcpy(state.reptS, s); }
  };

  typedef void (wxEvtHandler::*ProgressEventFunction)(ProgressEvent &);

  /* called from Progress-thread */
  void Progress(ProgressEvent &evt) {
    if (evt.state.mask &  1) BOTask->SetRange(evt.state.taskR);
    if (evt.state.mask &  2) BOTask->SetValue(evt.state.taskV);
    if (evt.state.mask &  4) BOEfficiency->SetRange(evt.state.effcR);
    if (evt.state.mask &  8) BOEfficiency->SetValue(evt.state.effcV);
    if (evt.state.mask & 16) BOSubject->SetLabel(evt.state.subjS);
    if (evt.state.mask & 32) BOReport->SetLabel(evt.state.reptS);
  }

  const char *lastpa;
  const char *lastpb;

public:
  /* all executed by Async-thread */
  void StartProgress(int rng) {
    Wait(); ProgressEvent event(idProgress, evtProgress);

    event.SetTaskRange(rng);

    wxPostEvent(this, event);
  }

  void InitProgress(const char *patterna, const char *patternb, int dne, int rng) {
    Wait(); ProgressEvent event(idProgress, evtProgress);

    char tmp[256];

    if (patterna) { sprintf(tmp, lastpa = patterna, "..."); event.SetSubject(tmp); }
    if (patternb) { sprintf(tmp, lastpb = patternb, "..."); event.SetReport(tmp); }

    event.SetTaskValue(dne);
    event.SetEffcRange(rng);
    event.SetEffcValue(rng);

    wxPostEvent(this, event);
  }

  int range1, value1;
  int range2, value2;

  void SetProgress(const char *str, int dne) {
    Wait(); ProgressEvent event(idProgress, evtProgress);

    char tmp[256];

    if (lastpa) { sprintf(tmp, lastpa, str  ); event.SetSubject(tmp); }
//  if (lastpb) { sprintf(tmp, lastpb, "..."); event.SetReport(tmp); }

    event.SetTaskValue(dne);

    wxPostEvent(this, event);
  }

  void SetProgress(size_t din, size_t dou) {
    Wait(); ProgressEvent event(idProgress, evtProgress);

    int rng = (int)(din ? ((unsigned __int64)dou * 0xFFFF) / din : 0);

    event.SetEffcRange(0xFFFF);
    event.SetEffcValue(rng);

    wxPostEvent(this, event);
  }

  void SetTopic(const char *topic) {
    Wait(); ProgressEvent event(idProgress, evtProgress);

    event.SetSubject(topic);

    wxPostEvent(this, event);
  }

  void SetReport(const char *status, size_t din, size_t dou, int dpl = -1) {
    Wait(); ProgressEvent event(idProgress, evtProgress);

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

    event.SetReport(tmp);

    wxPostEvent(this, event);
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

#ifdef	W7SUPPORT
      wxString str = BOSubject->GetLabel();
      const char *inc = str.data();
      wchar_t tmp[256]; mbstowcs(tmp, inc, 256);

      if (ptl) ptl->SetProgressValue((HWND)GetParent()->GetHWND(), BOTask->GetValue(), BOTask->GetRange());
      if (ptl) ptl->SetThumbnailTooltip((HWND)GetParent()->GetHWND(), tmp);
#endif

      char buf[256]; int
      minutes = tpass / 60,
      seconds = tpass % 60,
      hours   = minutes / 60;
      minutes = minutes % 60;

      sprintf(buf, "%02d:%02d:%02ds", hours, minutes, seconds);
      BORunning->SetLabel(buf);

      if (tfinal != 0x80000000) {
      	minutes = trem / 60;
      	seconds = trem % 60;
      	hours   = minutes / 60;
      	minutes = minutes % 60;

      	sprintf(buf, "%02d:%02d:%02ds", hours, minutes, seconds);
	BORemaining->SetLabel(buf);

      	minutes = tfinal / 60;
      	seconds = tfinal % 60;
      	hours   = minutes / 60;
      	minutes = minutes % 60;

      	sprintf(buf, "%02d:%02d:%02ds", hours, minutes, seconds);
	BOTotal->SetLabel(buf);
      }
      else {
	BORemaining->SetLabel("00:00:00s");
	BOTotal->SetLabel(buf);
      }
    }

//  Sleep(500);
//  event.RequestMore();
  }

  void Stopped() {
#ifdef	W7SUPPORT
    if (ptl) ptl->SetProgressState((HWND)GetParent()->GetHWND(), TBPF_ERROR);
#endif
  }

  void Resumed() {
#ifdef	W7SUPPORT
    if (ptl) ptl->SetProgressState((HWND)GetParent()->GetHWND(), TBPF_NORMAL);
#endif
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
#ifdef	W7SUPPORT
    if (ptl) ptl->SetProgressState((HWND)GetParent()->GetHWND(), TBPF_PAUSED);
#endif

    ResetEvent(evt);
  }

  void Unblock() {
#ifdef	W7SUPPORT
    if (ptl) ptl->SetProgressState((HWND)GetParent()->GetHWND(), TBPF_NORMAL);
#endif

    SetEvent(evt);
  }

public:
#ifdef	W7SUPPORT
  ITaskbarList3 *ptl;
#endif

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

    /* rebase the created thread, and possibly spawned OpenMP ones (via process) */
    SetThreadPriority(async, THREAD_PRIORITY_BELOW_NORMAL);
    SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);

#ifdef	W7SUPPORT
    if (ptl) ptl->SetProgressState((HWND)GetParent()->GetHWND(), TBPF_NORMAL);
#endif

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

    evtProgress = wxNewEventType();
    idProgress = wxNewId();

    /* Connect to event handler that will make us close */
    Connect(wxID_ANY, evtProgress,
      (wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)wxStaticCastEvent(ProgressEventFunction, &BSAoptPrg::Progress),
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

#ifdef	W7SUPPORT
    ptl = NULL; CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3, (void **)&ptl);
#endif

    SetSize(600, 265);
  }

  BSAoptPrg::~BSAoptPrg() {
    Disconnect(wxID_ANY, evtLeave,
      (wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)wxStaticCastEvent(LeaveEventFunction, &BSAoptPrg::Leave),
      NULL,
      this);
    Disconnect(wxID_ANY, evtProgress,
      (wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)wxStaticCastEvent(ProgressEventFunction, &BSAoptPrg::Progress),
      NULL,
      this);

    CloseHandle(evt);
    CloseHandle(end);

#ifdef	W7SUPPORT
    if (ptl) ptl->SetProgressState((HWND)GetParent()->GetHWND(), TBPF_NOPROGRESS);
    if (ptl) ptl->SetThumbnailTooltip((HWND)GetParent()->GetHWND(), L"Ready");
    if (ptl) ptl->Release();
#endif
  }
};

// ----------------------------------------------------------------------------
class BSAoptGUI; class BSAoptGUI *gui;
class BSAoptGUI : public wxBSAopt
{
  friend class BSAoptApp;

public:
  bool izip    , ozip    ;
  bool iarchive, oarchive;
  int  iversion, oversion;
  int  iactives, oactives;
  int  iprogres, oprogres;
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
      (IPath[0] != '\0') &&
      (OPath[0] != '\0')
    );
    
    try {
      izip     = iszip    (IPath);
      ozip     = iszip    (OPath);
      iarchive = isarchive(IPath, &iversion);
      oarchive = isarchive(OPath, &oversion);
    }
    catch(exception &e) {
      if (strcmp(e.what(), "ExitThread")) {
	wxMessageDialog d(this, e.what(), "BSAopt error");
	d.ShowModal();
      }
    }

    /**/ if ( iarchive &&  oarchive)
      BOConvert->SetLabel("Convert");
    else if (!iarchive && !oarchive)
      BOConvert->SetLabel("Copy");
    else if ( iarchive && !oarchive)
      BOConvert->SetLabel("Unpack");
    else if (!iarchive &&  oarchive)
      BOConvert->SetLabel("Pack");
    
    /**/ if (iarchive && (iversion == MW_BSAHEADER_VERSION))
      BOTypeIn->SetBitmap(wxBitmap( wxT("#117"), wxBITMAP_TYPE_RESOURCE ));
    else if (iarchive && (iversion == OB_BSAHEADER_VERSION))
      BOTypeIn->SetBitmap(wxBitmap( wxT("#118"), wxBITMAP_TYPE_RESOURCE ));
    else if (iarchive && (iversion == F3_BSAHEADER_VERSION) && (BOGame->FindChildItem(wxID_FALLOUT, NULL)->IsChecked() || BOGame->FindChildItem(wxID_FALLOUTXB, NULL)->IsChecked()))
      BOTypeIn->SetBitmap(wxBitmap( wxT("#120"), wxBITMAP_TYPE_RESOURCE ));
    else if (iarchive && (iversion == SK_BSAHEADER_VERSION) && (!BOGame->FindChildItem(wxID_FALLOUT, NULL)->IsChecked() && !BOGame->FindChildItem(wxID_FALLOUTXB, NULL)->IsChecked()))
      BOTypeIn->SetBitmap(wxBitmap( wxT("#115"), wxBITMAP_TYPE_RESOURCE ));
    else if (izip)
      BOTypeIn->SetBitmap(wxBitmap( wxT("#120"), wxBITMAP_TYPE_RESOURCE ));
    else
      BOTypeIn->SetBitmap(wxBitmap( wxT("#116"), wxBITMAP_TYPE_RESOURCE ));
    
    /**/ if (oarchive && (oversion == MW_BSAHEADER_VERSION))
      BOTypeOut->SetBitmap(wxBitmap( wxT("#117"), wxBITMAP_TYPE_RESOURCE ));
    else if (oarchive && (oversion == OB_BSAHEADER_VERSION))
      BOTypeOut->SetBitmap(wxBitmap( wxT("#118"), wxBITMAP_TYPE_RESOURCE ));
    else if (oarchive && (oversion == F3_BSAHEADER_VERSION) && (BOGame->FindChildItem(wxID_FALLOUT, NULL)->IsChecked() || BOGame->FindChildItem(wxID_FALLOUTXB, NULL)->IsChecked()))
      BOTypeOut->SetBitmap(wxBitmap( wxT("#120"), wxBITMAP_TYPE_RESOURCE ));
    else if (oarchive && (oversion == SK_BSAHEADER_VERSION) && (!BOGame->FindChildItem(wxID_FALLOUT, NULL)->IsChecked() && !BOGame->FindChildItem(wxID_FALLOUTXB, NULL)->IsChecked()))
      BOTypeOut->SetBitmap(wxBitmap( wxT("#115"), wxBITMAP_TYPE_RESOURCE ));
    else if (ozip)
      BOTypeOut->SetBitmap(wxBitmap( wxT("#120"), wxBITMAP_TYPE_RESOURCE ));
    else
      BOTypeOut->SetBitmap(wxBitmap( wxT("#116"), wxBITMAP_TYPE_RESOURCE ));

    BOTypeIn->Refresh();
    BOTypeOut->Refresh();
  }

  /* ---------------------------------------------------------------------------- */
  void ChangeToAuto(wxCommandEvent& event) {
    RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "0", 2);
  }
  void ChangeToMorrowind(wxCommandEvent& event) {
    RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "4", 2);
  }
  void ChangeToOblivion(wxCommandEvent& event) {
    RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "1", 2);
  }
  void ChangeToFallout(wxCommandEvent& event) {
    RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "2", 2);
  }
  void ChangeToFalloutXB(wxCommandEvent& event) {
    RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "5", 2);
  }
  void ChangeToSkyrim(wxCommandEvent& event) {
    RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "3", 2);
  }
  void ChangeToSkyrimXB(wxCommandEvent& event) {
    RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "6", 2);
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

      if (BOArchiveTree->HasChildren(item)) {
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

      if (BOArchiveTree->HasChildren(item)) {
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

  wxString FormatSize(size_t size) {
    static NUMBERFMT nfmt;
    static char s[8], t[8];
    static bool localed = false;

    if (!localed) {
      GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, s, 8);
      GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, t, 8);
    
      nfmt.NumDigits = 0;
      nfmt.LeadingZero = 0;
      nfmt.Grouping = 3;
      nfmt.lpDecimalSep = s;
      nfmt.lpThousandSep = t;
      nfmt.NegativeOrder = 0;
    }

    char number[64];
    char thousands[64];
    
    sprintf(number, "%u", size);

    GetNumberFormat(NULL/*LOCALE_NAME_USER_DEFAULT*/, 0, number, &nfmt, thousands, 64);

//  return wxString::Format(wxT("%i"), size);
    return wxString(thousands);
  }

#define OEX 1
#define IEX 2
#define CNT 4
  int Scan(const char *name, const char *locl, const wxTreeItemId &parent, size_t &accus, size_t &accuc, int lvl, bool o) {
    size_t ilen = (name ? strlen(name) : 0);
    size_t llen = (locl ? strlen(locl) : 0);
    string lut = locl; std::transform(lut.begin(), lut.end(), lut.begin(), ::tolower);
    int msk = 0;

    struct io::info info;
    if (!io::stat(name, &info)) {
      /* input: directory */
      if (info.io_type & IO_DIRECTORY) {
	const char *dle;
	char tmp[256];

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
	class iost *stat = &ddirectory[lut];
	size_t size = 0, num = 0;

	/* walk */
	struct io::dir *dir;
	if ((dir = io::opendir(name))) {
	  struct io::dirent *et;

	  while ((et = io::readdir(dir))) {
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

	    /* recurse */
	    int lmsk = Scan(nname, lname, entry, size, num, lvl + 1, o);
	    
	    /* break down the field */
	    msk = ((lmsk & ~3) + (msk & ~3)) | (lmsk & 3) | (msk & 3);

	    free(nname);
	    free(lname);
	  }

	  io::closedir(dir);

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
	
	accus += size;
	accuc += num;

	if (o) {
	  stat->osze = stat->isze + size;
	  stat->ocnt = stat->icnt + num;

	  BOArchiveTree->SetItemText(entry, 1, wxString::Format(wxT("%i"), stat->ocnt));
	  BOArchiveTree->SetItemText(entry, 2, FormatSize(stat->osze));
	}
	else {
	  stat->isze = size;
	  stat->icnt = num;

	  BOArchiveTree->SetItemText(entry, 1, wxString::Format(wxT("%i"), stat->icnt));
	  BOArchiveTree->SetItemText(entry, 2, FormatSize(stat->isze));
	}
      }
      /* input: file */
      else {
	const char *fle;
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

	bool firsttime = (fdirectory.count(lut) == 0);
	class ioio *nfo = &fdirectory[lut];
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

	  accus += nfo->ou.io_size - (nfo->iex ? nfo->in.io_size : 0);
	  accuc += 1 - nfo->iex;
	}
	else {
	  nfo->iex = true;
	  nfo->in = info;

	  iactives += (nfo->selected ? 1 : 0);

	  accus += nfo->in.io_size;
	  accuc += 1;
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

//  BOInText ->SetValue(IPath);
//  BOOutText->SetValue(OPath);

    skipexisting  = BOSettings->FindChildItem(wxID_SKIPE, NULL)->IsChecked();
    skipnewer     = BOSettings->FindChildItem(wxID_SKIPN, NULL)->IsChecked();
    skiphashcheck = BOSettings->FindChildItem(wxID_SKIPC, NULL)->IsChecked();
    skipbroken    = BOSettings->FindChildItem(wxID_SKIPB, NULL)->IsChecked();
    processhidden = BOSettings->FindChildItem(wxID_SKIPH, NULL)->IsChecked();

    wxBusyCursor busy;
    wxTreeItemId root;
    
    izip     = iszip    (BOInText ->GetValue().data());
    ozip     = iszip    (BOOutText->GetValue().data());
    iarchive = isarchive(BOInText ->GetValue().data(), &iversion);
    oarchive = isarchive(BOOutText->GetValue().data(), &oversion);

    if (!io) {
      iactives =
      oactives = 0;

      ddirectory.clear();
      fdirectory.clear();
    }
    else if (io == 1) {
      iactives = 0;

      {
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

      {
	stmap newmap;
	stmap::iterator walk = ddirectory.begin();
	while (walk != ddirectory.end()) {
	  if (walk->second.omsk & OEX) {
	    iost *m = &newmap[walk->first];

	    m->omsk = walk->second.omsk;
	  }

	  walk++;
	}

	ddirectory = newmap;
      }
    }
    else if (io == 2) {
      oactives = 0;

      {
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

      {
	stmap newmap;
	stmap::iterator walk = ddirectory.begin();
	while (walk != ddirectory.end()) {
	  if (walk->second.imsk & IEX) {
	    iost *m = &newmap[walk->first];

	    m->imsk = walk->second.imsk;
	  }

	  walk++;
	}

	ddirectory = newmap;
      }
    }

    BOArchiveList->DeleteAllItems();
    BOArchiveTree->DeleteRoot(); root =
    BOArchiveTree->AddRoot("\\");

    try {
      size_t size = 0, num = 0;

//    if (!io || (io == 1)) {
	BOStatusBar->SetStatusText(wxT("Skimming input ..."), 0);
	Scan(BOInText->GetValue().data(), "", root, size, num, 0, false);
//    }

//    if (!io || (io == 2)) {
	BOStatusBar->SetStatusText(wxT("Skimming output ..."), 0);
	Scan(BOOutText->GetValue().data(), "", root, size, num, 0, true);
//    }

      BOArchiveTree->SetItemText(root, 1, wxString::Format(wxT("%i"), num));
      BOArchiveTree->SetItemText(root, 2, FormatSize(size));
      BOArchiveTree->SortChildren(root);
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
    BOArchiveTree->SelectItem(root);

    /* refresh file-list to preselected root */
    ChangeTreeItem(currentpath = "");

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
      RegSetKeyValue(Settings, NULL, "Filter", RRF_RT_REG_SZ, spat.data(), (DWORD)spat.length() + 1);

    /* refresh file-list */
    ChangeTreeItem(currentpath);
  }

  void ResetFileList(wxCommandEvent& event) {
//  ResetCFileList();

    /* refresh file-list */
    ChangeTreeItem(currentpath, 1);
  }

  void ClearFileList(wxCommandEvent& event) {
//  ClearCFileList();

    /* refresh file-list */
    ChangeTreeItem(currentpath, 2);
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
    ChangeTreeItem(currentpath);
  }

  void ChangeSkipNewer(wxCommandEvent& event) {
    RegSetKeyValue(Settings, "Skip", "Newer", RRF_RT_REG_SZ, event.IsChecked() ? "1" : "0", 2);

    /* refresh file-list */
    ChangeTreeItem(currentpath);
  }

  void ChangeSkipHidden(wxCommandEvent& event) {
    RegSetKeyValue(Settings, "Skip", "Hidden", RRF_RT_REG_SZ, event.IsChecked() ? "1" : "0", 2);

    /* refresh file-list */
    ChangeTreeItem(currentpath);
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
    ChangeTreeItem(currentpath);
  }
  
#define wxID_IN	  1001
#define wxID_OUT  1002
#define wxID_BOTH 1003
  void ChangeTreeItemRoot(wxCommandEvent& event) {
    ioTreeItemData *iod = (mitem.IsOk() ? (ioTreeItemData *)BOArchiveTree->GetItemData(mitem) : NULL);
    
    if ((event.GetId() == wxID_IN) || (event.GetId() == wxID_BOTH)) {
      string fullipath = IPath; fullipath += iod->fullpath;

      BOInText->SetValue(fullipath);

      BrowseIn(fullipath);
    }
    
    if ((event.GetId() == wxID_OUT) || (event.GetId() == wxID_BOTH)) {
      string fullopath = OPath; fullopath += iod->fullpath;

      BOOutText->SetValue(fullopath);

      BrowseOut(fullopath);
    }
  }

  wxTreeItemId mitem;
  void MenuTreeItem(wxTreeEvent& event) {
    mitem = event.GetItem();

    ioTreeItemData *iod = (mitem.IsOk() ? (ioTreeItemData *)BOArchiveTree->GetItemData(mitem) : NULL);
    if (iod && (iod->fullpath != "")) {
      iost node = ddirectory[iod->fullpath];

      menu->Enable(wxID_BOTH, node.icnt && node.ocnt);
      menu->Enable(wxID_IN, node.icnt != 0);
      menu->Enable(wxID_OUT, node.ocnt != 0);
	
      // and then display
      PopupMenu(menu);
    }
  }

  void ChangeTreeItem(wxTreeEvent& event) {
    wxTreeItemId item = event.GetItem();
    ioTreeItemData *iod = (item.IsOk() ? (ioTreeItemData *)BOArchiveTree->GetItemData(item) : NULL);
    string updatedpath = (iod ? iod->fullpath : "");

    if (currentpath == updatedpath)
      return;
    currentpath = updatedpath;

    /* refresh file-list */
    ChangeTreeItem(currentpath);
  }

  void ChangeTreeItem(string &basedir, int op = 0) {
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
	BOArchiveList->Check((int)n, walk->second.selected && walk->second.iex);
      }

      return;
    }

    wxFont fnt = BOArchiveList->GetFont();
    const wxNativeFontInfo *fni = fnt.GetNativeFontInfo();
    wxNativeFontInfo sni = *fni; sni.lf.lfStrikeOut = TRUE;
    wxNativeFontInfo ini = *fni; ini.lf.lfItalic    = TRUE;
    wxNativeFontInfo jni = *fni; jni.lf.lfStrikeOut = TRUE;
				 jni.lf.lfItalic    = TRUE;
    wxFont str = fnt; str.SetNativeFontInfo(sni);
    wxFont itr = fnt; itr.SetNativeFontInfo(ini);
    wxFont jtr = fnt; jtr.SetNativeFontInfo(jni);

    skipexisting  = BOSettings->FindChildItem(wxID_SKIPE, NULL)->IsChecked();
    skipnewer     = BOSettings->FindChildItem(wxID_SKIPN, NULL)->IsChecked();
    skiphashcheck = BOSettings->FindChildItem(wxID_SKIPC, NULL)->IsChecked();
    skipbroken    = BOSettings->FindChildItem(wxID_SKIPB, NULL)->IsChecked();
    processhidden = BOSettings->FindChildItem(wxID_SKIPH, NULL)->IsChecked();

    BOArchiveList->DeleteAllItems(); int listcursor = 0;
/*  If we don't want root selectable do this: if (!basedir[0]) return; */

    BOStatusBar->SetStatusText(wxT("Refreshing file-list ..."), 0);
    ldirectory.clear();
    iomap::iterator walk = fdirectory.begin();
    while (walk != fdirectory.end()) {
      const char *fname = walk->first.data();

      if ((fname == stristr(fname, basedir.data()))) {
	const char *fbase = fname + basedir.size() + 1;
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
	  BOArchiveList->InsertItem(listcursor++, fbase);
	  BOArchiveList->Check(n, walk->second.selected && walk->second.iex);

	  /* client-data substitute */
	  if (n >= ldirectory.size())
	    ldirectory.resize(n + 1);
	  ldirectory[n] = walk;

	  /* mark destination-only */
	  BOArchiveList->Enable(n, walk->second.iex);

	  wxColour col(0, 0, 0, 255);
	  if (!walk->second.iex)
	    col = wxColour(140, 140, 140, 255);
	  else if (!walk->second.oex)
	    col = wxColour(0, 60, 0, 255);

	  /* mark no-overwrite */
	  if ((walk->second.iex && walk->second.oex) && (skipexisting || (skipnewer &&
	       (walk->second.in.io_time <= walk->second.ou.io_time)))) {
	    if (walk->second.in.io_size != walk->second.in.io_raws)
	      BOArchiveList->SetItemFont(n, jtr);
	    else
	      BOArchiveList->SetItemFont(n, str);
	    
	    BOArchiveList->SetItem(n, 1, FormatSize(walk->second.in.io_size) + " ");
	  }
	  /* mark compressed */
	  else if (walk->second.iex) {
	    if (walk->second.in.io_size != walk->second.in.io_raws)
	      BOArchiveList->SetItemFont(n, itr);

	    BOArchiveList->SetItem(n, 1, FormatSize(walk->second.in.io_size) + " ");
	  }
	  else if (walk->second.oex) {
	    if (walk->second.ou.io_size != walk->second.ou.io_raws)
	      BOArchiveList->SetItemFont(n, itr);

	    BOArchiveList->SetItem(n, 1, FormatSize(walk->second.ou.io_size) + " ");
	  }

	  BOArchiveList->SetItemTextColour(n, col);
	}
      }

      walk++;
    }

    BOStatusBar->SetStatusText(wxT("Ready"), 0);
  }

  virtual void ChangeSelectedFiles(wxListEvent& event) {
    int n = event.GetIndex();
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
  void TypedIn(wxCommandEvent& event) { return;
    wxString ph = event.GetString();
//  BOInText->SetValue(ph);

    if (ph.IsNull())
      return;
    /* does it exist? */
    if (io::size(ph.data()) == -1)
      return;

    BrowseIn(ph);
  }

  void TypedInDone(wxFocusEvent& event) {
    wxString ph = BOInText->GetValue();

    if (ph.IsNull())
      return;
    /* does it exist? */
    if (io::size(ph.data()) == -1)
      return;

    if (stricmp(IPath, ph.data()))
      BrowseIn(ph);
  }

  void BrowseIn(wxCommandEvent& event) {
    wxBusyCursor wait;
    HRESULT hr = AskInput(IPath);
    if (!SUCCEEDED(hr))
      return;

    wxString ph = selected_string;
    if (ph.IsNull())
      return;

    BOInText->SetValue(ph);
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
  void TypedOut(wxCommandEvent& event) { return;
    wxString ph = event.GetString();
//  BOOutText->SetValue(ph);

    if (ph.IsNull())
      return;

    BrowseOut(ph);
  }

  void TypedOutDone(wxFocusEvent& event) {
    wxString ph = BOOutText->GetValue();

    if (ph.IsNull())
      return;

    if (stricmp(OPath, ph.data()))
      BrowseOut(ph);
  }

  void BrowseOut(wxCommandEvent& event) {
    wxBusyCursor wait;
    HRESULT hr = AskOutput(OPath);
    if (!SUCCEEDED(hr))
      return;

    wxString ph = selected_string;
    if (ph.IsNull())
      return;

    BOOutText->SetValue(ph);
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
    /* does it exist?
    if (iosize(ph.data()) == -1)
      return; */

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
    io::flush();
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
    string lut = lcname; std::transform(lut.begin(), lut.end(), lut.begin(), ::tolower);

    bool rm = false;
    struct io::info iinfo, oinfo;
    if (!io::stat(inname, &iinfo)) {
      /* input: directory */
      if (iinfo.io_type & IO_DIRECTORY) {
	/* don't create output in simulation-mode */
	if (ouname && !simulation && io::stat(ouname, &oinfo)) {
	  rm = true;

	  if (io::mkdir(ouname)) {
	    sprintf(rerror, "Can't create the directory \"%s\"\n", ouname);
	    throw runtime_error(rerror);
	  }

	  if (io::stat(ouname, &oinfo)) {
	    sprintf(rerror, "Can't find the directory \"%s\"\n", ouname);
	    throw runtime_error(rerror);
	  }
	}

	if ((oinfo.io_type & IO_DIRECTORY) || simulation) {
	  struct io::dir *dir;

	  /* no BSAs inside BSAs! */
	  if (oarchive && isext(lcname, "bsa"))
	    return;

	  if ((dir = io::opendir(inname))) {
	    struct io::dirent *et;

	    while ((et = io::readdir(dir))) {
	      if (!strcmp(et->name, ".") ||
		  !strcmp(et->name, ".."))
		continue;

	      char *ninname = (char *)malloc(ilen + 1 + et->namelength + 1);
	      char *nouname = (char *)malloc(olen + 1 + et->namelength + 1);
	      char *nlcname = (char *)malloc(llen + 1 + et->namelength + 1);

	      strcpy(ninname, inname);
	      strcpy(nouname, ouname ? ouname : inname);
	      strcpy(nlcname, lcname);

	      strcat(ninname, "\\");
	      strcat(nouname, "\\");
	      strcat(nlcname, "\\");

	      strcat(ninname, et->name);
	      strcat(nouname, et->name);
	      strcat(nlcname, et->name);

	      Process(ninname, nouname, nlcname);

	      free(ninname);
	      free(nouname);
	      free(nlcname);
	    }

	    io::closedir(dir);

	    /* don't create output in simulation-mode */
	    if (ouname && !simulation && (dir = io::opendir(ouname))) {
	      struct io::dirent *et;
	      int num = 0;

	      while ((et = io::readdir(dir))) {
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

	      io::closedir(dir);

	      if (!num && rm)
		io::rmdir(ouname);
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
	struct io::dir *dir;
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
	if (!fdirectory[lut].selected)
	  return;

	if (!docopy)
	  prog->SetProgress(lcname, iprogres++);
	else
	  prog->SetProgress(lcname, iprogres++);

	/* if there is no destination, clone the source */
	if (ouname && !io::stat(ouname, &oinfo)) {
	  if (oinfo.io_type & IO_DIRECTORY) {
	    char *nouname = (char *)malloc(olen + 1 + strlen(fle) + 1);

	    /* if we push a single specific file into a BSA
	     * we have to open/close it to flush the BSA to disk
	     */
	    dir = io::opendir(ouname);

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
		io::cp(inname, ouname);
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
	  io::closedir(dir);
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
    verbose = true;

    gameversion = -1;
    /**/ if (BOGame->FindChildItem(wxID_MORROWIND, NULL)->IsChecked())
      gameversion = MW_BSAHEADER_VERSION, RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "4", 2);
    else if (BOGame->FindChildItem(wxID_OBLIVON, NULL)->IsChecked())
      gameversion = OB_BSAHEADER_VERSION, RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "1", 2);
    else if (BOGame->FindChildItem(wxID_FALLOUT, NULL)->IsChecked())
      gameversion = F3_BSAHEADER_VERSION, RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "2", 2);
    else if (BOGame->FindChildItem(wxID_FALLOUTXB, NULL)->IsChecked())
      gameversion = FX_BSAHEADER_VERSION, RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "5", 2);
    else if (BOGame->FindChildItem(wxID_SKYRIM, NULL)->IsChecked())
      gameversion = SK_BSAHEADER_VERSION, RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "3", 2);
    else if (BOGame->FindChildItem(wxID_SKYRIMXB , NULL)->IsChecked())
      gameversion = SX_BSAHEADER_VERSION, RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "6", 2);
    else
					  RegSetKeyValue(Settings, NULL, "Game", RRF_RT_REG_SZ, "0", 2);

    srchbestbsa = false;
    thresholdbsa = true;
    /**/ if (BOCompression->FindChildItem(wxID_BOZ0, NULL)->IsChecked())
      compresslevel = 0, compressbsa = false;
    else if (BOCompression->FindChildItem(wxID_BOZ1, NULL)->IsChecked())
      compresslevel = 1, compressbsa = true;
    else if (BOCompression->FindChildItem(wxID_BOZ2, NULL)->IsChecked())
      compresslevel = 2, compressbsa = true;
    else if (BOCompression->FindChildItem(wxID_BOZ3, NULL)->IsChecked())
      compresslevel = 3, compressbsa = true;
    else if (BOCompression->FindChildItem(wxID_BOZ4, NULL)->IsChecked())
      compresslevel = 4, compressbsa = true;
    else if (BOCompression->FindChildItem(wxID_BOZ5, NULL)->IsChecked())
      compresslevel = 5, compressbsa = true;
    else if (BOCompression->FindChildItem(wxID_BOZ6, NULL)->IsChecked())
      compresslevel = 6, compressbsa = true;
    else if (BOCompression->FindChildItem(wxID_BOZ7, NULL)->IsChecked())
      compresslevel = 7, compressbsa = true;
    else if (BOCompression->FindChildItem(wxID_BOZ8, NULL)->IsChecked())
      compresslevel = 8, compressbsa = true;
    else if (BOCompression->FindChildItem(wxID_BOZ9, NULL)->IsChecked())
      compresslevel = 9, compressbsa = true;
    else if (BOCompression->FindChildItem(wxID_BOZ10Q, NULL)->IsChecked())
      compresslevel = 9, compressbsa = true, srchbestbsa = true, optimizequick = true;
    else if (BOCompression->FindChildItem(wxID_BOZ10T, NULL)->IsChecked())
      compresslevel = 9, compressbsa = true, srchbestbsa = true, optimizequick = false;
    /**/ if (BOCompression->FindChildItem(wxID_FORCE , NULL)->IsChecked())
      thresholdbsa = false;
    
    char cl[] = "0"; cl[0] = '0' + compresslevel;
    RegSetKeyValue(Settings, "Compression", "Level", RRF_RT_REG_SZ, cl, 2);
    RegSetKeyValue(Settings, "Compression", "Search", RRF_RT_REG_SZ, srchbestbsa ? "1" : "0", 2);
    RegSetKeyValue(Settings, "Compression", "Quick", RRF_RT_REG_SZ, optimizequick ? "1" : "0", 2);
    RegSetKeyValue(Settings, "Compression", "Force", RRF_RT_REG_SZ, thresholdbsa ? "0" : "1", 2);
    
    if (gameversion == MW_BSAHEADER_VERSION)
      compressbsa = false;

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
      logfile = fopen(log.GetFullPath().data(), (skipexisting || skipnewer ? "ab" : "wb"));
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

  void SetCommandLine(const char *in, const char *ou) {
    warmup = true;

    if (in) BOInText->SetValue(in);
    if (ou) BOOutText->SetValue(ou);
    
    if (in) strcpy(IPath, in);
    if (ou) strcpy(OPath, ou);

    warmup = false;

    ResetHButtons();
  }

public:	
  wxMenu *menu;

  BSAoptGUI::BSAoptGUI(const wxString& title)
    : wxBSAopt(NULL, wxID_ANY, title) {

    menu = new wxMenu();
    menu->SetInvokingWindow(this);
    menu->Append(wxID_BOTH, wxT("Make root of both"), wxT(""));
    menu->Append(wxID_IN, wxT("Make root of input"), wxT(""));
    menu->Append(wxID_OUT, wxT("Make root of output"), wxT(""));
    menu->Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(BSAoptGUI::ChangeTreeItemRoot), NULL, this);

    BOArchiveTree->SetIndent(BOArchiveTree->GetIndent() * 2);
    BOArchiveList->Connect(wxEVT_COMMAND_LIST_ITEM_CHECKED, wxListEventHandler(BSAoptGUI::ChangeSelectedFiles), NULL, this);
    BOArchiveList->Connect(wxEVT_COMMAND_LIST_ITEM_UNCHECKED, wxListEventHandler(BSAoptGUI::ChangeSelectedFiles), NULL, this);
    BOArchiveList->InsertColumn( 0, wxT("File"), wxLIST_FORMAT_LEFT, 360 );
    BOArchiveList->InsertColumn( 1, wxT("Size"), wxLIST_FORMAT_RIGHT, 100 );

    gui = this;
    warmup = true;

    Settings = 0;
    /**/ if (RegOpenKeyEx  (HKEY_LOCAL_MACHINE, "Software\\Bethesda Softworks\\BSAopt", 0, KEY_READ | KEY_WRITE | KEY_WOW64_32KEY, &Settings) == ERROR_SUCCESS) {
    }
    else if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\Bethesda Softworks\\BSAopt", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE | KEY_WOW64_32KEY, NULL, &Settings, NULL) == ERROR_SUCCESS) {
    }

#ifdef W7SUPPORT
    char PPath[256] = ""; GetModuleFileName(0, PPath, sizeof(PPath) - 1); HKEY Application = 0;
    char APath[256]; strcpy(APath, "Applications"); char *exe = strrchr(PPath, '\\'); if (exe) strcat(APath, exe);
    /**/ if (RegOpenKeyEx  (HKEY_CLASSES_ROOT, APath, 0, KEY_READ | KEY_WRITE, &Application) != ERROR_SUCCESS) {
      RegCreateKeyEx(HKEY_CLASSES_ROOT, APath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &Application, NULL);

      strcpy(APath, "Open with "); strcat(APath, exe + 1); exe = strrchr(APath, '.'); if (exe) *exe = '\0';
      RegSetKeyValue(Application, "shell\\open", "", RRF_RT_REG_SZ, APath, strlen(APath) + 1);
      strcpy(APath, "\""); strcat(APath, PPath); strcat(APath, "\" \"%1\"");
      RegSetKeyValue(Application, "shell\\open\\command", "", RRF_RT_REG_SZ, APath, strlen(APath) + 1);
//    RegSetKeyValue(Application, "SupportedTypes", "Folder", RRF_RT_REG_SZ, "", 1);
    }
#endif

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
    if (TS[0]) BOSettings->FindChildItem(wxID_SKIPX, NULL)->Check(TS[0] == '1'); TSL = 1023; dropextras = (TS[0] == '1');
    TS[0] = 0; RegGetValue(Settings, NULL, "Logging", RRF_RT_REG_SZ, NULL, TS, &TSL);
    if (TS[0]) BOSettings->FindChildItem(wxID_LOGF, NULL)->Check(TS[0] == '1'); TSL = 1023;

    TS[0] = 0; RegGetValue(Settings, NULL, "Show Recursive", RRF_RT_REG_SZ, NULL, TS, &TSL);
    if (TS[0]) BORecursive->SetValue(TS[0] == '1'); TSL = 1023;
    TS[0] = 0; RegGetValue(Settings, NULL, "Game", RRF_RT_REG_SZ, NULL, TS, &TSL);
    switch (TS[0]) {
      case '0': BOGame->FindChildItem(wxID_AUTO     , NULL)->Check(true); break;
      case '1': BOGame->FindChildItem(wxID_OBLIVON  , NULL)->Check(true); break;
      case '2': BOGame->FindChildItem(wxID_FALLOUT  , NULL)->Check(true); break;
      case '3': BOGame->FindChildItem(wxID_SKYRIM   , NULL)->Check(true); break;
      case '4': BOGame->FindChildItem(wxID_MORROWIND, NULL)->Check(true); break;
      case '5': BOGame->FindChildItem(wxID_FALLOUTXB, NULL)->Check(true); break;
      case '6': BOGame->FindChildItem(wxID_SKYRIMXB , NULL)->Check(true); break;
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

    strcpy(IPath, BOInText ->GetValue());
    strcpy(OPath, BOOutText->GetValue());

//  BrowseIn (BOInText ->GetValue());
//  BrowseOut(BOOutText->GetValue());

    ResetHButtons();
  }

  BSAoptGUI::~BSAoptGUI() {
    if (Settings)
      RegCloseKey(Settings);
  }
};

char *cmdIn = NULL;
char *cmdOu = NULL;

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
  int ret = 0;

  {
    // break the command line in words
    char *cmdLine = strdup(lpCmdLine), *cL, *cN;
    vector<char *> argv; int argc = 0; cL = cmdLine;

    // argv[] must begin with the command itself
    char PPath[256] = "";
    char CPath[256] = "";
    GetModuleFileName(0, PPath, sizeof(PPath) - 1);
    GetCurrentDirectory(sizeof(CPath) - 1, CPath);

    argv.push_back(PPath);

    do {
      if (cL[0] == ' ')
	cL += 1;
      if (cL[0] == '\0')
	break;

      if (cL[0] == '"') {
	cL[0] = '\0', cL += 1;
	if ((cN = strchr(cL + 0, '"')))
	  cN[0] = '\0', cN += 1;
      }
      else {
	if ((cN = strchr(cL + 0, ' ')))
	  cN[0] = '\0', cN += 1;
      }

      argv.push_back(cL);
    } while ((cL = cN));

    // argv[] must be NULL-terminated
    argc = (int)argv.size();
    argv.push_back(NULL);
    
    bool isconsole = IsInsideConsole();
    if ((argc >= 1) && parseCommandline(argc, &argv[0], isconsole)) {
      RedirectIOToConsole();

      ret = main(argc, &argv[0]);
      
      if (!isconsole) {
	char key;
	cerr << "\n";
	cerr << "Press enter to close the window ...";
	cin.get(key);
      }
    }
    else {
      if (infile) {
	cmdIn = infile;

	/* we want absolute directories */
	if ((cmdIn[0] != '-') &&
	    (cmdIn[1] != ':') &&
	    (cmdIn[1] != '\\')) {
	  cmdIn = (char *)malloc(strlen(CPath) + 1 + strlen(infile) + 1);

	  strcpy(cmdIn, CPath);
	  strcat(cmdIn, "\\");
	  strcat(cmdIn, infile);
	}
      }

      if (outfile) {
	cmdOu = outfile;

	/* we want absolute directories */
	if ((cmdOu[0] != '-') &&
	    (cmdOu[1] != ':') &&
	    (cmdOu[1] != '\\')) {
	  cmdOu = (char *)malloc(strlen(CPath) + 1 + strlen(outfile) + 1);

	  strcpy(cmdOu, CPath);
	  strcat(cmdOu, "\\");
	  strcat(cmdOu, outfile);
	}
      }

#ifdef _DEBUG
      RedirectIOToConsole();
#endif

      io::init();

      ret = wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

      io::exit();
    }

    if ((argc > 1) && (cmdIn != argv[1]))
      free(cmdIn);
    if ((argc > 2) && (cmdOu != argv[2]))
      free(cmdOu);

    free(cmdLine);
  }

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
  frame->SetIcon(wxICON(IDI_MAIN_ICON));
  frame->Show(true);
//frame->Refresh();
//frame->Update();
  frame->SetCommandLine(cmdIn, cmdOu);
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
