#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "api.h"
#include "io/file_op.h"

// Note: first element of *exec is reserved
// exec[0].number defines the number of questions
unsigned int assemble(edit_list *init,task *exec){
	edit_list *offset_this = init;
	unsigned int num;
	if(!init)
		return 1;
	if(!(num=count(init)))
		return 0;
	for(unsigned int i=0;i<=num;i++){
		exec[i] = offset_this->data;
		offset_this = offset_this->next;
	}
	exec[0].number = num;
	return num;
}
int disassemble(edit_list *init,task *exec){
	edit_list *offset_this = init;
	edit_list *new = NULL;
	unsigned int counter,num;
	if(!init||!exec)return 1;
	num = exec[0].number;
	counter = num<=MAX_NUM?num:MAX_NUM;
	offset_this->data = exec[0];
	for(unsigned int i=1;i<=counter;i++){
		new = (edit_list*)calloc(1,sizeof(edit_list));
		if(!new){
			return MEM_ALLOC_FAIL;
		}
		new->next = NULL;
		new->data = exec[i];
		offset_this->next = new;
		offset_this = new;
	}
	init->data.number = counter;
	return 0;
}

// Removes all unecessary links
// must be called before executing task
void link(task *final){
	unsigned int counter,num_choice,lnk,num;
	if(!final)return;
	num = final[0].number;
	counter = num<=MAX_NUM?num:MAX_NUM;
	for(unsigned int i=1;i<=counter;i++){
		if(final[i].type==FILL_BLANK){
			final[i].number = 0;
			lnk = final[i].answer.keyword.correct;
			if(!(lnk<=counter||lnk==END_EXEC)){
				final[i].answer.keyword.correct = 0;
			}
			lnk = final[i].answer.keyword.incorrect;
			if(!(lnk<=counter||lnk==END_EXEC)){
				final[i].answer.keyword.incorrect = 0;
			}
		}
		else if(final[i].type==MULTIPLE_CHOICE){
			num_choice = final[i].number & 7;
			for(int j=0;j<num_choice;j++){
				lnk = final[i].answer.choices[j].link;
				final[i].number = num_choice;
				if(lnk<=counter||lnk==END_EXEC){
					continue;
				}
				final[i].answer.choices[j].link = 0;
			}
			final[i].number = num_choice;
		}
		else{
			final[i].type = FILL_BLANK;
			memset(&final[i].answer,0,sizeof(final[i].answer));
		}
	}
}
#if 0
// Will be enabled after implementing auto_naming
char *get_filename_ext(char *filename){
	char *dot = strrchr(filename,'.');
	if(!dot||dot == filename)return "";
	return dot + 1;
}
char *name_file(char *filename){
	char *name,*extension;
	extension = get_filename_ext(filename);
	name = malloc(strlen(filename)+4);
	if(!name)
		return NULL;
	else if(!strcmp("gt",extension))
		strcpy(name,filename);
	else
		sprintf(name,"%s.gt",filename);
	return name;
}
#endif
void safe_check(task *exec,unsigned int number,int cmpl){
	if(!exec)return;
	exec[0].number = number;
	if(cmpl==1)
		link(exec);
	for(unsigned int i=1;i<=number;i++){
		exec[i].question[3071] = 0x00;
		if(exec[i].type==MULTIPLE_CHOICE){
			exec[i].number = exec[i].number & 7;
			for(int j=0;j<exec[i].number;j++){
				exec[i].answer.choices[j].choice[511] = 0x00;
			}
		}
		else{
			exec[i].type = FILL_BLANK;
			exec[i].number = 0;
			exec[i].answer.keyword.word[2047] = 0x00;
		}
	}
}
unsigned int run_task(task *exec,unsigned int position,int choice,unsigned char *keyword){
	// returns the index of the next question
	// If answer is invalid type, return same position
	unsigned int next,number;
	unsigned char type;
	if(!exec)
		return END_EXEC;
	number = exec[0].number;
	if(position==END_EXEC||position>number)
		return END_EXEC;
	type = exec[position].type;
	if(type==MULTIPLE_CHOICE){
		choice = choice % exec[position].number;
		next = exec[position].answer.choices[abs(choice-1)].link;
	}
	else{
		if(!keyword)
			return position;
		if(!strstr((char*)keyword,(char*)exec[position].answer.keyword.word)){
			next = exec[position].answer.keyword.incorrect;
		}
		else{
			next = exec[position].answer.keyword.correct;
		}
	}
	if(!next){
		next = position + 1;
	}
	next = next>number?END_EXEC:next;
	return next;
}
int edit_task(edit_list *init,int cmd,...){
	va_list valist;
	int status = 0;
	va_start(valist,cmd);
	switch(cmd){
		case _INSERT:{
			void *question = va_arg(valist,unsigned char*);
			unsigned int q_index = va_arg(valist,unsigned int);
			task set={0};
			strncpy((char*)set.question,question,3071);
			status = insert(init,q_index,set,va_arg(valist,int));
			break;
		}
		case _DELETE:{
			unsigned int q_index = va_arg(valist,unsigned int);
			status = delete(init,q_index);
			break;
		}
		case _DELETE_ALL:{
			free_all(init);
			break;
		}
		case _EDIT:{
			void *question = va_arg(valist,unsigned char*);
			unsigned int q_index = va_arg(valist,unsigned int);
			edit_list *offset = seek(init,q_index);
			if(!offset)
				status = INVALID_INDEX;
			else
				strncpy((char*)offset->data.question,question,3071);
			break;
		}
		case _EDIT_ANSWER:{
			void *answer = va_arg(valist,keywd*);
			unsigned int q_index = va_arg(valist,unsigned int);
			edit_list *offset = seek(init,q_index);
			if(!offset)
				status = INVALID_INDEX;
			else
				status = edit_answer(&(offset->data),answer);
			break;
		}
		case _EDIT_CHOICE:
		case _DELETE_CHOICE:
		case _INSERT_CHOICE:{
			answer *ans;
			if(cmd!=_DELETE_CHOICE){
				ans = va_arg(valist,answer*);
			}
			unsigned int q_index = va_arg(valist,unsigned int);
			int c_index = abs(va_arg(valist,int));
			edit_list *offset = seek(init,q_index);
			if(!offset)
				status = INVALID_INDEX;
			else
				status = edit_choice(&(offset->data),ans,c_index,cmd);
			break;
		}
		case _CHANGE_TYPE:{
			edit_list *offset = seek(init,va_arg(valist,unsigned int));
			status = change_type(&(offset->data),va_arg(valist,int));
			break;
		}
		case _READ:{
			struct file_info info;
			task *buffer;
			int cmpl;
			info = read_from_file(va_arg(valist,char*),NAME);
			if(!info.content){ // maybe content can be NULL, WTF
				status = info.error;
			}
			else{
				buffer = info.content;
				cmpl = va_arg(valist,int);
				safe_check(buffer,buffer[0].number,cmpl);
				status = disassemble(init,buffer);
			}
			free(buffer);
			break;
		}
		case _WRITE:{
			char *name = va_arg(valist,char*);
			int cmpl = va_arg(valist,int);
			task *buffer = calloc(count(init)+1,sizeof(task));
			if(!buffer)
				status = MEM_ALLOC_FAIL;
			else{
				assemble(init,buffer);
				if(cmpl==1)
					link(buffer);
#define N (buffer[0].number+1)*sizeof(task)
				if(!access(name,F_OK))
					status = write_to_file(name,NAME,buffer,N);
				else
					status = FILE_NO_EXT;
#undef N
			}
			free(buffer);
			break;
		}
		default:
			status = 1;
			break;
	}
	va_end(valist);
	return status;
}
void finish(task *end){
	free(end);
}
task *mem_convert(edit_list *init){
	unsigned int number;
	task *buffer = NULL;
	if(!init)return buffer;
	number = count(init);
	buffer = calloc(number+1,sizeof(edit_list));
	if(!buffer)
		return NULL;
	assemble(init,buffer);
	return buffer;
}


// Debug test bellow
