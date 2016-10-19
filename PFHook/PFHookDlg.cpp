
// PFHookDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PFHook.h"
#include "PFHookDlg.h"
#include "afxdialogex.h"
#include <winsvc.h>
#include <TlHelp32.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPFHookDlg 对话框

#define DRIVER_NAME TEXT("PFHook")
#define DRIVER_PATH TEXT("PFHook.sys")

// 通信代码
#define PFHOOK_CODE_CHECKVT	0x800
#define PFHOOK_CODE_HOOK	0x801
#define PFHOOK_CODE_UNHOOK	0x802


#define PFHOOK_WELCOME_TEXT		TEXT("Version:1.0a\nThe program and driver are made by Xiaobao.\nQQ:1121402724\nHave fun!  O(∩_∩)O~~")

BOOL LoadNTDriver(TCHAR* lpszDriverName, TCHAR* lpszDriverPath);
BOOL UnloadNTDriver(TCHAR* szSvrName);

CPFHookDlg::CPFHookDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PFHOOK_DIALOG, pParent)
	, m_iDriverStatus(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPFHookDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_PROCESS, m_cbbProc);
	DDX_Control(pDX, IDC_LIST_HOOK_LIST, m_lbHook);
	DDX_Control(pDX, IDC_EDIT_HOOKED_ADDRESS2, m_editHookAddr);
	DDX_Control(pDX, IDC_EDIT_JMP_ADDRESS, m_editJmpAddr);
	DDX_Control(pDX, IDC_STATIC_DRIVER_STATUS, m_staticStatus);
}

BEGIN_MESSAGE_MAP(CPFHookDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CPFHookDlg::OnBnClickedButtonRefresh)
	ON_BN_CLICKED(IDC_BUTTON_HOOK, &CPFHookDlg::OnBnClickedButtonHook)
	ON_BN_CLICKED(IDC_BUTTON_Unhook, &CPFHookDlg::OnBnClickedButtonUnhook)
END_MESSAGE_MAP()


// CPFHookDlg 消息处理程序

BOOL CPFHookDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	MessageBox(PFHOOK_WELCOME_TEXT, TEXT("Welcome"), MB_OK | MB_ICONINFORMATION);
	RefreshProcess();

	BOOL bRet = LoadNTDriver(DRIVER_NAME, DRIVER_PATH);
	if (!bRet)
	{
		m_staticStatus.SetWindowText(TEXT("Fail to load the driver!"));
		m_iDriverStatus = 1;
	}
	else if (!CheckVT())
	{
		m_staticStatus.SetWindowText(TEXT("Fail to load the VT Engine!"));
		m_iDriverStatus = 2;
	}
	else
	{
		m_staticStatus.SetWindowText(TEXT("The driver is active now."));
		m_iDriverStatus = 0;
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPFHookDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPFHookDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CPFHookDlg::OnOK()
{
	//CDialogEx::OnOK();
}


BOOL CPFHookDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			return 0;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CPFHookDlg::OnClose()
{
	if (m_iDriverStatus == 1)
	{
		CDialogEx::OnClose();
		return;
	}

	UINT uResult = MessageBox(TEXT("You really wanna do this?"),TEXT("Warning"),MB_YESNO|MB_ICONQUESTION);
	if (uResult == IDYES)
	{
		if (m_iDriverStatus == 0)
		{ 
			for (int i = 0;i < (int)m_vecHookItem.size();i++)
			{
				RemoveHookItem(m_vecHookItem.at(i).iHookID);
			}
		}
		BOOL bRet = UnloadNTDriver(DRIVER_NAME);
		if (!bRet)
			MessageBox(TEXT("Opp! Fail to unload the driver!"), TEXT("Error"), MB_OK | MB_ICONERROR);
		CDialogEx::OnClose();
	}
}



BOOL LoadNTDriver(TCHAR* lpszDriverName, TCHAR* lpszDriverPath)
{
	TCHAR szDriverImagePath[256];
	//得到完整的驱动路径
	GetFullPathName(lpszDriverPath, 256, szDriverImagePath, NULL);

	BOOL bRet = FALSE;

	SC_HANDLE hServiceMgr = NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK = NULL;//NT驱动程序的服务句柄

								 //打开服务控制管理器
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hServiceMgr == NULL)
	{
		//OpenSCManager失败
		//printf("OpenSCManager() Faild %d ! \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		////OpenSCManager成功
		//printf("OpenSCManager() ok ! \n");
	}

	//创建驱动所对应的服务
	hServiceDDK = CreateService(hServiceMgr,
		lpszDriverName, //驱动程序的在注册表中的名字  
		lpszDriverName, // 注册表驱动程序的 DisplayName 值  
		SERVICE_ALL_ACCESS, // 加载驱动程序的访问权限  
		SERVICE_KERNEL_DRIVER,// 表示加载的服务是驱动程序  
		SERVICE_DEMAND_START, // 注册表驱动程序的 Start 值  
		SERVICE_ERROR_IGNORE, // 注册表驱动程序的 ErrorControl 值  
		szDriverImagePath, // 注册表驱动程序的 ImagePath 值  
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);

	DWORD dwRtn;
	//判断服务是否失败
	if (hServiceDDK == NULL)
	{
		dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)
		{
			//由于其他原因创建服务失败
			//printf("CrateService() Faild %d ! \n", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			//服务创建失败，是由于服务已经创立过
			//printf("CrateService() Faild Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! \n");
		}
		hServiceDDK = OpenService(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);
		if (hServiceDDK == NULL)
		{
			//如果打开服务也失败，则意味错误
			dwRtn = GetLastError();
			//printf("OpenService() Faild %d ! \n", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			//printf("OpenService() ok ! \n");
		}
	}
	else
	{
		//printf("CrateService() ok ! \n");
	}

	//开启此项服务
	bRet = StartService(hServiceDDK, NULL, NULL);
	if (!bRet)
	{
		DWORD dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING)
		{
			//printf("StartService() Faild %d ! \n", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			if (dwRtn == ERROR_IO_PENDING)
			{
				//设备被挂住
				//printf("StartService() Faild ERROR_IO_PENDING ! \n");
				bRet = FALSE;
				goto BeforeLeave;
			}
			else
			{
				//服务已经开启
				//printf("StartService() Faild ERROR_SERVICE_ALREADY_RUNNING ! \n");
				bRet = TRUE;
				goto BeforeLeave;

			}
		}
	}
	bRet = TRUE;
	//离开前关闭句柄
BeforeLeave:
	if (hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;
}

//卸载驱动程序  
BOOL UnloadNTDriver(TCHAR* szSvrName)
{
	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK = NULL;//NT驱动程序的服务句柄
	SERVICE_STATUS SvrSta;
	//打开SCM管理器
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		//带开SCM管理器失败
		//printf("OpenSCManager() Faild %d ! \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		//带开SCM管理器失败成功
		//printf("OpenSCManager() ok ! \n");
	}
	//打开驱动所对应的服务
	hServiceDDK = OpenService(hServiceMgr, szSvrName, SERVICE_ALL_ACCESS);

	if (hServiceDDK == NULL)
	{
		//打开驱动所对应的服务失败
		//printf("OpenService() Faild %d ! \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
//		printf("OpenService() ok ! \n");
	}
	//停止驱动程序，如果停止失败，只有重新启动才能，再动态加载。  
	if (!ControlService(hServiceDDK, SERVICE_CONTROL_STOP, &SvrSta))
	{
		//printf("ControlService() Faild %d !\n", GetLastError());
	}
	else
	{
		//打开驱动所对应的失败
		//printf("ControlService() ok !\n");
	}
	//动态卸载驱动程序。  
	if (!DeleteService(hServiceDDK))
	{
		//卸载失败
		//printf("DeleteSrevice() Faild %d !\n", GetLastError());
	}
	else
	{
		//卸载成功
		//printf("DelServer:eleteSrevice() ok !\n");
	}
	bRet = TRUE;
BeforeLeave:
	//离开前关闭打开的句柄
	if (hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;
}

HBRUSH CPFHookDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetSafeHwnd() == m_staticStatus.GetSafeHwnd())
	{
		if (m_iDriverStatus)
		{
			pDC->SetTextColor(RGB(255, 0, 0));
		}
		else 	pDC->SetTextColor(RGB(0, 255, 0));

	}
	// TODO:  在此更改 DC 的任何特性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}


bool CPFHookDlg::RefreshProcess()
{
	HANDLE hSnapshot = NULL;
	PROCESSENTRY32 proc;
	BOOL bMore;

	ZeroMemory(&proc, sizeof(proc));
	proc.dwSize = sizeof(proc);

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (!hSnapshot) return false;
	m_cbbProc.ResetContent();
	m_cbbProc.SetCurSel(-1);
	
	bMore = Process32First(hSnapshot, &proc);
	while (bMore)
	{
		int i = m_cbbProc.InsertString(-1, proc.szExeFile);
		m_cbbProc.SetItemData(i, proc.th32ProcessID);

		bMore = Process32Next(hSnapshot, &proc);
	}
	CloseHandle(hSnapshot);
	return true;
}


void CPFHookDlg::OnBnClickedButtonRefresh()
{
	RefreshProcess();
}


void CPFHookDlg::OnBnClickedButtonHook()
{
	CString strTemp,strProcName;
	int iRet;
	HOOKITEM item;
	int iID;
	DWORD dwPID,dwHookedAddress, dwJmpAddress;
	
	if (m_cbbProc.GetCurSel() == -1 || m_iDriverStatus)
		return;

	dwPID = m_cbbProc.GetItemData(m_cbbProc.GetCurSel());
	m_editHookAddr.GetWindowText(strTemp);
	dwHookedAddress = wcstol(strTemp,NULL,16);
	m_editJmpAddr.GetWindowText(strTemp);
	dwJmpAddress = wcstol(strTemp, NULL, 16);

	if (dwHookedAddress == 0 || dwJmpAddress == 0)
		return;

	iRet = AddHookItem(dwPID, (LPVOID)dwHookedAddress,(LPVOID)dwJmpAddress);
	if (iRet!=-1)
	{
		MessageBeep(MB_OK);
		m_cbbProc.GetLBText(m_cbbProc.GetCurSel(), strProcName);
		strTemp.Format(TEXT("[%d] %s[%d]    0x%X --> 0x%X"), iRet,strProcName,dwPID,dwHookedAddress,dwJmpAddress);
		iID = m_lbHook.InsertString(-1, strTemp);

		item.iID = iID;
		item.iHookID = iRet;
		item.dwPID = dwPID;
		item.lpHookedAddr = (LPVOID)dwHookedAddress;
		item.lpJmpAddr = (LPVOID)dwJmpAddress;
		m_vecHookItem.push_back(item);
	}
	else MessageBeep(MB_ICONERROR);
}


void CPFHookDlg::OnBnClickedButtonUnhook()
{
	HOOKITEM item;
	if (m_lbHook.GetCurSel() == -1 || m_iDriverStatus)
		return;

	item = m_vecHookItem.at(m_lbHook.GetCurSel());

	RemoveHookItem(item.iHookID);

	m_vecHookItem.erase(m_vecHookItem.begin() + m_lbHook.GetCurSel());
	m_lbHook.DeleteString(m_lbHook.GetCurSel());
}


int CPFHookDlg::AddHookItem(DWORD dwPID, LPVOID lpHookedAddr, LPVOID lpJmpAddr)
{
	DWORD dwRet;
	if (m_iDriverStatus)
		return FALSE;

	__asm
	{
		xor esi,esi
		dec esi
		mov eax, PFHOOK_CODE_HOOK
		mov ebx, dwPID
		mov ecx,lpHookedAddr
		mov edx,lpJmpAddr
		cpuid
		mov dwRet, esi
	}
	return dwRet;
}


BOOL CPFHookDlg::RemoveHookItem(int iItemID)
{
	BOOL bRet;
	if (m_iDriverStatus)
		return FALSE;

	__asm
	{
		xor esi, esi
		mov eax, PFHOOK_CODE_UNHOOK
		mov ecx, iItemID
		cpuid
		mov bRet, esi
	}
	return bRet;
}


BOOL CPFHookDlg::CheckVT()
{
	BOOL bRet;
	if (m_iDriverStatus)
		return FALSE;

	__asm
	{
		xor esi, esi
		mov eax, PFHOOK_CODE_CHECKVT
		cpuid
		mov bRet, esi
	}
	return bRet;
}
