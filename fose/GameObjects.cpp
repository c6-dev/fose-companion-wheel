#include "GameObjects.h"
#include "GameRTTI.h"
#include "GameExtraData.h"
#include "netimmerse.h"
ScriptEventList* TESObjectREFR::GetEventList() const
{
	BSExtraData* xData = extraDataList.GetByType(kExtraData_Script);
	if (xData)
	{
		ExtraScript* xScript = DYNAMIC_CAST(xData, BSExtraData, ExtraScript);
		if (xScript)
			return xScript->eventList;
	}

	return 0;
}

void TESObjectREFR::SetPos(NiPoint3 pos)
{
	ThisCall<void>(0x4EEAE0, this, &pos);
	MobileObject* mobileObj = DYNAMIC_CAST(this, TESObjectREFR, MobileObject);
	if (mobileObj) {
		void* charCtrl = ThisCall<void*>(0x764EA0, mobileObj);
		if (charCtrl && ThisCall<int>(0x919EF0, (UInt32*)charCtrl + 0x260) != 4) {
			ThisCall<int>(0x4E6580, charCtrl, &pos);
		}
	}
	NiNode* node = this->GetNiNode();
	if (node) {
		node->SetLocalTranslate(pos);
		CdeclCall<void>(0x8DA3B0, node, 1);
		NiUpdateData updateData;
		memset(&updateData, 0, sizeof(updateData));
		ThisCall<int>(0x8264C0, node, &updateData);
	}
}


static PlayerCharacter ** g_thePlayer = (PlayerCharacter **)0x0107A104;


bool PlayerCharacter::IsHostileCompassTarget(Actor* actor)
{
	if (!this->compassTargets.Empty()) {
		auto it = this->compassTargets.Head();
		do {
			if (it->data->target == actor && it->data->isHostile) return true;
		} while (it = it->next);
	}
	return false;
}

PlayerCharacter* PlayerCharacter::GetSingleton()
{
	return *g_thePlayer;
}

// shamelessly hijacking ExtraHasNoRumors
bool Actor::IsPassive()
{
	ExtraHasNoRumors* nr = (ExtraHasNoRumors*)this->extraDataList.GetByType(kExtraData_HasNoRumors);
	if (nr && nr->hasNoRumors) return true;
	return false;
}

void Actor::SetPassive(bool isPassive)
{
	if (isPassive = false) this->extraDataList.RemoveByType(kExtraData_HasNoRumors);
	ThisCall<UInt32>(0x40D220, &this->extraDataList, true);
}

float Actor::GetRefVarData(const char* name)
{
	TESForm* baseForm = this->baseForm;
	TESScriptableForm* scrf = DYNAMIC_CAST(baseForm, TESForm, TESScriptableForm);
	if (scrf) {
		Script* script = scrf->script;
		void* xScript = ThisCall<void*>(0x40AD00, &this->extraDataList);
		if (xScript) {
			UInt32 varPtr = 0;
			void* scriptVarInfo = ThisCall<void*>(0x517600, script, name, &varPtr);
			if (varPtr) {
				return (float)ThisCall<double>(0x516810, xScript, varPtr, 0);
			}
		}
	}
	return -1.0;
}
