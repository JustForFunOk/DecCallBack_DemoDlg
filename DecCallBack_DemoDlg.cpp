 // DecCallBack_DemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DecCallBack_Demo.h"
#include "DecCallBack_DemoDlg.h"

#include <opencv2/opencv.hpp>
#include <thread>
#include <iostream>

using namespace std;
#define WIDTH  1280//2048
#define HEIGTH 720//1536

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////
int iPicNum=0;//Set channel NO.
LONG nPort=-1;
FILE *Videofile=NULL;
FILE *Audiofile=NULL;
FILE *rgbFile = NULL;
char filename[100];
HWND hPlayWnd=NULL;
CString ErrorNum;

bool showCV = true;
unsigned char *rgb24Buf;

//////////////////////////////////////////////////////////////////////////
// IP to String
CString IPToStr(DWORD dwIP)
{
	CString strIP = _T("");
	WORD add1,add2,add3,add4;
	
	add1=(WORD)(dwIP&255);
	add2=(WORD)((dwIP>>8)&255);
	add3=(WORD)((dwIP>>16)&255);
	add4=(WORD)((dwIP>>24)&255);
	strIP.Format("%d.%d.%d.%d",add4,add3,add2,add1);
	return strIP;
}



/*clipValue()
//��ֵ��0��255֮��
//
*/
unsigned char clipValue0to255(int x)
{
	if (x <= 255 && x >= 0)
	{
		return x;	
	}
	else if (x > 255)
	{
		return 255;
	}
	else
	{
		return 0;
	}
}

/*yuv2rgb()
//
//
*/
bool yuv2rgb(char *yuvBuf, int w, int h,unsigned char *rgbBuf)
{	
	char *ptrY, *ptrU, *ptrV, *ptrUV;
	unsigned char *ptrRGB;//��ʼʱָ��ָ��λ��
//	unsigned char *ptrR, *ptrG, *ptrB;
	ptrY = yuvBuf;
	ptrU = yuvBuf + w * h;
	ptrUV = yuvBuf + w * h;
//	ptrV = yuvBuf + w * h *3/4;//ע��YUV420�Ĵ����ʽΪplanar
	ptrRGB = rgbBuf;//ע��RGB24�Ĵ����ʽΪpacked
//	ptrR = rgbBuf;
//	ptrG = rgbBuf + w * h;
//	ptrB = rgbBuf + w * h * 2;
	unsigned char y, u, v, r, g, b;//rgbȡֵ��Χ0~255
	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{	
			y = *(ptrY++);
			if (j % 2 == 0 && i % 2 == 0)//ÿ4��Y����һ��U��V
			{
				//u = *(ptrU++);
				//v = *(ptrV++);
				u = *(ptrUV++);
				v = *(ptrUV++);
			}			
			r = clipValue0to255(1.164 * (y - 16) + 1.596 * (v - 128));
			g = clipValue0to255(1.164 * (y - 16) - 0.813 * (v - 128) - 0.391 * (u - 128));
			b = clipValue0to255(1.164 * (y - 16) + 2.018 * (u - 128));
			//*(ptrR++) = r;
			//*(ptrG++) = g;
			//*(ptrB++) = b;
			*(ptrRGB++) = b;
			*(ptrRGB++) = g;
			*(ptrRGB++) = r;
		}
	}
	return true;
}

//����һ�����߳�
//DWORD WINAPI funproc(LPVOID lpparentet);
void funproc(char *pBuf)
{
	
	yuv2rgb(pBuf, WIDTH, HEIGTH, rgb24Buf);//yuvת����rgb
	//	unsigned char *pBuf = new unsigned char[WIDTH * HEIGTH * 3 / 2];
	IplImage *img = cvCreateImageHeader(cvSize(WIDTH, HEIGTH), IPL_DEPTH_8U, 3);
	cvSetData(img, rgb24Buf, WIDTH * 3);
//	memcpy(img->imageData, rgb24Buf, WIDTH*HEIGTH*3);
	
	cvShowImage("a", img);
	cvWaitKey(0);
	
	//cvReleaseImageHeader(**a);
	//cvDestroyWindow("a");
}


//////////////////////////////////////////////////////////////////////////
////����ص� ��ƵΪYUV����(YV12)����ƵΪPCM����
void CALLBACK DecCBFun(long nPort,char * pBuf,long nSize,FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2)
{
	long lFrameType = pFrameInfo->nType;	
	if (lFrameType ==T_AUDIO16)
	{
		TRACE("Audio nStamp:%d\n",pFrameInfo->nStamp);
		OutputDebugString("test_DecCb_Write Audio16 \n");
		if (Audiofile==NULL)
		{
			sprintf(filename,"AudionPCM.pcm",iPicNum);
			Audiofile = fopen(filename,"wb");
		}
		fwrite(pBuf,nSize,1,Audiofile);
	}

	else if(lFrameType ==T_YV12)
	{		
	    TRACE("Video nStamp:%d\n",pFrameInfo->nStamp);
		OutputDebugString("test_DecCb_Write YUV \n");
		
		//if (showCV)
		//{
		std::thread t1(&funproc, pBuf);
		t1.detach();

		//	showCV = false;
		//}
		/**/

/*
		if (Videofile==NULL)//ִ��һ��
		{
			sprintf(filename,"VideoYUV_2048x1536.yuv",iPicNum);
			Videofile = fopen(filename,"wb");
			sprintf(filename,"VideoRGB_2048x1536.rgb",iPicNum);
			rgbFile = fopen(filename, "wb");
		}
		fwrite(pBuf,nSize,1,Videofile);
		
		int w = 2048, h = 1536;
		unsigned char *rgb24Buf = (unsigned char *)malloc(w*h*3);//�ڶ�������һ���������ڴ�ż������rgb
		yuv2rgb(pBuf, w, h, rgb24Buf);
		fwrite(rgb24Buf, w*h * 3, 1, rgbFile);
		free(rgb24Buf);//�ͷ�����Ŀռ�	*/
		
	}
	else
	{

	}
}
//////////////////////////////////////////////////////////////////////////
///ʵʱ���ص�
void CALLBACK fRealDataCallBack(LONG lRealHandle,DWORD dwDataType,BYTE *pBuffer,DWORD dwBufSize,void *pUser)
{
	DWORD dRet = 0;
	BOOL inData = FALSE;

	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD:
		if(nPort >= 0)
		{
			break; //ͬһ·��������Ҫ��ε��ÿ����ӿ�
		}

		if (!PlayM4_GetPort(&nPort))
		{
			break;
		}
		if (!PlayM4_OpenStream(nPort,pBuffer,dwBufSize,1024*1024))
		{
			dRet=PlayM4_GetLastError(nPort);
			break;
		}

		//���ý���ص����� ֻ���벻��ʾ
// 		if (!PlayM4_SetDecCallBack(nPort,DecCBFun))
// 		{
// 			dRet=PlayM4_GetLastError(nPort);
// 			break;
// 		}
		
		//���ý���ص����� ��������ʾ
		if (!PlayM4_SetDecCallBackEx(nPort,DecCBFun,NULL,NULL))
		{
			dRet=PlayM4_GetLastError(nPort);
			break;
		}

		//����Ƶ����
		if (!PlayM4_Play(nPort,hPlayWnd))
		{
			dRet=PlayM4_GetLastError(nPort);
			break;
		}

		//����Ƶ����, ��Ҫ�����Ǹ�����
		if (!PlayM4_PlaySound(nPort))
		{
			dRet=PlayM4_GetLastError(nPort);
			break;
		}
		break;
		
	case NET_DVR_STREAMDATA:
        inData=PlayM4_InputData(nPort,pBuffer,dwBufSize);
		while (!inData)
		{
			Sleep(10);
			inData=PlayM4_InputData(nPort,pBuffer,dwBufSize);
			OutputDebugString("PlayM4_InputData failed \n");	
		}
		break;
	default:
		inData=PlayM4_InputData(nPort,pBuffer,dwBufSize);
		while (!inData)
		{
			Sleep(10);
			inData=PlayM4_InputData(nPort,pBuffer,dwBufSize);
			OutputDebugString("PlayM4_InputData failed \n");	
		}
		break;
	}
}
//////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDecCallBack_DemoDlg dialog

CDecCallBack_DemoDlg::CDecCallBack_DemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDecCallBack_DemoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDecCallBack_DemoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDecCallBack_DemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDecCallBack_DemoDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
    DDX_Control(pDX,IDC_STATIC_SCREEN,m_picplay);
	DDX_Text(pDX, IDC_EDIT_IPCH, iPChannel);
	DDX_Control(pDX, IDC_IPADDRESS, m_ctrlDeviceIP);
	DDX_Text(pDX, IDC_EDIT_Username, m_csUserName);
	DDX_Text(pDX, IDC_EDIT_Password, m_csPassword);
	DDX_Text(pDX, IDC_EDIT_Port, m_nLoginPort);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDecCallBack_DemoDlg, CDialog)
	//{{AFX_MSG_MAP(CDecCallBack_DemoDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_Login, OnBUTTONLogin)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE, OnButtonCapture)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CDecCallBack_DemoDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDecCallBack_DemoDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDecCallBack_DemoDlg message handlers

BOOL CDecCallBack_DemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
    UpdateData(TRUE);
	NET_DVR_Init();

	llRealHandle=-1;
	lUserID=-1;
	iPChannel=1;

	m_ctrlDeviceIP.SetAddress(192, 168, 1, 250);
	m_csUserName="admin";
	m_csPassword="shu12345678";
	m_nLoginPort=8000;

	hPlayWnd = GetDlgItem(IDC_STATIC_SCREEN)->m_hWnd;

    UpdateData(FALSE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDecCallBack_DemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDecCallBack_DemoDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDecCallBack_DemoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CDecCallBack_DemoDlg::OnBUTTONLogin() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	DWORD dwDeviceIP;
	char DeviceIP[16] = {0};
	char cUserName[100] = {0};
	char cPassword[100] = {0};
	CString csTemp;	

	m_ctrlDeviceIP.GetAddress(dwDeviceIP);
	csTemp = IPToStr(dwDeviceIP);
	sprintf(DeviceIP, "%s", csTemp.GetBuffer(0));
	strncpy(cUserName, m_csUserName, MAX_NAMELEN);
	strncpy(cPassword, m_csPassword, PASSWD_LEN);

	//��¼�豸����Ҫ�豸IP���˿ڡ��û��������� Login the device
	NET_DVR_DEVICEINFO_V30 devInfo;
	lUserID = NET_DVR_Login_V30(DeviceIP,m_nLoginPort,cUserName,cPassword,&devInfo);

	DWORD dwReturned = 0;

	if(lUserID<0)
		AfxMessageBox("Login failed!");
	else
	{
		AfxMessageBox("Login successfully!");
	}
	UpdateData(FALSE);
	return;
}

void CDecCallBack_DemoDlg::OnOK() 
{
	// TODO: Add extra validation here
	BOOL bRet1;
	UpdateData(TRUE);
	if (llRealHandle<0)
	{
		UpdateData(TRUE);

		if (lUserID<0)
		{
			ErrorNum.Format("Login failed Error number ��%d\n",NET_DVR_GetLastError());
			OutputDebugString(ErrorNum);
		}

		//////////////////////////////////////////////////////////////////////////
		NET_DVR_CLIENTINFO ClientInfo;
		ClientInfo.lChannel = iPChannel; //Channel number �豸ͨ����
		ClientInfo.hPlayWnd = NULL;  //����Ϊ�գ��豸SDK������ֻȡ��
		ClientInfo.lLinkMode = 0;    //Main Stream
		ClientInfo.sMultiCastIP = NULL;
        
		//Ԥ��ȡ�� 
     	llRealHandle = NET_DVR_RealPlay_V30(lUserID,&ClientInfo,fRealDataCallBack,NULL,TRUE);
        
		if (llRealHandle<0)
        {
			ErrorNum.Format("NET_DVR_RealPlay_V30 failed! Error number: %d\n",NET_DVR_GetLastError());
			AfxMessageBox(ErrorNum);
			return;
        }

		GetDlgItem(IDOK)->SetWindowText("Stop");

		//������ʾ����
		cvNamedWindow("a");
		rgb24Buf = (unsigned char *)malloc(WIDTH*HEIGTH * 3);//�ڶ�������һ���������ڴ�ż������rgb
	}
	else
	{	
		//ֹͣԤ��
		if (NET_DVR_StopRealPlay(llRealHandle))
		{
			bRet1=NET_DVR_GetLastError();
		}		
	    llRealHandle=-1;

		//ֹͣ����
		if (nPort>-1)
		{
			if (!PlayM4_StopSound())
			{
				bRet1=PlayM4_GetLastError(nPort);
			}
			if (!PlayM4_Stop(nPort))
			{
				bRet1=PlayM4_GetLastError(nPort);
			}
			if (!PlayM4_CloseStream(nPort))
			{
				bRet1=PlayM4_GetLastError(nPort);
			}
			PlayM4_FreePort(nPort);
			nPort=-1;
		}		

		//�رձ����������ݵ�����Ƶ�ļ�
		if (Audiofile!=NULL)
		{
			fclose(Audiofile);
			Audiofile=NULL;
		}

		if (Videofile!=NULL)
		{
			fclose(Videofile);
			Videofile=NULL;
		}
		//ҲҪ�رձ���rgb���ļ�
		if (rgbFile != NULL)
		{
			fclose(rgbFile);
			Videofile = NULL;
		}
		
		free(rgb24Buf);//�ͷ�����Ŀռ�
		cv::destroyAllWindows();

		GetDlgItem(IDOK)->SetWindowText("Start Play");
	}

	UpdateData(FALSE);
//	CDialog::OnOK();
}

void CDecCallBack_DemoDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	if(llRealHandle>=0)
	{
		AfxMessageBox("Stop preview!");
		return;
	}

	NET_DVR_Logout_V30(lUserID);
	NET_DVR_Cleanup();	

	CDialog::OnCancel();
}

void CDecCallBack_DemoDlg::OnButtonCapture() 
{
	// TODO: Add your control notification handler code here
	FILE *file=NULL;

	NET_DVR_JPEGPARA JpegPara;
    JpegPara.wPicQuality=0;
    JpegPara.wPicSize=0xff;

	char *JpegPicBuffer= new char[352*288*2]; 
	//����Ļ�������С��Ҫ����ץͼ�ķֱ��ʴ�С�Լ����ڣ��������ó�2*ͼƬ�ķֱ��ʿ�*ͼƬ�ķֱ��ʸ�
	
	DWORD  SizeReturned=0;	
	BOOL bRet= NET_DVR_CaptureJPEGPicture_NEW(lUserID, iPChannel,&JpegPara,JpegPicBuffer,352*288*2,&SizeReturned);
	if (!bRet)
	{
		ErrorNum.Format("NET_DVR_CaptureJPEGPicture_NEW failed! Error number: %d\n",NET_DVR_GetLastError());
		AfxMessageBox(ErrorNum);
		return;
	}
	else
	{
		AfxMessageBox("NET_DVR_CaptureJPEGPicture_NEW successful");
	}

	if (file==NULL)
	{
		sprintf(filename,"..\\bin\\JPEGCAPTest_%d.jpg",iPicNum);
		file = fopen(filename,"wb");
	}
    fwrite(JpegPicBuffer,SizeReturned,1,file);
    iPicNum++;

	delete JpegPicBuffer;
    fclose(file);
    file=NULL;

	return;
}


void CDecCallBack_DemoDlg::OnBnClickedOk()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	//CDialog::OnOK();
	CDecCallBack_DemoDlg::OnOK();

}


void CDecCallBack_DemoDlg::OnBnClickedCancel()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CDialog::OnCancel();
}
