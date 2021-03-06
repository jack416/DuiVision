#include "StdAfx.h"
#include "DuiTab.h"

CDuiTabCtrl::CDuiTabCtrl(HWND hWnd, CDuiObject* pDuiObject)
	: CControlBaseFont(hWnd, pDuiObject)
{
	m_pImageSeperator = NULL;
	m_pImageHover = NULL;
	m_nHoverItem = 0;
	m_nDownItem = 0;
	m_nOldItem = -1;
	m_nTabItemWidth = 0;
	m_nTabCtrlHeight = 0;
	m_clrText = Color(225, 255, 255, 255);
	m_bAnimateChangeTab = FALSE;
	m_nAnimateCount = 10;
	m_nCurXPos = 0;
	m_bInit = FALSE;
}

CDuiTabCtrl::CDuiTabCtrl(HWND hWnd, CDuiObject* pDuiObject, UINT uControlID, CRect rc, CString strTitle/*= TEXT("")*/, BOOL bIsVisible/* = TRUE*/, 
						   BOOL bIsDisable/* = FALSE*/ ,BOOL bIsPressDown/* = FALSE*/)
	: CControlBaseFont(hWnd, pDuiObject, uControlID, rc, strTitle, bIsVisible, bIsDisable)
{
	m_pImageSeperator = NULL;
	m_pImageHover = NULL;
	m_nHoverItem = 0;
	m_nDownItem = 0;
	m_nOldItem = -1;
	m_nTabItemWidth = 0;
	m_nTabCtrlHeight = 0;
	m_clrText = Color(225, 255, 255, 255);
	m_bAnimateChangeTab = FALSE;
	m_nAnimateCount = 10;
	m_nCurXPos = 0;
	m_bInit = FALSE;
}

CDuiTabCtrl::~CDuiTabCtrl(void)
{
	if(m_pImageSeperator != NULL)
	{
		delete m_pImageSeperator;
		m_pImageSeperator = NULL;
	}

	if(m_pImageHover != NULL)
	{
		delete m_pImageHover;
		m_pImageHover = NULL;
	}

	for(size_t i = 0; i < m_vecItemInfo.size(); i++)
	{
		TabItemInfo &itemInfoTemp = m_vecItemInfo.at(i);
		if(itemInfoTemp.pImage != NULL)
		{
			delete itemInfoTemp.pImage;
			itemInfoTemp.pImage = NULL;
		}
	}
}

// 图片属性的实现
DUI_IMAGE_ATTRIBUTE_IMPLEMENT(CDuiTabCtrl, Seperator, 1)
DUI_IMAGE_ATTRIBUTE_IMPLEMENT(CDuiTabCtrl, Hover, 2)

// 根据控件名创建控件实例
CControlBase* CDuiTabCtrl::_CreateControlByName(LPCTSTR lpszName)
{
	HWND hWnd = NULL;
	CDuiObject* pParentObj = GetParent();
	while((pParentObj != NULL) && (!pParentObj->IsClass(_T("dlg"))))
	{
		pParentObj = ((CControlBase*)pParentObj)->GetParent();
	}

	if((pParentObj != NULL) && pParentObj->IsClass(_T("dlg")))
	{
		return DuiSystem::CreateControlByName(lpszName, ((CDlgBase*)pParentObj)->GetSafeHwnd(), this);
	}

	return NULL;
}

// 重载加载XML节点函数，加载下层的tab页面内容
BOOL CDuiTabCtrl::Load(DuiXmlNode pXmlElem, BOOL bLoadSubControl)
{
	__super::Load(pXmlElem);

	if(pXmlElem == NULL)
	{
		return FALSE;
	}

	// 如果没有设置tabctrl高度,则按照hover图片的高度
	if((m_pImageHover != NULL) && (m_nTabCtrlHeight == 0))
	{
		m_nTabCtrlHeight = m_sizeHover.cy;
	}

	// 如果没有设置tab项的宽度,则按照hover图片的宽度
	if((m_pImageHover != NULL) && (m_nTabItemWidth == 0))
	{
		m_nTabItemWidth = m_sizeHover.cx;
	}

	BOOL bAllVisible = TRUE;
	
	// 加载下层的tab页节点信息
	int nIdIndex = m_vecItemInfo.size();
	for (DuiXmlNode pTabElem = pXmlElem.child(_T("tab")); pTabElem; pTabElem=pTabElem.next_sibling(_T("tab")))
	{
		CString strId = pTabElem.attribute(_T("id")).value();
		int nId = nIdIndex;
		if(strId != _T(""))
		{
			nId = _wtoi(strId);
		}

		CString strName = pTabElem.attribute(_T("name")).value();
		if(GetItemIndex(strName) != -1)
		{
			// 如果已经存在相同名字的tab页,则跳过
			continue;
		}

		CString strTabXml = pTabElem.attribute(_T("tabxml")).value();
		if(!strTabXml.IsEmpty())
		{
			// 从xml文件加载嵌套的tab页
			LoadTabXml(strTabXml);
			nIdIndex = m_vecItemInfo.size();
			continue;
		}

		CString strAction = pTabElem.attribute(_T("action")).value();
		CString strOutLink = pTabElem.attribute(_T("outlink")).value();
		BOOL bOutLink = ((strOutLink == _T("1")) || (strOutLink == _T("true")));
		CString strImage = pTabElem.attribute(_T("image")).value();
		CString strImageIndex = pTabElem.attribute(_T("img-index")).value();
		int nImageIndex = -1;
		if(!strImageIndex.IsEmpty())
		{
			nImageIndex = _wtoi(strImageIndex);
		}
		CString strImageCount = pTabElem.attribute(_T("img-count")).value();
		int nImageCount = -1;
		if(!strImageCount.IsEmpty())
		{
			nImageCount = _wtoi(strImageCount);
		}
		// visible属性可以用visible或show
		CString strVisible = pTabElem.attribute(_T("visible")).value();
		if(strVisible.IsEmpty())
		{
			strVisible = pTabElem.attribute(_T("show")).value();
		}
		BOOL bVisible = ((strVisible == _T("1")) || (strVisible == _T("true")) || (strVisible == _T("")));
		CString strActive = pTabElem.attribute(_T("active")).value();
		CString strDivXml = pTabElem.attribute(_T("div")).value();

		CString strScroll = pTabElem.attribute(_T("scroll")).value();
		BOOL bEnableScroll = (strScroll == _T("1"));

		// 加载Panel控件，每个Tab页都会自动创建一个Panel控件，即使没有加载子XML节点
		CDuiPanel* pControlPanel = (CDuiPanel*)_CreateControlByName(_T("div"));
		pControlPanel->SetName(strName);	// div控件的名字设置为tab的名字
		pControlPanel->SetEnableScroll(bEnableScroll);
		m_vecControl.push_back(pControlPanel);
		if(!strDivXml.IsEmpty())
		{
 			pControlPanel->LoadXmlFile(strDivXml);			
		}

		// 加载XML中Tab节点的各个下层控件节点
		pControlPanel->Load(pTabElem);

		CString strTitle = pControlPanel->GetTitle();
		
		// 通过Skin读取
		CString strSkin = _T("");
		if(strImage.Find(_T("skin:")) == 0)
		{
			strSkin = DuiSystem::Instance()->GetSkin(strImage);
		}else
		{
			strSkin = strImage;
		}

		if(strSkin.Find(_T(".")) != -1)	// 加载图片文件
		{
			CString strImgFile = strSkin;
			if(strSkin.Find(_T(":")) != -1)
			{
				strImgFile = strSkin;
			}
			InsertItem(-1, nId, strName, strTitle, strAction, strImgFile, pControlPanel, nImageCount, bOutLink);
		}else
		if(nImageIndex != -1)	// 索引图片
		{
			InsertItem(-1, nId, strName, strTitle, strAction, nImageIndex, pControlPanel, bOutLink);
		}else
		if(!strSkin.IsEmpty())	// 图片资源
		{
			UINT uResourceID = _wtoi(strSkin);
			InsertItem(-1, nId, strName, strTitle, strAction, uResourceID, pControlPanel, nImageCount, bOutLink);
		}else
		if(strSkin.IsEmpty())	// 图片为空
		{
			InsertItem(-1, nId, strName, strTitle, strAction, _T(""), pControlPanel, nImageCount, bOutLink);
		}

		TabItemInfo &itemInfo = m_vecItemInfo.at(nIdIndex);
		itemInfo.bVisible = bVisible;
		if(!bVisible)
		{
			bAllVisible = FALSE;
		}

		if(strActive == _T("true"))	// 设置为当前活动的页面
		{
			m_nHoverItem = nIdIndex;
			m_nDownItem = nIdIndex;
		}
		if(!bVisible && (m_nDownItem == nIdIndex))
		{
			m_nDownItem++;
			m_nHoverItem = m_nDownItem;
		}

		nIdIndex++;
	}

	// 如果是用于加载嵌套tab页定义,则不需要下面的流程
	if(!bLoadSubControl)
	{
		return TRUE;
	}

	// 只显示当前活动的tab页对应的Panel对象，其他页面的Panel对象都隐藏
	for(size_t i = 0; i < m_vecItemInfo.size(); i++)
	{
		TabItemInfo &itemInfo = m_vecItemInfo.at(i);
		if(itemInfo.pControl != NULL)
		{
			if(i == m_nDownItem)
			{
				itemInfo.pControl->SetVisible(TRUE);
			}else
			{
				itemInfo.pControl->SetVisible(FALSE);
			}
		}
	}

	// 如果有页面是隐藏的,则需要刷新所有页面
	if(!bAllVisible)
	{
		RefreshItems();
	}

	m_bInit = TRUE;

    return TRUE;
}

// 从嵌套的xml文件中加载tab页
BOOL CDuiTabCtrl::LoadTabXml(CString strFileName)
{
	// 首先将所有tab页都隐藏
	for(size_t i = 0; i < m_vecItemInfo.size(); i++)
	{
		TabItemInfo &itemInfo = m_vecItemInfo.at(i);
		if(itemInfo.pControl != NULL)
		{
			itemInfo.pControl->SetVisible(FALSE);
		}
	}

	DuiXmlDocument xmlDoc;
	DuiXmlNode pTabElem;

	if(DuiSystem::Instance()->LoadXmlFile(xmlDoc, strFileName))
	{
		pTabElem = xmlDoc.child(GetClassName());
		if(pTabElem != NULL)
		{
			// 加载下层tab页
			Load(pTabElem, FALSE);
		}
	}

	// 只显示当前活动的tab页对应的Panel对象，其他页面的Panel对象都隐藏
	for(size_t i = 0; i < m_vecItemInfo.size(); i++)
	{
		TabItemInfo &itemInfo = m_vecItemInfo.at(i);
		if(itemInfo.pControl != NULL)
		{
			if(i == m_nDownItem)
			{
				itemInfo.pControl->SetVisible(TRUE);
			}else
			{
				itemInfo.pControl->SetVisible(FALSE);
			}
		}
	}

	// 重新计算每个tab页的位置,并刷新界面
	RefreshItems();

	return TRUE;
}

// 增加tab页(使用资源图片)
BOOL CDuiTabCtrl::InsertItem(int nItem, UINT nItemID, CString strName, CString strItemText, CString strAction, UINT nResourceID, CControlBase* pControl, int nImageCount, BOOL bOutLink, int nItemWidth/* = 0*/, CString strType/*= TEXT("PNG")*/)
{
	TabItemInfo itemInfo;
	itemInfo.bVisible = TRUE;
	itemInfo.nItemID = nItemID;
	itemInfo.strName = strName;
	itemInfo.strText = strItemText;
	itemInfo.strAction = strAction;
	itemInfo.bOutLink = bOutLink;
	itemInfo.sizeImage.SetSize(0, 0);
	itemInfo.nImageCount = 3;
	if(nImageCount != -1)
	{
		itemInfo.nImageCount = nImageCount;
	}
	itemInfo.nImageIndex = -1;

	if(ImageFromIDResource(nResourceID, strType, itemInfo.pImage))
	{
		itemInfo.sizeImage.SetSize(itemInfo.pImage->GetWidth() / itemInfo.nImageCount, itemInfo.pImage->GetHeight());
	}

	itemInfo.rc.SetRect(m_rc.left, m_rc.top, m_rc.left + (nItemWidth == 0 ? itemInfo.sizeImage.cx : nItemWidth), m_rc.bottom);

	itemInfo.pControl = pControl;

	return InsertItem(nItem, itemInfo);
}

// 增加tab页(使用图片文件)
BOOL CDuiTabCtrl::InsertItem(int nItem, UINT nItemID, CString strName, CString strItemText, CString strAction, CString strImage, CControlBase* pControl, int nImageCount, BOOL bOutLink, int nItemWidth/* = 0*/)
{
	TabItemInfo itemInfo;
	itemInfo.bVisible = TRUE;
	itemInfo.nItemID = nItemID;
	itemInfo.strName = strName;
	itemInfo.strText = strItemText;
	itemInfo.strAction = strAction;
	itemInfo.bOutLink = bOutLink;
	itemInfo.sizeImage.SetSize(0, 0);
	itemInfo.nImageCount = 3;
	if(nImageCount != -1)
	{
		itemInfo.nImageCount = nImageCount;
	}
	itemInfo.nImageIndex = -1;

	BOOL bLoadImageOk = FALSE;
	if(strImage.IsEmpty())
	{
		itemInfo.pImage = NULL;
	}else
	{
		bLoadImageOk = DuiSystem::Instance()->LoadImageFile(strImage, m_bImageUseECM, itemInfo.pImage);
	}

	if(itemInfo.pImage && bLoadImageOk)
	{
		itemInfo.sizeImage.SetSize(itemInfo.pImage->GetWidth() / itemInfo.nImageCount, itemInfo.pImage->GetHeight());
	}

	int nWidth = (nItemWidth == 0 ? itemInfo.sizeImage.cx : nItemWidth);
	if(nWidth == 0)
	{
		nWidth = m_sizeHover.cx;
	}
	itemInfo.rc.SetRect(m_rc.left, m_rc.top, m_rc.left + nWidth, m_rc.bottom);

	itemInfo.pControl = pControl;

	return InsertItem(nItem, itemInfo);
}

// 增加tab页(索引图片方式)
BOOL CDuiTabCtrl::InsertItem(int nItem, UINT nItemID, CString strName, CString strItemText, CString strAction, int nImageIndex, CControlBase* pControl, BOOL bOutLink, int nItemWidth/* = 0*/)
{
	TabItemInfo itemInfo;
	itemInfo.bVisible = TRUE;
	itemInfo.nItemID = nItemID;
	itemInfo.strName = strName;
	itemInfo.strText = strItemText;
	itemInfo.strAction = strAction;
	itemInfo.bOutLink = bOutLink;
	itemInfo.pImage = NULL;
	itemInfo.sizeImage.SetSize(0, 0);
	itemInfo.nImageCount = 1;
	itemInfo.nImageIndex = nImageIndex;

	if((m_pImage != NULL) && (m_nImagePicCount > 0))
	{
		itemInfo.sizeImage.SetSize(m_pImage->GetWidth() / m_nImagePicCount, m_pImage->GetHeight());
	}

	itemInfo.rc.SetRect(m_rc.left, m_rc.top, m_rc.left + (nItemWidth == 0 ? itemInfo.sizeImage.cx : nItemWidth), m_rc.bottom);

	itemInfo.pControl = pControl;

	return InsertItem(nItem, itemInfo);
}

BOOL CDuiTabCtrl::InsertItem(int nItem, TabItemInfo &itemInfo)
{
	if(m_vecItemInfo.size() > 0)
	{
		CRect rc;
		m_vecRcSeperator.push_back(rc);
	}
	if(nItem <= -1 || nItem >= (int)m_vecItemInfo.size())
	{
		m_vecItemInfo.push_back(itemInfo);
	}
	else
	{
		m_vecItemInfo.insert(m_vecItemInfo.begin() + nItem, itemInfo);
	}

	int nXPos = m_rc.left;
	int nYPos = m_rc.top;

	for(size_t i = 0; i < (int)m_vecItemInfo.size(); i++)
	{
		TabItemInfo &itemInfoTemp = m_vecItemInfo.at(i);
		int nItemWidth = itemInfoTemp.rc.Width();
		int nItemHeight = itemInfoTemp.sizeImage.cy;

		if(m_nTabItemWidth == 0)
		{
			// 如果tabctrl没有设置每个tab页的宽度,则以第一个tab页的图片宽度为准
			m_nTabItemWidth = nItemWidth;
		}
		if(m_nTabCtrlHeight == 0)
		{
			// 如果tabctrl没有设置高度,则以第一个tab页的图片高度为准
			m_nTabCtrlHeight = nItemHeight;
		}

		itemInfoTemp.rc.SetRect(nXPos, nYPos, nXPos + m_nTabItemWidth, nYPos + m_nTabCtrlHeight);

		nXPos += m_nTabItemWidth;

		if(i < m_vecItemInfo.size() - 1 && m_pImageSeperator != NULL)
		{
			CRect &rc = m_vecRcSeperator.at(i);
			rc.SetRect(nXPos, nYPos, nXPos + m_sizeSeperator.cx, nYPos + m_sizeSeperator.cy);
			nXPos += m_sizeSeperator.cx;
		}
		/*
		if(itemInfoTemp.pControl != NULL)
		{
			CRect rc = itemInfoTemp.pControl->GetRect();
			rc.top += itemInfoTemp.sizeImage.cy;
			itemInfoTemp.pControl->SetRect(rc);
			//itemInfoTemp.pControl->UpdateControl();
		}*/
	}

	UpdateControl(true);
	return true;
}

// 根据tab名字获取索引
int CDuiTabCtrl::GetItemIndex(CString strTabName)
{
	for(size_t i = 0; i < m_vecItemInfo.size(); i++)
	{
		TabItemInfo &itemInfoTemp = m_vecItemInfo.at(i);
		if(itemInfoTemp.strName == strTabName)
		{
			return i;
		}
	}
	return -1;
}

// 获取指定tab页信息
TabItemInfo* CDuiTabCtrl::GetItemInfo(int nItem)
{
	if((nItem < 0) || (nItem >= (int)m_vecItemInfo.size()))
	{
		return NULL;
	}

	TabItemInfo &itemInfo = m_vecItemInfo.at(nItem);
	return &itemInfo;
}

// 设置选择的tab页
int CDuiTabCtrl::SetSelectItem(int nItem)
{
	int nOldDownItem = m_nDownItem;
	if(m_nDownItem != nItem && nItem >= 0 && nItem < (int)m_vecItemInfo.size())
	{
		TabItemInfo &itemInfo = m_vecItemInfo.at(nItem);
		if(itemInfo.bOutLink)	// 外部链接
		{
			m_nHoverItem = -1;
			SendMessage(MSG_BUTTON_DOWN, nItem, 0);
		}else
		{
			m_nOldItem = m_nDownItem;	// 保存切换前的页面索引,用于切换动画
			m_nDownItem = nItem;					
			m_nHoverItem = -1;

			SendMessage(MSG_BUTTON_DOWN, m_nDownItem, 0);

			// 只显示当前活动的tab页对应的Panel对象，其他页面的Panel对象都隐藏
			for(size_t i = 0; i < m_vecItemInfo.size(); i++)
			{
				TabItemInfo &itemInfo = m_vecItemInfo.at(i);
				if(itemInfo.pControl != NULL)
				{
					if(i == m_nDownItem)
					{
						itemInfo.pControl->SetVisible(TRUE);
						SetWindowFocus();
					}else
					{
						itemInfo.pControl->SetVisible(FALSE);
					}
				}
				// 如果启用了动画,则启动切换动画定时器
				if(m_bAnimateChangeTab)
				{
					m_nCurXPos = 0;
					m_bRunTime = true;
				}
			}
		}

		UpdateControl();
	}

	return nOldDownItem;
}

// 刷新所有Tab页
void CDuiTabCtrl::RefreshItems()
{
	// 重新计算每个tab页的位置,并刷新界面
	int nXPos = m_rc.left;
	int nYPos = m_rc.top;

	for(size_t i = 0; i < m_vecItemInfo.size(); i++)
	{
		TabItemInfo &itemInfoTemp = m_vecItemInfo.at(i);
		if(!itemInfoTemp.bVisible)
		{
			// 隐藏的tab页坐标设置为0
			itemInfoTemp.rc.SetRect(0,0,0,0);
			if(i < m_vecRcSeperator.size())
			{
				CRect &rc = m_vecRcSeperator.at(i);
				rc.SetRect(0,0,0,0);
			}			
			continue;
		}

		int nItemWidth = itemInfoTemp.rc.Width();
		int nItemHeight = itemInfoTemp.sizeImage.cy;

		if(m_nTabItemWidth == 0)
		{
			// 如果tabctrl没有设置每个tab页的宽度,则以第一个tab页的图片宽度为准
			m_nTabItemWidth = nItemWidth;
		}
		if(m_nTabCtrlHeight == 0)
		{
			// 如果tabctrl没有设置高度,则以第一个tab页的图片高度为准
			m_nTabCtrlHeight = nItemHeight;
		}

		itemInfoTemp.rc.SetRect(nXPos, nYPos, nXPos + m_nTabItemWidth, nYPos + m_nTabCtrlHeight);

		nXPos += m_nTabItemWidth;

		if(i < m_vecItemInfo.size() - 1 && m_pImageSeperator != NULL)
		{
			CRect &rc = m_vecRcSeperator.at(i);
			rc.SetRect(nXPos, nYPos, nXPos + m_sizeSeperator.cx, nYPos + m_sizeSeperator.cy);
			nXPos += m_sizeSeperator.cx;
		}
	}

	// 如果删除了最后一个页面,需要重新计算当前页面的索引
	if(m_nDownItem >= (int)m_vecItemInfo.size())
	{
		m_nDownItem = -1;
		for(size_t i = (int)m_vecItemInfo.size()-1; i >= 0; i--)
		{
			TabItemInfo &itemInfoTemp = m_vecItemInfo.at(i);
			if(itemInfoTemp.bVisible && !itemInfoTemp.bOutLink)
			{
				m_nDownItem = i;
				break;
			}
		}
	}
	if(m_nHoverItem >= (int)m_vecItemInfo.size())
	{
		m_nHoverItem = -1;
	}

	// 刷新显示当前页
	for(size_t i = 0; i < m_vecItemInfo.size(); i++)
	{
		TabItemInfo &itemInfo = m_vecItemInfo.at(i);
		if(itemInfo.pControl != NULL)
		{
			if(i == m_nDownItem)
			{
				itemInfo.pControl->SetVisible(TRUE);
				SetWindowFocus();
			}else
			{
				itemInfo.pControl->SetVisible(FALSE);
			}
		}
	}

	UpdateControl(true);
}

// 删除指定tab页
void CDuiTabCtrl::DeleteItem(int nItem)
{
	if((nItem < 0) || (nItem >= (int)m_vecItemInfo.size()))
	{
		return;
	}

	int nIndex = 0;
	vector<TabItemInfo>::iterator it;
	for(it=m_vecItemInfo.begin();it!=m_vecItemInfo.end();++it)
	{
		if(nIndex == nItem)
		{
			TabItemInfo &itemInfo = *it;
			if(itemInfo.pControl != NULL)
			{
				RemoveControl(itemInfo.pControl);
			}
			m_vecItemInfo.erase(it);
			break;
		}
		nIndex++;
	}

	// 重新计算每个tab页的位置,并刷新界面
	RefreshItems();
}

// 删除指定tab页(按照名字)
void CDuiTabCtrl::DeleteItem(CString strTabName)
{
	vector<TabItemInfo>::iterator it;
	for(it=m_vecItemInfo.begin();it!=m_vecItemInfo.end();++it)
	{
		TabItemInfo &itemInfo = *it;
		if(itemInfo.strName == strTabName)
		{
			if(itemInfo.pControl != NULL)
			{
				RemoveControl(itemInfo.pControl);
			}
			m_vecItemInfo.erase(it);
			break;
		}
	}

	// 重新计算每个tab页的位置,并刷新界面
	RefreshItems();
}

// 设置tab页的可见性
void CDuiTabCtrl::SetItemVisible(int nItem, BOOL bVisible)
{
	if((nItem < 0) || (nItem >= (int)m_vecItemInfo.size()))
	{
		return;
	}

	TabItemInfo &itemInfo = m_vecItemInfo.at(nItem);
	itemInfo.bVisible = bVisible;

	// 重新计算每个tab页的位置,并刷新界面
	RefreshItems();
}

// 设置tab页的可见性(根据tab名字)
void CDuiTabCtrl::SetItemVisible(CString strTabName, BOOL bVisible)
{
	int nItem = GetItemIndex(strTabName);
	if(nItem != -1)
	{
		SetItemVisible(nItem, bVisible);
	}
}

// 获取tab页的可见性
BOOL CDuiTabCtrl::GetItemVisible(int nItem)
{
	if((nItem < 0) || (nItem >= (int)m_vecItemInfo.size()))
	{
		return FALSE;
	}

	TabItemInfo &itemInfo = m_vecItemInfo.at(nItem);
	return itemInfo.bVisible;
}

// 获取tab页的可见性
BOOL CDuiTabCtrl::GetItemVisible(CString strTabName)
{
	int nItem = GetItemIndex(strTabName);
	if(nItem != -1)
	{
		return GetItemVisible(nItem);
	}

	return FALSE;
}

// 重载设置控件可见性的函数，需要调用子控件的函数
void CDuiTabCtrl::SetControlVisible(BOOL bIsVisible)
{
	//__super::SetControlVisible(bIsVisible);

	if(bIsVisible)
	{
		// 如果设置为可见状态,则仅可见页的控件设置为可见状态
		for(size_t i = 0; i < m_vecItemInfo.size(); i++)
		{
			TabItemInfo &itemInfo = m_vecItemInfo.at(i);
			if((itemInfo.pControl != NULL) && (i == m_nDownItem))
			{
				itemInfo.pControl->SetControlVisible(bIsVisible);
			}else
			if((itemInfo.pControl != NULL) && (i != m_nDownItem))
			{
				itemInfo.pControl->SetControlVisible(FALSE);
			}
		}
	}else
	{
		// 如果设置为隐藏状态,则所有页面的控件都需要隐藏
		for(size_t i = 0; i < m_vecItemInfo.size(); i++)
		{
			TabItemInfo &itemInfo = m_vecItemInfo.at(i);
			if(itemInfo.pControl != NULL)
			{
				itemInfo.pControl->SetControlVisible(FALSE);
			}
		}
	}
}

// 判断鼠标是否在控件可响应的区域
BOOL CDuiTabCtrl::OnCheckMouseResponse(UINT nFlags, CPoint point)
{
	CRect rc = m_rc;
	rc.bottom = rc.top + m_nTabCtrlHeight;
	if(!rc.PtInRect(point))
	{
		return true;
	}

	// 判断如果鼠标不在所有tab页范围内,则不做响应
	BOOL bPtInTabs = false;
	for(size_t i = 0; i < m_vecItemInfo.size(); i++)
	{
		TabItemInfo &itemInfo = m_vecItemInfo.at(i);
		if(itemInfo.bVisible && itemInfo.rc.PtInRect(point))
		{
			bPtInTabs = true;
			break;
		}
	}
	
	return bPtInTabs;
}

BOOL CDuiTabCtrl::OnControlMouseMove(UINT nFlags, CPoint point)
{
	int nOldHoverItem = m_nHoverItem;

	if(m_rc.PtInRect(point))
	{
		if(m_nHoverItem != -1)
		{
			TabItemInfo &itemInfo = m_vecItemInfo.at(m_nHoverItem);
			if(itemInfo.rc.PtInRect(point))
			{
				return false;
			}
			m_nHoverItem = -1;		
		}		

		BOOL bMousenDown = false;
		if(m_nDownItem != -1)
		{
			TabItemInfo &itemInfo = m_vecItemInfo.at(m_nDownItem);
			if(itemInfo.rc.PtInRect(point))
			{
				bMousenDown = true;
				m_nHoverItem = -1;
			}		
		}

		if(!bMousenDown)
		{
			for(size_t i = 0; i < m_vecItemInfo.size(); i++)
			{
				TabItemInfo &itemInfo = m_vecItemInfo.at(i);
				if(itemInfo.rc.PtInRect(point))
				{
					m_nHoverItem = i;
					break;
				}
			}
		}
	}
	else
	{
		m_nHoverItem = -1;
	}

	if(nOldHoverItem != m_nHoverItem)
	{
		UpdateControl();
		return true;
	}
	
	return false;
}

BOOL CDuiTabCtrl::OnControlLButtonDown(UINT nFlags, CPoint point)
{
	if(m_nHoverItem != -1)
	{
		TabItemInfo &itemInfo = m_vecItemInfo.at(m_nHoverItem);
		if(itemInfo.rc.PtInRect(point))
		{
			if(m_nDownItem != m_nHoverItem)
			{
				int nDownItem = m_nHoverItem;
				if(!itemInfo.strAction.IsEmpty())
				{
					// 如果action非空,则执行动作
					DuiSystem::AddDuiActionTask(GetID(), MSG_BUTTON_UP, nDownItem, 0, GetName(), itemInfo.strAction, GetParent());
				}

				if(itemInfo.bOutLink)	// 外部链接
				{
					m_nHoverItem = -1;
					SendMessage(MSG_BUTTON_DOWN, nDownItem, 0);
				}else
				{
					m_nOldItem = m_nDownItem;	// 保存切换前的页面索引,用于切换动画
					m_nDownItem = m_nHoverItem;					
					m_nHoverItem = -1;

					// 删除旧的Tooltip
					CDlgBase* pDlg = GetParentDialog();
					if(pDlg)
					{
						pDlg->ClearTooltip();
					}

					// 点击事件消息
					SendMessage(MSG_BUTTON_DOWN, m_nDownItem, 0);

					// 只显示当前活动的tab页对应的Panel对象，其他页面的Panel对象都隐藏
					for(size_t i = 0; i < m_vecItemInfo.size(); i++)
					{
						TabItemInfo &itemInfo = m_vecItemInfo.at(i);
						if(itemInfo.pControl != NULL)
						{
							if(i == m_nDownItem)
							{
								itemInfo.pControl->SetVisible(TRUE);
								SetWindowFocus();
							}else
							{
								itemInfo.pControl->SetVisible(FALSE);
							}
						}
						// 如果启用了动画,则启动切换动画定时器
						if(m_bAnimateChangeTab)
						{
							m_nCurXPos = 0;
							m_bRunTime = true;
						}
					}
				}

				UpdateControl();

				return true;
			}
		}		
	}	
	
	return false;
}

BOOL CDuiTabCtrl::OnControlLButtonUp(UINT nFlags, CPoint point)
{
	return false;
}

// 滚动事件处理
BOOL CDuiTabCtrl::OnControlScroll(BOOL bVertical, UINT nFlags, CPoint point)
{
	if(m_nDownItem != -1)
	{
		TabItemInfo &itemInfo = m_vecItemInfo.at(m_nDownItem);
		if(itemInfo.rc.PtInRect(point))
		{
			if(itemInfo.pControl != NULL)
			{
				return itemInfo.pControl->OnScroll(bVertical, nFlags, point);
			}
		}
	}

	return false;
}

// 键盘事件处理
BOOL CDuiTabCtrl::OnControlKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(m_nDownItem != -1)
	{
		TabItemInfo &itemInfo = m_vecItemInfo.at(m_nDownItem);
		if(itemInfo.pControl != NULL)
		{
			return itemInfo.pControl->OnKeyDown(nChar, nRepCnt, nFlags);
		}
	}

	return false;
}

void CDuiTabCtrl::DrawControl(CDC &dc, CRect rcUpdate)
{
	int nWidth = m_rc.Width();
	int nHeight = m_rc.Height();

	if(!m_bUpdate)
	{
		UpdateMemDC(dc, nWidth, nHeight * 3);

		Graphics graphics(m_memDC);

		BSTR bsFont = m_strFont.AllocSysString();
		FontFamily fontFamily(bsFont);
		Font font(&fontFamily, (REAL)m_nFontWidth, m_fontStyle, UnitPixel);
		::SysFreeString(bsFont);

		SolidBrush solidBrush(m_clrText);			// 正常文字画刷

		graphics.SetTextRenderingHint( TextRenderingHintClearTypeGridFit );

		// 文字的对齐方式
		StringFormat strFormat;
		strFormat.SetAlignment(StringAlignmentCenter);		// 水平方向中间对齐
		strFormat.SetLineAlignment(StringAlignmentCenter);	// 垂直方向中间对齐
		//strFormat.SetFormatFlags( StringFormatFlagsNoClip | StringFormatFlagsMeasureTrailingSpaces);
		
		for(int i = 0; i < 3; i++)
		{
			m_memDC.BitBlt(0, i * nHeight, nWidth, nHeight, &dc, m_rc.left ,m_rc.top, SRCCOPY);

			int nXPos = 0;
			int nYPos = i * nHeight;
			for(size_t j = 0; j < m_vecItemInfo.size(); j++)
			{
				TabItemInfo &itemInfo = m_vecItemInfo.at(j);
				
				if(!itemInfo.bVisible)
				{
					continue;
				}

				if(itemInfo.pImage != NULL)	// 使用tab页指定的图片
				{
					int nImageIndex = i;
					if(itemInfo.nImageCount == 1)
					{
						nImageIndex = 0;
					}
					int nX = (itemInfo.rc.Width() - itemInfo.sizeImage.cx) / 2;
					graphics.DrawImage(itemInfo.pImage, Rect(nXPos + nX, nYPos,  itemInfo.sizeImage.cx, itemInfo.sizeImage.cy),
						itemInfo.sizeImage.cx * nImageIndex, 0, itemInfo.sizeImage.cx, itemInfo.sizeImage.cy, UnitPixel);
				}else
				if((m_pImage != NULL) && (itemInfo.nImageIndex != -1))	// 使用tabctrl的索引图片
				{
					// 画底图
					int nX = (itemInfo.rc.Width() - itemInfo.sizeImage.cx) / 2;
					graphics.DrawImage(m_pImage, Rect(nXPos + nX, nYPos,  itemInfo.sizeImage.cx, itemInfo.sizeImage.cy),
						itemInfo.sizeImage.cx * itemInfo.nImageIndex, 0, itemInfo.sizeImage.cx, itemInfo.sizeImage.cy, UnitPixel);
				}

				// 画热点图(如果存在tabctrl设置的热点图的话)
				if((m_pImageHover != NULL) && (i > 0))
				{
					int nX = (itemInfo.rc.Width() - m_sizeHover.cx) / 2;
					if(nX < 0)
					{
						nX = 0;
					}
					graphics.DrawImage(m_pImageHover, Rect(nXPos + nX, nYPos,  m_sizeHover.cx, m_sizeHover.cy),
						m_sizeHover.cx * (i-1), 0, m_sizeHover.cx, m_sizeHover.cy, UnitPixel);
				}

				// 文字
				if(!itemInfo.strText.IsEmpty())
				{
					RectF rectText((Gdiplus::REAL)nXPos, (Gdiplus::REAL)(nYPos + itemInfo.sizeImage.cy + 1), (Gdiplus::REAL)itemInfo.rc.Width(),(Gdiplus::REAL)(m_nTabCtrlHeight - itemInfo.sizeImage.cy - 1));
					if(m_nTabCtrlHeight <= itemInfo.sizeImage.cy)
					{
						// 如果tabctrl高度小于图片高度,则文字直接居中显示
						rectText.Y = (Gdiplus::REAL)nYPos;
						rectText.Height = (Gdiplus::REAL)m_nTabCtrlHeight;
					}
					BSTR bsText = itemInfo.strText.AllocSysString();
					graphics.DrawString(bsText, (INT)wcslen(bsText), &font, rectText, &strFormat, &solidBrush);
					::SysFreeString(bsText);
				}

				nXPos += itemInfo.rc.Width();

				// 画分隔图片(采用拉伸方式)
				if(j < m_vecItemInfo.size() - 1 && m_pImageSeperator != NULL)
				{
					CRect &rc = m_vecRcSeperator.at(j);
					int nSepHeight = itemInfo.rc.Height();	// m_sizeSeperator.cy
					graphics.DrawImage(m_pImageSeperator, Rect(nXPos, nYPos, m_sizeSeperator.cx, nSepHeight),
						0, 0, m_sizeSeperator.cx, m_sizeSeperator.cy, UnitPixel);

					nXPos += m_sizeSeperator.cx;
				}
			}
		}
	}

	dc.BitBlt(m_rc.left,m_rc.top, m_rc.Width(), m_rc.Height(), &m_memDC, 0, 0, SRCCOPY);

	if((m_nHoverItem != -1) && (m_nHoverItem < (int)m_vecItemInfo.size()))
	{
		TabItemInfo &itemInfo = m_vecItemInfo.at(m_nHoverItem);

		dc.BitBlt(itemInfo.rc.left,itemInfo.rc.top, itemInfo.rc.Width(), itemInfo.rc.Height(), &m_memDC, itemInfo.rc.left - m_rc.left,itemInfo.rc.top - m_rc.top + m_rc.Height(), SRCCOPY);
	}

	if((m_nDownItem != -1) && (m_nDownItem < (int)m_vecItemInfo.size()))
	{
		TabItemInfo &itemInfo = m_vecItemInfo.at(m_nDownItem);

		dc.BitBlt(itemInfo.rc.left,itemInfo.rc.top, itemInfo.rc.Width(), itemInfo.rc.Height(), &m_memDC, itemInfo.rc.left - m_rc.left,itemInfo.rc.top - m_rc.top + m_rc.Height() * 2, SRCCOPY);
	}
}

// 控件的动画定时器
BOOL CDuiTabCtrl::OnControlTimer()
{
	if(!m_bRunTime || (m_nOldItem == -1))
	{
		return FALSE;
	}

	int nAnimatePos = m_rc.Width() / m_nAnimateCount;

	BOOL bFinish = FALSE;
	if(m_nDownItem > m_nOldItem)	// 向右切换
	{
		m_nCurXPos += nAnimatePos;
		if(m_nCurXPos >= m_rc.Width())
		{
			bFinish = TRUE;
		}
	}else	// 向左切换
	{
		m_nCurXPos -= nAnimatePos;
		if((m_rc.Width() + m_nCurXPos) <= 0)
		{
			bFinish = TRUE;
		}
	}

	if(bFinish)
	{
		// 结束切换动画定时器
		m_bRunTime = false;
	}

	UpdateControl();

	return true;
}

// 画子控件(支持切换tab页的动画显示)
BOOL CDuiTabCtrl::DrawSubControls(CDC &dc, CRect rcUpdate)
{
	if(!m_bRunTime)
	{
		// 如果没有处于动画过程,则直接画所有子控件
		return __super::DrawSubControls(dc, rcUpdate);
	}

	TabItemInfo* pOldTabInfo = GetItemInfo(m_nOldItem);
	TabItemInfo* pNewTabInfo = GetItemInfo(m_nDownItem);
	if((pOldTabInfo == NULL) || (pNewTabInfo == NULL))
	{
		return FALSE;
	}

	int nLeft = m_rc.left;
	int nTop = m_rc.top;
	int nWidth = m_rc.Width();
	int nHeight = m_rc.Height();

	// 初始化旧tab页动画dc,并复制背景到动画dc
	CBitmap	aniBitmapOld;
	CDC animateDCOld;
	animateDCOld.CreateCompatibleDC(&dc);
	aniBitmapOld.CreateCompatibleBitmap(&dc, m_rc.right, m_rc.bottom);
	CBitmap* pOldAniBitmapOld = animateDCOld.SelectObject(&aniBitmapOld);
	animateDCOld.BitBlt(nLeft, nTop, nWidth, nHeight, &dc, nLeft, nTop, SRCCOPY);	// 背景复制到动画dc

	// 初始化新tab页动画dc,并复制背景到动画dc
	CBitmap	aniBitmapNew;
	CDC animateDCNew;
	animateDCNew.CreateCompatibleDC(&dc);
	aniBitmapNew.CreateCompatibleBitmap(&dc, m_rc.right, m_rc.bottom);
	CBitmap* pOldAniBitmapNew = animateDCNew.SelectObject(&aniBitmapNew);
	animateDCNew.BitBlt(nLeft, nTop, nWidth, nHeight, &dc, nLeft, nTop, SRCCOPY);	// 背景复制到动画dc

	// 画旧tab页
	// 设置旧tab页为可见
	pOldTabInfo->pControl->SetVisible(TRUE);	// 此处旧页面可见会导致原生控件也被显示出来，最后没有被清掉？
	pNewTabInfo->pControl->SetVisible(FALSE);
	// 画旧tab页到动画dc
	pOldTabInfo->pControl->Draw(animateDCOld, rcUpdate);
	// 动画dc复制到dc
	int nTabTop = pOldTabInfo->pControl->GetRect().top;
	if(m_nCurXPos > 0)
	{
		dc.BitBlt(nLeft, nTabTop, nWidth-m_nCurXPos, m_rc.bottom-nTabTop, &animateDCOld, nLeft+m_nCurXPos, nTabTop, SRCCOPY);
	}else
	{
		dc.BitBlt(nLeft-m_nCurXPos, nTabTop, nWidth+m_nCurXPos, m_rc.bottom-nTabTop, &animateDCOld, nLeft, nTabTop, SRCCOPY);
	}

	// 释放旧tab页动画dc
	animateDCOld.SelectObject(pOldAniBitmapOld);
	animateDCOld.DeleteDC();
	aniBitmapOld.DeleteObject();

	// 画新tab页
	//UpdateAnimateDC(dc, m_rc.right, m_rc.bottom);	// 初始化动画dc
	//m_animateDC.BitBlt(nLeft, nTop, nWidth, nHeight, &dc, nLeft, nTop, SRCCOPY);	// 背景复制到动画dc
	// 设置新tab页为可见
	pOldTabInfo->pControl->SetVisible(FALSE);
	pNewTabInfo->pControl->SetVisible(TRUE);
	// 画新tab页到动画dc
	pNewTabInfo->pControl->Draw(animateDCNew, rcUpdate);
	// 动画dc复制到dc
	nTabTop = pNewTabInfo->pControl->GetRect().top;
	if(m_nCurXPos > 0)
	{
		dc.BitBlt(nLeft+nWidth-m_nCurXPos, nTabTop, m_nCurXPos, m_rc.bottom-nTabTop, &animateDCNew, nLeft, nTabTop, SRCCOPY);
	}else
	{
		dc.BitBlt(nLeft+0, nTabTop, -m_nCurXPos, m_rc.bottom-nTabTop, &animateDCNew, nLeft+nWidth+m_nCurXPos, nTabTop, SRCCOPY);
	}

	// 释放新tab页动画dc
	animateDCNew.SelectObject(pOldAniBitmapNew);
	animateDCNew.DeleteDC();
	aniBitmapNew.DeleteObject();

	return TRUE;
}