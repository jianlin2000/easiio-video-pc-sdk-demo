
// EasiioLibraryDemo.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CEasiioLibraryDemoApp: 
// �йش����ʵ�֣������ EasiioLibraryDemo.cpp
//

class CEasiioLibraryDemoApp : public CWinApp
{
public:
	CEasiioLibraryDemoApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CEasiioLibraryDemoApp theApp;