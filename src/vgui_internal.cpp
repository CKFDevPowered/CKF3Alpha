#include "vgui_internal.h"

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IPanel.h>
#include "FileSystem.h"
#include <vstdlib/IKeyValuesSystem.h>

namespace vgui
{

ISurface *g_pSurface = NULL;
ILocalize *g_pLocalize = NULL;
IPanel *g_pIPanel = NULL;

static void *InitializeInterface(char const *interfaceName, CreateInterfaceFn *factoryList, int numFactories)
{
	void *retval;

	for (int i = 0; i < numFactories; i++)
	{
		CreateInterfaceFn factory = factoryList[i];

		if (!factory)
			continue;

		retval = factory(interfaceName, NULL);

		if (retval)
			return retval;
	}

	return NULL;
}

bool VGui_InternalLoadInterfaces(CreateInterfaceFn *factoryList, int numFactories)
{
	g_pSurface = (ISurface *)InitializeInterface(VGUI_SURFACE_INTERFACE_VERSION, factoryList, numFactories);
	g_pLocalize = (ILocalize *)InitializeInterface(VGUI_LOCALIZE_INTERFACE_VERSION, factoryList, numFactories);
	g_pIPanel = (IPanel *)InitializeInterface(VGUI_PANEL_INTERFACE_VERSION, factoryList, numFactories);

	if (g_pSurface && g_pIPanel)
		return true;

	return false;
}

}