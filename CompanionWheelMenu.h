#pragma once
#include "fose/GameMenus.h"

class CompanionWheelMenu : public Menu
{
private:
	void Free();
	bool IsMenuActive();
	void TransitionToDialog();
public:
	CompanionWheelMenu() {
		memset(tiles, 0, sizeof(tiles));
		transitionToDialog = 0;
		lastTile = -1;
		sound = Sound();
	};
	~CompanionWheelMenu() {};
	virtual void	Destructor(bool doFree);
	virtual void	SetTile(UInt32 auiTileID, Tile* apValue);
	virtual void	HandleLeftClickPress(UInt32 auiTileID, Tile* apCctiveTile) {};
	virtual void	HandleClick(SInt32 auiTileID, Tile* apClickedTile);
	virtual void	HandleMouseover(UInt32 auiTileID, Tile* apActiveTile);
	virtual void	HandleUnmouseover(UInt32 auiTileID, Tile* apTile) {};
	virtual void	PostDragTileChange(UInt32 auiTileID, Tile* apNewTile, Tile* apActiveTile) {};
	virtual void	PreDragTileChange(UInt32 auiTileID, Tile* apOldTile, Tile* apActiveTile) {};
	virtual void	HandleActiveMenuClickHeld(UInt32 auiTileID, Tile* apActiveTile) {};
	virtual void	OnClickHeld(UInt32 auiTileID, Tile* apActiveTile) {};
	virtual void	HandleMousewheel(UInt32 auiTileID, Tile* apTile);
	virtual void	Update();
	virtual bool	HandleKeyboardInput(UInt32 auiInputChar);
	virtual UInt32	GetID();
	virtual bool	HandleSpecialKeyInput(MenuSpecialKeyboardInputCode aeCode, float afKeyState);
	virtual bool	HandleControllerInput(int aiInput, Tile* apTile) { return false; };
	virtual void    OnUpdateUserTrait(int aitileVal) {};
	virtual void	HandleControllerConnectOrDisconnect(bool abIsControllerConnected) {};


	enum Buttons
	{
		kAggressivePassive = 0x0,
		kScold = 0x0,
		kUseStimpak = 0x1,
		kSearchAmmo = 0x1,
		kStayFollow = 0x2,
		kSearchWeapons = 0x2,
		kTalkTo = 0x3,
		kBackUp = 0x4,
		kNearFar = 0x5,
		kSearchChems = 0x5,
		kOpenInventory = 0x6,
		kSearchFood = 0x6,
		kRangedMelee = 0x7,
		kPraise = 0x7,
		kTitle = 0x8,
		kButtonText = 0x9,
		kButtonContext = 0xA,
		kExit = 0xB,
		kBack = 0xB,
		kSubtitle = 0xC,
		k360ExitPrompt = 0xD,
		k360BackPrompt = 0xD,
		k360SelectPrompt = 0xE,
		k360NavigatePrompt = 0xF,
		kMax = 0x10
	};

	enum MenuSounds
	{
		UIMenuOK = 0x1,
		UIMenuCancel = 0x2,
		UIMenuPrevNext = 0x3,
		UIMenuFocus = 0x4,
		UIPopUpQuestNew = 0x8,
		UIPopUpMessageGeneral = 0xA,
		UIPopUpMessageGeneral_ = 0x13,
		UILevelUp = 0x15,
		UIMenuMode = 0x24,
		UIPipBoyScroll = 0x100,
	};

	enum {
		kFlagPassive = 4,
	};
	Tile* tiles[16];
	Actor* companion;
	bool	followingAtDistance;
	bool	aggressive;
	bool	preferRanged;
	bool	following;
	SInt32	lastTile;
	bool	transitionToDialog;
	UInt32	timeToClearSubtitle;
	Sound	sound;
	bool	dogmeatMode;
	bool	inSubmenu;
	void HandleTileSelection(UInt32 tileID, bool fromHandleClick = false);
	void HandleButtonContext(UInt32 tileID);
	bool HasTiles();

	void UseStimpak();
	void Exit(bool stopSound);
	void SayTopic(TESTopic* topic);
	void RunTopicScripts(TESTopic* topic);
	void RunTopic(const char* topic);
	void BackUp();
	void SwitchFollowDistance();
	void SwitchRangedMelee();
	void SwitchStayFollow();
	void SwitchAggressionMode();
	void OpenInventory();
	void OpenDogmeatCommands();
	void CloseDogmeatCommands();
	NiPoint3 CalculateBackUpPos();

	static CompanionWheelMenu* Create();

	static void InitHooks();
	static void PlayMenuSound(MenuSounds code);
	static bool ShowMenu(Actor* actor);

	void* CompanionWheelMenu::operator new(size_t size)
	{
		return GameHeapAlloc(size);
	}

	void CompanionWheelMenu::operator delete(void* ptr)
	{
		GameHeapFree(ptr);
	}
};
STATIC_ASSERT(sizeof(CompanionWheelMenu) == 0x8C);
