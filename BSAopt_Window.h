///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __BSAOPT_WINDOW_H__
#define __BSAOPT_WINDOW_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
class wxCheckedListCtrl;

#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/filepicker.h>
#include <wx/statbmp.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/treelistctrl/treelistctrl.h>
#include <wx/checkedlistctrl/checkedlistctrl.h>
#ifdef __VISUALC__
#include <wx/link_additions.h>
#endif //__VISUALC__
#include <wx/panel.h>
#include <wx/combobox.h>
#include <wx/toolbar.h>
#include <wx/checkbox.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/notebook.h>
#include <wx/statusbr.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/gauge.h>
#include <wx/statline.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

#define wxID_AUTO 1000
#define wxID_MORROWIND 1001
#define wxID_OBLIVON 1002
#define wxID_FALLOUT 1003
#define wxID_FALLOUTXB 1004
#define wxID_SKYRIM 1005
#define wxID_SKYRIMXB 1006
#define wxID_BOZ0 1007
#define wxID_BOZ1 1008
#define wxID_BOZ2 1009
#define wxID_BOZ3 1010
#define wxID_BOZ4 1011
#define wxID_BOZ5 1012
#define wxID_BOZ6 1013
#define wxID_BOZ7 1014
#define wxID_BOZ8 1015
#define wxID_BOZ9 1016
#define wxID_BOZ10Q 1017
#define wxID_BOZ10T 1018
#define wxID_FORCE 1019
#define wxID_SKIPE 1020
#define wxID_SKIPN 1021
#define wxID_SKIPH 1022
#define wxID_SKIPC 1023
#define wxID_SKIPB 1024
#define wxID_SKIPX 1025
#define wxID_LOGF 1026

///////////////////////////////////////////////////////////////////////////////
/// Class wxBSAopt
///////////////////////////////////////////////////////////////////////////////
class wxBSAopt : public wxFrame 
{
	private:
	
	protected:
		wxMenuBar* BOMenuBar;
		wxMenu* BOGame;
		wxMenu* BOCompression;
		wxMenu* BOSettings;
		wxNotebook* m_notebook2;
		wxPanel* m_panel5;
		wxSplitterWindow* m_splitter1;
		wxPanel* BOPanelContents;
		wxDirPickerCtrl* BOInput;
		wxStaticBitmap* BOTypeIn;
		wxTextCtrl* BOInText;
		wxButton* BOInBrowse;
		wxTreeListCtrl* BOArchiveTree;
		wxPanel* m_panel4;
		wxComboBox* BOFilter;
		wxButton* BOFilterApply;
		wxToolBar* BOPluginToolbar;
		wxCheckBox* BORecursive;
		wxCheckedListCtrl* BOArchiveList;
		wxFilePickerCtrl* BOOutput;
		wxStaticBitmap* BOTypeOut;
		wxTextCtrl* BOOutText;
		wxButton* BOOutBrowse;
		wxButton* BOConvert;
		wxStatusBar* BOStatusBar;
		
		// Virtual event handlers, overide them in your derived class
		virtual void ChangeToAuto( wxCommandEvent& event ) { event.Skip(); }
		virtual void ChangeToMorrowind( wxCommandEvent& event ) { event.Skip(); }
		virtual void ChangeToOblivion( wxCommandEvent& event ) { event.Skip(); }
		virtual void ChangeToFallout( wxCommandEvent& event ) { event.Skip(); }
		virtual void ChangeToFalloutXB( wxCommandEvent& event ) { event.Skip(); }
		virtual void ChangeToSkyrim( wxCommandEvent& event ) { event.Skip(); }
		virtual void ChangeToSkyrimXB( wxCommandEvent& event ) { event.Skip(); }
		virtual void ChangeForceCompression( wxCommandEvent& event ) { event.Skip(); }
		virtual void ChangeSkipExisting( wxCommandEvent& event ) { event.Skip(); }
		virtual void ChangeSkipNewer( wxCommandEvent& event ) { event.Skip(); }
		virtual void ChangeSkipHidden( wxCommandEvent& event ) { event.Skip(); }
		virtual void ChangeSkipHash( wxCommandEvent& event ) { event.Skip(); }
		virtual void ChangeSkipBroken( wxCommandEvent& event ) { event.Skip(); }
		virtual void ChangeUnselectExtras( wxCommandEvent& event ) { event.Skip(); }
		virtual void ChangeLogFile( wxCommandEvent& event ) { event.Skip(); }
		virtual void ChangePluginDir( wxFileDirPickerEvent& event ) { event.Skip(); }
		virtual void TypedInDone( wxFocusEvent& event ) { event.Skip(); }
		virtual void TypedIn( wxCommandEvent& event ) { event.Skip(); }
		virtual void BrowseIn( wxCommandEvent& event ) { event.Skip(); }
		virtual void DescentTreeItem( wxMouseEvent& event ) { event.Skip(); }
		virtual void ActivateTreeItem( wxTreeEvent& event ) { event.Skip(); }
		virtual void MenuTreeItem( wxTreeEvent& event ) { event.Skip(); }
		virtual void ChangeTreeItem( wxTreeEvent& event ) { event.Skip(); }
		virtual void ChangeFilter( wxCommandEvent& event ) { event.Skip(); }
		virtual void ApplyFilter( wxCommandEvent& event ) { event.Skip(); }
		virtual void ResetFileList( wxCommandEvent& event ) { event.Skip(); }
		virtual void ClearFileList( wxCommandEvent& event ) { event.Skip(); }
		virtual void ChangeRecursion( wxCommandEvent& event ) { event.Skip(); }
		virtual void TypedOutDone( wxFocusEvent& event ) { event.Skip(); }
		virtual void TypedOut( wxCommandEvent& event ) { event.Skip(); }
		virtual void BrowseOut( wxCommandEvent& event ) { event.Skip(); }
		virtual void ConversionStart( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		wxBSAopt( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 525,774 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		
		~wxBSAopt();
		
		void m_splitter1OnIdle( wxIdleEvent& )
		{
			m_splitter1->SetSashPosition( 0 );
			m_splitter1->Disconnect( wxEVT_IDLE, wxIdleEventHandler( wxBSAopt::m_splitter1OnIdle ), NULL, this );
		}
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class wxProgress
///////////////////////////////////////////////////////////////////////////////
class wxProgress : public wxDialog 
{
	private:
	
	protected:
		wxPanel* m_panel9;
		wxStaticText* BOSubject;
		wxGauge* BOTask;
		wxStaticText* BOReport;
		wxGauge* BOEfficiency;
		wxStaticText* m_staticText25;
		wxStaticText* BORunning;
		wxStaticText* m_staticText27;
		wxStaticText* BORemaining;
		wxStaticText* m_staticText29;
		wxStaticText* BOTotal;
		wxStaticLine* m_staticline7;
		wxButton* BOPause;
		wxButton* BOAbort;
		
		// Virtual event handlers, overide them in your derived class
		virtual void AbortProgress( wxCloseEvent& event ) { event.Skip(); }
		virtual void IdleProgress( wxIdleEvent& event ) { event.Skip(); }
		virtual void PauseProgress( wxCommandEvent& event ) { event.Skip(); }
		virtual void AbortProgress( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		wxProgress( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("BSAopt copy in progress ..."), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 569,265 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ); 
		~wxProgress();
	
};

#endif //__BSAOPT_WINDOW_H__
