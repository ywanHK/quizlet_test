#ifndef API
#define API

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAX_NUM 1000000 // Test value is 5
#define END_EXEC 0xffffff00

#define MEM_ALLOC_FAIL 1
#define MAX_EXCEEDED 2
#define INVALID_INDEX 3
#define DEL_EMPTY_SET 4

// Explanation is integrated in question
struct answer_t{
	unsigned char choice[250];
	unsigned int link;
};
struct task_t{
	unsigned char question[2304];
	unsigned int number;
	struct answer_t choices[7];
};

// The first structure "init" is reserved
// init->data.number defines the number of questions
struct edt{
	struct task_t data;
	struct edt *next;
};

typedef struct answer_t answer;
typedef struct task_t task;
typedef struct edt edit_list;


unsigned int count(edit_list *init);
int insert(edit_list *init,unsigned int index,task question);
int delete(edit_list *init,unsigned int index);
void free_all(edit_list *init);
unsigned int fsize(FILE *fp);
edit_list *seek(edit_list *init,unsigned int index);
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

#define _READ 0x17
#define _WRITE 0x18

enum search_t{
	_search_all_question = 0x19,
	_search_all_answer,
	_search_all,
	_search_in_range_question,
	_search_in_range_answer,
	_search_in_range_all,
	_search_question_only,
	_search_answer_only,
	_search_question_answer,
};
typedef enum search_t search_cmd;

unsigned int assemble(edit_list *init,task *exec);
int disassemble(edit_list *init,task *exec);
void link(task *final);
task *read_from_file(char *name);
int write_to_file(char *name,task *data);
task *run_task(task *exec,int answer);
void safe_check(task *exec,unsigned int number,int cmpl);

int edit_task(edit_list *init,int cmd,...);

#endif



/*
The format of edit_task is:


edit_task(init,_INSERT,"Question",q_index)
edit_task(init,_DELETE,q_index)
edit_task(init,_DELETE_ALL)
edit_task(init,_EDIT,"Edited question",q_index)

edit_task(init,_INSERT_CHOICE,answer,q_index,c_index)
edit_task(init,_EDIT_CHOICE,answer,q_index,c_index)
edit_task(init,_DELETE_CHOICE,q_index,c_index)

edit_task(init,_READ,"name",_CMPL)
edit_task(init,_WRITE,"name",_CMPL)
// _CMPL == 1 tells the program to link

*/
