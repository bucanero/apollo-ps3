#ifndef _COMMON_H_
#define _COMMON_H_

#define SUCCESS 	0
#define FAILED	 	-1

//----------------------------------------
//String Utils
//----------------------------------------
int is_char_integer(char c);
int is_char_letter(char c);

//----------------------------------------
//LOG
//----------------------------------------

extern int verbose ;

#ifdef ENABLE_LOG
int WriteToLog(char *str);

void CloseLog();

int Open_Log(char *file);

#endif

//----------------------------------------
//COBRA/MAMBA
//----------------------------------------

int is_cobra(void);
int is_mamba(void);

//----------------------------------------
//FILE UTILS
//----------------------------------------

int file_exists(const char *path);
int dir_exists(const char *path);
int unlink_secure(void *path);

#endif
