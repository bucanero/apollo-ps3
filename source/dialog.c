#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <sys/memory.h>
#include <sysutil/msg.h>
#include <sysutil/osk.h>
#include <sysutil/sysutil.h>

#include "menu.h"

#define SYSUTIL_OSK_INPUT_ENTERED        0x505

#define OSK_IME_DIALOG_MAX_TITLE_LENGTH  (128)
#define OSK_IME_DIALOG_MAX_TEXT_LENGTH   (512)

#define ARRAY_COUNTOF(arr) (sizeof(arr)/sizeof(0[arr]))

static uint64_t progbar_tmp;
static float bar1_countparts;
volatile int msg_dialog_action = 0;

static int g_ime_active;
static int osk_action = 0;
static int osk_level = 0;

static sys_mem_container_t container_mem;
static oskCallbackReturnParam OutputReturnedParam;

volatile int osk_event = 0;
volatile int osk_unloaded = 0;

static uint16_t g_ime_title[OSK_IME_DIALOG_MAX_TITLE_LENGTH];
static uint16_t g_ime_text[OSK_IME_DIALOG_MAX_TEXT_LENGTH];
static uint16_t g_ime_input[OSK_IME_DIALOG_MAX_TEXT_LENGTH + 1];

void msg_dialog_event(msgButton button, void *userdata)
{
    switch(button) {

        case MSG_DIALOG_BTN_YES:
            msg_dialog_action = 1;
            break;
        case MSG_DIALOG_BTN_NO:
        case MSG_DIALOG_BTN_ESCAPE:
        case MSG_DIALOG_BTN_NONE:
            msg_dialog_action = 2;
            break;
        default:
		    break;
    }
}

int show_dialog(int tdialog, const char * format, ...)
{
    msg_dialog_action = 0;
    char str[0x800];
    va_list	opt;

    va_start(opt, format);
    vsprintf((void*) str, format, opt);
    va_end(opt);

    msgType mtype = MSG_DIALOG_BKG_INVISIBLE | MSG_DIALOG_NORMAL;
    mtype |=  (tdialog ? (MSG_DIALOG_BTN_TYPE_YESNO  | MSG_DIALOG_DEFAULT_CURSOR_NO) : MSG_DIALOG_BTN_TYPE_OK);

    msgDialogOpen2(mtype, str, msg_dialog_event, NULL, NULL);

    while(!msg_dialog_action)
    {
        drawDialogBackground();
    }
    msgDialogAbort();
    usleep(100 *1000);

    return (msg_dialog_action == 1);
}

void init_progress_bar(const char* progress_bar_title, const char* msg)
{
    progbar_tmp = 0;
    bar1_countparts = 0.0f;
    msg_dialog_action = 0;

    msgDialogOpen2(MSG_DIALOG_BKG_INVISIBLE | MSG_DIALOG_SINGLE_PROGRESSBAR | MSG_DIALOG_MUTE_ON | MSG_DIALOG_DISABLE_CANCEL_ON, progress_bar_title, msg_dialog_event, NULL, NULL);
    msgDialogProgressBarSetMsg(MSG_PROGRESSBAR_INDEX0, msg);
    msgDialogProgressBarReset(MSG_PROGRESSBAR_INDEX0);

    drawDialogBackground();
}

void end_progress_bar(void)
{
    msgDialogAbort();
}

void update_progress_bar(uint64_t progress, const uint64_t total_size, const char* msg)
{
    if((progress - progbar_tmp) > 0) {
        bar1_countparts += (100.0f * ((double) (progress - progbar_tmp))) / ((double) total_size);
        progbar_tmp += (progress - progbar_tmp);
    }

    if(bar1_countparts >= 1.0f) {
        msgDialogProgressBarSetMsg(MSG_PROGRESSBAR_INDEX0, msg);
        msgDialogProgressBarInc(MSG_PROGRESSBAR_INDEX0, (u32) bar1_countparts);
        bar1_countparts -= (float) ((u32) bar1_countparts);
    }

    drawDialogBackground();
}

static int convert_to_utf16(const char* utf8, uint16_t* utf16, uint32_t available)
{
    int count = 0;
    while (*utf8)
    {
        uint8_t ch = (uint8_t)*utf8++;
        uint32_t code;
        uint32_t extra;

        if (ch < 0x80)
        {
            code = ch;
            extra = 0;
        }
        else if ((ch & 0xe0) == 0xc0)
        {
            code = ch & 31;
            extra = 1;
        }
        else if ((ch & 0xf0) == 0xe0)
        {
            code = ch & 15;
            extra = 2;
        }
        else
        {
            // TODO: this assumes there won't be invalid utf8 codepoints
            code = ch & 7;
            extra = 3;
        }

        for (uint32_t i=0; i<extra; i++)
        {
            uint8_t next = (uint8_t)*utf8++;
            if (next == 0 || (next & 0xc0) != 0x80)
            {
                return count;
            }
            code = (code << 6) | (next & 0x3f);
        }

        if (code < 0xd800 || code >= 0xe000)
        {
            if (available < 1) return count;
            utf16[count++] = (uint16_t)code;
            available--;
        }
        else // surrogate pair
        {
            if (available < 2) return count;
            code -= 0x10000;
            utf16[count++] = 0xd800 | (code >> 10);
            utf16[count++] = 0xdc00 | (code & 0x3ff);
            available -= 2;
        }
    }
    utf16[count]=0;
    return count;
}

static int convert_from_utf16(const uint16_t* utf16, char* utf8, uint32_t size)
{
    int count = 0;
    while (*utf16)
    {
        uint32_t code;
        uint16_t ch = *utf16++;
        if (ch < 0xd800 || ch >= 0xe000)
        {
            code = ch;
        }
        else // surrogate pair
        {
            uint16_t ch2 = *utf16++;
            if (ch < 0xdc00 || ch > 0xe000 || ch2 < 0xd800 || ch2 > 0xdc00)
            {
                return count;
            }
            code = 0x10000 + ((ch & 0x03FF) << 10) + (ch2 & 0x03FF);
        }

        if (code < 0x80)
        {
            if (size < 1) return count;
            utf8[count++] = (char)code;
            size--;
        }
        else if (code < 0x800)
        {
            if (size < 2) return count;
            utf8[count++] = (char)(0xc0 | (code >> 6));
            utf8[count++] = (char)(0x80 | (code & 0x3f));
            size -= 2;
        }
        else if (code < 0x10000)
        {
            if (size < 3) return count;
            utf8[count++] = (char)(0xe0 | (code >> 12));
            utf8[count++] = (char)(0x80 | ((code >> 6) & 0x3f));
            utf8[count++] = (char)(0x80 | (code & 0x3f));
            size -= 3;
        }
        else
        {
            if (size < 4) return count;
            utf8[count++] = (char)(0xf0 | (code >> 18));
            utf8[count++] = (char)(0x80 | ((code >> 12) & 0x3f));
            utf8[count++] = (char)(0x80 | ((code >> 6) & 0x3f));
            utf8[count++] = (char)(0x80 | (code & 0x3f));
            size -= 4;
        }
    }
    utf8[count]=0;
    return count;
}

static void osk_exit(void)
{
    if(osk_level == 2) {
        oskAbort();
        oskUnloadAsync(&OutputReturnedParam);
        
        osk_event = 0;
        osk_action=-1;
    }

    if(osk_level >= 1) {
        sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT1);
        sysMemContainerDestroy(container_mem);
    }
}

static void osk_event_handle(u64 status, u64 param, void * userdata)
{
    switch((u32) status) 
    {
	    case SYSUTIL_OSK_INPUT_CANCELED:
		    osk_event = SYSUTIL_OSK_INPUT_CANCELED;
		    break;

        case SYSUTIL_OSK_UNLOADED:
		    osk_unloaded = 1;
		    break;

        case SYSUTIL_OSK_INPUT_ENTERED:
	    	osk_event = SYSUTIL_OSK_INPUT_ENTERED;
		    break;

	    case SYSUTIL_OSK_DONE:
	    	osk_event = SYSUTIL_OSK_DONE;
		    break;

        default:
            break;
    }
}

static int osk_dialog_input_init(const char* title, const char* text, int maxlen)
{
    oskParam DialogOskParam;
    oskInputFieldInfo inputFieldInfo;
    oskPoint pos = {0.0, 0.0};

    osk_level = 0;
    g_ime_active = 0;
    
	if(sysMemContainerCreate(&container_mem, 8*1024*1024) < 0) {
        osk_exit();
        return 0;
    }

    osk_level = 1;

    convert_to_utf16(title, g_ime_title, ARRAY_COUNTOF(g_ime_title) - 1);
    convert_to_utf16(text, g_ime_text, ARRAY_COUNTOF(g_ime_text) - 1);
    
    inputFieldInfo.message =  g_ime_title;
    inputFieldInfo.startText = g_ime_text;
    inputFieldInfo.maxLength = maxlen;
       
    OutputReturnedParam.res = OSK_NO_TEXT;
    OutputReturnedParam.len = OSK_IME_DIALOG_MAX_TEXT_LENGTH;
    OutputReturnedParam.str = g_ime_input;

    DialogOskParam.controlPoint = pos;
    DialogOskParam.firstViewPanel = OSK_PANEL_TYPE_ALPHABET_FULL_WIDTH;
    DialogOskParam.allowedPanels = (OSK_PANEL_TYPE_ALPHABET | OSK_PANEL_TYPE_NUMERAL | OSK_PANEL_TYPE_ENGLISH | OSK_PANEL_TYPE_URL);
    DialogOskParam.prohibitFlags = OSK_PROHIBIT_RETURN;

    memset(g_ime_input, 0, sizeof(g_ime_input));

    if ((oskSetKeyLayoutOption (OSK_10KEY_PANEL | OSK_FULLKEY_PANEL) < 0) ||
        (oskAddSupportLanguage (DialogOskParam.allowedPanels) < 0) ||
        (oskSetLayoutMode(OSK_LAYOUTMODE_HORIZONTAL_ALIGN_CENTER | OSK_LAYOUTMODE_VERTICAL_ALIGN_CENTER) < 0) ||
        (oskSetInitialInputDevice(OSK_DEVICE_PAD) < 0))
    {
        osk_exit();
        return 0;
    }

    sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT1);
    sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT1, osk_event_handle, NULL);

    osk_action = 0;
    osk_unloaded = 0;

    if(oskLoadAsync(container_mem, &DialogOskParam, &inputFieldInfo) < 0) {
        osk_exit();
        return 0;
    }

    osk_level = 2;
    g_ime_active = 1;
    return 1;
}

static int osk_dialog_input_update(void)
{
    if (!osk_unloaded)
    {
        switch(osk_event)
        {
            case SYSUTIL_OSK_INPUT_ENTERED:
                oskGetInputText(&OutputReturnedParam);
                osk_event = 0;
                break;

            case SYSUTIL_OSK_INPUT_CANCELED:
                oskAbort();
                oskUnloadAsync(&OutputReturnedParam);
                osk_event = 0;
                osk_action = -1;
                break;

            case SYSUTIL_OSK_DONE:
                if (osk_action != -1) osk_action = 1;
                oskUnloadAsync(&OutputReturnedParam);
                osk_event = 0;
                break;

            default:
                break;
        }
    }
    else
    {
        g_ime_active = 0;

        if ((OutputReturnedParam.res == OSK_OK) && (osk_action == 1))
        {
            osk_exit();
            return 1;
        } 
         
        osk_exit();
        return (-1);
    }

    return 0;
}

int osk_dialog_get_text(const char* title, char* text, uint32_t size)
{
    if (size > OSK_IME_DIALOG_MAX_TEXT_LENGTH) size = OSK_IME_DIALOG_MAX_TEXT_LENGTH;

    if (!osk_dialog_input_init(title, text, size))
        return 0;

    while (g_ime_active)
    {
        if (osk_dialog_input_update() < 0)
            return 0;

        drawDialogBackground();
    }
    convert_from_utf16(g_ime_input, text, size - 1);

    return 1;
}
