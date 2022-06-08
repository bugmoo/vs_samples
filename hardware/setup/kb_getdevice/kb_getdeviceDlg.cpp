
// kb_getdeviceDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "kb_getdevice.h"
#include "kb_getdeviceDlg.h"
#include "afxdialogex.h"

// 包含设备安装类api
#include "setupapi.h"
// 包含GUID
#include "initguid.h"
#include <devguid.h>
#include <ntddkbd.h>

#pragma comment(lib,"setupapi.lib")


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


// CkbgetdeviceDlg 对话框



CkbgetdeviceDlg::CkbgetdeviceDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_KB_GETDEVICE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CkbgetdeviceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CkbgetdeviceDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_GET_KB, &CkbgetdeviceDlg::OnBnClickedButtonGetKb)
END_MESSAGE_MAP()


// CkbgetdeviceDlg 消息处理程序

BOOL CkbgetdeviceDlg::OnInitDialog()
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
	CListCtrl* list = (CListCtrl*)GetDlgItem(IDC_LIST_KB);
	list->InsertColumn(0, _T("键盘设备路径"), LVCFMT_FILL, 500);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CkbgetdeviceDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CkbgetdeviceDlg::OnPaint()
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
HCURSOR CkbgetdeviceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CkbgetdeviceDlg::OnBnClickedButtonGetKb()
{
	// TODO: 在此添加控件通知处理程序代码
	if ( TRUE == GetKBDevicePath(m_listKB))
		UpdateList();

}

/// <summary>
/// 更新 list control
/// </summary>
/// <param name=""></param>
void CkbgetdeviceDlg::UpdateList(void)
{
	int i = 0;
	CListCtrl* list = (CListCtrl*)GetDlgItem(IDC_LIST_KB);
	list->DeleteAllItems();
	//while (list->DeleteColumn(0));
	
	for (auto device : m_listKB)
	{
		CString kb;
		kb = device;
		list->InsertItem(i++, kb);
	}

}

/// <summary>
/// 获取本机所有键盘的设备路径列表
/// </summary>
/// <param name="listKB"></param>
/// <returns></returns>
BOOL CkbgetdeviceDlg::GetKBDevicePath(list<CString>& listKB)
{
	// 根据设备安装类GUID创建空的设备信息集合
	HDEVINFO deviceInfoSet;

	deviceInfoSet = SetupDiCreateDeviceInfoList(&GUID_DEVCLASS_KEYBOARD, NULL);

	if (deviceInfoSet == NULL) return FALSE;

	// 根据设备接口类GUID获取新的设备信息集合
	HDEVINFO deviceInfoElements = SetupDiGetClassDevsEx(
		&GUID_DEVINTERFACE_KEYBOARD,
		NULL,
		NULL,
		DIGCF_DEVICEINTERFACE | DIGCF_PRESENT,
		deviceInfoSet,
		NULL,
		NULL);
	if (deviceInfoElements == NULL) return FALSE;

	// 获取设备接口
	uint32_t memberIndex = 0;
	SP_DEVICE_INTERFACE_DATA deviceInterfaceData = { 0 };
	deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	listKB.clear();
	// 枚举所有键盘
	while (SetupDiEnumDeviceInterfaces(deviceInfoElements,
		NULL,
		&GUID_DEVINTERFACE_KEYBOARD,
		memberIndex++,
		&deviceInterfaceData))
	{
		// 获取接口细节             
		PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData = 0;
		DWORD length;
		BOOL ok = SetupDiGetDeviceInterfaceDetail(deviceInfoElements,
			&deviceInterfaceData,
			NULL, 0, &length, NULL);

		DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(length);
		DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		if (SetupDiGetDeviceInterfaceDetail(deviceInfoElements,
			&deviceInterfaceData,
			DeviceInterfaceDetailData,
			length,
			NULL,
			NULL))
		{
			// 获取设备路径
			listKB.push_back(DeviceInterfaceDetailData->DevicePath);
		}
		free(DeviceInterfaceDetailData);
		DeviceInterfaceDetailData = 0;
	}

	SetupDiDestroyDeviceInfoList(deviceInfoElements);

	if (listKB.size() == 0)
		return FALSE;
	else
		return TRUE;
}
