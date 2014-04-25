#pragma once
#include "ControlBase.h"

class CCheckButton : public CControlBaseFont
{
	DUIOBJ_DECLARE_CLASS_NAME(CCheckButton, "chkbtn")
public:
	CCheckButton(HWND hWnd, CDuiObject* pDuiObject);
	CCheckButton(HWND hWnd, CDuiObject* pDuiObject, UINT uControlID, CRect rc, CString strTitle= TEXT(""), BOOL bIsVisible = TRUE, BOOL bIsDisable = FALSE, BOOL bIsPressDown = FALSE);
	~CCheckButton(void);

	BOOL GetCheck();
	BOOL SetCheck(BOOL bCheck);
	HRESULT OnAttributeCheck(const CStringA& strValue, BOOL bLoading);

	void SetTextColor(Color clrText) { m_clrText = clrText; };

	virtual BOOL SetControlFocus(BOOL bFocus);

protected:
	virtual BOOL OnControlMouseMove(UINT nFlags, CPoint point);
	virtual BOOL OnControlLButtonDown(UINT nFlags, CPoint point);
	virtual BOOL OnControlLButtonUp(UINT nFlags, CPoint point);
	virtual BOOL OnControlKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	virtual	void SetControlDisable(BOOL bIsDisable);
	virtual void DrawControl(CDC &dc, CRect rcUpdate);
	
public:
	enumButtonState	m_enButtonState;
	BOOL			m_bDown;
	BOOL			m_bMouseDown;
	BOOL			m_bIsFocus;			// ��ǰ�Ƿ��ڽ���״̬

	Color			m_clrText;			// ������ɫ

	DUI_DECLARE_ATTRIBUTES_BEGIN()
		DUI_CUSTOM_ATTRIBUTE("check", OnAttributeCheck)
		DUI_COLOR_ATTRIBUTE("crtext", m_clrText, FALSE)
	DUI_DECLARE_ATTRIBUTES_END()
};