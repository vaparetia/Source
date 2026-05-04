#pragma once

#ifndef __TRANSFARRING_MENU_RENDERING__H__
#define __TRANSFARRING_MENU_RENDERING__H__

struct SMenuObjectTag;
struct STransfarringGlobalRenderingData;

struct STransfarringGlobalRenderingData* Transfarring_InitRenderingData();
void Transfarring_DestroyRenderingData(struct STransfarringGlobalRenderingData *data);

void Transfarring_InitMenuObjectRenderingData(struct SMenuObjectTag *obj);
void Transfarring_DestroyMenuObjectRenderingData(struct SMenuObjectTag *obj);

void Transfarring_RenderScene(struct STransfarringGlobalRenderingData *data);

void Transfarring_FlagBpTextChanged(struct SMenuObjectTag *obj);

//Approximated width of a kText sprite.
float Transfarring_GetApproximatedTextWidth(float scale, char const * const text);

#endif