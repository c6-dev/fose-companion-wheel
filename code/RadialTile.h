#pragma once
#include "fose/GameTiles.h"
#include "fose/GameInterface.h"

class RadialTile : public TileImage 
{
private:
	void Free();
public:
	RadialTile() {};
	~RadialTile() {};
	virtual void				Destroy(bool doFree);
	virtual void				Init(Tile* parent, const char* name, Tile* replacedChild);
	virtual NiNode*				CalcNode(void);
	virtual UInt32				GetType(void);		// returns one of kTileValue_XXX
	virtual const char*			GetTypeStr(void);	// 4-byte id
	virtual UInt32				UpdateField(UInt32 valueID, float floatValue, const char* strValue) { return ThisCall<UInt32>(0xBFD3D0, this, valueID, floatValue, strValue); }
	virtual void				ClearTextures(void);
	virtual TileShaderProperty* GetShaderProperty(void);
	virtual void				SetShaderPropertyColorAlpha(NiNode* node, float alpha, void* overlayColor);


	static void InitHooks();
	RadialTile* Create();
	bool		IsTargeted(float posX, float posY);

	void* RadialTile::operator new(size_t size)
	{
		return GameHeapAlloc(size);
	}

	void RadialTile::operator delete(void* ptr)
	{
		GameHeapFree(ptr);
	}
};

STATIC_ASSERT(sizeof(RadialTile) == 0x48);