﻿// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include <SouiFactory.h>
#include "MainDlg.h"

//#define RES_TYPE 0   //从文件中加载资源
#define RES_TYPE 1   //从PE资源中加载UI资源

#ifdef _DEBUG
#define SYS_NAMED_RESOURCE _T("soui-sys-resourced.dll")
#else
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
#endif


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
{
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));
    
    int nRet = 0; 
	SouiFactory souiFac;
    SComMgr *pComMgr = new SComMgr;

    //将程序的运行路径修改到项目所在目录所在的目录
    TCHAR szCurrentDir[MAX_PATH]={0};
    GetModuleFileName( NULL, szCurrentDir, sizeof(szCurrentDir) );
    LPTSTR lpInsertPos = _tcsrchr( szCurrentDir, _T('\\') );
    _tcscpy(lpInsertPos+1,_T("..\\souispy\\"));
    SetCurrentDirectory(szCurrentDir);
        
    {

        CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
        CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
        pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);
        pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);

        pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);

        SApplication *theApp=new SApplication(pRenderFactory,hInstance);
        theApp->RegisterWindowClass<SCaptureButton>();

        HMODULE hSysResource=LoadLibrary(SYS_NAMED_RESOURCE);
        if(hSysResource)
        {
            CAutoRefPtr<IResProvider> sysSesProvider;
            sysSesProvider.Attach(souiFac.CreateResProvider(RES_PE));
            sysSesProvider->Init((WPARAM)hSysResource,0);
            theApp->LoadSystemNamedResource(sysSesProvider);
        }
        
        	
        CAutoRefPtr<IResProvider>   pResProvider;
#if (RES_TYPE == 0)
		pResProvider.Attach(souiFac.CreateResProvider(RES_FILE));
        if(!pResProvider->Init((LPARAM)_T("uires"),0))
        {
            SASSERT(0);
            return 1;
        }
#else 
		pResProvider.Attach(souiFac.CreateResProvider(RES_PE));
        pResProvider->Init((WPARAM)hInstance,0);
#endif

        theApp->AddResProvider(pResProvider);
        
        // BLOCK: Run application
        {
            CMainDlg dlgMain;  
            dlgMain.Create(GetActiveWindow(),0,0,0,0);
            dlgMain.SendMessage(WM_INITDIALOG);
            dlgMain.CenterWindow(dlgMain.m_hWnd);
            dlgMain.ShowWindow(SW_SHOWNORMAL);
            nRet=theApp->Run(dlgMain.m_hWnd);
        }

        delete theApp;
    }
    
    delete pComMgr;
    OleUninitialize();
    return nRet;
}
