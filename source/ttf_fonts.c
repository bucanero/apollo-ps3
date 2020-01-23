#include <ft2build.h>
#include <freetype/freetype.h> 
#include <freetype/ftglyph.h>

/******************************************************************************************************************************************************/
/* TTF functions to load and convert fonts         
 * From fonts_from_ttf Tiny3D sample                                                                                                    */
 /*****************************************************************************************************************************************************/

int ttf_inited = 0;

FT_Library freetype;
FT_Face face;

/* TTFLoadFont can load TTF fonts from device or from memory:
path = path to the font or NULL to work from memory
from_memory = pointer to the font in memory. It is ignored if path != NULL.
size_from_memory = size of the memory font. It is ignored if path != NULL.
*/
int TTFLoadFont(char * path, void * from_memory, int size_from_memory)
{
   
    if(!ttf_inited)
        FT_Init_FreeType(&freetype);
    ttf_inited = 1;

    if(path) {
        if(FT_New_Face(freetype, path, 0, &face)) return -1;
    } else {
        if(FT_New_Memory_Face(freetype, from_memory, size_from_memory, 0, &face)) return -1;
        }

    return 0;
}

/* release all */
void TTFUnloadFont()
{
   FT_Done_FreeType(freetype);
   ttf_inited = 0;
}

/* function to render the character
chr : character from 0 to 255
bitmap: u8 bitmap passed to render the character character (max 256 x 256 x 1 (8 bits Alpha))
*w : w is the bitmap width as input and the width of the character (used to increase X) as output
*h : h is the bitmap height as input and the height of the character (used to Y correction combined with y_correction) as output
y_correction : the Y correction to display the character correctly in the screen
*/

/*
So basically libfont sucks (Not the TTF library).
If a character is greater than the set width (I'm talking to you W) then it just cuts it off... Which looks ugly.
So I've gone ahead and done a hackish patch to make it shrink it down so that it looks like a regular character.
- Dnawrkshp
*/
int doShrinkChar = 0;
void TTF_to_Bitmap(uint8_t chr, uint8_t * bitmap, short *w, short *h, short *y_correction)
{
    int width = *w;
    
    TTF_to_Bitmap_loop: ;
    FT_Set_Pixel_Sizes(face, (width), (*h));
    
    FT_GlyphSlot slot = face->glyph;

    if(FT_Load_Char(face, (char) chr, FT_LOAD_RENDER )) {(*w) = 0; return;}
    
    if (slot->bitmap.width > *w && width == *w)
    {
        width = (int)((float)*w * ((float)*w / (float)slot->bitmap.width)) - 1;
        goto TTF_to_Bitmap_loop;
    }
    
    memset(bitmap, 0, (*w) * (*h));
    
    int n, m, ww, mm;

    *y_correction = (*h) - 1 - slot->bitmap_top;
    
    ww = 0;
    
    if (doShrinkChar)
    {
        float mRatio = 0;
        for(n = 0; n < slot->bitmap.rows; n++) {
            for (m = 0; m < slot->bitmap.width; m++) {
                
                mRatio = (float)(slot->bitmap.width+1) / (float)(*w);
                if (mRatio > 1)
                {
                    mm = (int)((float)m * mRatio);
                    
                    bitmap[m] = (uint8_t) slot->bitmap.buffer[ww + mm];
                }
                else
                {
                    mm = m;
                    //if(m >= (*w) || n >= (*h)) continue;
                    bitmap[m] = (uint8_t) slot->bitmap.buffer[ww + mm];
                }
            }
        
            bitmap += *w;
            ww += slot->bitmap.width;
        }
        
        int width = ((slot->advance.x + 31) >> 6) + ((slot->bitmap_left < 0) ? -slot->bitmap_left : 0) - 1;
        *h = slot->bitmap.rows;
        
        if (width < *w)
            *w = width;
    }
    else
    {
        for(n = 0; n < slot->bitmap.rows; n++) {
            for (m = 0; m < slot->bitmap.width; m++) {
                    
                    if(m >= (*w) || n >= (*h)) continue;
                    
                    bitmap[m] = (uint8_t) slot->bitmap.buffer[ww + m];
                }
        
            bitmap += *w;
            ww += slot->bitmap.width;
        }
        
        *w = ((slot->advance.x + 31) >> 6) + ((slot->bitmap_left < 0) ? -slot->bitmap_left : 0) - 1;
        *h = slot->bitmap.rows;
    }
}
