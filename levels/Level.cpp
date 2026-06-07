

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
    f.open(scene_file_path.c_str());

    if (!f.is_open()) return;

    int rows, cols;
    f >> rows >> cols; 

    // Định nghĩa kích thước 1 ô vuông (Tile Size). 
    // Trong game Mario thường là 16x16 hoặc 32x32 pixel. Mình giả sử là 32.
    int tile_size = 32; 

    
    this->map_width = cols * tile_size;
    this->map_height = rows * tile_size;

    
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            int object_id;
            f >> object_id; // Đọc từng con số

            
            if (object_id == 0) continue;

            
            float x = (float)(c * tile_size);
            float y = (float)(r * tile_size);

            CGameObject* obj = NULL;

           
            switch (object_id)
            {
                case 1: 
                    obj = new CBrick(); 
                    break;
				case 4: 
					obj = new CCoin(); 
					break;
				case 5: 
					obj = new CGround(); 
					break;
                case 6: 
                    obj = new CPipe();   // Ống nước
                    break;
				case 7:
					obj = new CMushroom();
					break;
				case 8:
					obj = new CFlower();
					break;
				case 30:
					obj = new CPlatform();
					break;
				case 50:
					obj = new CPortal();
					break;

                
                // Thêm các case khác ở đây (ví dụ Mario, Nấm...)
            }

            // Nếu tạo thành công, set tọa độ và đẩy vào mảng quản lý
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

