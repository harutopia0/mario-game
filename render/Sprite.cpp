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

    float spriteWidth = (this->right - this->left + 1);
    float spriteHeight = (this->bottom - this->top + 1);

    float centerX = x + (spriteWidth / 2.0f);
    float centerY = y + (spriteHeight / 2.0f);

    D3DXMatrixTranslation(&matTranslation, centerX, centerY, 0.1f);

    this->spriteInfo.matWorld = (this->matScaling * matTranslation);
    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteInfo, 1, 0, 0);
}

void Sprite::Draw(float x, float y, int nx)
{
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation;

    float spriteWidth = (this->right - this->left + 1);
    float spriteHeight = (this->bottom - this->top + 1);

    float centerX = x + (spriteWidth / 2.0f);
    float centerY = y + (spriteHeight / 2.0f);

    D3DXMatrixTranslation(&matTranslation, centerX, centerY, 0.1f);

    D3DX10_SPRITE spriteToDraw = this->spriteInfo;
    spriteToDraw.matWorld = (this->matScaling * matTranslation);

    if (nx > 0)
    {
        spriteToDraw.TexCoord.x = this->spriteInfo.TexCoord.x + this->spriteInfo.TexSize.x;
        spriteToDraw.TexSize.x = -this->spriteInfo.TexSize.x;
    }

    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteToDraw, 1, 0, 0);
}

void Sprite::Draw(float x, float y, float angle)
{
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation, matRotation;

    float spriteWidth = (this->right - this->left + 1);
    float spriteHeight = (this->bottom - this->top + 1);

    float centerX = x + (spriteWidth / 2.0f);
    float centerY = y + (spriteHeight / 2.0f);

    D3DXMatrixRotationZ(&matRotation, angle);
    D3DXMatrixTranslation(&matTranslation, centerX, centerY, 0.1f);

    D3DX10_SPRITE spriteToDraw = this->spriteInfo;
    spriteToDraw.matWorld = (this->matScaling * matRotation * matTranslation);
    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteToDraw, 1, 0, 0);
}

void Sprite::Draw(float x, float y, D3DXCOLOR color)
{
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation;

    float spriteWidth = (this->right - this->left + 1);
    float spriteHeight = (this->bottom - this->top + 1);

    float centerX = x + (spriteWidth / 2.0f);
    float centerY = y + (spriteHeight / 2.0f);

    D3DXMatrixTranslation(&matTranslation, centerX, centerY, 0.1f);

    D3DX10_SPRITE spriteToDraw = this->spriteInfo;
    spriteToDraw.matWorld = (this->matScaling * matTranslation);
    spriteToDraw.ColorModulate = color;
    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteToDraw, 1, 0, 0);
}

void Sprite::Draw(float x, float y, float drawWidth, float drawHeight, float alpha)
{
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation, matScaling;

    D3DXMatrixScaling(&matScaling, drawWidth, drawHeight, 1.0f);

    float centerX = x + (drawWidth / 2.0f);
    float centerY = y + (drawHeight / 2.0f);

    D3DXMatrixTranslation(&matTranslation, centerX, centerY, 0.1f);

    D3DX10_SPRITE spriteToDraw = this->spriteInfo;
    spriteToDraw.matWorld = (matScaling * matTranslation);

    spriteToDraw.ColorModulate = D3DXCOLOR(1.0f, 1.0f, 1.0f, alpha);
    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteToDraw, 1, 0, 0);
}

void Sprite::DrawRotatedScaled(float x, float y, float angle, float scale, float alpha)
{
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation, matRotation, matScale;

    float spriteWidth = (this->right - this->left + 1);
    float spriteHeight = (this->bottom - this->top + 1);

    D3DXMatrixScaling(&matScale, spriteWidth * scale, spriteHeight * scale, 1.0f);
    D3DXMatrixRotationZ(&matRotation, angle);

    float centerX = x + (spriteWidth / 2.0f);
    float centerY = y + (spriteHeight / 2.0f);

    D3DXMatrixTranslation(&matTranslation, centerX, centerY, 0.1f);

    D3DX10_SPRITE spriteToDraw = this->spriteInfo;
    spriteToDraw.matWorld = (matScale * matRotation * matTranslation);
    spriteToDraw.ColorModulate = D3DXCOLOR(1.0f, 1.0f, 1.0f, alpha);

    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteToDraw, 1, 0, 0);
}

Sprite::~Sprite() {}