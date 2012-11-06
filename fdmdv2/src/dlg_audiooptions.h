//=========================================================================
// Name:          AudioInfoDisplay.h
// Purpose:       Declares simple wxWidgets application with GUI
//                created using wxFormBuilder.
// Author:
// Created:
// Copyright:
// License:       wxWidgets license (www.wxwidgets.org)
//
// Notes:         Note that all GUI creation code is declared in
//                gui.h source file which is generated by wxFormBuilder.
//=========================================================================
#ifndef __AudioOptsDialog__
#define __AudioOptsDialog__

#include "fdmdv2_main.h"

#define ID_AUDIO_OPTIONS    1000
#define AUDIO_IN            0
#define AUDIO_OUT           1

#define ICON_TRANSPARENT    1
#define ICON_CHECK          0
#define ICON_toolchar       2
#define ICON_tooldata       3
#define ICON_toolgame       4
#define ICON_toolnote       5
#define ICON_TOOLTIME       6
#define ICON_INARROW        7
#define ICON_OUTARROW       8
#define EXCHANGE_DATA_IN    0
#define EXCHANGE_DATA_OUT   1

// gui classes generated by wxFormBuilder
//#include "gui.h"

#include "portaudio.h"
#ifdef WIN32
#if PA_USE_ASIO
#include "pa_asio.h"
#endif
#endif

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=
// AudioInfoDisplay
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=
class AudioInfoDisplay
{
    public:
        wxListCtrl*     m_listDevices;
        int             direction;
        wxTextCtrl*     m_textDevice;
        wxComboBox*     m_cbSampleRate;
};

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=
// class AudioOptsDialog
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=
class AudioOptsDialog : public wxDialog
{
    private:

    protected:
        PaError         pa_err;
        bool            m_isPaInitialized;
        wxImageList     *m_imageListNormal;
        wxImageList     *m_imageListSmall;

        // protected event handlers
        //void OnCloseFrame( wxCloseEvent& event );
        void OnDeviceSelect( wxListEvent& event );

        //void OnExitClick( wxCommandEvent& event );
        //void DisplaySupportedSampleRates(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);
        void DisplaySupportedSampleRates(AudioInfoDisplay ai);
        void populateParams(AudioInfoDisplay);
        void showAPIInfo();

        AudioInfoDisplay m_RxInDevices;
        AudioInfoDisplay m_RxOutDevices;
        AudioInfoDisplay m_TxInDevices;
        AudioInfoDisplay m_TxOutDevices;
        wxPanel* m_panel1;
        wxNotebook* m_notebook1;
        wxPanel* m_panelRx;
        wxListCtrl* m_listCtrlRxInDevices;
        wxStaticText* m_staticText51;
        wxTextCtrl* m_textCtrlRxIn;
        wxStaticText* m_staticText6;
        wxComboBox* m_cbSampleRateRxIn;
        wxListCtrl* m_listCtrlRxOutDevices;
        wxStaticText* m_staticText9;
        wxTextCtrl* m_textCtrlRxOut;
        wxStaticText* m_staticText10;
        wxComboBox* m_cbSampleRateRxOut;
        wxPanel* m_panelTx;
        wxListCtrl* m_listCtrlTxInDevices;
        wxStaticText* m_staticText12;
        wxTextCtrl* m_textCtrlTxIn;
        wxStaticText* m_staticText11;
        wxComboBox* m_cbSampleRateTxIn;
        wxListCtrl* m_listCtrlTxOutDevices;
        wxStaticText* m_staticText81;
        wxTextCtrl* m_textCtrlTxOut;
        wxStaticText* m_staticText71;
        wxComboBox* m_cbSampleRateTxOut;
        wxPanel* m_panelAPI;
        wxStaticText* m_staticText7;
        wxTextCtrl* m_textStringVer;
        wxStaticText* m_staticText8;
        wxTextCtrl* m_textIntVer;
        wxStaticText* m_staticText5;
        wxTextCtrl* m_textCDevCount;
        wxStaticText* m_staticText4;
        wxTextCtrl* m_textAPICount;
        wxButton* m_btnRefresh;
        wxStdDialogButtonSizer* m_sdbSizer1;
        wxButton* m_sdbSizer1OK;
        wxButton* m_sdbSizer1Apply;
        wxButton* m_sdbSizer1Cancel;

        // Virtual event handlers, overide them in your derived class
        //virtual void OnActivateApp( wxActivateEvent& event ) { event.Skip(); }
//        virtual void OnCloseFrame( wxCloseEvent& event ) { event.Skip(); }
        void OnRxInDeviceSelect( wxListEvent& event );
        void OnRxOutDeviceSelect( wxListEvent& event );
        void OnTxInDeviceSelect( wxListEvent& event );
        void OnTxOutDeviceSelect( wxListEvent& event );
        void OnRefreshClick( wxCommandEvent& event );
        void OnApplyAudioParameters( wxCommandEvent& event );
        void OnCancelAudioParameters( wxCommandEvent& event );
        void OnOkAudioParameters( wxCommandEvent& event );
        // Virtual event handlers, overide them in your derived class
        void OnClose( wxCloseEvent& event ) { event.Skip(); }
        void OnHibernate( wxActivateEvent& event ) { event.Skip(); }
        void OnIconize( wxIconizeEvent& event ) { event.Skip(); }
        void OnInitDialog( wxInitDialogEvent& event );

    public:

        AudioOptsDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Audio Options"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 300,300 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
        ~AudioOptsDialog();
        void ExchangeData(int inout);
};
#endif //__AudioOptsDialog__
