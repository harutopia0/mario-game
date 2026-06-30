#include "Level.h"
#include "Ground.h"
#include "Brick.h"
#include "GameObject.h"
#include "Pipe.h"

void CLevel1 :: LoadLevel()
{

	this->Map_Height = 5000;
	this->Map_Width = 1000; 
	
	CGround* ground = new CGround();
	ground->SetPosition(0, 950);
	Objects.push_back(ground); 



}