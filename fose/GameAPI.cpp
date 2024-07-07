#include "GameAPI.h"
#include "GameRTTI.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "GameData.h"

#ifdef RUNTIME

// arg1 = 1, ignored if canCreateNew is false, passed to 'init' function if a new object is created
typedef void * (* _GetSingleton)(bool canCreateNew);



const _Fallout_DynamicCast Fallout_DynamicCast = (_Fallout_DynamicCast)0x00C0050A;

const _FormHeap_Allocate FormHeap_Allocate = (_FormHeap_Allocate)0x00401000;
const _FormHeap_Free FormHeap_Free = (_FormHeap_Free)0x00401010;
const _LookupFormByID LookupFormByID = (_LookupFormByID)0x00455190;
const _LookupFormByEDID LookupFormByEDID = (_LookupFormByEDID)0x4551C0;
const _CreateFormInstance CreateFormInstance = (_CreateFormInstance)0x0043CDA0;

const UInt32* g_TlsIndexPtr = (UInt32*)0x01179AF4;

const _GetSingleton ConsoleManager_GetSingleton = (_GetSingleton)0x0062B5D0;

DataHandler ** g_dataHandler = (DataHandler **)0x0106CDCC;
TESSaveLoadGame ** g_saveLoadGame = (TESSaveLoadGame **)0x01079CA4;
SaveGameManager ** g_saveGameManager = (SaveGameManager**)0x010799FC;

const _GetActorValueName GetActorValueName = (_GetActorValueName)0x0005A2370;	// See Cmd_GetActorValue_Eval

#endif

UInt32 AddFormToDataHandler(TESForm * form)
{
	return CALL_MEMBER_FN(*g_dataHandler, DoAddForm)(form);
}

UInt32 AddFormToCreatedBaseObjectsList(TESForm* form)
{
	return CALL_MEMBER_FN(*g_saveLoadGame, AddCreatedForm)(form);
}

TESSaveLoadGame* TESSaveLoadGame::GetSingleton()
{
	return *g_saveLoadGame;
}

SaveGameManager* SaveGameManager::GetSingleton()
{
	return *g_saveGameManager;
}

std::string GetSavegamePath()
{
	char path[0x104];
	CALL_MEMBER_FN(SaveGameManager::GetSingleton(), ConstructSavegamePath)(path);
	return path;
}

ConsoleManager * ConsoleManager::GetSingleton(void)
{
	return (ConsoleManager *)ConsoleManager_GetSingleton(true);
}

void Console_Print(const char * fmt, ...)
{
	ConsoleManager	* mgr = ConsoleManager::GetSingleton();
	if(mgr)
	{
		va_list	args;

		va_start(args, fmt);

		CALL_MEMBER_FN(mgr, Print)(fmt, args);

		va_end(args);
	}
}

const char * GetFullName(TESForm * baseForm)
{
	if(baseForm)
	{
		TESFullName* fullName = baseForm->GetFullName();
		if(fullName && fullName->name.m_data)
		{
			if (fullName->name.m_dataLen)
				return fullName->name.m_data;
		}
	}

	return "<no name>";
}


// g_baseActorValueNames is only filled in after oblivion's global initializers run
const char* GetActorValueString(UInt32 actorValue)
{
	char* name = 0;
	if (actorValue <= eActorVal_FalloutMax)
		name = GetActorValueName(actorValue);
	if (!name)
		name = "unknown";

	return name;
}

// 2A4
struct TLSData
{
	// thread local storage

	UInt32			unk00;							// 000
	UInt32			unk04;							// 004
	void			* unk08;						// 008 Seen ExtraDataList*
	UInt32			unk0C[(0x044 - 0x00C) >> 2]; 	// 00C
	void			* unk44;						// 044 Seen ExtraScript*
	UInt32			unk048[(0x244 - 0x048) >> 2];	// 048
	UInt32			unk244;							// 244
	UInt32			unk248;							// 248
	UInt32			unk24C;							// 24C 
	bool			bConsoleMode;					// 250 was executing command called from console?
	UInt8			unk24D[3];
	UInt32			unk250[(0x278 - 0x254) >> 2];	// 254
	UInt32			unk278;							// 278
	UInt32			unk27C[(0x294 - 0x27C) >> 2];	// 27C
	UInt32			unk294;							// 294 often retrieved & modified before alloc'ing mem for some large object
	UInt32			unk298[(0x2A4 - 0x298) >> 2];	// 298
};

STATIC_ASSERT(sizeof(TLSData) == 0x2A4);
STATIC_ASSERT(offsetof(TLSData, bConsoleMode) == 0x250);	// 1.1.35 patch changed offset from 0x24C



static TLSData* GetTLSData()
{
	UInt32 TlsIndex = *g_TlsIndexPtr;
	TLSData* data = NULL;

	__asm {
		mov     ecx,	[TlsIndex]   
		mov     edx,	fs:[2Ch]	// linear address of thread local storage array
		mov     eax,	[edx+ecx*4]
		mov		[data], eax
	}

	return data;
}

void DumpTLSData()
{
	TLSData* data = GetTLSData();
	DumpClass(data, sizeof(TLSData)/4);
}

bool IsConsoleMode()
{
	TLSData* tlsData = GetTLSData();
	if (tlsData)
		return tlsData->bConsoleMode;

	return false;
}

