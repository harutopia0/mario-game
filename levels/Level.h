#pragma once 
#include <Windows.h>
#include <vector>
#include <string>


#include "Animations.h"
#include "Texture.h"
#include "Sprites.h"
#include "Collision.h"

class CGameObject;
typedef CGameObject* LPGAMEOBJECT;

class CLevel
{
protected:
	int level_id; 
	int time_limit;
	bool isCompleted; 

	int Map_Height;
	int Map_Width;

	std::string scene_file_path;  
	std::vector<LPGAMEOBJECT> Objects; 
public:
	CLevel(int lv_id, int time, std::string FilePath);
	virtual ~CLevel(); 
	virtual void LoadLevel() ; 
	virtual void UnLoadLevel() = 0;

	virtual void Render() {};
	virtual void Update(DWORD dt) {}; 

	virtual void PurgeDeletedObjects() {};
	//Marking GameObject have isDeleted = true

	int GetHeight()
	{
		return this->Map_Height;
	}
	int GetWidth()
	{
		return this->Map_Width;
	}
	std::vector<LPGAMEOBJECT> GetObjects() const { return objects; }
};

