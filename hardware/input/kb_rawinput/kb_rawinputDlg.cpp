
// kb_rawinputDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "kb_rawinput.h"
#include "kb_rawinputDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CkbrawinputDlg 对话框



CkbrawinputDlg::CkbrawinputDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_KB_RAWINPUT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CkbrawinputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CkbrawinputDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_INPUT()
END_MESSAGE_MAP()


// CkbrawinputDlg 消息处理程序

BOOL CkbrawinputDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	// 注册raw input
	RAWINPUTDEVICE rawinputdevice[1];
	rawinputdevice[0].usUsagePage = 0x01; // Generic Desktop Controls
	rawinputdevice[0].usUsage = 0x06;    // Keyboard
	rawinputdevice[0].dwFlags = RIDEV_INPUTSINK;// If set, this enables the caller to receive the input even when the caller is not in the foreground. Note that hwndTarget must be specified. 
	rawinputdevice[0].hwndTarget = AfxGetMainWnd()->m_hWnd;

	BOOL ret = RegisterRawInputDevices(&rawinputdevice[0], 1, sizeof(RAWINPUTDEVICE));
	if (ret == FALSE)
	{
		AfxMessageBox(_T("RegisterRawInputDevices return FALSE！"));
		exit(0);
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CkbrawinputDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CkbrawinputDlg::OnPaint()
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
HCURSOR CkbrawinputDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CkbrawinputDlg::OnRawInput(UINT nInputcode, HRAWINPUT hRawInput)
{
	// 该功能要求使用 Windows XP 或更高版本。
	// 符号 _WIN32_WINNT 必须 >= 0x0501。
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CString sInfo = _T("");
	UINT uiSize = 0;

	GetRawInputData((HRAWINPUT)hRawInput, RID_INPUT, NULL, &uiSize, sizeof(RAWINPUTHEADER));

	LPBYTE pData = new BYTE[uiSize];

	if (GetRawInputData((HRAWINPUT)hRawInput, RID_INPUT, pData, &uiSize, sizeof(RAWINPUTHEADER)) != uiSize) //获取消息信息
	{
		sInfo = _T("GetRawInputData error!\r\n");
		GetDlgItem(IDC_STATIC_INFO)->SetWindowText(sInfo);
	}
	else
	{
		RAWINPUT* pRawinput = (RAWINPUT*)pData;

		if (pRawinput->header.dwType == RIM_TYPEKEYBOARD)
		{
			GetRawInputDeviceInfo(pRawinput->header.hDevice, RIDI_DEVICENAME, NULL, &uiSize);
			TCHAR sBuffer[1024] = { 0 };
			GetRawInputDeviceInfo(pRawinput->header.hDevice, RIDI_DEVICENAME, sBuffer, &uiSize);
			sInfo += _T("device name:");

			if (uiSize > 0)
				sInfo += sBuffer;

			if (pRawinput->data.keyboard.Message == WM_KEYDOWN || pRawinput->data.keyboard.Message == WM_SYSKEYDOWN)
			{
				TCHAR sAscii[10] = { 0 };
				BYTE sKeyState[256] = { 0 };
				sInfo += _T("\r\ninput:");

				switch (pRawinput->data.keyboard.VKey)
				{
				case VK_BACK:sInfo += _T("back"); break;
				case VK_TAB:sInfo += _T("tab"); break;
				case VK_RETURN:sInfo += _T("enter"); break;
				case VK_SHIFT:sInfo += _T("shift"); break;
				case VK_CONTROL:sInfo += _T("ctrl"); break;
				case VK_CAPITAL:sInfo += _T("caps lock"); break;
				case VK_ESCAPE:sInfo += _T("escape"); break;
				case VK_SPACE:sInfo += _T("space"); break;
				case VK_END:sInfo += _T("end"); break;
				case VK_HOME:sInfo += _T("home"); break;
				case VK_LEFT:sInfo += _T("left"); break;
				case VK_UP:sInfo += _T("up"); break;
				case VK_RIGHT:sInfo += _T("right"); break;
				case VK_DOWN:sInfo += _T("down"); break;
				case VK_SNAPSHOT:sInfo += _T("print"); break;
				case VK_INSERT:sInfo += _T("insert"); break;
				case VK_DELETE:sInfo += _T("delete"); break;
				case VK_MENU:sInfo += _T("alt"); break;
				case VK_F1:sInfo += _T("F1"); break;
				case VK_F2:sInfo += _T("F2"); break;
				case VK_F3:sInfo += _T("F3"); break;
				case VK_F4:sInfo += _T("F4"); break;
				case VK_F5:sInfo += _T("F5"); break;
				case VK_F6:sInfo += _T("F6"); break;
				case VK_F7:sInfo += _T("F7"); break;
				case VK_F8:sInfo += _T("F8"); break;
				case VK_F9:sInfo += _T("F9"); break;
				case VK_F10:sInfo += _T("F10"); break;
				case VK_F11:sInfo += _T("F11"); break;
				case VK_F12:sInfo += _T("F12"); break;
				default:
					if (ToAscii(pRawinput->data.keyboard.VKey, pRawinput->data.keyboard.MakeCode, sKeyState, (LPWORD)sAscii, 0))
						sInfo += sAscii;
					break;
				}

				GetDlgItem(IDC_STATIC_INFO)->SetWindowText(sInfo);
			}

		}
	}
	delete[] pData;
	pData = nullptr;
	CDialogEx::OnRawInput(nInputcode, hRawInput);
}


BOOL CkbrawinputDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_RETURN: // 屏蔽回车
			return TRUE;
		case VK_ESCAPE: // 屏蔽ESC
			if (IDCANCEL == AfxMessageBox(_T("exit?"), MB_OKCANCEL))
				return TRUE;
			break;
		default:
			break;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}
