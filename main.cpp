#include "fose/PluginAPI.h"
#include "fose/CommandTable.h"
#include "fose/GameAPI.h"
#include "fose/ParamInfos.h"
#include "fose/GameObjects.h"
#include <string>
#include "CompanionWheelMenu.h"
#include "RadialTile.h"
#include "fose_version.h"
IDebugLog		gLog("fose_plugin_example.log");

PluginHandle	g_pluginHandle = kPluginHandle_Invalid;


extern "C" {

bool FOSEPlugin_Query(const FOSEInterface * fose, PluginInfo * info)
{

	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "fose_plugin_example";
	info->version = 1;
	_MESSAGE("query");
	// version checks
	if (fose->isEditor) {
		return false;
	}
	
	if(fose->foseVersion < FOSE_VERSION_INTEGER)
	{
		_ERROR("FOSE version too old (got %08X expected at least %08X)", fose->foseVersion, FOSE_VERSION_INTEGER);
		return false;
	}

	if(fose->runtimeVersion != FALLOUT_VERSION_1_7)
	{
		_ERROR("incorrect runtime version (got %08X need %08X)", fose->runtimeVersion, FALLOUT_VERSION_1_7);
		return false;
	}
	
	return true;
}


bool FOSEPlugin_Load(const FOSEInterface * fose)
{

	g_pluginHandle = fose->GetPluginHandle();
	CompanionWheelMenu::InitHooks();
	RadialTile::InitHooks();
	return true;
}

};
