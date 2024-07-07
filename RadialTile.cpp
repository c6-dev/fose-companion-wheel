#include "RadialTile.h"

void RadialTile::Free()
{
	TileImage::Free();
}

void RadialTile::Destroy(bool doFree)
{
	this->Free();
	if (doFree) {
		GameHeapFree(this);
	}
}

void RadialTile::Init(Tile* parent, const char* name, Tile* replacedChild)
{
	ThisCall<void>(0xBFD430, this, parent, name, replacedChild);
}

NiNode* RadialTile::CalcNode()
{
	return ThisCall<NiNode*>(0xBFD920, this);
}

UInt32 RadialTile::GetType()
{
	return 902;
}

const char* RadialTile::GetTypeStr()
{
	return "IMGR";
}

constexpr float PI = 3.14159265358979323846;
constexpr float TWO_PI = 2 * PI;
constexpr float HALF_PI = PI / 2;

bool RadialTile::IsTargeted(float posX, float posY) {
	float centerX = this->GetValueFloat(kTileValue_user0);
	float centerY = this->GetValueFloat(kTileValue_user1);
	float minAngle = this->GetValueFloat(kTileValue_user2);
	float maxAngle = this->GetValueFloat(kTileValue_user3);
	float minMagnitude = this->GetValueFloat(kTileValue_user4);
	float maxMagnitude = this->GetValueFloat(kTileValue_user5);

	// Retrieve width and adjust if necessary
	float width = (float)StdCall<double>(0x61A670);
	if (width == 0.0) width = 1.0;
	if (InterfaceManager::GetSingleton()->menuRoot) {
		float rootWidth = InterfaceManager::GetSingleton()->menuRoot->GetValueFloat(kTileValue_width);
		if (rootWidth > 0) {
			width = rootWidth / width;
		}
		else {
			width = 1.0;
		}
	}
	else {
		width = 1.0;
	}

	// Transform coordinates
	float fX = posX * width - centerX;
	float fY = posY * width - centerY;

	// Calculate the angle
	float angle = 0.0;
	if (fY == 0.0) {
		angle = fX <= 0.0 ? 3 * HALF_PI : HALF_PI;
	}
	else {
		angle = atan(-fX / fY);

	}
	if (fY > 0.0) angle += PI;
	if (fX < 0.0 && fY < 0.0) angle += TWO_PI;

	// Check angle range
	if (angle < minAngle || angle > maxAngle) return false;

	// Calculate magnitude
	float magnitude = sqrt(fX * fX + fY * fY);

	// Check magnitude range
	return (magnitude >= minMagnitude && magnitude <= maxMagnitude);

}


void RadialTile::ClearTextures()
{
	ThisCall<void>(0xBEF350, this);
}

TileShaderProperty* RadialTile::GetShaderProperty()
{
	return ThisCall<TileShaderProperty*>(0xBEB3D0, this);
}

void RadialTile::SetShaderPropertyColorAlpha(NiNode* node, float alpha, void* overlayColor)
{
	ThisCall<void>(0xBEB400, this, node, alpha, overlayColor);
}
Tile* __cdecl TileCreateHook(UInt32 tileID) {
	if (tileID == 908) {
		return new RadialTile();
	}
	return CdeclCall<Tile*>(0xBF1B90, tileID);
}
bool __cdecl CheckTileTarget(Tile* tile, float posX, float posY) {
	if (tile->GetType() == 902 && tile->GetTypeStr()[3] == 'R') { // ugly as all hell but otherwise it's 15 more hooks
		RadialTile* rTile = (RadialTile*)tile;
		return rTile->IsTargeted(posX, posY);
	}
	return true;
}
__declspec(naked) void TileTargetHook() {
	__asm {

		sub esp, 8
		fld dword ptr ss : [esp + 0x20]
		fstp dword ptr ss : [esp + 4]
		fld dword ptr ss : [esp + 0x3C]
		fstp dword ptr ss : [esp]
		mov ecx, edi
		push ecx
		call CheckTileTarget
		add esp, 0xC
		test al, al
		jz DONE
		push 0xFAF
		mov eax, 0x6252CF
		mov ecx, edi
		jmp eax
		DONE :
		mov eax, 0x62531F
			jmp eax
	}
}
void RadialTile::InitHooks()
{
	WriteRelCall(0xBF1D6D, (UInt32)TileCreateHook);
	WriteRelJump(0x6252C8, (UInt32)TileTargetHook);
}

RadialTile* RadialTile::Create()
{
	return new RadialTile();
}