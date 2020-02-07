#include <string.h>
#include <tiny3d.h>
#include <pngdec/pngdec.h>
#include <sys/thread.h>
#include <unistd.h>

#include "libfont.h"
#include "menu.h"

/*
    From sprite2D source
    I'm not going to document them for that reason
*/

// draw one background color in virtual 2D coordinates
void DrawBackground2D(u32 rgba)
{
	tiny3d_SetPolygon(TINY3D_QUADS);

	tiny3d_VertexPos(-apollo_config.marginH, -apollo_config.marginV, 65535);
	tiny3d_VertexColor(rgba);

	tiny3d_VertexPos(847 + apollo_config.marginH, -apollo_config.marginV, 65535);

	tiny3d_VertexPos(847 + apollo_config.marginH, 511 + apollo_config.marginV, 65535);

	tiny3d_VertexPos(-apollo_config.marginH, 511 + apollo_config.marginV, 65535);
	tiny3d_End();
}

void DrawSprites2D(float x, float y, float layer, float dx, float dy, u32 rgba)
{
    tiny3d_SetPolygon(TINY3D_QUADS);

    tiny3d_VertexPos(x     , y     , layer);
    tiny3d_VertexColor(rgba);
    tiny3d_VertexTexture(0.01f, 0.01f);

    tiny3d_VertexPos(x + dx, y     , layer);
    tiny3d_VertexTexture(0.99f, 0.01f);

    tiny3d_VertexPos(x + dx, y + dy, layer);
    tiny3d_VertexTexture(0.99f, 0.99f);

    tiny3d_VertexPos(x     , y + dy, layer);
    tiny3d_VertexTexture(0.01f, 0.99f);

    tiny3d_End();
}

void DrawSpritesRot2D(float x, float y, float layer, float dx, float dy, u32 rgba, float angle)
{
    dx /= 2.0f; dy /= 2.0f;

    MATRIX matrix;
    
    // rotate and translate the sprite
    matrix = MatrixRotationZ(angle);
    matrix = MatrixMultiply(matrix, MatrixTranslation(x + dx, y + dy, 0.0f));
    
    // fix ModelView Matrix
    tiny3d_SetMatrixModelView(&matrix);
   
    tiny3d_SetPolygon(TINY3D_QUADS);

    tiny3d_VertexPos(-dx, -dy, layer);
    tiny3d_VertexColor(rgba);
    tiny3d_VertexTexture(0.0f , 0.0f);

    tiny3d_VertexPos(dx , -dy, layer);
    tiny3d_VertexTexture(0.99f, 0.0f);

    tiny3d_VertexPos(dx , dy , layer);
    tiny3d_VertexTexture(0.99f, 0.99f);

    tiny3d_VertexPos(-dx, dy , layer);
    tiny3d_VertexTexture(0.0f , 0.99f);

    tiny3d_End();

    tiny3d_SetMatrixModelView(NULL); // set matrix identity

}

void DrawSelector(int x, int y, int w, int h, int hDif, u8 alpha)
{
	int i = 0;
	for (i = 0; i < 848; i++)
		DrawTexture(menu_textures[mark_line_png_index], i, y, 0, menu_textures[mark_line_png_index].texture.width, menu_textures[mark_line_png_index].texture.height + hDif, 0xFFFFFF00 | alpha);

	DrawTextureCentered(menu_textures[mark_arrow_png_index], x, y, 0, w, h, 0xFFFFFF00 | alpha);
}

void DrawHeader_Ani(png_texture icon, char * headerTitle, char * headerSubTitle, u32 rgba, u32 bgrgba, int ani, int div)
{
	u8 icon_a = (u8)(((ani * 2) > 0xFF) ? 0xFF : (ani * 2));
	int w, h, c;

	//------------ Backgrounds
	
	//Background
	DrawBackgroundTexture(0, (u8)bgrgba);

	//------------- Menu Bar

	c = header_line_png_index;
	int cnt, cntMax = ((ani * div) > 800) ? 800 : (ani * div);
	for (cnt = MENU_ICON_OFF; cnt < cntMax; cnt++)
	{
		w = menu_textures[c].texture.width; h = menu_textures[c].texture.height / 2;
		DrawTexture(menu_textures[c], cnt, 40, 0, w, h, 0xffffffff);
	}
	DrawTexture(menu_textures[header_dot_png_index], cnt - 4, 40, 0, menu_textures[header_dot_png_index].texture.width / 2, menu_textures[header_dot_png_index].texture.height / 2, 0xffffff00 | icon_a);

	//header mini icon
	DrawTextureCenteredX(icon, MENU_ICON_OFF - 20, 32, 0, 48, 48, 0xffffff00 | icon_a);

	//header title string
	SetFontColor(rgba | icon_a, 0);
	SetCurrentFont(font_comfortaa_regular);
	if (headerTitle)
	{
		SetFontSize(APP_FONT_SIZE_TITLE);
		DrawString(MENU_ICON_OFF + 10, 31, headerTitle);
	}

	//header sub title string
	if (headerSubTitle)
	{
		int width = 800 - (MENU_ICON_OFF + MENU_TITLE_OFF + WidthFromStr((u8*)headerTitle)) - 30;
		SetFontSize(APP_FONT_SIZE_SUBTITLE);
		char * tName = malloc(strlen(headerSubTitle) + 1);
		strcpy(tName, headerSubTitle);
		while (WidthFromStr((u8*)tName) > width)
		{
			tName[strlen(tName) - 1] = 0;
		}
		SetFontAlign(2);
		DrawString(800, 35, tName);
		free(tName);
		SetFontAlign(0);
	}
}

void DrawHeader(png_texture icon, int xOff, char * headerTitle, char * headerSubTitle, u32 rgba, u32 bgrgba, int mode)
{
	int c, w, h;

	//Background
	DrawBackgroundTexture(xOff, (u8)bgrgba);

	//------------ Menu Bar
	c = header_line_png_index;
	int cnt = 0;
	for (cnt = xOff + MENU_ICON_OFF; cnt < 800; cnt++)
	{
		w = menu_textures[c].texture.width; h = menu_textures[c].texture.height / 2;
		DrawTexture(menu_textures[c], cnt, 40, 0, w, h, 0xffffffff);
	}
	DrawTexture(menu_textures[header_dot_png_index], cnt - 4, 40, 0, menu_textures[header_dot_png_index].texture.width / 2, menu_textures[header_dot_png_index].texture.height / 2, 0xffffffff);

	//header mini icon
	if (mode)
		DrawTextureCenteredX(icon, xOff + MENU_ICON_OFF - 12, 40, 0, 32, 32, 0xffffffff);
	else
		DrawTextureCenteredX(icon, xOff + MENU_ICON_OFF - 20, 32, 0, 48, 48, 0xffffffff);

	//header title string
	SetFontColor(rgba, 0);
	SetCurrentFont(font_comfortaa_regular);
	if (mode)
	{
		SetFontSize(APP_FONT_SIZE_SUBTITLE);
		if (headerTitle)
			DrawString(xOff + MENU_ICON_OFF + 10, 35, headerTitle);
	}
	else
	{
		SetFontSize(APP_FONT_SIZE_TITLE);
		if (headerTitle)
			DrawString(xOff + MENU_ICON_OFF + 10, 31, headerTitle);
	}

	//header sub title string
	if (headerSubTitle)
	{
		int width = 800 - (MENU_ICON_OFF + MENU_TITLE_OFF + WidthFromStr((u8*)headerTitle)) - 30;
		SetFontSize(APP_FONT_SIZE_SUBTITLE);
		char * tName = malloc(strlen(headerSubTitle) + 1);
		strcpy(tName, headerSubTitle);
		while (WidthFromStr((u8*)tName) > width)
		{
			tName[strlen(tName) - 1] = 0;
		}
		SetFontAlign(2);
		DrawString(800, 35, tName);
		free(tName);
		SetFontAlign(0);
	}
}

void DrawBackgroundTexture(int x, u8 alpha)
{
	if (x == 0)
		DrawTexture(menu_textures[bgimg_png_index], x - apollo_config.marginH, -apollo_config.marginV, 0, 848 - x + (apollo_config.marginH * 2), 512 + (apollo_config.marginV * 2), 0xFFFFFF00 | alpha);
	else
		DrawTexture(menu_textures[bgimg_png_index], x, -apollo_config.marginV, 0, 848 - x + apollo_config.marginH, 512 + (apollo_config.marginV * 2), 0xFFFFFF00 | alpha);
}

void DrawTexture(png_texture tex, int x, int y, int z, int w, int h, u32 rgba)
{
    tiny3d_SetTexture(0, tex.texture_off, tex.texture.width,
        tex.texture.height, tex.texture.pitch,  
        TINY3D_TEX_FORMAT_A8R8G8B8, TEXTURE_LINEAR);
    DrawSprites2D(x, y, z, w, h, rgba);
}

void DrawTextureCentered(png_texture tex, int x, int y, int z, int w, int h, u32 rgba)
{
	x -= w / 2;
	y -= h / 2;

	tiny3d_SetTexture(0, tex.texture_off, tex.texture.width,
		tex.texture.height, tex.texture.pitch,
		TINY3D_TEX_FORMAT_A8R8G8B8, TEXTURE_LINEAR);
	DrawSprites2D(x, y, z, w, h, rgba);
}

void DrawTextureCenteredX(png_texture tex, int x, int y, int z, int w, int h, u32 rgba)
{
	x -= w / 2;

	tiny3d_SetTexture(0, tex.texture_off, tex.texture.width,
		tex.texture.height, tex.texture.pitch,
		TINY3D_TEX_FORMAT_A8R8G8B8, TEXTURE_LINEAR);
	DrawSprites2D(x, y, z, w, h, rgba);
}

void DrawTextureCenteredY(png_texture tex, int x, int y, int z, int w, int h, u32 rgba)
{
	y -= h / 2;

	tiny3d_SetTexture(0, tex.texture_off, tex.texture.width,
		tex.texture.height, tex.texture.pitch,
		TINY3D_TEX_FORMAT_A8R8G8B8, TEXTURE_LINEAR);
	DrawSprites2D(x, y, z, w, h, rgba);
}

void DrawTextureRotated(png_texture tex, int x, int y, int z, int w, int h, u32 rgba, float angle)
{
	x -= w / 2;
	y -= h / 2;

	tiny3d_SetTexture(0, tex.texture_off, tex.texture.width,
		tex.texture.height, tex.texture.pitch,
		TINY3D_TEX_FORMAT_A8R8G8B8, TEXTURE_LINEAR);
	DrawSpritesRot2D(x, y, z, w, h, rgba, angle);
}

static int please_wait;

void loading_screen_thread(void* user_data)
{
    float angle = 0;
    while (please_wait == 1)
    {
        angle += 0.1f;
    	tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);
    	tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);
    	tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
    		TINY3D_BLEND_FUNC_SRC_ALPHA_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_RGB_ONE_MINUS_SRC_ALPHA,
    		TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);

    	tiny3d_Project2D();

		DrawBackgroundTexture(0, 0xFF);

        //Loading animation
        DrawTextureCentered(menu_textures[circle_loading_bg_png_index], 424, 256, 0, 89, 89, 0xFFFFFFFF);
        DrawTextureRotated(menu_textures[circle_loading_seek_png_index], 424, 256, 0, 89, 89, 0xFFFFFFFF, angle);

    	tiny3d_Flip();
	}

    please_wait = -1;
    sysThreadExit (0);
}

int init_loading_screen()
{
    sys_ppu_thread_t tid;
    please_wait = 1;
    
    int ret = sysThreadCreate (&tid, loading_screen_thread, NULL, 1000, 16*1024, THREAD_JOINABLE, "please_wait");

    return ret;
}

void stop_loading_screen()
{
    if (please_wait != 1)
        return;

    please_wait = 0;

    while (please_wait != -1)
        usleep(1000);
}
