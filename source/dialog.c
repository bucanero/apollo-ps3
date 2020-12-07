#include <unistd.h>
#include <sysutil/msg.h>

#include <tiny3d.h>

#define MDIALOG_OK		0
#define MDIALOG_YESNO	1

void DrawBackgroundTexture(int x, u8 alpha);
void _drawListBackground(int off, int icon);

static float bar1_countparts;
volatile int msg_dialog_action = 0;

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

void update_dialog_background()
{
    tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);
    tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);
    tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
        TINY3D_BLEND_FUNC_SRC_ALPHA_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_RGB_ONE_MINUS_SRC_ALPHA,
        TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);

    tiny3d_Project2D();

    DrawBackgroundTexture(0, 0xFF);
    _drawListBackground(0, 0xFFFFFFFF);

    tiny3d_Flip();
}

void wait_dialog() 
{
    while(!msg_dialog_action)
    {
        update_dialog_background();
    }

    msgDialogAbort();
    usleep(100 *1000);
}

int show_dialog(int tdialog, const char * str)
{
    msg_dialog_action = 0;

    msgType mtype = MSG_DIALOG_BKG_INVISIBLE | MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_OK;
    if (tdialog == MDIALOG_YESNO)
        mtype = MSG_DIALOG_BKG_INVISIBLE | MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_YESNO  | MSG_DIALOG_DEFAULT_CURSOR_NO;

    msgDialogOpen2(mtype, str, msg_dialog_event, NULL, NULL);
    wait_dialog();
    return (msg_dialog_action == 1);
}

void init_progress_bar(const char* progress_bar_title, const char* msg)
{
	bar1_countparts = 0.0f;

	msgType mdialogprogress = MSG_DIALOG_BKG_INVISIBLE | MSG_DIALOG_SINGLE_PROGRESSBAR | MSG_DIALOG_MUTE_ON;
    msgDialogOpen2(mdialogprogress, progress_bar_title, NULL, NULL, NULL);

    msgDialogProgressBarSetMsg(MSG_PROGRESSBAR_INDEX0, msg);
    msgDialogProgressBarReset(MSG_PROGRESSBAR_INDEX0);

    update_dialog_background();
}

void end_progress_bar(void)
{
    msgDialogAbort();
}

void update_progress_bar(uint64_t* progress, const uint64_t total_size, const char* msg)
{
	if(*progress > 0) {
		bar1_countparts += (100.0f * ((double) *progress)) / ((double) total_size);
        *progress = 0;
	}

	if(bar1_countparts >= 1.0f) {
        msgDialogProgressBarSetMsg(MSG_PROGRESSBAR_INDEX0, msg);
        msgDialogProgressBarInc(MSG_PROGRESSBAR_INDEX0, (u32) bar1_countparts);
            
       	bar1_countparts -= (float) ((u32) bar1_countparts);
	}

	update_dialog_background();
}
