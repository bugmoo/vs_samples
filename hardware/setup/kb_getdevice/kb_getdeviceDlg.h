
// kb_getdeviceDlg.h: 头文件
//

#pragma once


// 包含list模板
#include <list>
using namespace::std;


// CkbgetdeviceDlg 对话框
class CkbgetdeviceDlg : public CDialogEx
{
// 构造
public:
	CkbgetdeviceDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_KB_GETDEVICE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonGetKb();

public:
	list<CString> m_listKB;

	void UpdateList(void);
	BOOL GetKBDevicePath(list<CString>& listKB);
};
