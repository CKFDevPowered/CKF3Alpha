#if !defined (_X360)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <interface.h>
#include <vgui/VGUI.h>
#include <vgui/Dar.h>
#include <vgui/IInputInternal.h>
#include <vgui/IPanel.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IClientPanel.h>
#include <vgui/IScheme.h>
#include <KeyValues.h>
#include <string.h>
#include <Assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <tier0/dbg.h>
#include <tier1/utlhandletable.h>
#include "vgui_internal.h"
#include "VPanel.h"
#include "IMessageListener.h"
#include "tier3/tier3.h"
#include "UtlLinkedList.h"
#include "UtlPriorityQueue.h"
#include "UtlVector.h"
#include "tier0/vprof.h"
#include "tier0/icommandline.h"

#ifdef GetCursorPos
#undef GetCursorPos
#endif

#include <tier0/memdbgon.h>

using namespace vgui;

static const int WARN_PANEL_NUMBER = 4096;
static const int INITIAL_PANEL_NUMBER = 1024;
static const int GROW_PANEL_NUMBER = 1024;

namespace vgui
{
struct SerialPanel_t
{
	unsigned short m_iSerialNumber;
	VPANEL m_pPanel;
};
}

struct MessageItem_t
{
	KeyValues *_params;
	HPanel _messageTo;
	HPanel _from;
	float _arrivalTime;
	int _messageID;
};

bool PriorityQueueComp(const MessageItem_t &x, const MessageItem_t &y)
{
	if (x._arrivalTime > y._arrivalTime)
		return true;
	else if (x._arrivalTime < y._arrivalTime)
		return false;

	return (x._messageID > y._messageID);
}

class CVGui : public IVGui
{
public:
	CVGui(void);
	~CVGui(void);

public:
	virtual bool Init(CreateInterfaceFn *factoryList, int numFactories);
	virtual void Shutdown(void);
	virtual void RunFrame(void);
	virtual void ShutdownMessage(unsigned int shutdownID);
	virtual VPANEL AllocPanel(void);
	virtual void FreePanel(VPANEL panel);
	virtual void DPrintf(const char *format, ...);
	virtual void DPrintf2(const char *format, ...);
	virtual void SpewAllActivePanelNames(void);
	virtual HPanel PanelToHandle(VPANEL panel);
	virtual VPANEL HandleToPanel(HPanel index);
	virtual void MarkPanelForDeletion(VPANEL panel);
	virtual void AddTickSignal(VPANEL panel, int intervalMilliseconds = 0);
	virtual void RemoveTickSignal(VPANEL panel);
	virtual void PostMessage(VPANEL target, KeyValues *params, VPANEL from, float delaySeconds = 0.0);
	virtual HContext CreateContext(void);
	virtual void DestroyContext(HContext context);
	virtual void AssociatePanelWithContext(HContext context, VPANEL pRoot);
	virtual void ActivateContext(HContext context);

	virtual void Start(void)
	{
		m_bRunning = true;
	}

	virtual void Stop(void)
	{
		m_bRunning = false;
	}

	virtual bool IsRunning(void)
	{
		return m_bRunning;
	}

	virtual void SetSleep(bool state) { m_bDoSleep = state; }
	virtual bool GetShouldVGuiControlSleep(void) { return m_bDoSleep; }

public:
	bool IsDispatchingMessages(void)
	{
		return m_InDispatcher;
	}

private:
	struct Context_t
	{
		HInputContext m_hInputContext;
	};

	struct Tick_t
	{
		VPANEL panel;
		int interval;
		int nexttick;

		char panelname[64];
	};

	Context_t *GetContext(HContext context);

	void PanelCreated(VPanel *panel);
	void PanelDeleted(VPanel *panel);
	bool DispatchMessages(void);
	void DestroyAllContexts(void);
	void ClearMessageQueues(void);

	inline bool IsReentrant(void) const
	{ 
		return m_nReentrancyCount > 0;
	}

	CUtlLinkedList<SerialPanel_t, unsigned short> m_PanelList;
	unsigned short m_iNextSerialNumber;
	int m_iPanelsAlloced;
	bool m_bInitialized;
	int m_iCurrentMessageID;

	bool m_bRunning : 1;
	bool m_bDoSleep : 1;
	bool m_InDispatcher : 1;
	bool m_bDebugMessages : 1;
	int m_nReentrancyCount;

	CUtlVector<Tick_t *> m_TickSignalVec;
	CUtlLinkedList<Context_t> m_Contexts;

	HContext m_hContext;
	Context_t m_DefaultContext;

#ifdef DEBUG
	int m_iDeleteCount, m_iDeletePanelCount;
#endif

	CUtlLinkedList<MessageItem_t, ushort> m_MessageQueue;
	CUtlLinkedList<MessageItem_t, ushort> m_SecondaryQueue;
	CUtlPriorityQueue<MessageItem_t> m_DelayedMessageQueue;
};

CVGui g_VGui;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CVGui, IVGui, VGUI_IVGUI_INTERFACE_VERSION, g_VGui);

bool IsDispatchingMessageQueue(void)
{
	return g_VGui.IsDispatchingMessages();
}

namespace vgui
{
IVGui *g_pIVgui = &g_VGui;
}

CVGui::CVGui(void) : m_DelayedMessageQueue(0, 4, PriorityQueueComp)
{
	m_iPanelsAlloced = 0;
	m_bInitialized = false;
	m_bRunning = false;
	m_InDispatcher = false;
	m_bDebugMessages = false;
	m_bDoSleep = true;
	m_nReentrancyCount = 0;
	m_hContext = DEFAULT_VGUI_CONTEXT;
	m_iNextSerialNumber = 0;
	m_DefaultContext.m_hInputContext = DEFAULT_INPUT_CONTEXT;
}

CVGui::~CVGui(void)
{
#ifdef _DEBUG
	if (m_iPanelsAlloced > 0)
	{
		Msg("Memory leak: panels left in memory: %d\n", m_iPanelsAlloced);
	}

	if (m_PanelList.Count() > 0)
	{
		Msg("Memory leak: panels left in Panel List: %d\n", m_PanelList.Count());
	}
#endif
}

void CVGui::SpewAllActivePanelNames(void)
{
	int nCount = m_PanelList.Count();

	for (int i = 0; i < nCount; ++i)
	{
		if (m_PanelList[i].m_pPanel)
		{
			VPanel *pPanel = (VPanel *)m_PanelList[i].m_pPanel;
			Msg("\tpanel '%s' of type '%s' leaked\n", g_pIPanel->GetName((VPANEL)pPanel), ((VPanel *)pPanel)->GetClassName());
		}
	}
}

HContext CVGui::CreateContext(void)
{
	HContext i = m_Contexts.AddToTail();
	m_Contexts[i].m_hInputContext = g_pInput->CreateInputContext();
	return i;
}

void CVGui::DestroyContext(HContext context)
{
	Assert(context != DEFAULT_VGUI_CONTEXT);

	if (m_hContext == context)
		ActivateContext(DEFAULT_VGUI_CONTEXT);

	g_pInput->DestroyInputContext(GetContext(context)->m_hInputContext);
	m_Contexts.Remove(context);
}

void CVGui::DestroyAllContexts(void)
{
	HContext next;
	HContext i = m_Contexts.Head();

	while (i != m_Contexts.InvalidIndex())
	{
		next = m_Contexts.Next(i);
		DestroyContext(i);
		i = next;
	}
}

CVGui::Context_t *CVGui::GetContext(HContext context)
{
	if (context == DEFAULT_VGUI_CONTEXT)
		return &m_DefaultContext;

	return &m_Contexts[context];
}

void CVGui::AssociatePanelWithContext(HContext context, VPANEL pRoot)
{
	Assert(context != DEFAULT_VGUI_CONTEXT);
	g_pInput->AssociatePanelWithInputContext(GetContext(context)->m_hInputContext, pRoot);
}

void CVGui::ActivateContext(HContext context)
{
	Assert((context == DEFAULT_VGUI_CONTEXT) || m_Contexts.IsValidIndex(context));

	if (m_hContext != context)
	{
		if (!IsReentrant())
			DispatchMessages();

		m_hContext = context;
		g_pInput->ActivateInputContext(GetContext(m_hContext)->m_hInputContext);

		if (context != DEFAULT_VGUI_CONTEXT && !IsReentrant())
			g_pInput->RunFrame();
	}
}

void CVGui::RunFrame(void)
{
	bool bIsReentrant = m_InDispatcher;

	if (bIsReentrant)
	{
		++m_nReentrancyCount;
	}

	g_pSurface->RunFrame();
	g_pSystem->RunFrame();

	if (IsPC() && !IsReentrant())
	{
		int cursorX, cursorY;
		g_pInput->GetCursorPosition(cursorX, cursorY);
		g_pInput->UpdateMouseFocus(cursorX, cursorY);

	}

	if (!bIsReentrant)
		g_pInput->RunFrame();

	if (!bIsReentrant)
	{
		DispatchMessages();

		int time = g_pSystem->GetTimeMillis();
		int count = m_TickSignalVec.Count();

		for (int i = count - 1; i >= 0; i--)
		{
			Tick_t *t = m_TickSignalVec[i];
			VPANEL tickTarget = t->panel;

			if (!tickTarget)
			{
				m_TickSignalVec.Remove(i);
				delete t;
				continue;
			}

			if (t->interval != 0)
			{
				if (time < t->nexttick)
					continue;

				t->nexttick = time + t->interval;
			}

			PostMessage(tickTarget, new KeyValues("Tick"), NULL);
		}
	}

	g_pSurface->SolveTraverse(g_pSurface->GetEmbeddedPanel());
	g_pSurface->ApplyChanges();

	Assert(IsX360() || (IsPC() && _heapchk() == _HEAPOK));

	if (bIsReentrant)
	{
		--m_nReentrancyCount;
	}
}

VPANEL CVGui::AllocPanel(void)
{
	m_iPanelsAlloced++;
#ifdef DEBUG
	m_iDeleteCount++;
#endif
	VPanel *panel = new VPanel;
	PanelCreated(panel);
	return (VPANEL)panel;
}

void CVGui::FreePanel(VPANEL ipanel)
{
	PanelDeleted((VPanel *)ipanel);
	delete (VPanel *)ipanel;
	m_iPanelsAlloced--;
#ifdef DEBUG
	m_iDeleteCount--;
#endif
}

HPanel CVGui::PanelToHandle(VPANEL panel)
{
	if (panel)
	{
		HPanelList baseIndex = ((VPanel *)panel)->GetListEntry();
		return (baseIndex << 16) + m_PanelList[baseIndex].m_iSerialNumber;
	}

	return INVALID_PANEL;
}

VPANEL CVGui::HandleToPanel(HPanel index)
{
	HPanelList baseIndex = static_cast<HPanelList>((index >> 16));
	int serial = index - (baseIndex << 16);

	if (index == INVALID_PANEL || !m_PanelList.IsValidIndex(baseIndex))
	{
		return NULL;
	}

	if (m_PanelList[baseIndex].m_iSerialNumber != serial)
		return NULL;

	return (VPANEL)(m_PanelList[baseIndex].m_pPanel);
}

void CVGui::PanelCreated(VPanel *panel)
{
	SerialPanel_t slot;
	slot.m_pPanel = (VPANEL)panel;
	slot.m_iSerialNumber = m_iNextSerialNumber++;
	HPanelList index = m_PanelList.AddToTail();
	m_PanelList[index] = slot;

	Assert(m_PanelList.Count() < WARN_PANEL_NUMBER);

	((VPanel *)panel)->SetListEntry(index);
	g_pSurface->AddPanel((VPANEL)panel);
}

void CVGui::PanelDeleted(VPanel *focus)
{
	Assert(focus);

	g_pSurface->ReleasePanel((VPANEL)focus);
	g_pInput->PanelDeleted((VPANEL)focus);

	Assert(m_PanelList.IsValidIndex(((VPanel *)focus)->GetListEntry()));

	if (m_PanelList.IsValidIndex(((VPanel *)focus)->GetListEntry()))
		m_PanelList.Remove(((VPanel *)focus)->GetListEntry());

	((VPanel *)focus)->SetListEntry(INVALID_PANELLIST);
	RemoveTickSignal((VPANEL)focus);
}

void CVGui::AddTickSignal(VPANEL panel, int intervalMilliseconds)
{
	Tick_t *t;
	int count = m_TickSignalVec.Count();

	for (int i = 0; i < count; i++)
	{
		Tick_t *t = m_TickSignalVec[i];

		if (t->panel == panel)
		{
			t->interval = intervalMilliseconds;
			t->nexttick = g_pSystem->GetTimeMillis() + t->interval;
			return;
		}
	}

	t = new Tick_t;
	t->panel = panel;
	t->interval = intervalMilliseconds;
	t->nexttick = g_pSystem->GetTimeMillis() + t->interval;

	if (strlen(((VPanel *)panel)->Client()->GetName()) > 0)
		strncpy(t->panelname, ((VPanel *)panel)->Client()->GetName(), sizeof(t->panelname));
	else
		strncpy(t->panelname, ((VPanel *)panel)->Client()->GetClassName(), sizeof(t->panelname));

	m_TickSignalVec.AddToTail(t);
}

void CVGui::RemoveTickSignal(VPANEL panel)
{
	int count = m_TickSignalVec.Count();

	for (int i = 0; i < count; i++)
	{
		Tick_t *tick = m_TickSignalVec[i];

		if (tick->panel == panel)
		{
			m_TickSignalVec.Remove(i);
			delete tick;
			return;
		}
	}
}

bool CVGui::DispatchMessages(void)
{
	int time = g_pSystem->GetTimeMillis();

	m_InDispatcher = true;

	bool doneWork = (m_MessageQueue.Count() > 12);
	bool bUsingDelayedQueue = (m_DelayedMessageQueue.Count() > 0);

	int nPassCount = 0;

	while (nPassCount < 2)
	{
		while (m_MessageQueue.Count() > 0 || (m_SecondaryQueue.Count() > 0) || bUsingDelayedQueue)
		{
			MessageItem_t *messageItem = NULL;
			int messageIndex = 0;
			bool bUsingSecondaryQueue = (m_SecondaryQueue.Count() > 0);

			if (bUsingSecondaryQueue)
			{
				doneWork = true;
				messageIndex = m_SecondaryQueue.Head();
				messageItem = &m_SecondaryQueue[messageIndex];
			}
			else if (bUsingDelayedQueue)
			{
				if (m_DelayedMessageQueue.Count() > 0)
				{
					messageItem = (MessageItem_t *)&m_DelayedMessageQueue.ElementAtHead();
				}

				if (!messageItem || messageItem->_arrivalTime > time)
				{
					bUsingDelayedQueue = false;
					continue;
				}
			}
			else
			{
				messageIndex = m_MessageQueue.Head();
				messageItem = &m_MessageQueue[messageIndex];
			}

			if (m_bDebugMessages)
			{
				char *qname = bUsingSecondaryQueue ? "Secondary" : "Primary";

				if (strcmp(messageItem->_params->GetName(), "Tick") && strcmp(messageItem->_params->GetName(), "MouseFocusTicked") && strcmp(messageItem->_params->GetName(), "KeyFocusTicked") && strcmp(messageItem->_params->GetName(), "CursorMoved"))
				{
					if (!stricmp(messageItem->_params->GetName(), "command"))
						g_pIVgui->DPrintf2("%s Queue dispatching command( %s, %s -- %i )\n", qname, messageItem->_params->GetName(), messageItem->_params->GetString("command"), messageItem->_messageID);
					else
						g_pIVgui->DPrintf2("%s Queue dispatching( %s -- %i )\n", qname, messageItem->_params->GetName(), messageItem->_messageID);
				}
			}

			KeyValues *params = messageItem->_params;

			if (messageItem->_messageTo == 0xFFFFFFFF)
			{
				if (!Q_stricmp(params->GetName(), "SetCursorPosInternal"))
				{
					int nXPos = params->GetInt("xpos", 0);
					int nYPos = params->GetInt("ypos", 0);
					g_pInput->UpdateCursorPosInternal(nXPos, nYPos);
				}
			}
			else
			{
				VPanel *vto = (VPanel *)g_pIVgui->HandleToPanel(messageItem->_messageTo);

				if (vto)
					vto->SendMessage(params, g_pIVgui->HandleToPanel(messageItem->_from));
			}

			if (params)
				params->deleteThis();

			if (bUsingSecondaryQueue)
			{
				m_SecondaryQueue.Remove(messageIndex);
			}
			else if (bUsingDelayedQueue)
			{
				m_DelayedMessageQueue.RemoveAtHead();
			}
			else
			{
				m_MessageQueue.Remove(messageIndex);
			}
		}

		++nPassCount;

		if (nPassCount == 1)
		{
			g_pInput->PostCursorMessage();
		}
	}

	g_pInput->HandleExplicitSetCursor();

	m_InDispatcher = false;
	return doneWork;
}

void CVGui::MarkPanelForDeletion(VPANEL panel)
{
	PostMessage(panel, new KeyValues("Delete"), NULL);
}

void CVGui::PostMessage(VPANEL target, KeyValues *params, VPANEL from, float delay)
{
	if (IsReentrant())
	{
		Assert(0);

		if (params)
			params->deleteThis();

		return;
	}

	if (!target)
	{
		if (params)
			params->deleteThis();

		return;
	}

	MessageItem_t messageItem;
	messageItem._messageTo = (target != (VPANEL)-1) ? g_pIVgui->PanelToHandle(target) : 0xFFFFFFFF;
	messageItem._params = params;
	Assert(params->GetName());
	messageItem._from = g_pIVgui->PanelToHandle(from);
	messageItem._arrivalTime = 0;
	messageItem._messageID = m_iCurrentMessageID++;

	if (delay > 0.0f)
	{
		messageItem._arrivalTime = g_pSystem->GetTimeMillis() + (delay * 1000);
		m_DelayedMessageQueue.Insert(messageItem);
	}
	else if (m_InDispatcher)
	{
		m_SecondaryQueue.AddToTail(messageItem);
	}
	else
	{
		m_MessageQueue.AddToTail(messageItem);
	}
}

void CVGui::ShutdownMessage(unsigned int shutdownID)
{
	VPANEL panel = g_pSurface->GetEmbeddedPanel();

	for (int i = 0; i < ((VPanel *)panel)->GetChildCount(); i++)
		g_pIVgui->PostMessage((VPANEL)((VPanel *)panel)->GetChild(i), new KeyValues("ShutdownRequest", "id", shutdownID), NULL);

	g_pIVgui->PostMessage(panel, new KeyValues("ShutdownRequest", "id", shutdownID), NULL);
}

void CVGui::ClearMessageQueues(void)
{
	Assert(!m_InDispatcher);

	FOR_EACH_LL(m_MessageQueue, i)
	{
		if (m_MessageQueue[i]._params)
			m_MessageQueue[i]._params->deleteThis();
	}

	m_MessageQueue.RemoveAll();

	FOR_EACH_LL(m_SecondaryQueue, i)
	{
		if (m_SecondaryQueue[i]._params)
			m_SecondaryQueue[i]._params->deleteThis();
	}

	m_SecondaryQueue.RemoveAll();

	while (m_DelayedMessageQueue.Count() > 0)
	{
		if (m_DelayedMessageQueue.ElementAtHead()._params)
			m_DelayedMessageQueue.ElementAtHead()._params->deleteThis();

		m_DelayedMessageQueue.RemoveAtHead();
	}
}

void CVGui::DPrintf(const char *format, ...)
{
	char buf[2048];
	va_list argList;

	va_start(argList, format);
	Q_vsnprintf(buf, sizeof(buf), format,argList);
	va_end(argList);

	::OutputDebugString(buf);
}

void CVGui::DPrintf2(const char *format, ...)
{
	char buf[2048];
	va_list argList;
	static int ctr = 0;

	Q_snprintf(buf, sizeof(buf), "%d:", ctr++);

	va_start(argList, format);
	Q_vsnprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), format, argList);
	va_end(argList);

	::OutputDebugString(buf);
}

void vgui::vgui_strcpy(char* dst,int dstLen,const char* src)
{
	Assert(dst != null);
	Assert(dstLen >= 0);
	Assert(src != null);

	int srcLen = strlen(src) + 1;

	if (srcLen > dstLen)
		srcLen = dstLen;

	memcpy(dst, src, srcLen - 1);
	dst[srcLen - 1] = 0;
}

bool CVGui::Init(CreateInterfaceFn *factoryList, int numFactories)
{
	m_hContext = DEFAULT_VGUI_CONTEXT;
	m_bDebugMessages = CommandLine()->CheckParm("-vguimessages") ? true : false;

	if (!m_bInitialized)
	{
		m_bInitialized = true;
		return VGui_InternalLoadInterfaces(factoryList, numFactories);
	}

	return true;
}

void CVGui::Shutdown(void)
{
	g_pSystem->SaveUserConfigFile();

	DestroyAllContexts();
	ClearMessageQueues();

	g_pSystem->Shutdown();
	g_pScheme->Shutdown(true);
	g_pSurface->Shutdown();

}