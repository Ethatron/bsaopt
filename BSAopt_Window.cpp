///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  4 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "BSAopt_Window.h"

///////////////////////////////////////////////////////////////////////////

wxBSAopt::wxBSAopt( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	BOMenuBar = new wxMenuBar( 0 );
	BOGame = new wxMenu();
	wxMenuItem* BOAuto;
	BOAuto = new wxMenuItem( BOGame, wxID_AUTO, wxString( wxT("Automatic") ) , wxEmptyString, wxITEM_RADIO );
	BOGame->Append( BOAuto );
	BOAuto->Check( true );
	
	wxMenuItem* BOOblivion;
	BOOblivion = new wxMenuItem( BOGame, wxID_OBLIVON, wxString( wxT("Oblivion") ) , wxEmptyString, wxITEM_RADIO );
	BOGame->Append( BOOblivion );
	
	wxMenuItem* BOFallOut;
	BOFallOut = new wxMenuItem( BOGame, wxID_FALLOUT, wxString( wxT("Fallout 3") ) , wxEmptyString, wxITEM_RADIO );
	BOGame->Append( BOFallOut );
	
	wxMenuItem* BOSkyrim;
	BOSkyrim = new wxMenuItem( BOGame, wxID_SKYRIM, wxString( wxT("Skyrim") ) , wxEmptyString, wxITEM_RADIO );
	BOGame->Append( BOSkyrim );
	
	BOMenuBar->Append( BOGame, wxT("Game") ); 
	
	BOCompression = new wxMenu();
	wxMenuItem* BOZ0;
	BOZ0 = new wxMenuItem( BOCompression, wxID_BOZ0, wxString( wxT("0 (off)") ) , wxEmptyString, wxITEM_RADIO );
	BOCompression->Append( BOZ0 );
	
	wxMenuItem* BOZ1;
	BOZ1 = new wxMenuItem( BOCompression, wxID_BOZ1, wxString( wxT("1") ) , wxEmptyString, wxITEM_RADIO );
	BOCompression->Append( BOZ1 );
	
	wxMenuItem* BOZ2;
	BOZ2 = new wxMenuItem( BOCompression, wxID_BOZ2, wxString( wxT("2") ) , wxEmptyString, wxITEM_RADIO );
	BOCompression->Append( BOZ2 );
	
	wxMenuItem* BOZ3;
	BOZ3 = new wxMenuItem( BOCompression, wxID_BOZ3, wxString( wxT("3") ) , wxEmptyString, wxITEM_RADIO );
	BOCompression->Append( BOZ3 );
	
	wxMenuItem* BOZ4;
	BOZ4 = new wxMenuItem( BOCompression, wxID_BOZ4, wxString( wxT("4") ) , wxEmptyString, wxITEM_RADIO );
	BOCompression->Append( BOZ4 );
	
	wxMenuItem* BOZ5;
	BOZ5 = new wxMenuItem( BOCompression, wxID_BOZ5, wxString( wxT("5") ) , wxEmptyString, wxITEM_RADIO );
	BOCompression->Append( BOZ5 );
	
	wxMenuItem* BOZ6;
	BOZ6 = new wxMenuItem( BOCompression, wxID_BOZ6, wxString( wxT("6") ) , wxEmptyString, wxITEM_RADIO );
	BOCompression->Append( BOZ6 );
	
	wxMenuItem* BOZ7;
	BOZ7 = new wxMenuItem( BOCompression, wxID_BOZ7, wxString( wxT("7") ) , wxEmptyString, wxITEM_RADIO );
	BOCompression->Append( BOZ7 );
	
	wxMenuItem* BOZ8;
	BOZ8 = new wxMenuItem( BOCompression, wxID_BOZ8, wxString( wxT("8") ) , wxEmptyString, wxITEM_RADIO );
	BOCompression->Append( BOZ8 );
	
	wxMenuItem* BOZ9;
	BOZ9 = new wxMenuItem( BOCompression, wxID_BOZ9, wxString( wxT("9") ) , wxEmptyString, wxITEM_RADIO );
	BOCompression->Append( BOZ9 );
	
	wxMenuItem* BOZ10Quick;
	BOZ10Quick = new wxMenuItem( BOCompression, wxID_BOZ10Q, wxString( wxT("10 (quick)") ) , wxEmptyString, wxITEM_RADIO );
	BOCompression->Append( BOZ10Quick );
	
	wxMenuItem* BOZ10Thorough;
	BOZ10Thorough = new wxMenuItem( BOCompression, wxID_BOZ10T, wxString( wxT("10 (maximum)") ) , wxEmptyString, wxITEM_RADIO );
	BOCompression->Append( BOZ10Thorough );
	BOZ10Thorough->Check( true );
	
	wxMenuItem* m_separator1;
	m_separator1 = BOCompression->AppendSeparator();
	
	wxMenuItem* BOForce;
	BOForce = new wxMenuItem( BOCompression, wxID_FORCE, wxString( wxT("Forced") ) , wxEmptyString, wxITEM_CHECK );
	BOCompression->Append( BOForce );
	
	BOMenuBar->Append( BOCompression, wxT("Compression") ); 
	
	BOSettings = new wxMenu();
	wxMenuItem* BOSkipExisting;
	BOSkipExisting = new wxMenuItem( BOSettings, wxID_SKIPE, wxString( wxT("Skip existing files") ) , wxEmptyString, wxITEM_CHECK );
	BOSettings->Append( BOSkipExisting );
	BOSkipExisting->Check( true );
	
	wxMenuItem* BOSkipNewer;
	BOSkipNewer = new wxMenuItem( BOSettings, wxID_SKIPN, wxString( wxT("Skip older files") ) , wxEmptyString, wxITEM_CHECK );
	BOSettings->Append( BOSkipNewer );
	BOSkipNewer->Check( true );
	
	wxMenuItem* BOSkipHidden;
	BOSkipHidden = new wxMenuItem( BOSettings, wxID_SKIPH, wxString( wxT("Skip hidden files") ) , wxEmptyString, wxITEM_CHECK );
	BOSettings->Append( BOSkipHidden );
	BOSkipHidden->Check( true );
	
	wxMenuItem* m_separator2;
	m_separator2 = BOSettings->AppendSeparator();
	
	wxMenuItem* BOSkipHash;
	BOSkipHash = new wxMenuItem( BOSettings, wxID_SKIPC, wxString( wxT("Skip hash-check") ) , wxEmptyString, wxITEM_CHECK );
	BOSettings->Append( BOSkipHash );
	
	wxMenuItem* BOSkipBroken;
	BOSkipBroken = new wxMenuItem( BOSettings, wxID_SKIPB, wxString( wxT("Skip broken files") ) , wxEmptyString, wxITEM_CHECK );
	BOSettings->Append( BOSkipBroken );
	
	wxMenuItem* m_separator3;
	m_separator3 = BOSettings->AppendSeparator();
	
	wxMenuItem* BOUnselectExtras;
	BOUnselectExtras = new wxMenuItem( BOSettings, wxID_SKIPX, wxString( wxT("Unselect inappropriate") ) , wxEmptyString, wxITEM_CHECK );
	BOSettings->Append( BOUnselectExtras );
	BOUnselectExtras->Check( true );
	
	wxMenuItem* m_separator31;
	m_separator31 = BOSettings->AppendSeparator();
	
	wxMenuItem* BOLogfile;
	BOLogfile = new wxMenuItem( BOSettings, wxID_LOGF, wxString( wxT("Write logfile") ) , wxEmptyString, wxITEM_CHECK );
	BOSettings->Append( BOLogfile );
	
	BOMenuBar->Append( BOSettings, wxT("Settings") ); 
	
	this->SetMenuBar( BOMenuBar );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	m_notebook2 = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_panel5 = new wxPanel( m_notebook2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxVERTICAL );
	
	m_splitter1 = new wxSplitterWindow( m_panel5, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_splitter1->SetSashGravity( 0 );
	m_splitter1->Connect( wxEVT_IDLE, wxIdleEventHandler( wxBSAopt::m_splitter1OnIdle ), NULL, this );
	
	BOPanelContents = new wxPanel( m_splitter1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer36;
	bSizer36 = new wxBoxSizer( wxVERTICAL );
	
	BOInput = new wxDirPickerCtrl( BOPanelContents, wxID_ANY, wxT("./in"), wxT("Select a folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DIR_MUST_EXIST|wxDIRP_USE_TEXTCTRL );
	BOInput->Enable( false );
	BOInput->Hide();
	
	bSizer36->Add( BOInput, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxHORIZONTAL );
	
	BOInText = new wxTextCtrl( BOPanelContents, wxID_ANY, wxT(".\\in"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer32->Add( BOInText, 1, wxALL, 5 );
	
	BOInBrowse = new wxButton( BOPanelContents, wxID_ANY, wxT("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer32->Add( BOInBrowse, 0, wxALL, 5 );
	
	bSizer36->Add( bSizer32, 0, wxEXPAND, 5 );
	
	BOArchiveTree = new wxTreeCtrl( BOPanelContents, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE|wxTR_SINGLE );
	BOArchiveTree->SetToolTip( wxT("The directories present under the given source") );
	
	bSizer36->Add( BOArchiveTree, 1, wxALL|wxEXPAND, 5 );
	
	BOPanelContents->SetSizer( bSizer36 );
	BOPanelContents->Layout();
	bSizer36->Fit( BOPanelContents );
	m_panel4 = new wxPanel( m_splitter1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );
	
	BOFilter = new wxComboBox( m_panel4, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	BOFilter->Append( wxEmptyString );
	BOFilter->Append( wxT("*.*") );
	BOFilter->Append( wxT("*.dds") );
	BOFilter->Append( wxT("*.nif") );
	bSizer10->Add( BOFilter, 1, wxALL|wxEXPAND, 5 );
	
	BOFilterApply = new wxButton( m_panel4, wxID_ANY, wxT("Apply"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( BOFilterApply, 0, wxALL, 5 );
	
	bSizer13->Add( bSizer10, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer39;
	bSizer39 = new wxBoxSizer( wxHORIZONTAL );
	
	BOPluginToolbar = new wxToolBar( m_panel4, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL ); 
	BOPluginToolbar->AddTool( wxID_RESET, wxT("Reset"), wxBitmap( wxT("#112"), wxBITMAP_TYPE_RESOURCE ), wxNullBitmap, wxITEM_NORMAL, wxT("Selects all visible files"), wxEmptyString, NULL ); 
	BOPluginToolbar->AddTool( wxID_CLEAR, wxT("tool"), wxBitmap( wxT("#110"), wxBITMAP_TYPE_RESOURCE ), wxNullBitmap, wxITEM_NORMAL, wxT("Clears all visible files from the selection"), wxEmptyString, NULL ); 
	BOPluginToolbar->Realize();
	
	bSizer39->Add( BOPluginToolbar, 0, wxEXPAND, 5 );
	
	BORecursive = new wxCheckBox( m_panel4, wxID_ANY, wxT("Show recursive"), wxDefaultPosition, wxDefaultSize, 0 );
	BORecursive->SetToolTip( wxT("Show files in sub-directories as well") );
	
	bSizer39->Add( BORecursive, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer13->Add( bSizer39, 0, wxEXPAND, 5 );
	
	wxString BOArchiveListChoices[] = { wxT("a.esp"), wxT("b.esp") };
	int BOArchiveListNChoices = sizeof( BOArchiveListChoices ) / sizeof( wxString );
	BOArchiveList = new wxCheckListBox( m_panel4, wxID_ANY, wxDefaultPosition, wxDefaultSize, BOArchiveListNChoices, BOArchiveListChoices, 0 );
	BOArchiveList->SetToolTip( wxT("The files in the currently selected directory") );
	
	bSizer13->Add( BOArchiveList, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer42;
	bSizer42 = new wxBoxSizer( wxHORIZONTAL );
	
	BOOutput = new wxFilePickerCtrl( m_panel4, wxID_ANY, wxT("./out"), wxT("Select a file"), wxT("*.land;*.raw"), wxDefaultPosition, wxDefaultSize, wxFLP_OVERWRITE_PROMPT|wxFLP_SAVE|wxFLP_USE_TEXTCTRL );
	BOOutput->Enable( false );
	BOOutput->Hide();
	
	bSizer42->Add( BOOutput, 1, wxALL, 5 );
	
	wxBoxSizer* bSizer321;
	bSizer321 = new wxBoxSizer( wxHORIZONTAL );
	
	BOOutText = new wxTextCtrl( m_panel4, wxID_ANY, wxT(".\\out"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer321->Add( BOOutText, 1, wxALL, 5 );
	
	BOOutBrowse = new wxButton( m_panel4, wxID_ANY, wxT("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer321->Add( BOOutBrowse, 0, wxALL, 5 );
	
	bSizer42->Add( bSizer321, 1, wxEXPAND, 5 );
	
	BOConvert = new wxButton( m_panel4, wxID_ANY, wxT("Convert"), wxDefaultPosition, wxDefaultSize, 0 );
	BOConvert->Enable( false );
	BOConvert->SetToolTip( wxT("Copy/Convert all files from the source into the destination") );
	
	bSizer42->Add( BOConvert, 0, wxALL, 5 );
	
	bSizer13->Add( bSizer42, 0, wxEXPAND, 5 );
	
	m_panel4->SetSizer( bSizer13 );
	m_panel4->Layout();
	bSizer13->Fit( m_panel4 );
	m_splitter1->SplitHorizontally( BOPanelContents, m_panel4, 0 );
	bSizer14->Add( m_splitter1, 1, wxEXPAND, 5 );
	
	m_panel5->SetSizer( bSizer14 );
	m_panel5->Layout();
	bSizer14->Fit( m_panel5 );
	m_notebook2->AddPage( m_panel5, wxT("Browser"), false );
	
	bSizer1->Add( m_notebook2, 1, wxEXPAND | wxALL, 0 );
	
	this->SetSizer( bSizer1 );
	this->Layout();
	BOStatusBar = this->CreateStatusBar( 1, wxST_SIZEGRIP, wxID_ANY );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( BOAuto->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeToAuto ) );
	this->Connect( BOOblivion->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeToOblivion ) );
	this->Connect( BOFallOut->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeToFallout ) );
	this->Connect( BOSkyrim->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeToSkyrim ) );
	this->Connect( BOForce->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeForceCompression ) );
	this->Connect( BOSkipExisting->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeSkipExisting ) );
	this->Connect( BOSkipNewer->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeSkipNewer ) );
	this->Connect( BOSkipHidden->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeSkipHidden ) );
	this->Connect( BOSkipHash->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeSkipHash ) );
	this->Connect( BOSkipBroken->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeSkipBroken ) );
	this->Connect( BOUnselectExtras->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeUnselectExtras ) );
	this->Connect( BOLogfile->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeLogFile ) );
	BOInput->Connect( wxEVT_COMMAND_DIRPICKER_CHANGED, wxFileDirPickerEventHandler( wxBSAopt::ChangePluginDir ), NULL, this );
	BOInText->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( wxBSAopt::TypedInDone ), NULL, this );
	BOInText->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( wxBSAopt::TypedIn ), NULL, this );
	BOInBrowse->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxBSAopt::BrowseIn ), NULL, this );
	BOArchiveTree->Connect( wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler( wxBSAopt::ActivateTreeItem ), NULL, this );
	BOArchiveTree->Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( wxBSAopt::ChangeTreeItem ), NULL, this );
	BOFilter->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeFilter ), NULL, this );
	BOFilterApply->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxBSAopt::ApplyFilter ), NULL, this );
	this->Connect( wxID_RESET, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( wxBSAopt::ResetFileList ) );
	this->Connect( wxID_CLEAR, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( wxBSAopt::ClearFileList ) );
	BORecursive->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxBSAopt::ChangeRecursion ), NULL, this );
	BOArchiveList->Connect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( wxBSAopt::ChangeSelectedFiles ), NULL, this );
	BOOutText->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( wxBSAopt::TypedOutDone ), NULL, this );
	BOOutText->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( wxBSAopt::TypedOut ), NULL, this );
	BOOutBrowse->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxBSAopt::BrowseOut ), NULL, this );
	BOConvert->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxBSAopt::ConversionStart ), NULL, this );
}

wxBSAopt::~wxBSAopt()
{
	// Disconnect Events
	this->Disconnect( wxID_AUTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeToAuto ) );
	this->Disconnect( wxID_OBLIVON, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeToOblivion ) );
	this->Disconnect( wxID_FALLOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeToFallout ) );
	this->Disconnect( wxID_SKYRIM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeToSkyrim ) );
	this->Disconnect( wxID_FORCE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeForceCompression ) );
	this->Disconnect( wxID_SKIPE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeSkipExisting ) );
	this->Disconnect( wxID_SKIPN, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeSkipNewer ) );
	this->Disconnect( wxID_SKIPH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeSkipHidden ) );
	this->Disconnect( wxID_SKIPC, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeSkipHash ) );
	this->Disconnect( wxID_SKIPB, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeSkipBroken ) );
	this->Disconnect( wxID_SKIPX, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeUnselectExtras ) );
	this->Disconnect( wxID_LOGF, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeLogFile ) );
	BOInput->Disconnect( wxEVT_COMMAND_DIRPICKER_CHANGED, wxFileDirPickerEventHandler( wxBSAopt::ChangePluginDir ), NULL, this );
	BOInText->Disconnect( wxEVT_KILL_FOCUS, wxFocusEventHandler( wxBSAopt::TypedInDone ), NULL, this );
	BOInText->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( wxBSAopt::TypedIn ), NULL, this );
	BOInBrowse->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxBSAopt::BrowseIn ), NULL, this );
	BOArchiveTree->Disconnect( wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler( wxBSAopt::ActivateTreeItem ), NULL, this );
	BOArchiveTree->Disconnect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( wxBSAopt::ChangeTreeItem ), NULL, this );
	BOFilter->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( wxBSAopt::ChangeFilter ), NULL, this );
	BOFilterApply->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxBSAopt::ApplyFilter ), NULL, this );
	this->Disconnect( wxID_RESET, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( wxBSAopt::ResetFileList ) );
	this->Disconnect( wxID_CLEAR, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( wxBSAopt::ClearFileList ) );
	BORecursive->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxBSAopt::ChangeRecursion ), NULL, this );
	BOArchiveList->Disconnect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( wxBSAopt::ChangeSelectedFiles ), NULL, this );
	BOOutText->Disconnect( wxEVT_KILL_FOCUS, wxFocusEventHandler( wxBSAopt::TypedOutDone ), NULL, this );
	BOOutText->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( wxBSAopt::TypedOut ), NULL, this );
	BOOutBrowse->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxBSAopt::BrowseOut ), NULL, this );
	BOConvert->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxBSAopt::ConversionStart ), NULL, this );
	
}

wxProgress::wxProgress( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 400,265 ), wxDefaultSize );
	
	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxVERTICAL );
	
	m_panel9 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer26;
	bSizer26 = new wxBoxSizer( wxVERTICAL );
	
	BOSubject = new wxStaticText( m_panel9, wxID_ANY, wxT("File:"), wxDefaultPosition, wxDefaultSize, 0 );
	BOSubject->Wrap( -1 );
	bSizer26->Add( BOSubject, 0, wxALL, 5 );
	
	BOTask = new wxGauge( m_panel9, wxID_ANY, 100, wxDefaultPosition, wxSize( -1,25 ), wxGA_HORIZONTAL|wxGA_SMOOTH );
	bSizer26->Add( BOTask, 0, wxALL|wxEXPAND, 5 );
	
	BOReport = new wxStaticText( m_panel9, wxID_ANY, wxT("Efficiency:"), wxDefaultPosition, wxDefaultSize, 0 );
	BOReport->Wrap( -1 );
	bSizer26->Add( BOReport, 0, wxALL, 5 );
	
	BOEfficiency = new wxGauge( m_panel9, wxID_ANY, 100, wxDefaultPosition, wxSize( -1,25 ), wxGA_HORIZONTAL|wxGA_SMOOTH );
	bSizer26->Add( BOEfficiency, 0, wxALL|wxEXPAND, 5 );
	
	wxGridSizer* gSizer6;
	gSizer6 = new wxGridSizer( 2, 2, 0, 25 );
	
	m_staticText25 = new wxStaticText( m_panel9, wxID_ANY, wxT("Running:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText25->Wrap( -1 );
	gSizer6->Add( m_staticText25, 0, wxALL, 2 );
	
	BORunning = new wxStaticText( m_panel9, wxID_ANY, wxT("00:00:00s"), wxDefaultPosition, wxDefaultSize, 0 );
	BORunning->Wrap( -1 );
	gSizer6->Add( BORunning, 0, wxALIGN_RIGHT|wxALL, 2 );
	
	m_staticText27 = new wxStaticText( m_panel9, wxID_ANY, wxT("Remaining:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText27->Wrap( -1 );
	gSizer6->Add( m_staticText27, 0, wxALL, 2 );
	
	BORemaining = new wxStaticText( m_panel9, wxID_ANY, wxT("00:00:00s"), wxDefaultPosition, wxDefaultSize, 0 );
	BORemaining->Wrap( -1 );
	gSizer6->Add( BORemaining, 0, wxALIGN_RIGHT|wxALL, 2 );
	
	m_staticText29 = new wxStaticText( m_panel9, wxID_ANY, wxT("Total:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText29->Wrap( -1 );
	gSizer6->Add( m_staticText29, 0, wxALL, 2 );
	
	BOTotal = new wxStaticText( m_panel9, wxID_ANY, wxT("00:00:00s"), wxDefaultPosition, wxDefaultSize, 0 );
	BOTotal->Wrap( -1 );
	gSizer6->Add( BOTotal, 0, wxALIGN_RIGHT|wxALL, 1 );
	
	bSizer26->Add( gSizer6, 0, wxALIGN_CENTER, 5 );
	
	m_staticline7 = new wxStaticLine( m_panel9, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer26->Add( m_staticline7, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer27;
	bSizer27 = new wxBoxSizer( wxHORIZONTAL );
	
	BOPause = new wxButton( m_panel9, wxID_ANY, wxT("Pause"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer27->Add( BOPause, 0, wxALL, 5 );
	
	BOAbort = new wxButton( m_panel9, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer27->Add( BOAbort, 0, wxALL, 5 );
	
	bSizer26->Add( bSizer27, 0, wxALIGN_CENTER, 5 );
	
	m_panel9->SetSizer( bSizer26 );
	m_panel9->Layout();
	bSizer26->Fit( m_panel9 );
	bSizer25->Add( m_panel9, 1, wxEXPAND | wxALL, 0 );
	
	this->SetSizer( bSizer25 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( wxProgress::AbortProgress ) );
	this->Connect( wxEVT_IDLE, wxIdleEventHandler( wxProgress::IdleProgress ) );
	BOPause->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxProgress::PauseProgress ), NULL, this );
	BOAbort->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxProgress::AbortProgress ), NULL, this );
}

wxProgress::~wxProgress()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( wxProgress::AbortProgress ) );
	this->Disconnect( wxEVT_IDLE, wxIdleEventHandler( wxProgress::IdleProgress ) );
	BOPause->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxProgress::PauseProgress ), NULL, this );
	BOAbort->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxProgress::AbortProgress ), NULL, this );
	
}
