

#include "Level.h"
#include "Ground.h"
#include "Brick.h"
#include "GameObject.h"
#include "Pipe.h"
#include "Flower.h"
#include "Enemy.h"
#include "Mario.h"

#include <fstream>
#include <sstream> 

#include "Level.h"
#include "Ground.h"
#include "Brick.h"
#include "GameObject.h"
#include "Pipe.h"
#include "Flower.h"
#include "Enemy.h"
#include "Mario.h"

#include <fstream>
#include <sstream> 

CLevel:: CLevel(int level_id, int time_limit, std::string FilePath)
{
	this->level_id = level_id;
	this->time_limit = time_limit;
	this->scene_file_path = FilePath;
	this->isCompleted = false; 

	
	this->Map_Height = 0;
	this->Map_Width = 0;
}

CLevel:: ~CLevel()
{
	UnLoadLevel();
}

void CLevel::Update(DWORD dt)
{     // Tải đối tượng lên 
	for (size_t i = 0; i < Objects.size(); i++)
	{
		//Này để kiểm xem đối tượng đã bị tiêu diệt chưa
		// Nếu đã bị tiêu diệt aka isDeleted = true, không update
		if (Objects[i]->IsDeleted == false)
		{
			Objects[i]->Update(dt, &Objects); 
		}

	}
}

void CLevel::LoadLevel()
{
	ifstream f;
	f.open(scene_file_path.c_str()); // Mở đúng file được truyền vào từ constructor

	if (!f.is_open()) return;

	int section = SECTION_UNKNOWN;
	char str[MAX_LINE_LENGTH];

	while (f.getline(str, MAX_LINE_LENGTH))
	{
		std::string line(str);

		if (line.empty() || line[0] == '#') continue; 

		if (line == "[ASSETS]") { 
			section = SECTION_ASSETS; 
			continue; 
		}
		if (line == "[OBJECTS]") { 
			section = SECTION_OBJECTS; 
			continue; 
		}

		if (section == SECTION_ASSETS)
		{
			// Xử lý load file tài nguyên (mario.txt, brick.txt...)
		}
		else if (section == SECTION_OBJECTS)
		{
			stringstream ss(line);
			int type;
			float x, y;

			ss >> type >> x >> y;

			CGameObject* obj = NULL;

			switch (type)
			{
				case 0: 
				   
					break;
				case 1: 
					obj = new CBrick(x, y);
					break;
				case 2: 
					obj = new CGoomba(x, y);
				case 3:
					obj = new CKoopas(x, y);
					break;
				case 4: 
					obj = new CCoin(x, y);
					break;
				case 5:
					obj = new CGround(x, y);
					break;
				case 6:
					obj = new CPipe(x, y);
					break;
				case 7:	
					obj = new CMushroom(x, y);
					break;
				case 8:
					obj = new CFlower(x, y);
					break;
				case 30: // PLATFORM 
				{
					float w, h, l;
					int sprite1, sprite2, sprite3;
					ss >> w >> h >> l >> sprite1 >> sprite2 >> sprite3;
					
					// obj = new CPlatform(w, h, l, sprite1, sprite2, sprite3);
					break;
				}
				case 50: // PORTAL 
				{
					float target_x, target_y;
					int target_level;
					ss >> target_x >> target_y >> target_level;
					
					// obj = new CPortal(target_x, target_y, target_level);
					break;
				}
			}

			if (obj != NULL)
			{
				obj->SetPosition(x, y);
				objects.push_back(obj);
			}
		}
	}

	f.close();
}
void CLevel::Render()
{
	// Đưa đối tượng lên màn hình 
	for (size_t i = 0; i < Objects.size(); i++)
	{  //Này để kiểm xem đối tượng đã bị tiêu diệt chưa
		// Nếu đã bị tiêu diệt aka isDeleted = true, không render
		if (Objects[i]->IsDeleted == false)
		{
			Objects[i]->Render();
		}
		
	}
}

void CLevel::UnLoadLevel()
// trả tài nguyên 
{
	for (size_t i = 0; i < Objects.size(); i++)
	{
		delete Objects[i];

	}
	Objects.clear(); 
}

void CLevel::PurgeDeletedObjects()
{
	vector<LPGAMEOBJECT>::iterator it;
	for (it = objects.begin(); it != objects.end(); )
	{
		LPGAMEOBJECT o = *it;
		if (o->IsDeleted())
		{
			delete o;               // Xóa khỏi RAM
			it = objects.erase(it); // Xóa khỏi danh sách vector
		}
		else
		{
			it++;
		}
	}
}

