#include "IMessageListener.h"
#include "VPanel.h"
#include "VGUI_Internal.h"

#include <KeyValues.h>
#include "vgui/IClientPanel.h"
#include "vgui/IVGUI.h"

using namespace vgui;

class CMessageListener : public IMessageListener
{
public:
	virtual void Message(VPanel *pSender, VPanel *pReceiver, KeyValues *pKeyValues, MessageSendType_t type);
};

void CMessageListener::Message(VPanel *pSender, VPanel *pReceiver, KeyValues *pKeyValues, MessageSendType_t type)
{
	char const *pSenderName = "NULL";

	if (pSender)
		pSenderName = pSender->Client()->GetName();

	char const *pSenderClass = "NULL";

	if (pSender)
		pSenderClass = pSender->Client()->GetClassName();

	char const *pReceiverName = "unknown name";

	if (pReceiver)
		pReceiverName = pReceiver->Client()->GetName();

	char const *pReceiverClass = "unknown class";

	if (pReceiver)
		pReceiverClass = pReceiver->Client()->GetClassName();

	if (!strcmp(pKeyValues->GetName(), "KeyFocusTicked"))
		return;
	else if (!strcmp(pKeyValues->GetName(), "MouseFocusTicked"))
		return;
	else if (!strcmp(pKeyValues->GetName(), "CursorMoved"))
		return;
	else if (!strcmp(pKeyValues->GetName(), "CursorEntered"))
		return;
	else if (!strcmp(pKeyValues->GetName(), "CursorExited"))
		return;
	else if (!strcmp(pKeyValues->GetName(), "MouseCaptureLost"))
		return;
	else if (!strcmp(pKeyValues->GetName(), "MousePressed"))
		return;
	else if (!strcmp(pKeyValues->GetName(), "MouseReleased"))
		return;
	else if (!strcmp(pKeyValues->GetName(), "Tick"))
		return;

	Msg("%s : (%s (%s) - > %s (%s)) )\n", pKeyValues->GetName(), pSenderClass, pSenderName, pReceiverClass, pReceiverName);
}

static CMessageListener s_MessageListener;

IMessageListener *MessageListener(void)
{
	return &s_MessageListener;
}