#include "CompanionWheelMenu.h"
#include "fose/GameExtraData.h"

const double radialTraitId = 908.0;

char iniFilename[MAX_PATH];

TESPackage* g_travelPackage = nullptr;

UInt32 CompanionWheelMenu::GetID()
{
	return 1050;
}

CompanionWheelMenu* CompanionWheelMenu::Create()
{
	return new CompanionWheelMenu();
}

void CompanionWheelMenu::SetTile(UInt32 idx, Tile* tile)
{
	if (idx <= k360NavigatePrompt) {
		this->tiles[idx] = tile;
	}
}

void CompanionWheelMenu::Destructor(bool doFree)
{

	this->Free();
	if (doFree) {
		GameHeapFree(this);
	}
}


bool CompanionWheelMenu::HasTiles()

{
	for (Tile* tile : this->tiles)
	{
		if (!tile) return false;
	}

	return true;

}

void CompanionWheelMenu::Free()
{
	if (this->transitionToDialog) {
		this->transitionToDialog = false;
		InterfaceManager::GetSingleton()->currentMode = 1;
		this->TransitionToDialog();
	}
	Menu::Free();
}

void CompanionWheelMenu::Exit(bool stopSound)
{
	if (stopSound) {
		this->sound.FadeOut(100);
	}
	if (!BGSSaveLoadGame::GetSingleton()->IsLoading()) {
		ThisCall<void>(0x791920, ProcessManager::GetSingleton());
	}
	this->rootTile->SetFloat(6002, 1);
	this->FadeOutAndClose();

}

void CompanionWheelMenu::PlayMenuSound(MenuSounds code)
{
	if (code == UIPipBoyScroll) {
		Sound sound("UIPipBoyScroll", 0x121);
		sound.Play();
	}
	else {
		CdeclCall<void>(0x61E7D0, code);
	}
}

bool CompanionWheelMenu::IsMenuActive() {
	return CdeclCall<UInt8>(0x61C490, 1050) != 0;
}


void CompanionWheelMenu::UseStimpak()
{

	if (this->dogmeatMode) {
		this->RunTopic("DogmeatHeal");
		return;
	}

	Actor* companion = this->companion;
	TESForm* stimpak = CdeclCall<TESForm*>(0x508E40, 0);
	TESBoundObject* stimpakBO = DYNAMIC_CAST(stimpak, TESForm, TESBoundObject);
	ContChangesEntry* data = ThisCall<ContChangesEntry*>(0x4E8AB0, PlayerCharacter::GetSingleton(), stimpakBO, 0);
	if (!data) return;

	float healthPct = companion->avOwner.GetActorValue(eActorVal_Health) / companion->avOwner.GetBaseActorValueInt(eActorVal_Health);

	bool hasCrippledLimbs = false;

	for (int i = eActorVal_BodyPartStart; i <= eActorVal_BodyPartEnd; i++) {
		if (companion->avOwner.GetLimbAV(i) == 0.0) {
			hasCrippledLimbs = true;
			break;
		}
	}
	if (healthPct < 1.0 || hasCrippledLimbs) {
		if (healthPct < 1.0) {
			AlchemyItem* alch = DYNAMIC_CAST(data->type, TESBoundObject, AlchemyItem);
			ThisCall<bool>(0x7213F0, companion, alch, data->extendData->GetFirstItem(), 1);
		}

		ThisCall<bool>(0x4EE8A0, PlayerCharacter::GetSingleton(), data->type, data->extendData->GetFirstItem(), 1, 0, 0, 0, 0, 0, 1, 0);
		ThisCall<void>(0x723100, companion, 2);
		for (int i = eActorVal_BodyPartStart; i <= eActorVal_BodyPartEnd; i++) {
			ThisCall<void>(0x6F6120, companion, i, 1000.0f);
		}
	}

	data->Destroy();

}


void CompanionWheelMenu::SayTopic(TESTopic* topic)
{
	if (topic == nullptr) return;
	Actor* companion = this->companion;
	TESTopicInfo* topicInfo = topic->GetTopicInfo(companion, PlayerCharacter::GetSingleton());
	if (topicInfo == nullptr) return;
	TESQuest* quest = ThisCall<TESQuest*>(0x5642C0, topic, topicInfo);

	void* mem = FormHeap_Allocate(0x1C);
	DialogueItem* item = ThisCall<DialogueItem*>(0x6C72B0, mem, quest, topic, topicInfo, companion);
	item->responses.current = &item->responses.list;
	if (item->responses.current && item->responses.current->data) {
		DialogueResponse* r = item->responses.current->data;
		if (r->soundID) {
			Sound sound = Sound();
			ThisCall<Sound*>(0x766C00, companion, &sound, r->soundID->refID, 0, 0x102, 1);
			this->sound = sound;
		}
		else {
			ThisCall<void>(0x70EAF0, companion, r->voiceFilePath.m_data, &this->sound, r->emotionType, r->emotionValue, r->responseText.GetLen(), r->speakerAnimation, r->listenerAnimation, PlayerCharacter::GetSingleton(), 1, 1, 0, 0, 1);

		}
		UInt8 bGeneralSubtitles = *(UInt8*)0x1075A28;
		if (r->responseText.GetLen() && bGeneralSubtitles) {
			this->tiles[kSubtitle]->SetString(r->responseText.m_data);
			float duration = (float)this->sound.GetDuration();
			if (duration == 0.0) {
				float fNoticeTextTimePerCharacter = *(float*)0xF63318;
				duration = fNoticeTextTimePerCharacter * r->responseText.GetLen() * 1000.0f;

			}
			this->timeToClearSubtitle = GetTickCount() + duration;
		}
	}
	ThisCall<void>(0x6C7860, item);

}

void CompanionWheelMenu::RunTopicScripts(TESTopic* topic)
{
	if (topic == nullptr) return;
	Actor* companion = this->companion;
	TESTopicInfo* topicInfo = topic->GetTopicInfo(companion, PlayerCharacter::GetSingleton());
	if (topicInfo == nullptr) return;

	void* xScript = ThisCall<void*>(0x40AD00, &companion->extraDataList);

	Script* scriptBefore = topicInfo->GetScript(0);
	if (scriptBefore && scriptBefore->info.dataLength) {
		scriptBefore->Run(companion, xScript, nullptr, true);
	}

	Script* scriptAfter = topicInfo->GetScript(1);
	if (scriptAfter && scriptAfter->info.dataLength) {
		scriptAfter->Run(companion, xScript, nullptr, true);
	}

}

void CompanionWheelMenu::RunTopic(const char* topicName)
{
	TESForm* form = LookupFormByEDID(topicName);
	if (!form) return;
	TESTopic* topic = DYNAMIC_CAST(form, TESForm, TESTopic);
	if (!topic) return;
	this->SayTopic(topic);
	this->RunTopicScripts(topic);
}

NiPoint3 CompanionWheelMenu::CalculateBackUpPos() {
	NiPoint3* actorPos = this->companion->GetPos();
	NiPoint3* playerPos = PlayerCharacter::GetSingleton()->GetPos();
	NiPoint3 out;
	out.Init(actorPos);
	out.Subtract(playerPos);
	out = out.normal();
	out.Scale(300.0);
	out.Add(actorPos);
	TESObjectCELL* cell = this->companion->parentCell;
	TESWorldSpace* wspc = this->companion->GetWorldSpace();
	NiPoint3 posOut;
	posOut.Init(&out);
	CdeclCall<bool>(0x5F16B0, wspc, cell, &out, 200.0f, &posOut);
	return posOut;

}

// tracking placeatme'd xmarker refID using the ini, since it's persistent
UInt32 GetMarkerID() {
	return GetPrivateProfileInt("INTERNAL", "iMarkerRefID", 0, iniFilename);
}

void SaveMarkerID(UInt32 id) {
	char buffer[11];
	sprintf(buffer, "0x%X", id);
	WritePrivateProfileString("INTERNAL", "iMarkerRefID", buffer, iniFilename);
}

TESObjectREFR* GetTravelRef(NiPoint3* pos) {
	TESObjectREFR* ref = nullptr;
	TESForm* markerForm = nullptr;

	UInt32 markerID = GetMarkerID();
	if (markerID) markerForm = LookupFormByID(markerID);
	if (markerForm) {
		ref = DYNAMIC_CAST(markerForm, TESForm, TESObjectREFR);
		if (ref && ref->baseForm && ref->baseForm->refID == kForms_xMarker) {
			ref->MoveTo(PlayerCharacter::GetSingleton());
		}
	}
	if (!ref) {
		TESForm* xMarker = LookupFormByID(kForms_xMarker);
		ref = PlayerCharacter::GetSingleton()->PlaceAtMe(xMarker);
		SaveMarkerID(ref->refID);

	}
	if (!ref) return nullptr;
	ref->SetPos(*pos);
	return ref;

}

void CompanionWheelMenu::BackUp()
{
	NiPoint3 pos = this->CalculateBackUpPos();
	if (g_travelPackage && g_travelPackage->typeID == kFormType_Package) {
		TESPackage::LocationData* location = g_travelPackage->GetLocationData();
		location->object.refr = GetTravelRef(&pos);
	}
	else {
		g_travelPackage = TESPackage::Create(TESPackage::kPackageType_Travel);
		g_travelPackage->packageFlags = g_travelPackage->packageFlags & ~6u | 4;
		g_travelPackage->packageFlags |= (1 << 2);
		g_travelPackage->packageFlags |= (1 << 13);
		g_travelPackage->procedureType = 0;
		TESPackage::LocationData* location = g_travelPackage->GetLocationData();
		location->locationType = TESPackage::LocationData::kPackLocation_NearReference;
		location->object.refr = GetTravelRef(&pos);
		location->radius = 50;
	}
	ThisCall<void>(0x765FF0, this->companion, g_travelPackage, 1, 1);
}

void CompanionWheelMenu::SwitchFollowDistance()
{
	this->followingAtDistance = this->followingAtDistance == 0;
	this->RunTopic(this->followingAtDistance ? "FollowersTacticsDistanceLong" : "FollowersTacticsDistanceDefault");
	this->tiles[kNearFar]->SetFloat(kTileValue_user11, this->followingAtDistance);
}

void CompanionWheelMenu::SwitchRangedMelee()
{
	this->preferRanged = this->preferRanged == 0;
	this->RunTopic(this->preferRanged ? "FollowersTacticsCombatRanged" : "FollowersTacticsCombatMelee");
	this->tiles[kRangedMelee]->SetFloat(kTileValue_user11, this->preferRanged);
}

void CompanionWheelMenu::SwitchAggressionMode()
{
	// no topic for Passive/Aggressive in FO3, and no script variable
	// hijacking unused (but serialized) extradata instead, 
	// because cosave interface doesn't work in FOSE either (is anyone surprised)

	this->aggressive = this->aggressive == 0;
	this->companion->SetPassive(!this->aggressive);
	this->tiles[kAggressivePassive]->SetFloat(kTileValue_user11, this->aggressive);
}

void CompanionWheelMenu::OpenInventory()
{
	if (dogmeatMode) {
		this->RunTopic("DogmeatPraise");
		this->Exit(false);
		CdeclCall<void>(0x61D5A0, 1, this->companion, 0, 0, 3);
	}
	else {
		this->Exit(true);
		this->RunTopic("FollowersTrade");
	}
}

void CompanionWheelMenu::OpenDogmeatCommands()
{
	this->inSubmenu = true;
	this->tiles[kSubtitle]->SetString("\0");
	this->tiles[kTitle]->SetString("Dogmeat Commands");
	this->tiles[k360BackPrompt]->SetString("Back");
	this->tiles[kBack]->SetString("Back");
	this->tiles[kTalkTo]->SetVisible(false);
	this->tiles[kBackUp]->SetVisible(false);
	this->tiles[kButtonText]->SetString("Praise");

	for (int i = kScold; i < kTitle; i++) {
		if (i == kTalkTo || i == kBackUp) continue;
		this->tiles[i]->SetFloat(kTileValue_user14, 1);
	}
}

void CompanionWheelMenu::CloseDogmeatCommands()
{
	this->inSubmenu = false;
	this->tiles[kSubtitle]->SetString("\0");
	this->tiles[kTitle]->SetString("Companion Commands");
	this->tiles[k360ExitPrompt]->SetString("Exit");
	this->tiles[kExit]->SetString("Exit");
	this->tiles[kTalkTo]->SetVisible(true);
	this->tiles[kBackUp]->SetVisible(true);
	this->tiles[kButtonText]->SetString(this->preferRanged ? "Use Melee" : "Use Ranged");
	for (int i = kAggressivePassive; i < kTitle; i++) {
		if (i == kTalkTo || i == kBackUp) continue;
		this->tiles[i]->SetFloat(kTileValue_user14, 0);
	}
}

void CompanionWheelMenu::SwitchStayFollow()
{
	this->following = this->following == 0;
	if (this->dogmeatMode && !this->following) {
		this->RunTopic("DogmeatWait");
	}
	else {
		this->RunTopic(this->following ? "FollowersLetsGo" : "FollowersWait");
	}
	this->tiles[kStayFollow]->SetFloat(kTileValue_user11, !this->following);

}


void CompanionWheelMenu::TransitionToDialog()
{
	VATSCameraData* cData = VATSCameraData::GetSingleton();
	ThisCall<void>(0x7D0710, cData, 0, 0);
	CdeclCall<void>(0x61D5A0, 4, this->companion, 0, 0, 1);

}


void CompanionWheelMenu::HandleClick(SInt32 tileID, Tile* clickedTile)
{
	if (!IsMenuActive()) return;
	if (this->inSubmenu) {
		PlayMenuSound(UIMenuOK);
		switch (tileID)
		{
		case kPraise:
			this->RunTopic("DogmeatPraise");
			break;
		case kScold:
			this->RunTopic("DogmeatScold");
			break;
		case kSearchAmmo:
			this->RunTopic("DogmeatSearchAmmo");
			this->Exit(true);
			break;
		case kSearchWeapons:
			this->RunTopic("DogmeatSearchWeapons");
			this->Exit(true);
			break;
		case kSearchChems:
			this->RunTopic("DogmeatSearchChems");
			this->Exit(true);
			break;
		case kSearchFood:
			this->RunTopic("DogmeatSearchFood");
			this->Exit(true);
			break;
		case kBack:
			this->CloseDogmeatCommands();
			break;
		default:
			break;
		}
	}
	else {
		switch (tileID)
		{
		case kAggressivePassive:
			this->SwitchAggressionMode();
			this->HandleTileSelection(tileID, 1);
			PlayMenuSound(UIMenuOK);
			break;
		case kUseStimpak:
			this->UseStimpak();
			this->HandleTileSelection(tileID, 1);
			PlayMenuSound(UIMenuOK);
			break;
		case kStayFollow:
			this->SwitchStayFollow();
			this->HandleTileSelection(tileID, 1);
			PlayMenuSound(UIMenuOK);
			break;
		case kTalkTo:
			PlayMenuSound(UIMenuOK);
			this->transitionToDialog = true;
			this->Exit(true);
			break;
		case kBackUp:
			PlayMenuSound(UIMenuOK);
			this->BackUp();
			this->Exit(true);
			break;
		case kNearFar:
			this->SwitchFollowDistance();
			this->HandleTileSelection(tileID, 1);
			PlayMenuSound(UIMenuOK);
			break;
		case kOpenInventory:
			PlayMenuSound(UIMenuOK);
			this->OpenInventory();
			break;
		case kRangedMelee:
			if (this->dogmeatMode) {
				this->OpenDogmeatCommands();
			}
			else {
				this->SwitchRangedMelee();
				this->HandleTileSelection(tileID, 1);
			}	
			PlayMenuSound(UIMenuOK);
			break;
		case kExit:
			PlayMenuSound(UIMenuOK);
			this->Exit(true);
			break;
		default:
			break;
		}
	}
}

void CompanionWheelMenu::HandleMouseover(UInt32 tileID, Tile* activeTile)
{
	if (IsMenuActive() && tileID >= kAggressivePassive && (tileID <= kRangedMelee || tileID == kExit)) {
		this->HandleTileSelection(tileID);
	}
}

bool IsMouseNotVisibleAndControllerConnected() {
	return ThisCall<bool>(0x774E70, InterfaceManager::GetSingleton()) == 0;
}

void CompanionWheelMenu::HandleMousewheel(UInt32 auiTileID, Tile* apTile)
{
	if (!IsMenuActive()) return;
	float wheelDirection = InterfaceManager::GetSingleton()->mouseWheel;

	if (this->inSubmenu) {
		if (wheelDirection < 0) {
			if (this->lastTile < kScold || this->lastTile >= kPraise) {
				this->HandleTileSelection(kScold);
			}
			else if (this->lastTile == kSearchWeapons) {
				this->HandleTileSelection(kSearchChems);
			} else {
				this->HandleTileSelection(this->lastTile + 1);
			}
		}
		else {
			if (this->lastTile <= kScold || this->lastTile > kPraise) {
				this->HandleTileSelection(kPraise);
			}
			else if (this->lastTile == kSearchChems) {
				this->HandleTileSelection(kSearchWeapons);
			}
			else {
				this->HandleTileSelection(this->lastTile - 1);
			}
		}
	}
	else {
		if (wheelDirection < 0) {
			if (this->lastTile < kAggressivePassive || this->lastTile >= kRangedMelee) {
				this->HandleTileSelection(kAggressivePassive);
			}
			else {
				this->HandleTileSelection(this->lastTile + 1);
			}
		}
		else {
			if (this->lastTile <= kAggressivePassive || this->lastTile > kRangedMelee) {
				this->HandleTileSelection(kRangedMelee);
			}
			else {
				this->HandleTileSelection(this->lastTile - 1);
			}
		}
	}
}

void CompanionWheelMenu::Update()
{
	bool stickMoved = false;
	float fThumbLX = 0.0;
	float fThumbLY = 0.0;
	float angle = 0.0;
	if (this->timeToClearSubtitle && GetTickCount() > this->timeToClearSubtitle) {
		this->timeToClearSubtitle = 0;
		this->tiles[kSubtitle]->SetString("\0");
	}
	if (!IsMenuActive()) return;

	if (IsMouseNotVisibleAndControllerConnected()) {
		SInt16 sThumbLYLast = *(SInt16*)0x1075B0A;
		SInt16 sThumbLXCurr = *(SInt16*)0x1075B18;
		SInt16 sThumbLXLast = *(SInt16*)0x1075B08;
		if (abs(sThumbLYLast) > 7849 || abs(sThumbLXCurr) > 7849) {
			stickMoved = true;
			fThumbLX = (float)sThumbLXLast;
			fThumbLY = (float)sThumbLYLast;

		}
	}
	if (stickMoved) {

		fThumbLY = -fThumbLY;

		if (fThumbLY == 0.0) {
			angle = fThumbLX <= 0.0 ? 4.7123799 : 1.57075;
		}
		else {
			angle = atan(-fThumbLX / fThumbLY);
		}

		if (fThumbLY > 0.0) {
			angle += 3.141590118408203;
		}

		if (fThumbLX < 0.0 && fThumbLY < 0.0)
			angle += 6.283180236816406;


		if (angle < 0.0 || angle >= 5.759578) {
			angle = 5.6999998;
		}
		else if (angle >= 3.665186) {
			// angle remains unchanged
		}
		else if (angle >= 3.14159) {
			angle = 3.7;
		}
		else if (angle >= 2.617990016937256) {
			angle = 2.5999999;
		}
		else if (angle >= 0.5235980153083801) {
			// angle remains unchanged
		}
		else {
			angle = 0.55000001;
		}

		for (UInt32 tileID = kAggressivePassive; tileID < kMax; ++tileID) {
			if (strcmp(this->tiles[tileID]->GetTypeStr(), "IMGR") == 0) {
				float minAngle = this->tiles[tileID]->GetValueFloat(kTileValue_user2);
				float maxAngle = this->tiles[tileID]->GetValueFloat(kTileValue_user3);
				if (angle >= minAngle && angle < maxAngle) {
					this->HandleTileSelection(tileID);
					break;
				}
			}
		}
	}

}

bool CompanionWheelMenu::HandleKeyboardInput(UInt32 inputCode)
{
	if (!IsMenuActive()) return false;
	switch (inputCode)
	{
	case 'x':
	case 'X':
		this->HandleClick(kExit, this->tiles[kExit]);
		return true;
	case ' ':
		if (this->lastTile != -1) {
			this->HandleClick(this->lastTile, this->tiles[this->lastTile]);
			return true;
		}
		break;
	default:
		break;
	}

	return false;
}


// handles controller Accept and Exit buttons
bool CompanionWheelMenu::HandleSpecialKeyInput(MenuSpecialKeyboardInputCode code, float arg1)
{
	if (!IsMenuActive()) return false;

	if (code == kMenu_Space) {
		if (this->lastTile == -1) {
			this->HandleClick(kExit, this->tiles[kExit]);
		}
		else {
			this->HandleClick(this->lastTile, this->tiles[this->lastTile]);
		}
		return true;
	}
	else if (code == kMenu_Tab) {
		this->HandleClick(kExit, this->tiles[kExit]);
		return true;
	}
	return false;
}


void CompanionWheelMenu::HandleTileSelection(UInt32 tileID, bool clicked) {
	if (this->inSubmenu) {
		PlayMenuSound(UIPipBoyScroll);
		this->tiles[this->lastTile]->SetFloat(kTileValue_user10, 0.0);
		this->lastTile = tileID;
		this->tiles[tileID]->SetFloat(kTileValue_user10, 1.0);
		switch (tileID) {
		case kPraise:
			this->tiles[kButtonText]->SetString("Praise");
			break;
		case kScold:
			this->tiles[kButtonText]->SetString("Scold");
			break;
		case kSearchAmmo:
			this->tiles[kButtonText]->SetString("Look for Ammo");
			break;
		case kSearchChems:
			this->tiles[kButtonText]->SetString("Look for Chems");
			break;
		case kSearchWeapons:
			this->tiles[kButtonText]->SetString("Look for Weapons");
			break;
		case kSearchFood:
			this->tiles[kButtonText]->SetString("Look for Food");
			break;
		case kBack:
			this->tiles[kButtonText]->SetString("Back");
			break;
		default:
			this->tiles[kButtonText]->SetString("\0");
			break;
		}
	}
	else {
		if (clicked || this->lastTile != tileID) {
			PlayMenuSound(UIPipBoyScroll);

			// handle last tile
			if (this->lastTile != -1) {
				this->tiles[this->lastTile]->SetFloat(kTileValue_user10, 0.0);

				Tile* lastTile = this->tiles[this->lastTile];
				switch (this->lastTile) {
				case kAggressivePassive:
					lastTile->SetFloat(kTileValue_user11, this->aggressive);
					break;
				case kStayFollow:
					lastTile->SetFloat(kTileValue_user11, !this->following);
					break;
				case kNearFar:
					lastTile->SetFloat(kTileValue_user11, this->followingAtDistance);
					break;
				case kRangedMelee:
					if (!this->dogmeatMode) lastTile->SetFloat(kTileValue_user11, this->preferRanged);
					break;
				default:
					break;
				}
			}

			// handle current tile
			this->lastTile = tileID;
			if (tileID != -1) {
				this->tiles[tileID]->SetFloat(kTileValue_user10, 1.0);
			}
			switch (tileID) {
			case kAggressivePassive:
				this->tiles[kButtonText]->SetString(this->aggressive ? "Be Passive" : "Be Aggressive");
				this->tiles[kAggressivePassive]->SetFloat(kTileValue_user11, !this->aggressive);
				break;
			case kUseStimpak:
				this->tiles[kButtonText]->SetString("Use Stimpak");
				break;
			case kStayFollow:
				this->tiles[kButtonText]->SetString(this->following ? "Wait Here" : "Follow Me");
				this->tiles[kStayFollow]->SetFloat(kTileValue_user11, this->following);
				break;
			case kTalkTo:
				this->tiles[kButtonText]->SetString("Talk To");
				break;
			case kBackUp:
				this->tiles[kButtonText]->SetString("Back Up");
				break;
			case kNearFar:
				this->tiles[kButtonText]->SetString(this->followingAtDistance ? "Stay Close" : "Keep Distance");
				this->tiles[kNearFar]->SetFloat(kTileValue_user11, !this->followingAtDistance);
				break;
			case kOpenInventory:
				this->tiles[kButtonText]->SetString("Open Inventory");
				break;
			case kRangedMelee:
				if (this->dogmeatMode) {
					this->tiles[kButtonText]->SetString("Dogmeat Commands");
				}
				else {
					this->tiles[kButtonText]->SetString(this->preferRanged ? "Use Melee" : "Use Ranged");
					this->tiles[kRangedMelee]->SetFloat(kTileValue_user11, !this->preferRanged);
				}
				break;
			case kExit:
				this->tiles[kButtonText]->SetString("Exit");
				break;
			default:
				this->tiles[kButtonText]->SetString("\0");
				break;
			}
			this->HandleButtonContext(this->lastTile);
		}
	}
}

void CompanionWheelMenu::HandleButtonContext(UInt32 tileID)
{
	char strBuf[MAX_PATH];
	*strBuf = '\0';
	switch (tileID)
	{
	case kUseStimpak:
	{
		Actor* companion = this->companion;
		int health = companion->avOwner.GetActorValue(eActorVal_Health);
		int maxHealth = companion->avOwner.GetBaseActorValueInt(eActorVal_Health);;
		UInt32 stimpakCount = 0;
		TESForm* stimpak = CdeclCall<TESForm*>(0x508E40, 0);
		TESBoundObject* stimpakBO = DYNAMIC_CAST(stimpak, TESForm, TESBoundObject);
		ExtraContainerChanges::EntryData* data = ThisCall<ExtraContainerChanges::EntryData*>(0x4E8AB0, PlayerCharacter::GetSingleton(), stimpakBO, 0);
		if (data) {
			stimpakCount = data->countDelta >= 0 ? data->countDelta : 0;
		}
		snprintf(strBuf, MAX_PATH, "HP %d/%d\nStimpak  %d", health, maxHealth, stimpakCount);
		break;
	}
	case kOpenInventory:
	{
		float inventoryWeight = this->companion->GetInventoryWeight();
		float maxCarryWeight = this->companion->GetCalculatedCarryWeight();
		snprintf(strBuf, MAX_PATH, "Wg  %.f/%.f", inventoryWeight, maxCarryWeight);
		break;
	}
	case kRangedMelee:
	{
		ContChangesEntry* preferredWeapon = (ContChangesEntry*)this->companion->GetPreferredWeapon(this->preferRanged ? 1 : 2);
		if (preferredWeapon) {
			snprintf(strBuf, MAX_PATH, "%s", preferredWeapon->GetFullName());
		}
		break;
	}
	default:
		break;
	}

	this->tiles[kButtonContext]->SetString(strBuf);

}

bool CompanionWheelMenu::ShowMenu(Actor* actor)
{
	Tile* menuTile = Tile::GetMenuTile(1050);
	if (menuTile != nullptr) menuTile->Destroy(true);

	static bool hasAddedNewTrait;
	if (!hasAddedNewTrait)
	{
		hasAddedNewTrait = true;
		Tile::RegisterTrait("&CompanionWheelMenu;", 1050);
		Tile::RegisterTrait("radial", 908);
		Tile::RegisterTrait("&xbuttonl;", 17);
		Tile::RegisterTrait("&xbuttonr;", 18);

	}
	Tile* tile = InterfaceManager::GetSingleton()->menuRoot->ReadXML("Data\\Menus\\companion_wheel_menu.xml");
	auto menu = (CompanionWheelMenu*)tile->GetMenu();
	if (!menu || menu->GetID() != 1050) return false;
	menu->RegisterTile(tile, 0);
	if (!menu->HasTiles()) return false;

	menu->companion = actor;
	menu->aggressive = !actor->IsPassive();
	menu->followingAtDistance = actor->GetRefVarData("IsFollowingLong") == 1.0;
	menu->preferRanged = actor->GetRefVarData("CombatStyleRanged") == 1.0;
	menu->following = actor->GetRefVarData("Waiting") == 0.0;

	menu->dogmeatMode = actor->baseForm->refID == kForms_Dogmeat;
	menu->inSubmenu = false;
	menu->tiles[kAggressivePassive]->SetFloat(kTileValue_user11, menu->aggressive);
	menu->tiles[kStayFollow]->SetFloat(kTileValue_user11, !menu->following);
	menu->tiles[kNearFar]->SetFloat(kTileValue_user11, menu->followingAtDistance);
	menu->tiles[kRangedMelee]->SetFloat(kTileValue_user11, menu->preferRanged);
	menu->tiles[kSubtitle]->SetString("\0");
	menu->tiles[kTitle]->SetString("Companion Commands");
	menu->tiles[k360ExitPrompt]->SetString("Exit");
	menu->tiles[k360SelectPrompt]->SetString("Select");
	menu->tiles[k360NavigatePrompt]->SetString("Navigate");
	if (menu->dogmeatMode) {
		menu->tiles[kRangedMelee]->SetString(kTileValue_user8, "Interface\\companion_wheel\\comp_dogmeat_off.dds");
		menu->tiles[kRangedMelee]->SetString(kTileValue_user12, "Interface\\companion_wheel\\comp_dogmeat_off.dds");
		menu->tiles[kRangedMelee]->SetString(kTileValue_user9, "Interface\\companion_wheel\\comp_dogmeat_on.dds");
		menu->tiles[kRangedMelee]->SetString(kTileValue_user13, "Interface\\companion_wheel\\comp_dogmeat_on.dds");
	}
	Sound sound("UIPopUpMessageGeneral", 0x121);
	sound.Play();
	float stackingType = tile->GetValueFloat(kTileValue_stackingtype);

	if (stackingType == 6006.0 || stackingType == 102.0)
	{
		float newDepth = StdCall<float>(0xBFCB40);
		tile->SetFloat(kTileValue_depth, newDepth, 1);
	}

	menu->HideTitle(false);
	return true;
}

Menu* __fastcall CreateMenuHook(void* eax, void* edx, UInt32 menuID) {
	if (menuID == 1050) return CompanionWheelMenu::Create();
	return ThisCall<Menu*>(0x6315E0, eax, menuID);
}

bool CreateForInteractionObject() {

	TESObjectREFR* ref = *(TESObjectREFR**)0xF6A36C;
	Actor* actor = DYNAMIC_CAST(ref, TESObjectREFR, Actor);

	return CompanionWheelMenu::ShowMenu(actor);
}

__declspec(naked) void QueuedMenuRequestHook() {
	static const UInt32 retnAddr = 0x61CAAC;
	__asm {
		cmp eax, 0x8
		jne DONE
		call CreateForInteractionObject
		mov bl, al
		mov eax, 0x61CB61
		jmp eax
		DONE :
		add eax, 0xffffffff
			cmp eax, 6
			jmp retnAddr
	}
}

bool __fastcall IsConsciousAndNotAttackingPlayer(Actor* actor) {
	UInt32 factionPtr = 0;
	bool isConscious = ThisCall<bool>(0x6FAF20, actor);
	bool shouldAttack = ThisCall<UInt8>(0x710630, actor, PlayerCharacter::GetSingleton(), 0, &factionPtr, 0);
	return isConscious && !shouldAttack;
}

__declspec(naked) void ActivationHook() {
	static const UInt32 activationQueueAddr = 0x61D5A0;
	__asm {
		cmp byte ptr ds:[esi + 0x181], 0
		jz DONE
		cmp edi, 0x107A104
		jz DONE
		mov ecx, esi
		call IsConsciousAndNotAttackingPlayer
		test al, al
		jz DONERET
		push 0
		push 0
		push 0
		push esi
		push 8
		call activationQueueAddr
		add esp, 0x14
		DONERET:
		xor al, al
			mov ecx, 0x5573A7
			jmp ecx
			DONE :
		lea ebx, dword ptr ds : [edi + 0x40]
			mov ecx, ebx
			mov eax, 0x5569FE
			jmp eax
	}

}

__declspec(naked) void CreatureHook() {
	static const UInt32 activationQueueAddr = 0x61D5A0;
	static const UInt32 HTAddr = 0x563CF0;
	__asm {
		cmp byte ptr ds:[esi + 0x181], 0
		jz DONE
		mov ecx, esi
		call IsConsciousAndNotAttackingPlayer
		test al, al
		jz DONERET
		push 0
		push 0
		push 0
		push esi
		push 8
		call activationQueueAddr
		add esp, 0x14
		DONERET:
		xor al, al
			mov ecx, 0x5500F0
			jmp ecx
			DONE :
		push 0
			push 0
			call HTAddr
			mov ebp, eax
			add esp, 8
			mov ecx, 0x54FC7C
			jmp ecx
	}
}

void __cdecl CreatureInCombatHook(int type, Actor* a2, int a3, int a4, int mode) {
	if (IsConsciousAndNotAttackingPlayer(a2)) {
		CdeclCall<void>(0x61D5A0, 8, a2, 0, 0, 0);
	}
}

__declspec(naked) void ActorMagicItemHook() {
	static const float timeDelta = 0.001;
	__asm {
		cmp byte ptr ss : [esp + 0x2C] , 0x2
		jne DONE
		fld dword ptr ds : [timeDelta]
		fstp dword ptr ss : [esp + 0x2C]
		mov ecx, 0x7231F4
		jmp ecx
		DONE :
		fld ST(0)
			fsub dword ptr ss : [esi + 0x108]
			mov ecx, 0x72319C
			jmp ecx
	}
}


bool __fastcall GetShouldAttackHook(Actor* source, void* esi, Actor* target, bool inActiveCombat, FactionRelation* outRelation, bool ignoreActiveCombat) {
	if (source == target) return false;
	FactionRelation factionRelation = kFactionRelation_Neutral;
	bool isTeammateAggressive = false;
	PlayerCharacter* player = PlayerCharacter::GetSingleton();

	if (source->isTeammate) {
		if (target == PlayerCharacter::GetSingleton()) return source->PlayerCommitedCrime();

		if (target->GetIsChildSize(false)) return false;

		if (source->IsPassive()) return target->inCombat && (target->InCombatAgainst(player) || target->InCombatAgainst(source));
	}

	else if (target->isTeammate) return GetShouldAttackHook(source, esi, player, inActiveCombat, &factionRelation, 0);

	ACTOR_AGGRESSION aggression = source->isTeammate ? ACTOR_AGGRESSION_AGGRESSIVE : source->GetCachedAggression();

	if (aggression == ACTOR_AGGRESSION_FRENZIED) return true;

	if (!ignoreActiveCombat && (target->InCombatAgainst(source) || source->isTeammate && target->InCombatAgainst(player))) inActiveCombat = true;

	UInt8 out = 0;
	FactionRelation relation = source->isTeammate ? target->GetFactionRelation(player, &out) : source->GetFactionRelation(target, &out);
	*outRelation = relation;

	bool result = false;

	if (target == player && source->PlayerCommitedCrime()) result = true;

	else if (inActiveCombat && (target->GetCachedAggression() == ACTOR_AGGRESSION_FRENZIED || relation != kFactionRelation_Friend && relation != kFactionRelation_Ally))
		result = true;

	else {
		switch (aggression) {
		case ACTOR_AGGRESSION_AGGRESSIVE:
			if (relation == kFactionRelation_Enemy && (!source->isTeammate || player->IsHostileCompassTarget(target))) result = true;
			break;
		case ACTOR_AGGRESSION_VERY_AGGRESSIVE:
			if (relation <= kFactionRelation_Enemy) result = true;
			break;
		case ACTOR_AGGRESSION_FRENZIED:
			result = true;
			break;
		}
	}
	CdeclCall<void>(0x543420, 15, source, target, &result); // ApplyPerkModifiers
	return result;

}

double __fastcall GetDetectedHook(Actor* toDetect, void* edi, Actor* player, int i, int i1) {
	if (toDetect->isTeammate && (toDetect->IsPassive() && !toDetect->inCombat && player->IsSneaking() && !player->inCombat)) return 9000.0;
	return ThisCall<double>(0x4EDBF0, toDetect, player, i, i1);
}

void CompanionWheelMenu::InitHooks()
{

	// show menu when it's queued
	WriteRelJump(0x61CAA6, (UInt32)QueuedMenuRequestHook);

	// create companion wheel when its ID is passed
	WriteRelCall(0x61A921, (UInt32)CreateMenuHook);

	// queue new menu on npc activation
	WriteRelJump(0x5569F9, (UInt32)ActivationHook);

	// queue new menu on creature activation
	WriteRelJump(0x54FC6E, (UInt32)CreatureHook);
	WriteRelCall(0x54F9D6, (UInt32)CreatureInCombatHook);

	// change TileImage logic to include RadialTile ID
	SafeWrite32(0xBEF209, (UInt32)&radialTraitId);
	SafeWrite32(0xBEEF46, (UInt32)&radialTraitId);

	// process stimpaks applied to companions instantly
	SafeWrite8(0x723175, 0x01);
	SafeWrite8(0x72317A, 0x75);
	WriteRelJump(0x723194, (UInt32)ActorMagicItemHook);

	// hide companion wheel in pause menu
	SafeWrite8(0x681D4A, 0x00);

	// add passive/aggressive logic to GetShouldAttack
	WriteRelJump(0x710630, (UInt32)GetShouldAttackHook);

	// change GetDetected to not trigger on passive companions
	WriteRelCall(0x708747, (UInt32)GetDetectedHook);

	GetModuleFileNameA(NULL, iniFilename, MAX_PATH);
	strcpy((char*)(strrchr(iniFilename, '\\') + 1), "Data\\fose\\plugins\\companion_wheel.ini");

}