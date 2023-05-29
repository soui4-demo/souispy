#pragma once

#include <control/scmnctrl.h>

namespace SOUI
{
	DEF_EVT_EXT(EventCapture,EVT_EXTERNAL_BEGIN,{
		CPoint pt_;
	});

	DEF_EVT_EXT(EventCaptureFinish,EVT_EXTERNAL_BEGIN+1,{
		CPoint pt_;
	});

    class SCaptureButton : public SWindow
    {
        DEF_SOBJECT(SWindow,L"captureButton")
    public:        
        SCaptureButton(void);
        ~SCaptureButton(void);

    protected:
        SIZE MeasureContent(int nWid,int nHei)  OVERRIDE;  
        
        BOOL IsChecked();      
    protected:
        void OnLButtonDown(UINT nFlags, CPoint point);
        void OnLButtonUp(UINT nFlags, CPoint point);
        void OnMouseMove(UINT nFlags, CPoint point);
        void OnMouseLeave();
        
        void OnPaint(IRenderTarget *pRT);
        
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_LBUTTONDOWN(OnLButtonDown)
            MSG_WM_LBUTTONUP(OnLButtonUp)
            MSG_WM_MOUSEMOVE(OnMouseMove)
            MSG_WM_MOUSELEAVE(OnMouseLeave)
            MSG_WM_PAINT_EX(OnPaint)
        SOUI_MSG_MAP_END()
    };
}
