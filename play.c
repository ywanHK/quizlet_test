#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "api.h"

// Note: first element of *exec is reserved
// exec[0].number defines the number of questions
unsigned int assemble(edit_list *init,task *exec){
	edit_list *offset_this = init;
	unsigned int num = count(init);
	if(!num)return 0;
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
	if(!exec)return 0;
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
void link(task *final){
	unsigned int num,num_choice,lnk;
	unsigned int counter = final[0].number;
	num = counter<=MAX_NUM?counter:MAX_NUM;
	for(unsigned int i=1;i<=num;i++){
		num_choice = final[i].number & 7;
		for(int j=0;j<num_choice;j++){
			lnk = final[i].choices[j].link;
			final[i].number = num_choice;
			if(lnk<=counter||lnk==END_EXEC){
				continue;
			}
			final[i].choices[j].link = 0;
		}
	}
}
task *read_from_file(char *name){
	FILE *fp = NULL;
	task *buf = NULL;
	unsigned int size,tmp;
	int dt_size = sizeof(task);
	fp = fopen(name,"rb");
	if(!fp)
		goto ret;
	tmp = fsize(fp);
	size = tmp<=MAX_NUM*dt_size?tmp:MAX_NUM*dt_size;
	size = size - size%dt_size;
	if(size<dt_size)
		goto err;
	buf = calloc(1,size);
	if(!buf)
		goto err;
	fread(buf,1,size,fp);
	buf[0].number = (unsigned int)(size/dt_size)-1;
	fclose(fp);
	return buf;
	err:{
		fclose(fp);
		ret:
			return NULL;
	}
}
int write_to_file(char *name,task *data){
	FILE *fp = NULL;
	int number;
	fp = fopen(name,"wb+");
	if(!fp)
		return 1;
	number = data[0].number+1;
	fwrite(data,1,number*sizeof(task),fp);
	fclose(fp);
	return 0;
}
int edit_task(edit_list *init,int cmd,...){
	va_list valist;
	edit_list *offset = NULL;
	int status = 0,c_index,cmpl;
	unsigned int q_index;
	void *data = NULL;
	task task={0},*buffer = NULL;
	va_start(valist,cmd);
	switch(cmd){
		case _INSERT:{
			data = va_arg(valist,unsigned char*);
			q_index = va_arg(valist,unsigned int);
			strncpy((char*)task.question,data,2303);
			status = insert(init,q_index,task);
			break;
		}
		case _DELETE:{
			q_index = va_arg(valist,unsigned int);
			status = delete(init,q_index);
			break;
		}
		case _DELETE_ALL:{
			free_all(init);
			break;
		}
		case _EDIT:{
			data = va_arg(valist,unsigned char*);
			q_index = va_arg(valist,unsigned int);
			offset = seek(init,q_index);
			if(!offset)
				status = INVALID_INDEX;
			else
				strncpy((char*)offset->data.question,data,2303);
			break;
		}
		case _EDIT_CHOICE:
		case _DELETE_CHOICE:
		case _INSERT_CHOICE:{
			if(cmd!=_DELETE_CHOICE){
				data = va_arg(valist,answer*);
			}
			q_index = va_arg(valist,unsigned int);
			c_index = va_arg(valist,int);
			offset = seek(init,q_index);
			if(!offset)
				status = INVALID_INDEX;
			else
				status = edit_choice(&(offset->data),data,c_index,cmd);
			break;
		}
		case _READ:{
			data = va_arg(valist,char*);
			buffer = read_from_file(data);
			cmpl = va_arg(valist,int);
			if(!buffer)
				status = 1;
			else{
				safe_check(buffer,buffer[0].number,cmpl);
				status = disassemble(init,buffer);
			}
			free(buffer);
			break;
		}
		case _WRITE:{
			data = va_arg(valist,char*);
			cmpl = va_arg(valist,int);
			buffer = calloc(count(init)+1,sizeof(task));
			if(!buffer)
				status = MEM_ALLOC_FAIL;
			else{
				assemble(init,buffer);
				if(cmpl==1)
					link(buffer);
				status = write_to_file(data,buffer);
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
task *run_task(task *exec,int answer){
	// This function is not yet tested
	unsigned int next;
	task *ptr = exec;
	next = exec->choices[answer].link;
	link(exec);
	if(!next)
		ptr = exec + sizeof(task);
	else if(next==END_EXEC)
		ptr = NULL; // Task ends
	else
		ptr = exec + next*sizeof(task);
	return ptr;
}
void safe_check(task *exec,unsigned int number,int cmpl){
	unsigned int i;
	if(cmpl==1){
		link(exec);
	}
	for(i=1;i<=number;i++){
		exec[i].question[2303] = 0x00;
		exec[i].number = exec[i].number & 7;
		for(int j=0;j<exec[i].number;j++){
			exec[i].choices[j].choice[249] = 0x00;
		}
	}
}



// Debug test bellow

