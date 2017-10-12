
// EasiioLibraryDemoDlg.h : 头文件
//

#pragma once


// CEasiioLibraryDemoDlg 对话框
class CEasiioLibraryDemoDlg : public CDialogEx
{
// 构造
public:
	CEasiioLibraryDemoDlg(CWnd* pParent = NULL);	// 标准构造函数
	typedef struct _CallInfo{
		int callId;
		int isIncoming;
		int status;
		int isAlive;
		int hold;
		int mute;
		int record;
		char* number;
		char* postValue;
		char* callUUID;
		int meetflag;
		int meethost;
	}CallInfo;

	virtual void showLoginResultDialog(CString msg);
	virtual void setPJSIPStatusEdit(CString status);
	virtual void setCallStatusEdit(CString status);
	virtual void setIncomingEdit(CString number);
	virtual void createCallInfo(int callId, int isIncoming, int status, int isAlive, char* number, char* postValue, char* callUUID);
	virtual void updateCallStatus(int callId, int status, char* callUUID);
	virtual void unAliveCallInfo(int callId);
	virtual void updateCallHoldState(int callId, int hold);
	virtual void updateCallMuteState(int callId, int mute);
	virtual void updateCallRecordState(CString callUUID, int record);
	virtual void loginSuccess();
	virtual void inCall();
	virtual void outCall();
	virtual void showMeetinginfo(CString desc, CString meetingkey,int priority);
	virtual void updatejoinMeeting(CString meetingkey);
	virtual void updateleaveMeeting();

// 对话框数据
	enum { IDD = IDD_EASIIOLIBRARYDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual void InitConsoleWindow();
	//virtual void onLoginResult(int resultCode, const char* resultMsg);
	//virtual void onPJSIPServiceStateChanged(int stateCode); // PJSIP状态变化通知
	//virtual void onCallStatusChanged(int callId, int callState, const char* resultCode, const char* callUUID); // 通话状态变化通知
	//virtual void onTwoWayCallStatusChanged(int callId, int callState, const char* caller, const char* callee, const char* createTime); // 双向呼叫通话状态通知
	//virtual void onIncomingCall(int callId, int callType, const char* fromNumber, const char* postValue); // 来电通知

	//// response
	//virtual void onMakeTwoWayCallResult(EasiioResponseReason reason, const char* caller, const char* callee, const char* callId, const char* retryId); //双向呼叫结果
	//virtual void onCallRecordingReaponse(EasiioResponseReason reason, const char* callUUID); // 通话中开启、停止录音调用结果
	
// 实现
protected:
	HICON m_hIcon;
	CallInfo* mCallInfo;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonLogin();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedCall();
	afx_msg void OnBnClickedAnswer();
	afx_msg void OnBnClickedHangup();
	afx_msg void OnBnClickedHold();
	afx_msg void OnBnClickedUnhold();
	afx_msg void OnBnClickedMute();
	afx_msg void OnBnClickedUnmute();
	afx_msg void OnBnClickedMakTwoWayCall();
	afx_msg void OnBnClickedStartRecord();
	afx_msg void OnBnClickedStopRecord();
	afx_msg void OnBnClickedReject();
	afx_msg void OnBnClickedSendDtmf();
	afx_msg void OnBnClickedLogout();
	afx_msg void OnBnClickedGetAllRecord0();
	afx_msg void OnBnClickedGetAllRecord1();
	afx_msg void OnBnClickedRefreshMic();
	afx_msg void OnBnClickedDestroy();
	afx_msg void OnBnClickedCallRecording();
	afx_msg void OnEnChangeEdit();
	afx_msg void OnBnClickedCreateMeeting();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedModify();
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedJoinMeeting();
	afx_msg void OnBnClickedStartPreview();
	afx_msg void OnBnClickedStopPreview();
};
