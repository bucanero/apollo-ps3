#include <unistd.h>
#include <sysutil/msg.h>

#include <tiny3d.h>

#define MDIALOG_OK		0
#define MDIALOG_YESNO	1

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

void wait_dialog() 
{
    while(!msg_dialog_action)
    {
    	tiny3d_Flip();
    }

    msgDialogAbort();
    usleep(100 *1000);
}

int show_dialog(int tdialog, const char * str)
{
    msg_dialog_action = 0;

    msgType mtype = MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_OK;
    if (tdialog == MDIALOG_YESNO)
        mtype = MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_YESNO  | MSG_DIALOG_DEFAULT_CURSOR_NO;

    msgDialogOpen2(mtype, str, msg_dialog_event, (void*)  0x0000aaaa, NULL );
    wait_dialog();
    return (msg_dialog_action == 1);
}

void init_progress_bar(const char* progress_bar_title, const char* msg)
{
	bar1_countparts = 0.0f;

	msgType mdialogprogress = MSG_DIALOG_SINGLE_PROGRESSBAR | MSG_DIALOG_MUTE_ON;
    msgDialogOpen2(mdialogprogress, progress_bar_title, NULL, (void *) 0xadef0045, NULL);

    msgDialogProgressBarSetMsg(MSG_PROGRESSBAR_INDEX0, msg);
    msgDialogProgressBarReset(MSG_PROGRESSBAR_INDEX0);

    tiny3d_Flip();
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

	tiny3d_Flip();
}
