/* 
   TINY3D - font library / (c) 2010 Hermes  <www.elotrolado.net>

*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pngdec/pngdec.h>

#include "libfont.h"
#include "menu.h"

struct t_font_description
{
    int w, h, bh;
    
    u8 first_char;
    u8 last_char;
    
    u32 rsx_text_offset;
    u32 rsx_bytes_per_char; 
    u32 color_format;

    short fw[256]; // chr width
    short fy[256]; // chr y correction
};

static struct t_font_datas
{

    int number_of_fonts;

    int current_font;

    struct t_font_description fonts[8];

    int sx, sy;
	int mono;
	int extra;

    u32 color, bkcolor;

    int align; //0 = left, 1 = center, 2 = right
    int autonewline;

    float X,Y,Z;

} font_datas;

typedef struct t_special_char
{
	char value;

	short fw;
	short fy;
	float sx;
	float sy;

	png_texture image;
} special_char;

static special_char special_chars[MAX_SPECIAL_CHARS];
static int special_char_index = 0;


special_char* GetSpecialCharFromValue(const char value)
{
	int x;
	special_char* ret = NULL;

	for (x = 0; x < special_char_index; x++)
	{
		if (special_chars[x].value == value)
			ret = &(special_chars[x]);
	}
	return ret;
}

void ResetFont()
{
    font_datas.current_font = font_datas.number_of_fonts =0;

    font_datas.color = 0xffffffff;
    font_datas.bkcolor = 0;
    font_datas.align = 0;
    font_datas.X = font_datas.Y = font_datas.Z = 0.0f;
    font_datas.autonewline = 0;

    font_datas.sx = font_datas.sy = 8;
	font_datas.mono = 0;
}

u8 * AddFontFromBitmapArray(u8 *font, u8 *texture, u8 first_char, u8 last_char, int w, int h, int bits_per_pixel, int byte_order)
{
    int n, a, b;
    u8 i;
    
    if(font_datas.number_of_fonts >= 8) return texture;

    font_datas.fonts[font_datas.number_of_fonts].w = w;
    font_datas.fonts[font_datas.number_of_fonts].h = h;
    font_datas.fonts[font_datas.number_of_fonts].bh = h;
    font_datas.fonts[font_datas.number_of_fonts].color_format = TINY3D_TEX_FORMAT_A4R4G4B4; //TINY3D_TEX_FORMAT_A8R8G8B8;
    font_datas.fonts[font_datas.number_of_fonts].first_char = first_char;
    font_datas.fonts[font_datas.number_of_fonts].last_char  = last_char;
    font_datas.align =0;

    font_datas.color = 0xffffffff;
    font_datas.bkcolor = 0x0;

    font_datas.sx = w;
    font_datas.sy = h;

    font_datas.Z = 0.0f;

    for(n = 0; n < 256; n++) {
        font_datas.fonts[font_datas.number_of_fonts].fw[n] = 0; 
        font_datas.fonts[font_datas.number_of_fonts].fy[n] = 0;
    }

       
    for(n = first_char; n <= last_char; n++) {

        font_datas.fonts[font_datas.number_of_fonts].fw[n] = w;

        texture = (u8 *) ((((long) texture) + 15) & ~15);

        if(n == first_char) font_datas.fonts[font_datas.number_of_fonts].rsx_text_offset = tiny3d_TextureOffset(texture);

        if(n == first_char+1) font_datas.fonts[font_datas.number_of_fonts].rsx_bytes_per_char = tiny3d_TextureOffset(texture)
            - font_datas.fonts[font_datas.number_of_fonts].rsx_text_offset;

        for(a = 0; a < h; a++) {
            for(b = 0; b < w; b++) {
                
                i = font[(b * bits_per_pixel)/8];

                if(byte_order) 
                    i= (i << ((b & (7/bits_per_pixel)) * bits_per_pixel))>> (8-bits_per_pixel);
                else
                    i >>= (b & (7/bits_per_pixel)) * bits_per_pixel;
                
                i = (i & ((1 << bits_per_pixel)-1)) * 255 / ((1 << bits_per_pixel)-1);

                if(i) {//TINY3D_TEX_FORMAT_A1R5G5B5
                    //i>>=3;
                    //*((u16 *) texture) = (1<<15) | (i<<10) | (i<<5) | (i);
                    //TINY3D_TEX_FORMAT_A4R4G4B4
                    i>>=4;
                    *((u16 *) texture) = (i<<12) | 0xfff;

                } else {
              
                    texture[0] = texture[1] = 0x0; //texture[2] = 0x0;
                    //texture[3] = 0x0; // alpha
                } 
                texture+=2;
               
            }

            font += (w * bits_per_pixel) / 8;
                
        }
    
    }

    texture = (u8 *) ((((long) texture) + 15) & ~15);

    font_datas.number_of_fonts++;

    return texture;
}

u8 * AddFontFromTTF(u8 *texture, u8 first_char, u8 last_char, int w, int h, 
    void (* ttf_callback) (u8 chr, u8 * bitmap, short *w, short *h, short *y_correction))
{
    int n, a, b;
    u8 i;
    u8 *font;
    static u8 letter_bitmap[257 * 256];
    
    int bits_per_pixel = 8;
    
    if(font_datas.number_of_fonts >= 8) return texture;

    if(h < 8) h = 8;
    if(w < 8) w = 8;
    if(h > 256) h = 256;
    if(w > 256) w = 256;

    font_datas.fonts[font_datas.number_of_fonts].w = w;
    font_datas.fonts[font_datas.number_of_fonts].h = h;
    font_datas.fonts[font_datas.number_of_fonts].bh = h+4;
    font_datas.fonts[font_datas.number_of_fonts].color_format = TINY3D_TEX_FORMAT_A4R4G4B4;
    font_datas.fonts[font_datas.number_of_fonts].first_char = first_char;
    font_datas.fonts[font_datas.number_of_fonts].last_char  = last_char;
    font_datas.align =0;

    font_datas.color = 0xffffffff;
    font_datas.bkcolor = 0x0;

    font_datas.sx = w;
    font_datas.sy = h;

    font_datas.Z = 0.0f;

    for(n = 0; n < 256; n++) {
        font_datas.fonts[font_datas.number_of_fonts].fw[n] = 0; 
        font_datas.fonts[font_datas.number_of_fonts].fy[n] = 0;
    }

       
    for(n = first_char; n <= last_char; n++) {
        
        short hh = h;

        font = letter_bitmap;

        font_datas.fonts[font_datas.number_of_fonts].fw[n] = (short) w;
		
		ttf_callback((u8) (n & 255), letter_bitmap, &font_datas.fonts[font_datas.number_of_fonts].fw[n], &hh,  &font_datas.fonts[font_datas.number_of_fonts].fy[n]);

        // letter background correction
        if((hh + font_datas.fonts[font_datas.number_of_fonts].fy[n]) > font_datas.fonts[font_datas.number_of_fonts].bh) 
            font_datas.fonts[font_datas.number_of_fonts].bh = hh + font_datas.fonts[font_datas.number_of_fonts].fy[n];

        texture = (u8 *) ((((long) texture) + 15) & ~15);

        if(n == first_char) font_datas.fonts[font_datas.number_of_fonts].rsx_text_offset = tiny3d_TextureOffset(texture);

        if(n == first_char+1) font_datas.fonts[font_datas.number_of_fonts].rsx_bytes_per_char = tiny3d_TextureOffset(texture)
            - font_datas.fonts[font_datas.number_of_fonts].rsx_text_offset;

        for(a = 0; a < h; a++) {
            for(b = 0; b < w; b++) {
                
                i = font[(b * bits_per_pixel)/8];

                i >>= (b & (7/bits_per_pixel)) * bits_per_pixel;
                
                i = (i & ((1 << bits_per_pixel)-1)) * 255 / ((1 << bits_per_pixel)-1);

                if(i) {//TINY3D_TEX_FORMAT_A4R4G4B4
                    i>>=4;
                    *((u16 *) texture) = (i<<12) | 0xfff;
                } else {
              
                    texture[0] = texture[1] = 0x0; //texture[2] = 0x0;
                    //texture[3] = 0x0; // alpha
                } 
                texture+=2;
               
            }

            font += (w * bits_per_pixel) / 8;
                
        }
    
    }

    texture = (u8 *) ((((long) texture) + 15) & ~15);

    font_datas.number_of_fonts++;

    return texture;
}

void SetCurrentFont(int nfont)
{
    if(nfont < 0 || nfont >= font_datas.number_of_fonts) nfont = 0;

    font_datas.current_font = nfont;
}

void SetFontSize(int sx, int sy)
{
    if(sx < 8) sx = 8;
    if(sy < 8) sy = 8;

    font_datas.sx = sx;
    font_datas.sy = sy;
}

void SetFontColor(u32 color, u32 bkcolor)
{
    font_datas.color   = color;
    font_datas.bkcolor = bkcolor;
}

void SetFontAlign(int mode)
{
    font_datas.align  = mode;
    font_datas.autonewline = 0;
}

void SetFontAutoNewLine(int width)
{
    font_datas.autonewline = width;
    font_datas.align  = 0;
}

void SetFontZ(float z)
{
    font_datas.Z  = z;
}

float GetFontX()
{
    return font_datas.X;
}

float GetFontY()
{
    return font_datas.Y;
}

void SetMonoSpace(int space)
{
	font_datas.mono = space;
}

void SetExtraSpace(int space)
{
	font_datas.extra = space;
}

void RegisterSpecialCharacter(char value, short fw, short fy, float sx, float sy, png_texture image)
{
	special_char chr;
	chr.value = value;
	chr.fw = fw;
	chr.fy = fy;
	chr.sx = sx;
	chr.sy = sy;
	chr.image = image;

	// Verify special character
	if (chr.value == 0)
		return;
	if (chr.image.texture_off == 0)
		return;
	if (chr.image.size == 0)
		return;
	if (chr.image.texture.width == 0 || chr.image.texture.height == 0)
		return;
	
	// Verify value is not in use
	if (GetSpecialCharFromValue(chr.value))
		return;

	// Verify room in array
	if ((special_char_index + 1) < MAX_SPECIAL_CHARS)
	{
		special_chars[special_char_index] = chr;
		special_char_index++;
	}

}

int WidthFromStr(u8 * str)
{
    int w = 0;

    while(*str) {
		special_char* schr = GetSpecialCharFromValue(*str);
		if (schr)
			w += ((font_datas.sx * schr->sx) + font_datas.extra) * schr->fw / (float)font_datas.fonts[font_datas.current_font].w;
		else
			w += (font_datas.sx + font_datas.extra) * font_datas.fonts[font_datas.current_font].fw[*str] / (float)font_datas.fonts[font_datas.current_font].w;
		str++;
    }

    return w;
}

int WidthFromStrMono(u8 * str)
{
    int w = 0;

    while(*str) {
		w += (font_datas.sx * font_datas.mono) / font_datas.fonts[font_datas.current_font].w;
		str++;
    }

    return w;
}

void DrawCharSpecial(float x, float y, float z, const special_char* schr)
{
	float h = (float)font_datas.fonts[font_datas.current_font].h;
	float dx = font_datas.sx * schr->sx, dy = font_datas.sy * schr->sy;

	y += (float)((schr->fy * font_datas.sy) / h) / schr->sy;
	
	// Load sprite texture
	tiny3d_SetTexture(0, schr->image.texture_off, schr->image.texture.width,
		schr->image.texture.height, schr->image.texture.pitch,
		TINY3D_TEX_FORMAT_A8R8G8B8, 1);

	tiny3d_SetPolygon(TINY3D_QUADS);

	tiny3d_VertexPos(x, y, z);
	tiny3d_VertexColor(font_datas.color);
	tiny3d_VertexTexture(0.0f, 0.0f);

	tiny3d_VertexPos(x + dx, y, z);
	tiny3d_VertexTexture(0.999f, 0.0f);

	tiny3d_VertexPos(x + dx, y + dy + 1, z);
	tiny3d_VertexTexture(0.999f, 0.999f);

	tiny3d_VertexPos(x, y + dy + 1, z);
	tiny3d_VertexTexture(0.0f, 0.999f);

	tiny3d_End();
}

void DrawCharMono(float x, float y, float z, u8 chr)
{
	special_char* schr = GetSpecialCharFromValue(chr);
	if (schr)
	{
		DrawCharSpecial(x, y, z, schr);
		return;
	}

	float dx = font_datas.sx, dy = font_datas.sy;
	float dx2 = (dx * font_datas.mono) / font_datas.fonts[font_datas.current_font].w;
	float dy2 = (float)(dy * font_datas.fonts[font_datas.current_font].bh) / (float)font_datas.fonts[font_datas.current_font].h;

	if (font_datas.number_of_fonts <= 0) return;

	if (chr < font_datas.fonts[font_datas.current_font].first_char) return;

	if (font_datas.bkcolor) {
		tiny3d_SetPolygon(TINY3D_QUADS);

		tiny3d_VertexPos(x, y, z);
		tiny3d_VertexColor(font_datas.bkcolor);

		tiny3d_VertexPos(x + dx2, y, z);

		tiny3d_VertexPos(x + dx2, y + dy2, z);

		tiny3d_VertexPos(x, y + dy2, z);

		tiny3d_End();
	}

	y += (float)(font_datas.fonts[font_datas.current_font].fy[chr] * font_datas.sy) / (float)(font_datas.fonts[font_datas.current_font].h);

	if (chr > font_datas.fonts[font_datas.current_font].last_char) return;

	// Load sprite texture
	tiny3d_SetTexture(0, font_datas.fonts[font_datas.current_font].rsx_text_offset + font_datas.fonts[font_datas.current_font].rsx_bytes_per_char
		* (chr - font_datas.fonts[font_datas.current_font].first_char), font_datas.fonts[font_datas.current_font].w,
		font_datas.fonts[font_datas.current_font].h, font_datas.fonts[font_datas.current_font].w *
		((font_datas.fonts[font_datas.current_font].color_format == TINY3D_TEX_FORMAT_A8R8G8B8) ? 4 : 2),
		font_datas.fonts[font_datas.current_font].color_format, 1);

	tiny3d_SetPolygon(TINY3D_QUADS);

	tiny3d_VertexPos(x, y, z);
	tiny3d_VertexColor(font_datas.color);
	tiny3d_VertexTexture(0.0f, 0.0f);

	tiny3d_VertexPos(x + dx, y, z);
	tiny3d_VertexTexture(0.95f, 0.0f);

	tiny3d_VertexPos(x + dx, y + dy, z);
	tiny3d_VertexTexture(0.95f, 0.95f);

	tiny3d_VertexPos(x, y + dy, z);
	tiny3d_VertexTexture(0.0f, 0.95f);

	tiny3d_End();
}

void DrawChar(float x, float y, float z, u8 chr)
{
	special_char* schr = GetSpecialCharFromValue(chr);
	if (schr)
	{
		DrawCharSpecial(x, y, z, schr);
		return;
	}

	float dx = font_datas.sx, dy = font_datas.sy;
	float dx2 = (dx * font_datas.fonts[font_datas.current_font].fw[chr]) / font_datas.fonts[font_datas.current_font].w;
	float dy2 = (float)(dy * font_datas.fonts[font_datas.current_font].bh) / (float)font_datas.fonts[font_datas.current_font].h;

	if (font_datas.number_of_fonts <= 0) return;

	if (chr < font_datas.fonts[font_datas.current_font].first_char) return;

	if (font_datas.bkcolor) {
		tiny3d_SetPolygon(TINY3D_QUADS);

		tiny3d_VertexPos(x, y, z);
		tiny3d_VertexColor(font_datas.bkcolor);

		tiny3d_VertexPos(x + dx2, y, z);

		tiny3d_VertexPos(x + dx2, y + dy2, z);

		tiny3d_VertexPos(x, y + dy2, z);

		tiny3d_End();
	}

	y += (float)(font_datas.fonts[font_datas.current_font].fy[chr] * font_datas.sy) / (float)(font_datas.fonts[font_datas.current_font].h);

	if (chr > font_datas.fonts[font_datas.current_font].last_char) return;

	// Load sprite texture
	tiny3d_SetTexture(0, font_datas.fonts[font_datas.current_font].rsx_text_offset + font_datas.fonts[font_datas.current_font].rsx_bytes_per_char
		* (chr - font_datas.fonts[font_datas.current_font].first_char), font_datas.fonts[font_datas.current_font].w,
		font_datas.fonts[font_datas.current_font].h, font_datas.fonts[font_datas.current_font].w *
		((font_datas.fonts[font_datas.current_font].color_format == TINY3D_TEX_FORMAT_A8R8G8B8) ? 4 : 2),
		font_datas.fonts[font_datas.current_font].color_format, 1);

	tiny3d_SetPolygon(TINY3D_QUADS);

	tiny3d_VertexPos(x, y, z);
	tiny3d_VertexColor(font_datas.color);
	tiny3d_VertexTexture(0.0f, 0.0f);

	tiny3d_VertexPos(x + dx, y, z);
	tiny3d_VertexTexture(0.95f, 0.0f);

	tiny3d_VertexPos(x + dx, y + dy, z);
	tiny3d_VertexTexture(0.95f, 0.95f);

	tiny3d_VertexPos(x, y + dy, z);
	tiny3d_VertexTexture(0.0f, 0.95f);

	tiny3d_End();
}

static int i_must_break_line(char *str, float x)
{
    int xx =0;
	int dx = (font_datas.sx+font_datas.extra);
	
    while(*str) {
        if(((u8)*str) <= 32) break;
        xx += dx * font_datas.fonts[font_datas.current_font].fw[((u8)*str)] / font_datas.fonts[font_datas.current_font].w;
        str++;
    }

    
    if(*str && (x+xx) >= font_datas.autonewline) return 1;

    return 0;
}

float DrawStringMono(float x, float y, char *str)
{
	if (!font_datas.mono)
		return DrawString(x, y, str);
	
	float initX = x;
	int dx = font_datas.sx;
	
    if(font_datas.align == 1) {
    
        x= (848 - WidthFromStrMono((u8 *) str)) / 2;

    }
	else if (font_datas.align == 2) {
		x -= WidthFromStrMono((u8 *) str);
	}
	else if (font_datas.align == 3) {
		x -= WidthFromStrMono((u8 *) str)/2;
	}

    while (*str) {
        
        if(*str == '\n') {
            x = initX; 
            y += font_datas.sy * font_datas.fonts[font_datas.current_font].bh / font_datas.fonts[font_datas.current_font].h;
            str++;
            continue;
        } else {
            if(font_datas.autonewline && i_must_break_line(str, x)) {
                x = initX; 
                y += font_datas.sy * font_datas.fonts[font_datas.current_font].bh / font_datas.fonts[font_datas.current_font].h;
            }
        }

        DrawChar(x, y, font_datas.Z, (u8) *str);
		x += (dx * font_datas.mono) / font_datas.fonts[font_datas.current_font].w;
        str++; 
    }

    font_datas.X = x; font_datas.Y = y;

    return x;
}

float DrawString(float x, float y, char *str)
{
	int dx = (font_datas.sx+font_datas.extra);
	float initX = x;
	
    if(font_datas.align == 1) {
    
        x= (848 - WidthFromStr((u8 *) str)) / 2;

    }
	else if (font_datas.align == 2) {
		x -= WidthFromStr((u8 *) str);
	}
	else if (font_datas.align == 3) {
		x -= WidthFromStr((u8 *) str)/2;
	}

    while (*str) {
        
		special_char* schr = GetSpecialCharFromValue(*str);

        if(*str == '\n') {
            x = initX; 
            y += font_datas.sy * font_datas.fonts[font_datas.current_font].bh / font_datas.fonts[font_datas.current_font].h;
            str++;
            continue;
        } else {
            if(font_datas.autonewline && i_must_break_line(str, x)) {
                x = initX; 
                y += font_datas.sy * font_datas.fonts[font_datas.current_font].bh / font_datas.fonts[font_datas.current_font].h;
            }
        }

        DrawChar(x, y, font_datas.Z, (u8) *str);
		
		if (schr)
			x += (font_datas.sx * schr->sx) * schr->fw / (float)font_datas.fonts[font_datas.current_font].w;
		else
		{
			//Make font look nicer by fixing bad spacing
			float ddX = dx * font_datas.fonts[font_datas.current_font].fw[((u8)*str)] / font_datas.fonts[font_datas.current_font].w;
			if (str[1] == 'j')
				ddX *= 2.0 / 3.0;
			if (str[0] == 'm' || str[0] == 'M')
				ddX *= 0.9;
			if (str[0] == '.')
				ddX *= 3.0 / 2.0;
			x += ddX;
		}
        str++; 
    }

    font_datas.X = x; font_datas.Y = y;

    return x;
}

static char buff[4096];

float DrawFormatString(float x, float y, char *format, ...)
{
	int dx = font_datas.sx;
	float initX = x;
    char *str = (char *) buff;
    va_list	opt;
	
	va_start(opt, format);
	vsprintf( (void *) buff, format, opt);
	va_end(opt);

    if(font_datas.align == 1) {
    
        x = (848 - WidthFromStr((u8 *) str)) / 2;

    }
	else if (font_datas.align == 2) {
		x -= WidthFromStr((u8 *) str);
	}
	else if (font_datas.align == 3) {
		x -= WidthFromStr((u8 *) str)/2;
	}

    while (*str) {
        
        if(*str == '\n') {
            x = initX; 
            y += font_datas.sy * font_datas.fonts[font_datas.current_font].bh / font_datas.fonts[font_datas.current_font].h; 
            str++;
            continue;
        } else {
            if(font_datas.autonewline && i_must_break_line(str, x)) {
                x = initX; 
                y += font_datas.sy * font_datas.fonts[font_datas.current_font].bh / font_datas.fonts[font_datas.current_font].h;
            }
        }

        DrawChar(x, y, font_datas.Z, (u8) *str);
       
        x += dx * font_datas.fonts[font_datas.current_font].fw[((u8)*str)] / font_datas.fonts[font_datas.current_font].w;
        str++;
    }

    font_datas.X = x; font_datas.Y = y;

    return x;
}
