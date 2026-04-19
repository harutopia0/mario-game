#include "../render/Sprite.h"
#include "../core/Game.h"

Sprite::Sprite(int id, int left, int top, int right, int bottom, LPDIRECT3DTEXTURE tex)
{
    this->id = id;
    this->left = left;
    this->top = top;
    this->right = right;
    this->bottom = bottom;

    this->texture = tex->GetShaderResourceView();
    int texWidth = tex->GetWidth();
    int texHeight = tex->GetHeight();

    spriteInfo.pTexture = this->texture;

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

void Sprite::Draw(float x, float y, float drawWidth, float drawHeight, float alpha)
{
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation, matScaling;

    D3DXMatrixScaling(&matScaling, drawWidth, drawHeight, 1.0f);
    D3DXMatrixTranslation(&matTranslation, x, y, 0.1f);

    D3DX10_SPRITE spriteToDraw = this->spriteInfo;
    spriteToDraw.matWorld = (matScaling * matTranslation);

    spriteToDraw.ColorModulate = D3DXCOLOR(1.0f, 1.0f, 1.0f, alpha);

    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteToDraw, 1, 0, 0);
}

Sprite::~Sprite() {}