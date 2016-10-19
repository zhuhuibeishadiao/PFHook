
// PFHookDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include <vector>

using namespace std;

typedef struct _HOOKITEM
{
	int iID;
	int iHookID;
	DWORD dwPID;
	LPVOID lpHookedAddr;
	LPVOID lpJmpAddr;
}HOOKITEM, *PHOOKITEM;

// CPFHookDlg 对话框
class CPFHookDlg : public CDialogEx
{
// 构造
public:
	CPFHookDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PFHOOK_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_cbbProc;
	virtual void OnOK();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	int m_iDriverStatus;
	bool RefreshProcess();
	afx_msg void OnBnClickedButtonRefresh();
	afx_msg void OnBnClickedButtonHook();
	afx_msg void OnBnClickedButtonUnhook();
	CListBox m_lbHook;
	CEdit m_editHookAddr;
	CEdit m_editJmpAddr;
	int AddHookItem(DWORD dwPID, LPVOID lpHookedAddr, LPVOID lpJmpAddr);
	BOOL RemoveHookItem(int iItemID);
	vector<HOOKITEM> m_vecHookItem;
	CStatic m_staticStatus;
	BOOL CheckVT();
};
