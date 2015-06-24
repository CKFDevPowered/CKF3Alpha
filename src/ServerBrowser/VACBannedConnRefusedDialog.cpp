#include "VACBannedConnRefusedDialog.h"

using namespace vgui;

CVACBannedConnRefusedDialog::CVACBannedConnRefusedDialog(VPANEL hVParent, const char *name) : BaseClass(NULL, name)
{
	SetParent(hVParent);
	SetSize(480, 220);
	SetSizeable(false);

	LoadControlSettings("servers/VACBannedConnRefusedDialog.res");
	MoveToCenterOfScreen();
}
