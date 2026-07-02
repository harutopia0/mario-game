#include "../render/Sprite.h"
#include "../core/Game.h"
#include <cmath>

float Sprite::globalScale = 1.0f;

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

    spriteInfo.TexCoord.x = (float)this->left / (float)texWidth;
    spriteInfo.TexCoord.y = (float)this->top / (float)texHeight;

    int spriteWidth = (this->right - this->left + 1);
    int spriteHeight = (this->bottom - this->top + 1);

    spriteInfo.TexSize.x = (float)spriteWidth / (float)texWidth;
    spriteInfo.TexSize.y = (float)spriteHeight / (float)texHeight;

    spriteInfo.ColorModulate = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
    spriteInfo.TextureIndex = 0;

    D3DXMatrixScaling(&this->matScaling, (FLOAT)spriteWidth, (FLOAT)spriteHeight, 1.0f);
}

void Sprite::Draw(float x, float y)
{
    if (this->texture == NULL) return;
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation, matScale;

    float actualWidth = (this->right - this->left + 1) * Sprite::globalScale;
    float actualHeight = (this->bottom - this->top + 1) * Sprite::globalScale;

    float centerX = std::round(x * Sprite::globalScale) + (actualWidth / 2.0f);
    float centerY = std::round(y * Sprite::globalScale) + (actualHeight / 2.0f);

    D3DXMatrixTranslation(&matTranslation, centerX, centerY, 0.1f);
    D3DXMatrixScaling(&matScale, Sprite::globalScale, Sprite::globalScale, 1.0f);

    D3DX10_SPRITE spriteToDraw = this->spriteInfo;
    spriteToDraw.matWorld = (this->matScaling * matScale * matTranslation);
    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteToDraw, 1, 0, 0);
}

void Sprite::Draw(float x, float y, int nx)
{
    if (this->texture == NULL) return;
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation, matScale;

    float actualWidth = (this->right - this->left + 1) * Sprite::globalScale;
    float actualHeight = (this->bottom - this->top + 1) * Sprite::globalScale;

    float centerX = std::round(x * Sprite::globalScale) + (actualWidth / 2.0f);
    float centerY = std::round(y * Sprite::globalScale) + (actualHeight / 2.0f);

    D3DXMatrixTranslation(&matTranslation, centerX, centerY, 0.1f);
    D3DXMatrixScaling(&matScale, Sprite::globalScale, Sprite::globalScale, 1.0f);

    D3DX10_SPRITE spriteToDraw = this->spriteInfo;
    spriteToDraw.matWorld = (this->matScaling * matScale * matTranslation);

    if (nx > 0)
    {
        spriteToDraw.TexCoord.x = this->spriteInfo.TexCoord.x + this->spriteInfo.TexSize.x;
        spriteToDraw.TexSize.x = -this->spriteInfo.TexSize.x;
    }

    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteToDraw, 1, 0, 0);
}

void Sprite::Draw(float x, float y, int nx, int ny)
{
    if (this->texture == NULL) return;
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation, matScale;

    float actualWidth = (this->right - this->left + 1) * Sprite::globalScale;
    float actualHeight = (this->bottom - this->top + 1) * Sprite::globalScale;

    float centerX = std::round(x * Sprite::globalScale) + (actualWidth / 2.0f);
    float centerY = std::round(y * Sprite::globalScale) + (actualHeight / 2.0f);

    D3DXMatrixTranslation(&matTranslation, centerX, centerY, 0.1f);
    D3DXMatrixScaling(&matScale, Sprite::globalScale, Sprite::globalScale, 1.0f);

    D3DX10_SPRITE spriteToDraw = this->spriteInfo;
    spriteToDraw.matWorld = (this->matScaling * matScale * matTranslation);

    if (nx > 0)
    {
        spriteToDraw.TexCoord.x = this->spriteInfo.TexCoord.x + this->spriteInfo.TexSize.x;
        spriteToDraw.TexSize.x = -this->spriteInfo.TexSize.x;
    }

    if (ny > 0)
    {
        spriteToDraw.TexCoord.y = this->spriteInfo.TexCoord.y + this->spriteInfo.TexSize.y;
        spriteToDraw.TexSize.y = -this->spriteInfo.TexSize.y;
    }

    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteToDraw, 1, 0, 0);
}

void Sprite::Draw(float x, float y, float angle)
{
    if (this->texture == NULL) return;
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation, matRotation, matScale;

    float actualWidth = (this->right - this->left + 1) * Sprite::globalScale;
    float actualHeight = (this->bottom - this->top + 1) * Sprite::globalScale;

    float centerX = std::round(x * Sprite::globalScale) + (actualWidth / 2.0f);
    float centerY = std::round(y * Sprite::globalScale) + (actualHeight / 2.0f);

    D3DXMatrixRotationZ(&matRotation, angle);
    D3DXMatrixTranslation(&matTranslation, centerX, centerY, 0.1f);
    D3DXMatrixScaling(&matScale, Sprite::globalScale, Sprite::globalScale, 1.0f);

    D3DX10_SPRITE spriteToDraw = this->spriteInfo;
    spriteToDraw.matWorld = (this->matScaling * matScale * matRotation * matTranslation);
    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteToDraw, 1, 0, 0);
}

void Sprite::Draw(float x, float y, D3DXCOLOR color)
{
    if (this->texture == NULL) return;
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation, matScale;

    float actualWidth = (this->right - this->left + 1) * Sprite::globalScale;
    float actualHeight = (this->bottom - this->top + 1) * Sprite::globalScale;

    float centerX = std::round(x * Sprite::globalScale) + (actualWidth / 2.0f);
    float centerY = std::round(y * Sprite::globalScale) + (actualHeight / 2.0f);

    D3DXMatrixTranslation(&matTranslation, centerX, centerY, 0.1f);
    D3DXMatrixScaling(&matScale, Sprite::globalScale, Sprite::globalScale, 1.0f);

    D3DX10_SPRITE spriteToDraw = this->spriteInfo;
    spriteToDraw.matWorld = (this->matScaling * matScale * matTranslation);
    spriteToDraw.ColorModulate = color;
    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteToDraw, 1, 0, 0);
}

void Sprite::Draw(float x, float y, float drawWidth, float drawHeight, float alpha)
{
    if (this->texture == NULL) return;
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation, matScaling;

    float actualDrawWidth = drawWidth * Sprite::globalScale;
    float actualDrawHeight = drawHeight * Sprite::globalScale;

    D3DXMatrixScaling(&matScaling, actualDrawWidth, actualDrawHeight, 1.0f);

    float centerX = std::round(x * Sprite::globalScale) + (actualDrawWidth / 2.0f);
    float centerY = std::round(y * Sprite::globalScale) + (actualDrawHeight / 2.0f);

    D3DXMatrixTranslation(&matTranslation, centerX, centerY, 0.1f);

    D3DX10_SPRITE spriteToDraw = this->spriteInfo;
    spriteToDraw.matWorld = (matScaling * matTranslation);

    spriteToDraw.ColorModulate = D3DXCOLOR(1.0f, 1.0f, 1.0f, alpha);
    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteToDraw, 1, 0, 0);
}

void Sprite::Draw(float x, float y, float drawWidth, float drawHeight, D3DXCOLOR color)
{
    if (this->texture == NULL) return;
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation, matScaling;

    float actualDrawWidth = drawWidth * Sprite::globalScale;
    float actualDrawHeight = drawHeight * Sprite::globalScale;

    D3DXMatrixScaling(&matScaling, actualDrawWidth, actualDrawHeight, 1.0f);

    float centerX = std::round(x * Sprite::globalScale) + (actualDrawWidth / 2.0f);
    float centerY = std::round(y * Sprite::globalScale) + (actualDrawHeight / 2.0f);

    D3DXMatrixTranslation(&matTranslation, centerX, centerY, 0.1f);

    D3DX10_SPRITE spriteToDraw = this->spriteInfo;
    spriteToDraw.matWorld = (matScaling * matTranslation);

    spriteToDraw.ColorModulate = color;
    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteToDraw, 1, 0, 0);
}

void Sprite::DrawRotatedScaled(float x, float y, float angle, float scale, float alpha)
{
    if (this->texture == NULL) return;
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation, matRotation, matScale;

    float spriteWidth = (this->right - this->left + 1);
    float spriteHeight = (this->bottom - this->top + 1);

    D3DXMatrixScaling(&matScale, spriteWidth * scale * Sprite::globalScale, spriteHeight * scale * Sprite::globalScale, 1.0f);
    D3DXMatrixRotationZ(&matRotation, angle);

    float actualWidth = spriteWidth * Sprite::globalScale;
    float actualHeight = spriteHeight * Sprite::globalScale;

    float centerX = std::round(x * Sprite::globalScale) + (actualWidth / 2.0f);
    float centerY = std::round(y * Sprite::globalScale) + (actualHeight / 2.0f);

    D3DXMatrixTranslation(&matTranslation, centerX, centerY, 0.1f);

    D3DX10_SPRITE spriteToDraw = this->spriteInfo;
    spriteToDraw.matWorld = (matScale * matRotation * matTranslation);
    spriteToDraw.ColorModulate = D3DXCOLOR(1.0f, 1.0f, 1.0f, alpha);

    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteToDraw, 1, 0, 0);
}

void Sprite::DrawRotatedScaled(float x, float y, float angle, float scale, D3DXCOLOR color)
{
    if (this->texture == NULL) return;
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation, matRotation, matScale;

    float spriteWidth = (this->right - this->left + 1);
    float spriteHeight = (this->bottom - this->top + 1);

    D3DXMatrixScaling(&matScale, spriteWidth * scale * Sprite::globalScale, spriteHeight * scale * Sprite::globalScale, 1.0f);
    D3DXMatrixRotationZ(&matRotation, angle);

    float actualWidth = spriteWidth * Sprite::globalScale;
    float actualHeight = spriteHeight * Sprite::globalScale;

    float centerX = std::round(x * Sprite::globalScale) + (actualWidth / 2.0f);
    float centerY = std::round(y * Sprite::globalScale) + (actualHeight / 2.0f);

    D3DXMatrixTranslation(&matTranslation, centerX, centerY, 0.1f);

    D3DX10_SPRITE spriteToDraw = this->spriteInfo;
    spriteToDraw.matWorld = (matScale * matRotation * matTranslation);
    spriteToDraw.ColorModulate = color;

    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteToDraw, 1, 0, 0);
}

void Sprite::DrawRotatedScaled(float x, float y, float angle, float scaleX, float scaleY, D3DXCOLOR color)
{
    if (this->texture == NULL) return;
    Game* game = Game::GetInstance();
    D3DXMATRIX matTranslation, matRotation, matScale;

    float spriteWidth = (this->right - this->left + 1);
    float spriteHeight = (this->bottom - this->top + 1);

    D3DXMatrixScaling(&matScale, spriteWidth * scaleX * Sprite::globalScale, spriteHeight * scaleY * Sprite::globalScale, 1.0f);
    D3DXMatrixRotationZ(&matRotation, angle);

    float actualWidth = spriteWidth * Sprite::globalScale;
    float actualHeight = spriteHeight * Sprite::globalScale;

    float centerX = std::round(x * Sprite::globalScale) + (actualWidth / 2.0f);
    float centerY = std::round(y * Sprite::globalScale) + (actualHeight / 2.0f);

    D3DXMatrixTranslation(&matTranslation, centerX, centerY, 0.1f);

    D3DX10_SPRITE spriteToDraw = this->spriteInfo;
    spriteToDraw.matWorld = (matScale * matRotation * matTranslation);
    spriteToDraw.ColorModulate = color;

    game->GetSpriteHandler()->DrawSpritesImmediate(&spriteToDraw, 1, 0, 0);
}

Sprite::~Sprite() {}

