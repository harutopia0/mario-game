#include "../render/Sprite.h"
#include "../core/Game.h"

Sprite::Sprite(int id, int left, int top, int right, int bottom, ID3D10ShaderResourceView* tex, int texWidth, int texHeight)
{
    this->id = id;
    this->left = left;
    this->top = top;
    this->right = right;
    this->bottom = bottom;
    this->texture = tex;

    spriteInfo.pTexture = tex;

    spriteInfo.TexCoord.x = this->left / (float)texWidth;
    spriteInfo.TexCoord.y = this->top / (float)texHeight;

    int spriteWidth = (this->right - this->left + 1);
    int spriteHeight = (this->bottom - this->top + 1);

    spriteInfo.TexSize.x = spriteWidth / (float)texWidth;
    spriteInfo.TexSize.y = spriteHeight / (float)texHeight;

    spriteInfo.ColorModulate = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
    spriteInfo.TextureIndex = 0;

    D3DXMatrixScaling(&this->matScaling, (FLOAT)spriteWidth, (FLOAT)spriteHeight, 1.0f);
}

void Sprite::Draw(float x, float y)
{
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation;

    D3DXMatrixTranslation(&matTranslation, x, y, 0.1f);

    this->spriteInfo.matWorld = (this->matScaling * matTranslation);
    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteInfo, 1, 0, 0);
}

Sprite::~Sprite() {}