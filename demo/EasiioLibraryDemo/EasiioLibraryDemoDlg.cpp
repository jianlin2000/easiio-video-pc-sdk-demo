
// EasiioLibraryDemoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "EasiioLibraryDemo.h"
#include "EasiioLibraryDemoDlg.h"
#include "afxdialogex.h"
#include "EasiioLibrary.h"

#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string>

//#pragma comment(lib, "EasiioLibrary.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const UINT_PTR TIMER_ID_LOGIN = 1;

CEasiioLibraryDemoDlg* mEasiioDlg;
CProgressCtrl *mLoginProgress;

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{

public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAboutDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CEasiioLibraryDemoDlg �Ի���
CEasiioLibraryDemoDlg::CEasiioLibraryDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEasiioLibraryDemoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	mEasiioDlg = this;
	mCallInfo = new CallInfo;
	memset(mCallInfo, 0, sizeof(CallInfo));
}


EASIIO_CALLBACK_INTERFACE *cbInterface;

/**
* ��¼����ص�
*/
void onLoginResult(int resultCode, const char* resultMsg)
{
	printf("onLoginResult resultCode=%d, resultMsg=%s\n", resultCode, resultMsg);
	mLoginProgress->ShowWindow(false);
	KillTimer(mEasiioDlg->m_hWnd, TIMER_ID_LOGIN);
	if (resultCode == LOGIN_SUCCESS){
		mEasiioDlg->loginSuccess();
		EasiioGetMeetingList();
	}
	CString str = CString(resultMsg);
	MessageBox(mEasiioDlg->m_hWnd, str, "Login Result", MB_OK);
}

/**
* PJSIP ״̬�ص�
*/
void onPJSIPServiceStateChanged(int stateCode)
{
	printf("onPJSIPServiceStateChanged stateCode=%d\n", stateCode);
	CString str;
	str.Format("%d", stateCode);
	mEasiioDlg->setPJSIPStatusEdit(str);

}

/**
* ͨ��״̬�ص�
*/
void onCallStatusChanged(int callId, int callState, const char* resultCode, const char* callUUID)
{
	printf("onCallStatusChanged callId=%d, callState=%d, resultCode=%s, callUUID = %s\n", callId, callState, resultCode, callUUID);
	char* call_uuid = new char[100];
	strcpy(call_uuid, callUUID);

	CString str;
	str.Format("%d", callState);
	mEasiioDlg->setCallStatusEdit(str);
	mEasiioDlg->updateCallStatus(callId, callState, call_uuid);
}

/**
* ˫�����״̬�ص�
*/
void onTwoWayCallStatusChanged(int callState, const char* callId, const char* caller, const char* callee, int retrying)
{
	printf("onTwoWayCallStatusChanged callId=%s, callState=%d, caller=%s, callee = %s, retrying = %d\n", callId, callState, caller, callee, retrying);

}

/**
* ������
*/
void onIncomingCall(int callId, int callType, const char* fromNumber, const char* postValue,const char* meeting_key)
{
	printf("onLoginResult callId=%d, callType=%d, fromNumber=%s, postValue=%s,meeting_key=%s.\n", callId, callType, fromNumber, postValue,meeting_key);

	mEasiioDlg->inCall();

	char* from_number = new char[100];
	strcpy(from_number, fromNumber);

	char* post_value = new char[1024];
	strcpy(post_value, postValue);

	CString str;
	str.Format("%s", fromNumber);
	mEasiioDlg->setIncomingEdit(str);
	mEasiioDlg->createCallInfo(callId, 1, CALL_STATE_INCOMING, 1, from_number, post_value, NULL);
	MessageBox(mEasiioDlg->m_hWnd, "From: " + str, "New Incoming Call", MB_OK);
}

/**
* ���н��������Call ID�� ���С��0����ʧ��
* -300   û����˷�����
* -301   û����˷����
* -302   û����˷��������
*/

void onMakeCallResult(int callId, const char* number,const char* meetingkey){
	printf("onMakeCallResult callId=%d, number=%s\n", callId, number);
	char* toNumber = new char[100];
	strcpy(toNumber, number);
	mEasiioDlg->createCallInfo(callId, 0, CALL_STATE_INIT, 1, toNumber, NULL, NULL);
	mEasiioDlg->inCall();
	if (strlen(meetingkey) > 1) {
		mEasiioDlg->updatejoinMeeting(meetingkey);
	}
}

/**
* ����˫����н���ص�
*/
void onMakeTwoWayCallResult(EasiioResponseReason reason, const char* caller, const char* callee, const char* callUUID, const char* retryId)
{
	printf("onMakeTwoWayCallResult reason=%d, msg=%s, caller=%s, callee=%s, callId=%s, retryId=%s\n", reason.reason, reason.msg, caller, callee, callUUID, retryId);
}

/**
* ͨ����¼��������رյ��ûص�
*/
void onCallRecordingReaponse(EasiioResponseReason reason, int switchRecord, const char* callUUID)
{
	printf("onCallRecordingReaponse reason=%d, msg=%s, callUUID=%s, switchRecord=%d\n", reason.reason, reason.msg, callUUID, switchRecord);

	if (reason.reason == REQUEST_SUCCESS){
		mEasiioDlg->updateCallRecordState(callUUID, switchRecord);
	}
}

/**
* ��ȡ����¼���ص�
*/
void onGetAllRecordResponse(EasiioResponseReason reason, int count, EasiioRecord* record)
{
	printf("onGetAllRecordResponse reason=%d, msg=%s, count=%d\n", reason.reason, reason.msg, count);
	if (record == NULL || count == 0){
		printf("onGetAllRecordResponse record is null or count is 0\n");
		return;
	}

	EasiioRecord* p = record;
	for (int i = 0; i < count; i++){
		printf("EasiioRecord:i=%d, from_user=%s, call_uuid=%s, record_count=%d, direction=%s, to_user=%s, answer_time=%s\n", i, p->from_user, p->call_uuid, p->record_count, p->direction, p->to_user, p->answer_time);
		if (i<count - 1)
		{
			p++;
		}
	}

}

/**
* ��ȡ����ͨ��¼���ص�
*/
void onGetRecordResponse(EasiioResponseReason reason, const char* callUUID, int count, EasiioRecordItem* recordItem)
{
	printf("onGetRecordResponse reason=%d, msg=%s, callUUID=%s, count=%d\n", reason.reason, reason.msg, callUUID, count);
	if (recordItem == NULL || count == 0){
		printf("onGetRecordResponse record is null or count is 0\n");
		return;
	}

	EasiioRecordItem* p = recordItem;
	for (int i = 0; i < count; i++){
		printf("EasiioRecordItem:i=%d, insert_time=%s, uri=%s, record_ms=%.2f, uuid=%s, answer_time=%s\n", i, p->insert_time, p->uri, p->record_ms, p->uuid, p->answer_time);
		if (i<count - 1)
		{
			p++;
		}
	}
}

void onMeetingResult(EasiioResponseReason reason, int count, MeetingInfo *info) {
	printf("onMeetingResult reason=%d, msg=%s, count=%d\n", reason.reason, reason.msg, count);
	if (count == 0 || info == NULL) {
		printf("MeetingInfo is empty.\n");
		return;
	}
	MeetingInfo *item = info;
	mEasiioDlg->showMeetinginfo(item->desc, item->meeting_key,item->priority);

}

void CEasiioLibraryDemoDlg::InitConsoleWindow()
{
	AllocConsole();
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle, _O_TEXT);
	FILE * hf = _fdopen(hCrt, "w");
	*stdout = *hf;
}

void CEasiioLibraryDemoDlg::showLoginResultDialog(CString msg){
	MessageBox(msg, "Login Result", MB_OK);
}

void CEasiioLibraryDemoDlg::setPJSIPStatusEdit(CString msg){
	GetDlgItem(PJSIP_STATUS_EDIT)->SetWindowText(msg);
}

void CEasiioLibraryDemoDlg::setCallStatusEdit(CString msg){
	GetDlgItem(CALL_STATUS_EDIT)->SetWindowText(msg);
}

void CEasiioLibraryDemoDlg::setIncomingEdit(CString number){
	GetDlgItem(INCOMING_EDIT)->SetWindowText(number);
}

void CEasiioLibraryDemoDlg::createCallInfo(int callId, int isIncoming, int status, int isAlive, char* number, char* postValue, char* callUUID)
{
	if (mCallInfo == NULL){
		mCallInfo = new CallInfo;
		memset(mCallInfo, 0, sizeof(CallInfo));
	}
	mCallInfo->callId = callId;
	mCallInfo->isIncoming = isIncoming;
	mCallInfo->status = status;
	mCallInfo->isAlive = isAlive;
	mCallInfo->number = number;
	mCallInfo->postValue = postValue;
	mCallInfo->callUUID = "";
	mCallInfo->hold = 0;
	mCallInfo->mute = 0;
	mCallInfo->record = 0;
}

void CEasiioLibraryDemoDlg::updateCallStatus(int callId, int status, char* callUUID)
{
	if (mCallInfo == NULL || mCallInfo->isAlive == 0 || mCallInfo->callId != callId){
		return;
	}
	mCallInfo->status = status;
	mCallInfo->callUUID = callUUID;
	if (status == CALL_STATE_DISCONNECTED){
		mCallInfo->isAlive = 0;
		mEasiioDlg->outCall();
		EasiioGetRecord(RECORD_TYPE_IN_CALL, mCallInfo->callUUID);
		EasiioGetRecord(RECORD_TYPE_FROM_CALL_START, mCallInfo->callUUID);
		if (!mCallInfo->meethost && mCallInfo->meetflag) {
			updateleaveMeeting();
		}
	}
}

void CEasiioLibraryDemoDlg::unAliveCallInfo(int callId)
{
	if (mCallInfo == NULL || mCallInfo->isAlive == 0 || mCallInfo->callId != callId){
		return;
	}
	mCallInfo->isAlive = 0;
}

void CEasiioLibraryDemoDlg::updateCallHoldState(int callId, int hold)
{
	if (mCallInfo == NULL || mCallInfo->isAlive == 0 || mCallInfo->callId != callId){
		return;
	}
	mCallInfo->hold = hold;
}
void CEasiioLibraryDemoDlg::updateCallMuteState(int callId, int mute)
{
	if (mCallInfo == NULL || mCallInfo->isAlive == 0 || mCallInfo->callId != callId){
		return;
	}
	mCallInfo->mute = mute;
}

void CEasiioLibraryDemoDlg::updateCallRecordState(CString callUUID, int record)
{
	if (mCallInfo == NULL || mCallInfo->isAlive == 0 || strcmp(callUUID, mCallInfo->callUUID)){
		return;
	}
	mCallInfo->record = record;
}

void CEasiioLibraryDemoDlg::loginSuccess()
{
	CButton* loginBtn = (CButton*)GetDlgItem(BUTTON_LOGIN);
	loginBtn->ShowWindow(false);
	CButton* logoutBtn = (CButton*)GetDlgItem(BUTTON_LOGOUT);
	logoutBtn->ShowWindow(true);

	CEdit* callEdit = (CEdit*)GetDlgItem(CALL_EDIT);
	callEdit->SetReadOnly(false);

	CEdit* fromEdit = (CEdit*)GetDlgItem(FROM_PHONE_EDIT);
	fromEdit->SetReadOnly(false);

	CEdit* toEdit = (CEdit*)GetDlgItem(TO_PHONE_EDIT);
	toEdit->SetReadOnly(false);
}

void CEasiioLibraryDemoDlg::inCall()
{
	CEdit* callEdit = (CEdit*)GetDlgItem(CALL_EDIT);
	callEdit->SetReadOnly(true);

	CEdit* dtmfEdit = (CEdit*)GetDlgItem(DTMF_EDIT);
	dtmfEdit->SetReadOnly(false);
}

void CEasiioLibraryDemoDlg::outCall()
{
	CEdit* callEdit = (CEdit*)GetDlgItem(CALL_EDIT);
	callEdit->SetReadOnly(false);

	CEdit* dtmfEdit = (CEdit*)GetDlgItem(DTMF_EDIT);
	dtmfEdit->SetReadOnly(true);

}

void CEasiioLibraryDemoDlg::showMeetinginfo(CString desc, CString meetingkey,int priority) {

	GetDlgItem(MEETING_DESC_EDIT)->SetWindowText(desc);

	GetDlgItem(MEETING_KEY_EDIT)->SetWindowText(meetingkey);

	CEdit* descEdit = (CEdit*)GetDlgItem(DESC_EDIT);
	descEdit->SetReadOnly(true);
	descEdit->SetWindowTextA(desc);

	CString priorityStr;
	priorityStr.Format("%d", priority);
	CEdit* priorityEdit = (CEdit*)GetDlgItem(PRIORITY_EDIT);
	priorityEdit->SetReadOnly(true);
	priorityEdit->SetWindowTextA(priorityStr);

	CButton* createBtn = (CButton*)GetDlgItem(BUTTON_CREATE_MEETING);
	createBtn->EnableWindow(false);

	CButton* modifyBtn = (CButton*)GetDlgItem(BUTTON_MODIFY);
	modifyBtn->EnableWindow(true);

	CButton* saveBtn = (CButton*)GetDlgItem(BUTTON_SAVE);
	saveBtn->EnableWindow(false);

	CButton* deleteBtn = (CButton*)GetDlgItem(BUTTON_DELETE);
	deleteBtn->EnableWindow(true);

}

void CEasiioLibraryDemoDlg::updatejoinMeeting(CString meetingkey) {
	char pbxAccount[11] = { 0 };
	EasiioGetCurrentPBXAccount(pbxAccount);
	const char* meeting_key = meetingkey.GetBuffer(sizeof(meetingkey));
	if (!mCallInfo->meethost) {
		int ret = EasiioUpdateJoinMeetingInfo(meeting_key, "join", pbxAccount);
		if (!ret) {
			mCallInfo->meetflag = 1;
		}
	}
}

void CEasiioLibraryDemoDlg::updateleaveMeeting() {
	char pbxAccount[11] = { 0 };
	EasiioGetCurrentPBXAccount(pbxAccount);
	CString str;
	GetDlgItem(CALL_EDIT)->GetWindowText(str);
	const char* meetingkey = str.GetBuffer(sizeof(str));

	int ret = EasiioUpdateJoinMeetingInfo(meetingkey, "leave", pbxAccount);
	if (!ret) {
		mCallInfo->meetflag = 0;
	}
}

void CEasiioLibraryDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CEasiioLibraryDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDCANCEL, &CEasiioLibraryDemoDlg::OnBnClickedCancel)
	ON_BN_CLICKED(BUTTON_LOGIN, &CEasiioLibraryDemoDlg::OnBnClickedButtonLogin)
	ON_EN_CHANGE(IDC_EDIT1, &CEasiioLibraryDemoDlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON2, &CEasiioLibraryDemoDlg::OnBnClickedButton2)
	ON_BN_CLICKED(BUTTON_CALL, &CEasiioLibraryDemoDlg::OnBnClickedCall)
	ON_BN_CLICKED(BUTTON_ANSWER, &CEasiioLibraryDemoDlg::OnBnClickedAnswer)
	ON_BN_CLICKED(BUTTON_HANGUP, &CEasiioLibraryDemoDlg::OnBnClickedHangup)
	ON_BN_CLICKED(BUTTON_HOLD, &CEasiioLibraryDemoDlg::OnBnClickedHold)
	ON_BN_CLICKED(BUTTON_UNHOLD, &CEasiioLibraryDemoDlg::OnBnClickedUnhold)
	ON_BN_CLICKED(BUTTON_MUTE, &CEasiioLibraryDemoDlg::OnBnClickedMute)
	ON_BN_CLICKED(BUTTON_UNMUTE, &CEasiioLibraryDemoDlg::OnBnClickedUnmute)
	ON_BN_CLICKED(BUTTON_MAK_TWO_WAY_CALL, &CEasiioLibraryDemoDlg::OnBnClickedMakTwoWayCall)
	ON_BN_CLICKED(BUTTON_START_RECORD, &CEasiioLibraryDemoDlg::OnBnClickedStartRecord)
	ON_BN_CLICKED(BUTTON_STOP_RECORD, &CEasiioLibraryDemoDlg::OnBnClickedStopRecord)
	ON_BN_CLICKED(BUTTON_REJECT, &CEasiioLibraryDemoDlg::OnBnClickedReject)
	ON_BN_CLICKED(BUTTON_SEND_DTMF, &CEasiioLibraryDemoDlg::OnBnClickedSendDtmf)
	ON_BN_CLICKED(BUTTON_LOGOUT, &CEasiioLibraryDemoDlg::OnBnClickedLogout)
	ON_BN_CLICKED(BUTTON_GET_ALL_RECORD_0, &CEasiioLibraryDemoDlg::OnBnClickedGetAllRecord0)
	ON_BN_CLICKED(BUTTON_GET_ALL_RECORD_1, &CEasiioLibraryDemoDlg::OnBnClickedGetAllRecord1)
	ON_BN_CLICKED(BUTTON_REFRESH_MIC, &CEasiioLibraryDemoDlg::OnBnClickedRefreshMic)
	ON_BN_CLICKED(BUTTON_DESTROY, &CEasiioLibraryDemoDlg::OnBnClickedDestroy)
	ON_BN_CLICKED(BUTTON_CALL_RECORDING, &CEasiioLibraryDemoDlg::OnBnClickedCallRecording)
	ON_BN_CLICKED(BUTTON_CREATE_MEETING, &CEasiioLibraryDemoDlg::OnBnClickedCreateMeeting)
	ON_BN_CLICKED(BUTTON_DELETE, &CEasiioLibraryDemoDlg::OnBnClickedDelete)
	ON_BN_CLICKED(BUTTON_MODIFY, &CEasiioLibraryDemoDlg::OnBnClickedModify)
	ON_BN_CLICKED(BUTTON_SAVE, &CEasiioLibraryDemoDlg::OnBnClickedSave)
	ON_BN_CLICKED(BUTTON_START, &CEasiioLibraryDemoDlg::OnBnClickedStart)
	ON_BN_CLICKED(BUTTON_STOP, &CEasiioLibraryDemoDlg::OnBnClickedStop)
	ON_BN_CLICKED(BUTTON_JOIN_MEETING, &CEasiioLibraryDemoDlg::OnBnClickedJoinMeeting)
	ON_BN_CLICKED(BUTTON_START_PREVIEW, &CEasiioLibraryDemoDlg::OnBnClickedStartPreview)
	ON_BN_CLICKED(BUTTON_STOP_PREVIEW, &CEasiioLibraryDemoDlg::OnBnClickedStopPreview)
END_MESSAGE_MAP()


// CEasiioLibraryDemoDlg ��Ϣ�������

CString GetWorkDir()
{
	char pFileName[MAX_PATH];
	int nPos = GetCurrentDirectory(MAX_PATH, pFileName);

	CString csFullPath(pFileName);
	if (nPos < 0)
		return CString("");
	else
		return csFullPath;
}

BOOL CEasiioLibraryDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��
	mLoginProgress = (CProgressCtrl*)GetDlgItem(LOGIN_PROGRESS);
	mLoginProgress->ShowWindow(false);
	InitConsoleWindow();
	printf("Debug log start... \n");
	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	cbInterface = new EASIIO_CALLBACK_INTERFACE;
	memset(cbInterface, 0, sizeof(EASIIO_CALLBACK_INTERFACE));
	cbInterface->onLoginResult = &onLoginResult;
	cbInterface->onPJSIPServiceStateChanged = &onPJSIPServiceStateChanged;
	cbInterface->onCallStatusChanged = &onCallStatusChanged;
	cbInterface->onTwoWayCallStatusChanged = &onTwoWayCallStatusChanged;
	cbInterface->onIncomingCall = &onIncomingCall;
	cbInterface->onMakeCallResult = &onMakeCallResult;
	cbInterface->onMakeTwoWayCallResult = &onMakeTwoWayCallResult;
	cbInterface->onCallRecordingReaponse = &onCallRecordingReaponse;
	cbInterface->onGetAllRecordResponse = &onGetAllRecordResponse;
	cbInterface->onGetRecordResponse = &onGetRecordResponse;
	cbInterface->onMeetingResult = &onMeetingResult;
	CString dir = GetWorkDir() + "\\easiio";
	CreateDirectory(dir, NULL);
	EasiioSetLogDir(dir);
	EasiioInit(cbInterface);

	EasiioInitDeveloperKeyAndToken("04ed2ff8e149eaa647s415aicbaa6e5e946a", "CefgvsgcpM");;
	//EasiioSetProxyUrl("10.90.1.133:8080");


	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CEasiioLibraryDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if (nID == SC_CLOSE)
	{
		EasiioLogout();
		FreeConsole();
		CDialogEx::OnCancel();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CEasiioLibraryDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CEasiioLibraryDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CEasiioLibraryDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialog::OnTimer(nIDEvent);

	if (nIDEvent == TIMER_ID_LOGIN){
		int pos = mLoginProgress->GetPos();
		pos += 10;
		if (pos > 100){
			pos = 0;
		}
		mLoginProgress->SetPos(pos);
	}
}


void CEasiioLibraryDemoDlg::OnBnClickedCancel()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	EasiioDestroy();
	FreeConsole();
	delete mCallInfo;
	CDialogEx::OnCancel();
}


void CAboutDlg::OnBnClickedOk()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CDialogEx::OnOK();
}

/**
* ��¼
*/
void CEasiioLibraryDemoDlg::OnBnClickedButtonLogin()
{
	CString str;
	GetDlgItem(IDC_EDIT1)->GetWindowText(str);
	const char* loginAccount = str.GetBuffer(sizeof(str));
	EasiioLoginParams* params = new EasiioLoginParams[1];
	// Demo��ѡ���������弯�ɰ���Ҫ����޸�,�ɲ���
	// ��ѡ����user_flag
	params->param_key = new char[strlen(LOGIN_PARAM_USER_FLAG) + 1];
	strcpy_s(params->param_key, strlen(LOGIN_PARAM_USER_FLAG) + 1, LOGIN_PARAM_USER_FLAG);
	params->param_value = "Test";

	// ��ѡ���� server_url �� access_token��������֤������OAUTH
	//(params + 1)->param_key = new char[strlen(LOGIN_PARAM_SERVER_URL)];
	//strcpy_s((params + 1)->param_key, strlen(LOGIN_PARAM_SERVER_URL) + 1, LOGIN_PARAM_SERVER_URL);
	//(params + 1)->param_value = "testcloud.quanshi.com";

	//(params + 2)->param_key = new char[strlen(LOGIN_PARAM_ACCESS_TOKEN)];
	//strcpy_s((params + 2)->param_key, strlen(LOGIN_PARAM_ACCESS_TOKEN) + 1, LOGIN_PARAM_ACCESS_TOKEN);
	//(params + 2)->param_value = "c3c6924f-6c1b-4498-be6b-c804ed3849c1";

	EasiioLogin(loginAccount, 1, params);

	mLoginProgress->ShowWindow(true);
	mLoginProgress->SetRange(0, 100);
	mLoginProgress->SetPos(0);

	SetTimer(TIMER_ID_LOGIN, 50, NULL);
}


void CEasiioLibraryDemoDlg::OnEnChangeEdit1()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}

/**
* ��ȡPBX����
*/
void CEasiioLibraryDemoDlg::OnBnClickedButton2()
{
	char pbxAccount[11] = { 0 };
	//memset(pbxAccount, 0, sizeof(pbxAccount));
	EasiioGetCurrentPBXAccount(pbxAccount);
	CString str = CString(pbxAccount);
	MessageBox(str, "PBX Account", MB_OK);

}

/**
* ���е绰
*/
void CEasiioLibraryDemoDlg::OnBnClickedCall()
{
	if (mCallInfo != NULL && mCallInfo->meetflag) {
		return;
	}
	CString str;
	GetDlgItem(CALL_EDIT)->GetWindowText(str);
	if (str.IsEmpty()){
		return;
	}
	const char* number = str.GetBuffer(sizeof(str));
	int result = EasiioMakeCall(number, VIDEO_CALL, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
	if (result < 0){
		printf("Make call failed!");
		return;
	}
	
}

/**
* �����绰
*/
void CEasiioLibraryDemoDlg::OnBnClickedAnswer()
{
	if (mCallInfo == NULL || mCallInfo->isIncoming != 1){
		printf("OnBnClickedAnswer failed.");
		return;
	}

	EasiioAnswerCall(mCallInfo->callId, VIDEO_CALL);
}

/**
* �Ҷϵ绰
*/
void CEasiioLibraryDemoDlg::OnBnClickedHangup()
{
	if (mCallInfo == NULL || mCallInfo->isAlive == 0){
		printf("OnBnClickedHangup failed.");
		return;
	}
	EasiioHangupCall(mCallInfo->callId);
}

/**
* ͨ������
*/
void CEasiioLibraryDemoDlg::OnBnClickedHold()
{
	if (mCallInfo == NULL || mCallInfo->isAlive == 0){
		printf("OnBnClickedHold failed.");
		return;
	}
	if (mCallInfo->hold == 1){
		printf("OnBnClickedHold failed, already hold.");
		return;
	}
	EasiioHoldCall(mCallInfo->callId);
	updateCallHoldState(mCallInfo->callId, 1);
}

/**
* ȡ��ͨ������
*/
void CEasiioLibraryDemoDlg::OnBnClickedUnhold()
{
	if (mCallInfo == NULL || mCallInfo->isAlive == 0){
		printf("OnBnClickedUnhold failed.");
		return;
	}
	if (mCallInfo->hold == 0){
		printf("OnBnClickedUnhold failed, already unhold.");
		return;
	}
	EasiioUnHoldCall(mCallInfo->callId);
	updateCallHoldState(mCallInfo->callId, 0);
}

/**
* ͨ������
*/
void CEasiioLibraryDemoDlg::OnBnClickedMute()
{
	if (mCallInfo == NULL || mCallInfo->isAlive == 0){
		printf("OnBnClickedMute failed.");
		return;
	}
	if (mCallInfo->mute == 1){
		printf("OnBnClickedMute failed, already mute.");
		return;
	}
	EasiioMuteCall(mCallInfo->callId);
	updateCallMuteState(mCallInfo->callId, 1);
}

/**
* ȡ��ͨ������
*/
void CEasiioLibraryDemoDlg::OnBnClickedUnmute()
{
	if (mCallInfo == NULL || mCallInfo->isAlive == 0){
		printf("OnBnClickedUnmute failed.");
		return;
	}
	if (mCallInfo->mute == 0){
		printf("OnBnClickedUnmute failed, already unmute.");
		return;
	}
	EasiioUnMuteCall(mCallInfo->callId);
	updateCallMuteState(mCallInfo->callId, 0);
}

/**
* ˫�����
*/
void CEasiioLibraryDemoDlg::OnBnClickedMakTwoWayCall()
{
	CString fromPhoneStr;
	GetDlgItem(FROM_PHONE_EDIT)->GetWindowText(fromPhoneStr);
	if (fromPhoneStr.IsEmpty()){
		return;
	}
	const char* fromPhone = fromPhoneStr.GetBuffer(sizeof(fromPhoneStr));

	CString toPhoneStr;
	GetDlgItem(TO_PHONE_EDIT)->GetWindowText(toPhoneStr);
	if (toPhoneStr.IsEmpty()){
		return;
	}
	const char* toPhone = toPhoneStr.GetBuffer(sizeof(toPhoneStr));

	EasiioMakeTwoWayCall(toPhone, fromPhone, 0);
}

/**
* ����ͨ����¼��
*/
void CEasiioLibraryDemoDlg::OnBnClickedStartRecord()
{
	if (mCallInfo == NULL || mCallInfo->isAlive == 0){
		printf("OnBnClickedStartRecord failed.");
		return;
	}
	if (mCallInfo->record == RECORD_START){
		printf("OnBnClickedStartRecord failed, already record.");
		return;
	}
	EasiioSwitchRecordInCall(RECORD_START, mCallInfo->callUUID);
	
}

/**
* �ر�ͨ����¼��
*/
void CEasiioLibraryDemoDlg::OnBnClickedStopRecord()
{
	if (mCallInfo == NULL || mCallInfo->isAlive == 0){
		printf("OnBnClickedStopRecord failed.");
		return;
	}
	if (mCallInfo->record == RECORD_STOP){
		printf("OnBnClickedStopRecord failed, already stop record.");
		return;
	}
	EasiioSwitchRecordInCall(RECORD_STOP, mCallInfo->callUUID);
}

/**
* ����ܽ�
*/
void CEasiioLibraryDemoDlg::OnBnClickedReject()
{
	if (mCallInfo == NULL || mCallInfo->isAlive == 0){
		printf("OnBnClickedReject failed.");
		return;
	}
	if (mCallInfo->isIncoming == 0){
		printf("OnBnClickedReject failed, not incoming call");
		return;
	}
	EasiioRejectCall(mCallInfo->callId);
}


void CEasiioLibraryDemoDlg::OnBnClickedSendDtmf()
{
	if (mCallInfo == NULL || mCallInfo->isAlive == 0){
		printf("OnBnClickedReject failed.");
		return;
	}

	CString dtmfStr;
	GetDlgItem(DTMF_EDIT)->GetWindowText(dtmfStr);
	if (dtmfStr.IsEmpty()){
		return;
	}
	const char* dtmf = dtmfStr.GetBuffer(sizeof(dtmfStr));

	EasiioSendDTMF(mCallInfo->callId, dtmf);
}


void CEasiioLibraryDemoDlg::OnBnClickedLogout()
{
	EasiioLogout();
	CButton* loginBtn = (CButton*)GetDlgItem(BUTTON_LOGIN);
	loginBtn->ShowWindow(true);
	CButton* logoutBtn = (CButton*)GetDlgItem(BUTTON_LOGOUT);
	logoutBtn->ShowWindow(false);

	CEdit* callEdit = (CEdit*)GetDlgItem(CALL_EDIT);
	callEdit->SetReadOnly(true);

	CEdit* fromEdit = (CEdit*)GetDlgItem(FROM_PHONE_EDIT);
	fromEdit->SetReadOnly(true);

	CEdit* toEdit = (CEdit*)GetDlgItem(TO_PHONE_EDIT);
	toEdit->SetReadOnly(true);
}


void CEasiioLibraryDemoDlg::OnBnClickedGetAllRecord0()
{
	EasiioGetAllRecord(RECORD_TYPE_FROM_CALL_START);
}


void CEasiioLibraryDemoDlg::OnBnClickedGetAllRecord1()
{
	EasiioGetAllRecord(RECORD_TYPE_IN_CALL);
}


void CEasiioLibraryDemoDlg::OnBnClickedRefreshMic()
{
	EasiioRefreshMic();
}


void CEasiioLibraryDemoDlg::OnBnClickedDestroy()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	EasiioDestroy();
}




void CEasiioLibraryDemoDlg::OnBnClickedCallRecording()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if (mCallInfo != NULL && mCallInfo->meetflag) {
		return;
	}
	CString str;
	GetDlgItem(CALL_EDIT)->GetWindowText(str);
	if (str.IsEmpty()){
		return;
	}
	const char* number = str.GetBuffer(sizeof(str));
	int result = EasiioMakeCallWithRecording(number, VOICE_CALL);
	if (result < 0){
		printf("Make call failed!");
		return;
	}
}



void CEasiioLibraryDemoDlg::OnBnClickedCreateMeeting() {
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString descStr;
	GetDlgItem(DESC_EDIT)->GetWindowText(descStr);
	if (descStr.IsEmpty()) {
		return;
	}
	const char* desc = descStr.GetBuffer(sizeof(descStr));
	CString priorityStr;
	GetDlgItem(PRIORITY_EDIT)->GetWindowText(priorityStr);
	const char* Priority = priorityStr.GetBuffer(sizeof(priorityStr));
	int priority = atoi(Priority);
	EasiioCreateMeeting(desc, priority);
}


void CEasiioLibraryDemoDlg::OnBnClickedDelete() {
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString meetingkeyStr;
	GetDlgItem(MEETING_KEY_EDIT)->GetWindowText(meetingkeyStr);
	if (meetingkeyStr.IsEmpty()) {
		return;
	}
	const char* meetingkey = meetingkeyStr.GetBuffer(sizeof(meetingkeyStr));
	int ret = EasiioDeleteMeeting(meetingkey);
	if (!ret) {
		GetDlgItem(MEETING_DESC_EDIT)->SetWindowText("");

		GetDlgItem(MEETING_KEY_EDIT)->SetWindowText("");

		CEdit* descEdit = (CEdit*)GetDlgItem(DESC_EDIT);
		descEdit->SetReadOnly(false);

		CEdit* priorityEdit = (CEdit*)GetDlgItem(PRIORITY_EDIT);
		priorityEdit->SetReadOnly(false);

		CButton* createBtn = (CButton*)GetDlgItem(BUTTON_CREATE_MEETING);
		createBtn->EnableWindow(true);

		CButton* modifyBtn = (CButton*)GetDlgItem(BUTTON_MODIFY);
		modifyBtn->EnableWindow(false);

		CButton* saveBtn = (CButton*)GetDlgItem(BUTTON_SAVE);
		saveBtn->EnableWindow(false);

		CButton* deleteBtn = (CButton*)GetDlgItem(BUTTON_DELETE);
		deleteBtn->EnableWindow(false);
	}
}


void CEasiioLibraryDemoDlg::OnBnClickedModify() {
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CEdit* descEdit = (CEdit*)GetDlgItem(DESC_EDIT);
	descEdit->SetReadOnly(false);

	CEdit* priorityEdit = (CEdit*)GetDlgItem(PRIORITY_EDIT);
	priorityEdit->SetReadOnly(false);

	CButton* modifyBtn = (CButton*)GetDlgItem(BUTTON_MODIFY);
	modifyBtn->EnableWindow(false);

	CButton* saveBtn = (CButton*)GetDlgItem(BUTTON_SAVE);
	saveBtn->EnableWindow(true);
}


void CEasiioLibraryDemoDlg::OnBnClickedSave() {
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString descStr;
	GetDlgItem(DESC_EDIT)->GetWindowText(descStr);
	if (descStr.IsEmpty()) {
		return;
	}
	const char* desc = descStr.GetBuffer(sizeof(descStr));
	CString priorityStr;
	GetDlgItem(PRIORITY_EDIT)->GetWindowText(priorityStr);
	const char* Priority = priorityStr.GetBuffer(sizeof(priorityStr));
	int priority = atoi(Priority);

	CString meetingkeyStr;
	GetDlgItem(MEETING_KEY_EDIT)->GetWindowText(meetingkeyStr);
	if (meetingkeyStr.IsEmpty()) {
		return;
	}
	const char* meetingkey = meetingkeyStr.GetBuffer(sizeof(meetingkeyStr));
	int ret = EasiioUpdateMeeting(meetingkey, desc, priority);
	if (!ret) {
		CEdit* descEdit = (CEdit*)GetDlgItem(DESC_EDIT);
		descEdit->SetReadOnly(true);

		CEdit* priorityEdit = (CEdit*)GetDlgItem(PRIORITY_EDIT);
		priorityEdit->SetReadOnly(true);

		CButton* modifyBtn = (CButton*)GetDlgItem(BUTTON_MODIFY);
		modifyBtn->EnableWindow(true);

		CButton* saveBtn = (CButton*)GetDlgItem(BUTTON_SAVE);
		saveBtn->EnableWindow(false);

		GetDlgItem(MEETING_DESC_EDIT)->SetWindowText(descStr);
	}
}

void CEasiioLibraryDemoDlg::OnBnClickedStart() {
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (mCallInfo != NULL && (mCallInfo->isAlive == 1 || mCallInfo->meetflag)) {
		return;
	}
	if (mCallInfo == NULL) {
		mCallInfo = new CallInfo;
		memset(mCallInfo, 0, sizeof(CallInfo));
	}
	CString meetingkeyStr;
	GetDlgItem(MEETING_KEY_EDIT)->GetWindowText(meetingkeyStr);
	if (meetingkeyStr.IsEmpty()) {
		return;
	}
	const char* meetingkey = meetingkeyStr.GetBuffer(sizeof(meetingkeyStr));
	char* start_time = new char[20];
	int status = EasiioGetMeetingStatus(meetingkey);
	if (!status) {
		int ret = EasiioStartMeeting(meetingkey, "720p", "windows", start_time);
		if (!ret) {
			mCallInfo->meetflag = 1;
			mCallInfo->meethost = 1;
			GetDlgItem(MEETING_STATUS_EDIT)->SetWindowText("1");
		}
	}

}


void CEasiioLibraryDemoDlg::OnBnClickedStop() {
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString meetingkeyStr;
	GetDlgItem(MEETING_KEY_EDIT)->GetWindowText(meetingkeyStr);
	if (meetingkeyStr.IsEmpty()) {
		return;
	}
	const char* meetingkey = meetingkeyStr.GetBuffer(sizeof(meetingkeyStr));
	int status = EasiioGetMeetingStatus(meetingkey);
	if (status) {
		int ret = EasiioStopMeeting(meetingkey, "vga", "windows");
		if (!ret) {
			if (mCallInfo != NULL) {
				mCallInfo->meetflag = 0;
				mCallInfo->meethost = 0;
			}
			GetDlgItem(MEETING_STATUS_EDIT)->SetWindowText("0");
		}
	}
}


void CEasiioLibraryDemoDlg::OnBnClickedJoinMeeting() {
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (mCallInfo != NULL && (mCallInfo->isAlive == 1 || mCallInfo->meetflag)) {
		return;
	}
	CString str;
	GetDlgItem(CALL_EDIT)->GetWindowText(str);
	if (str.IsEmpty()) {
		return;
	}
	const char* meetingkey = str.GetBuffer(sizeof(str));
	int ret = EasiioJoinMeeting(meetingkey);
	if (!ret) {

	}
}


void CEasiioLibraryDemoDlg::OnBnClickedStartPreview()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//int devcount;
	//int ret = EasiioGetDeviceCount(&devcount);
	//if (ret < 0) {
	//	return;
	//}
	//	char deviceinfo[256] = { 0 };
	//	ret = EasiioGetDeviceInfoById(0, deviceinfo);
	//vector<char *> infolist = SplitCString(deviceinfo, "|");
	//char *url = infolist.at(infolist.size() - 1);
	CString urlstr;
	GetDlgItem(MONITOR_URL_EDIT)->GetWindowText(urlstr);
	char *url = urlstr.GetBuffer(sizeof(urlstr));
	EasiioStartPreview(1, &url);
	CEdit *urledit = (CEdit *)GetDlgItem(MONITOR_URL_EDIT);
	urledit->SetReadOnly(true);
}


void CEasiioLibraryDemoDlg::OnBnClickedStopPreview()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	EasiioStopPreview();
	CEdit *urledit = (CEdit *)GetDlgItem(MONITOR_URL_EDIT);
	urledit->SetReadOnly(false);
}
