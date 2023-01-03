#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sysutil/msg.h>

#define MDIALOG_OK		0
#define MDIALOG_YESNO	1

void drawDialogBackground();

static uint64_t progbar_tmp;
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

    msgDialogOpen2(MSG_DIALOG_BKG_INVISIBLE | MSG_DIALOG_SINGLE_PROGRESSBAR | MSG_DIALOG_MUTE_ON, progress_bar_title, NULL, NULL, NULL);
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
