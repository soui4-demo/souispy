// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited=FALSE;
} 

CMainDlg::~CMainDlg()
{
}


void CMainDlg::OnShowWindow( BOOL bShow, UINT nStatus )
{
	if(bShow)
	{
 		AnimateHostWindow(200,AW_CENTER);
	}
}


BOOL CMainDlg::OnInitDialog( HWND hWnd, LPARAM lParam )
{
    m_bLayoutInited=TRUE;
    
    m_pTreeCtrl = FindChildByName2<STreeCtrl>(L"tree_swnd");
    SASSERT(m_pTreeCtrl);
    
    m_wndFrame.Create(m_hWnd);

    return 0;
}

BOOL CMainDlg::OnEventCaptureHost( EventArgs *pEvt )
{
    EventCapture *pEvt2=(EventCapture*)pEvt;
    CPoint pt = pEvt2->pt_;
    ClientToScreen(&pt);
    HWND hWnd=::WindowFromPoint(pt);
    if(hWnd == m_hWnd) return TRUE;

    SStringT str;
    str.Format(_T("0x%08x"),hWnd);
    FindChildByName(L"txt_hosthwnd")->SetWindowText(str);
    TCHAR szClassName[100];
    ::GetClassName(hWnd,szClassName,100);
    FindChildByName(L"txt_hostclass")->SetWindowText(szClassName);
    return FALSE;
}

BOOL CMainDlg::OnEventCaptureHostFinish( EventArgs *pEvt )
{
    EventCaptureFinish *pEvt2=(EventCaptureFinish*)pEvt;
    CPoint pt = pEvt2->pt_;
    ClientToScreen(&pt);
    HWND hWnd=::WindowFromPoint(pt);
    if(hWnd == m_hWnd) return TRUE;

    SStringT str;
    str.Format(_T("0x%08x"),hWnd);
    FindChildByName(L"txt_hosthwnd")->SetWindowText(str);
    TCHAR szClassName[100];
    ::GetClassName(hWnd,szClassName,100);
    FindChildByName(L"txt_hostclass")->SetWindowText(szClassName);

    m_pTreeCtrl->RemoveAllItems();
    
    LRESULT lRet=::SendMessage(hWnd,SPYMSG_SETSPY,0,(LPARAM)m_hWnd);

    if(lRet == 1)
    {
        SStringT strRoot;
        strRoot.Format(_T("0x%08x"),hWnd);
        m_pTreeCtrl->SetUserData((LPARAM)hWnd);
        HSTREEITEM hRoot = m_pTreeCtrl->InsertItem(strRoot,2,2,(LPARAM)0);
        EnumSwnd(hWnd,0,hRoot);
    }

    return TRUE;
}

SWNDINFO swndInfo;
void CMainDlg::EnumSwnd( HWND hHost ,DWORD dwSwnd, HSTREEITEM hItem)
{
    DWORD  dwChild = ::SendMessage(hHost,SPYMSG_SWNDENUM,dwSwnd,GSW_FIRSTCHILD);
    while(dwChild != 0)
    {
        if(0!=::SendMessage(hHost,SPYMSG_SWNDINFO,dwChild,0))
        {
            SStringW str;
            str.Format(L"class:%s,swnd:%d,name:%s",swndInfo.szClassName,swndInfo.swnd,swndInfo.szName[0]==0?L"[undef]":swndInfo.szName);
            int iIcon = swndInfo.bVisible?0:1;
            HSTREEITEM hChild=m_pTreeCtrl->InsertItem((LPCTSTR)S_CW2T(str),iIcon,iIcon,(LPARAM)swndInfo.swnd,hItem,STVI_LAST);
            EnumSwnd(hHost,dwChild,hChild);
        }
        dwChild = ::SendMessage(hHost,SPYMSG_SWNDENUM,dwChild,GSW_NEXTSIBLING);
    }
}

BOOL CMainDlg::OnCopyData( HWND hSend, PCOPYDATASTRUCT pCopyDataStruct )
{
    if(pCopyDataStruct->dwData != SPYMSG_SWNDINFO) return FALSE;
    SWNDINFO *pSwndInfo = (SWNDINFO*)pCopyDataStruct->lpData;
    memcpy(&swndInfo,pCopyDataStruct->lpData,sizeof(swndInfo));    
    return TRUE;
}

void CMainDlg::OnBtnSearch()
{
    SStringT strTarget = FindChildByName(L"edit_search")->GetWindowText();
    strTarget.MakeLower();
    HSTREEITEM hItemStart= m_pTreeCtrl->GetSelectedItem();
    if(!hItemStart) hItemStart = m_pTreeCtrl->GetRootItem();
    
    if(!hItemStart) return;

    HSTREEITEM hItem = m_pTreeCtrl->GetNextItem(hItemStart);
    while(hItem)
    {
        SStringT strText;
        m_pTreeCtrl->GetItemText(hItem,strText);
        strText.MakeLower();
        if(-1 != strText.Find(strTarget))
        {
            m_pTreeCtrl->SelectItem(hItem);
            return;
        }
        hItem = m_pTreeCtrl->GetNextItem(hItem);
    }
    
    if(SMessageBox(m_hWnd,_T("从当前位置开始没有找到匹配窗口，是否从头开始？"),_T("提示"),MB_ICONQUESTION|MB_OKCANCEL) == IDCANCEL)
        return;
    hItem = m_pTreeCtrl->GetRootItem();
    while(hItem && hItem != hItemStart)
    {
        SStringT strText;
        m_pTreeCtrl->GetItemText(hItem,strText);
        strText.MakeLower();
        if(-1 != strText.Find(strTarget))
        {
            m_pTreeCtrl->SelectItem(hItem);
            return;
        }
        hItem = m_pTreeCtrl->GetNextItem(hItem);
    }    
}

BOOL CMainDlg::OnEventTreeSelChanged( EventArgs *pEvt )
{
    EventTCSelChanged *pEvt2 = (EventTCSelChanged*)pEvt;
    HSTREEITEM hSelItem = pEvt2->hNewSel;
    if(hSelItem)
    {
        int swnd = m_pTreeCtrl->GetItemData(hSelItem);
        if(swnd)
        {
            HWND hHost = (HWND)m_pTreeCtrl->GetUserData();
            if(0!=::SendMessage(hHost,SPYMSG_SWNDINFO,swnd,0))
            {
                if(swndInfo.bVisible)
                {
                    CRect rcWnd=swndInfo.rcWnd;
                    ::ClientToScreen(hHost,(LPPOINT)&rcWnd);
                    ::ClientToScreen(hHost,((LPPOINT)&rcWnd)+1);
                    ::BringWindowToTop(hHost);
                    m_wndFrame.Show(rcWnd);
                }
                FindChildByName(L"edit_xml")->SetWindowText(S_CW2T(swndInfo.szXmlStr));
            }
        }
    }
    return TRUE;
}

BOOL CMainDlg::OnEventCaptureSwnd( EventArgs *pEvt )
{
    EventCapture *pEvt2=(EventCapture*)pEvt;
    HWND hWnd = (HWND) m_pTreeCtrl->GetUserData();
    CPoint pt = pEvt2->pt_;
    ClientToScreen(&pt);
    HWND hWnd2=::WindowFromPoint(pt);
    if(hWnd2 != hWnd) return TRUE;

    int swnd = ::SendMessage(hWnd,SPYMSG_HITTEST,0,MAKELPARAM(pt.x,pt.y));
    if(swnd != 0)
    {
        HWND hHost = (HWND)m_pTreeCtrl->GetUserData();
        if(0!=::SendMessage(hHost,SPYMSG_SWNDINFO,swnd,0))
        {
            CRect rcWnd=swndInfo.rcWnd;
            ::ClientToScreen(hHost,(LPPOINT)&rcWnd);
            ::ClientToScreen(hHost,((LPPOINT)&rcWnd)+1);
            m_wndFrame.Show(rcWnd);
        }
    }
    return TRUE;
}

BOOL CMainDlg::OnEventCaptureSwndFinish( EventArgs *pEvt )
{
    EventCapture *pEvt2=(EventCapture*)pEvt;
    HWND hWnd = (HWND) m_pTreeCtrl->GetUserData();
    CPoint pt = pEvt2->pt_;
    ClientToScreen(&pt);
    HWND hWnd2=::WindowFromPoint(pt);
    if(hWnd2 != hWnd) return TRUE;
    
    int swnd = ::SendMessage(hWnd,SPYMSG_HITTEST,0,MAKELPARAM(pt.x,pt.y));
    if(swnd)
    {//locate swnd in treectrl
        HSTREEITEM hItem=m_pTreeCtrl->GetRootItem();
        while(hItem)
        {
            int swnd2 = m_pTreeCtrl->GetItemData(hItem);
            if(swnd == swnd2)
            {
                m_pTreeCtrl->SelectItem(hItem,TRUE);
                break;
            }
            hItem = m_pTreeCtrl->GetNextItem(hItem);
        }
    }
    return TRUE;
}
