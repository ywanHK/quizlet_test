#ifndef API
#define API

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#define MAX_NUM 400000 // Except the first task (the title)
#define END_EXEC 0xffffff00

#define MULTIPLE_CHOICE 0x06
#define FILL_BLANK 0x04

/* Error codes */
#define MEM_ALLOC_FAIL 1
#define MAX_EXCEEDED 2
#define INVALID_INDEX 3
#define EMPTY_SET 4
#define INVALID_TYPE 5
#define FILE_NO_EXT 6

struct answer_choice_t{
	unsigned char choice[480];
	unsigned char explanation[512];
	unsigned int link;
};
struct answer_keywd_t{
	unsigned char word[1920];
	unsigned char explanation[2048];
	unsigned int correct;
	unsigned int incorrect;
};
struct task_t{
	unsigned char question[3072];
	unsigned char type;
	unsigned int number;
	union{
		struct answer_choice_t choices[7];
		struct answer_keywd_t keyword;
	}
	answer;
};

struct edt{
	struct task_t data;
	struct edt *next;
};

typedef struct answer_choice_t answer;
typedef struct answer_keywd_t keywd;
typedef struct task_t task;
typedef struct edt edit_list;


unsigned int count(edit_list *init); // Does not count the first one
int insert(edit_list *init,unsigned int index,task question,char type);
int delete(edit_list *init,unsigned int index);
void free_all(edit_list *init);
unsigned int fsize(FILE *fp);
edit_list *seek(edit_list *init,unsigned int index);
int change_type(task *node,unsigned char type);
int edit_answer(task *node,keywd *ans);
int edit_choice(task *node,answer *data,int index,int cmd);
edit_list **search_all(edit_list *init,char *match,int option);
edit_list **search_in(edit_list *init,
                      char *match,int option,
                      unsigned int start,
                      unsigned int end);
char *search(edit_list *node,char *match,int option);

//
// Following are API commands and functions
//
#define _INSERT 0x10
#define _INSERT_CHOICE 0x11
#define _DELETE 0x12
#define _DELETE_CHOICE 0x13
#define _DELETE_ALL 0x14
#define _EDIT 0x15
#define _EDIT_CHOICE 0x16
#define _EDIT_ANSWER 0x17

#define _CHANGE_TYPE 0x18
#define _READ 0x19
#define _WRITE 0x1a

enum search_t{
	_search_all_question = 0x1b,
	_search_all_answer,
	_search_all,
	_search_in_range_question,
	_search_in_range_answer,
	_search_in_range_all,
	_search_question_only,
	_search_answer_only,
	_search_question_answer,
};


unsigned int assemble(edit_list *init,task *exec);
int disassemble(edit_list *init,task *exec);
void link(task *final);
#if 0
	static int auto_naming = 0;
	char *get_filename_ext(char *filename);
	char *name_file(char *filename);
#endif
struct result run_task(task *exec,unsigned int position,int choice,unsigned char *keyword);
void safe_check(task *exec,unsigned int number,int cmpl);
task *mem_convert(edit_list *init);
void finish(task *end);

int edit_task(edit_list *init,int cmd,...); /* returns 0 upon success */

#endif

/*

The format of edit_task is:

edit_task(edit_list *init,int _INSERT,char *Question,unsigned int q_index,int type)
edit_task(edit_list *init,int _DELETE,unsigned q_index)
edit_task(edit_list *init,int _DELETE_ALL)
edit_task(edit_list *init,int _EDIT,char *Edited_question,unsigned int q_index)

edit_task(edit_list *init,int _INSERT_CHOICE,answer *answer,unsigned int q_index,int c_index)
edit_task(edit_list *init,int _EDIT_CHOICE,answer *answer,unsigned int q_index,int c_index)
edit_task(edit_list *init,int _DELETE_CHOICE,unsigned int q_index,int c_index)

edit_task(edit_list *init,int _CHANGE_TYPE,unsigned int q_index,int type)
edit_task(edit_list *init,int _EDIT_ANSWER,keywd *keyword,unsigned int q_index)

edit_task(edit_list *init,int _READ,char *name,int _CMPL)
edit_task(edit_list *init,int _WRITE,char *name",int _CMPL)

// _CMPL == 1 : remove extra links

*/
