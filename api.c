#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "api.h"


unsigned int count(edit_list *init){
	edit_list *offset_this = init;
	unsigned int number = 0;
	while(offset_this->next&&number<MAX_NUM){
		offset_this = offset_this->next;
		number++;
	}
	return number;
}
int insert(edit_list *init,unsigned int index,task question){
	edit_list *offset_this=init,*offset_next=NULL;
	edit_list *new=NULL;
	int counter;
	if(!index){
		counter = 1;
		while(offset_this->next){
			counter++;
			if(counter>MAX_NUM)
				return MAX_EXCEEDED;
			offset_this = offset_this->next;
		}
		new = (edit_list*)calloc(1,sizeof(edit_list));
		if(!new)
			return MEM_ALLOC_FAIL;
		new->next = NULL;
		new->data = question;
		offset_this->next = new;
	}
	else{
		counter = count(init);
		if(counter<index)
			return INVALID_INDEX;
		else if(counter>=MAX_NUM)
			return MAX_EXCEEDED;
		counter = 1;
		offset_next = init->next;
		while(counter<index){
			counter++;
			offset_this = offset_next;
			offset_next = offset_next->next;
		}
		new = (edit_list*)calloc(1,sizeof(edit_list));
		if(!new)
			return MEM_ALLOC_FAIL;
		new->next = offset_next;
		new->data = question;
		offset_this->next = new;
	}
	init->data.number++;
	return 0;
}
int delete(edit_list *init,unsigned int index){
	edit_list *offset_this=NULL;
	edit_list *offset_next=init;
	int counter = count(init);
	if(!counter)return DEL_EMPTY_SET;
	if(index==0){
		for(int i=0;i<counter;i++){
			offset_this = offset_next;
			offset_next = offset_next->next;
		}
		free(offset_next);
		offset_this->next = NULL;
	}
	else{
		if(counter<index)
			return INVALID_INDEX;
		for(int i=0;i<index;i++){
			offset_this = offset_next;
			offset_next = offset_next->next;
		}
		offset_this->next = offset_next->next;
		free(offset_next);
	}
	init->data.number--;
	return 0;
}
void free_all(edit_list *init){
	edit_list *offset_this=init;
	edit_list *offset_next=init->next;
	while(offset_next){
		offset_this = offset_next;
		offset_next = offset_next->next;
		free(offset_this);
	}
	memset(init,0,sizeof(edit_list));
}
unsigned int fsize(FILE *fp){
	unsigned int prev,sz;
	prev = ftell(fp);
	fseek(fp,0L,SEEK_END);
	sz = ftell(fp);
	fseek(fp,prev,SEEK_SET); //go back to where we were
	return sz;
}
edit_list *seek(edit_list *init,unsigned int index){
	edit_list *offset_this = NULL;
	unsigned int counter = 1;
	if(index>MAX_NUM||!init){
		return NULL;
	}
	if(!index){
		if(!init->next);
		else{
			offset_this = init; // Seek to the last element
			while(offset_this->next&&counter<=MAX_NUM){
				offset_this = offset_this->next;
				counter++;
			}
		}
	}
	else{
		offset_this = init->next;
		while(offset_this){
			if(counter==index||counter==MAX_NUM){
				break;
			}
			offset_this = offset_this->next;
			counter++;
		}
	}
	return offset_this;
}
int edit_choice(task *node,answer *data,int index,int cmd){
	int number = node->number;
	int status = 0;
	if(index>7||index>number)
		return INVALID_INDEX;
	switch(cmd){
		case _INSERT_CHOICE:{
			if(number>=7){
				status = MAX_EXCEEDED;
				node->number = 7;
				break;
			}
			if(!index)
				node->choices[number] = *data;
			else{
				for(int i=number;i>=index;i--){
					node->choices[i] = node->choices[i-1];
				}
				node->choices[index-1] = *data;
			}
			node->number++;
			break;
		}
		case _DELETE_CHOICE:{
			if(!number)
				status = DEL_EMPTY_SET;
			else if(number>7){
				node->number = 7;
				status = 1;
			}
			else{
				node->number--;
				if(!index)
					memset(&node->choices[number-1],0,sizeof(answer));
				else{
					for(int i=index;i<=number;i++){
						node->choices[i-1] = node->choices[i];
					}
					memset(&node->choices[number-1],0,sizeof(answer));
				}
			}
			break;
		}
		case _EDIT_CHOICE:{
			if(!number){
				status = INVALID_INDEX;
				break;
			}
			else if(number>7){
				status = 1;
				node->number = 7;
				break;
			}
			node->choices[index-1] = *data;
			break;
		}
		default:
			status = 1;
			break;
	}
	return status;
}
edit_list **search_all(edit_list *init,char *match,int option){
	edit_list **indeces = NULL,*offset = NULL;
	unsigned int counter = 0,i;
	size_t size = sizeof(edit_list*);
	indeces = calloc(2,size);
	if(!indeces||!init){
		free(indeces);
		return NULL;
	}
	offset = init->next;
	switch(option){
		case _search_all_question:{
			while(offset){
				if(strstr((char*)offset->data.question,match)){
					indeces[counter] = offset;
					indeces = realloc(indeces,(counter+2)*size);
					indeces[counter+1] = NULL;
					counter++;
				}
				offset = offset->next;
			}
			break;
		}
		case _search_all_answer:{
			while(offset){
				for(i=0;i<offset->data.number;i++){
					if(strstr((char*)offset->data.choices[i].choice,match)){
						indeces[counter] = offset;
						indeces = realloc(indeces,(counter+2)*size);
						indeces[counter+1] = NULL;
						counter++;
						break;
					}
				}
				offset = offset->next;
			}
			break;
		}
		case _search_all:{
			while(offset){
				if(strstr((char*)offset->data.question,match)){
					indeces[counter] = offset;
					indeces = realloc(indeces,(counter+2)*size);
					indeces[counter+1] = NULL;
					counter++;
				}
				else{
					for(i=0;i<offset->data.number;i++){
						if(strstr((char*)offset->data.choices[i].choice,match)){
							indeces[counter] = offset;
							indeces = realloc(indeces,(counter+2)*size);
							indeces[counter+1] = NULL;
							counter++;
							break;
						}
					}
				}
				offset = offset->next;
			}
			break;
		}
		default:
			free(indeces);
			indeces = NULL;
			break;
	}
	return indeces;
}
edit_list **search_in(edit_list *init,
                      char *match,int option,
                      unsigned int start,
                      unsigned int end){
	edit_list **indeces = NULL,*offset = NULL;
	edit_list *ptr_start = NULL,*ptr_end = NULL;
	unsigned int counter = 0,i;
	size_t size = sizeof(edit_list*);
	indeces = calloc(2,size);
	ptr_start = seek(init,start);
	ptr_end = seek(init,end);
	if(!ptr_start||!ptr_end||!indeces){
		free(indeces);
		return NULL;
	}
	ptr_end = ptr_end->next;
	offset = ptr_start;
	switch(option){
		case _search_in_range_question:{
			while(offset!=ptr_end){
				if(strstr((char*)offset->data.question,match)){
					indeces[counter] = offset;
					indeces = realloc(indeces,(counter+2)*size);
					indeces[counter+1] = NULL;
					counter++;
				}
				offset = offset->next;
			}
			break;
		}
		case _search_in_range_answer:{
			while(offset!=ptr_end){
				for(i=0;i<offset->data.number;i++){
					if(strstr((char*)offset->data.choices[i].choice,match)){
						indeces[counter] = offset;
						indeces = realloc(indeces,(counter+2)*size);
						indeces[counter+1] = NULL;
						counter++;
						break;
					}
				}
				offset = offset->next;
			}
			break;
		}
		case _search_in_range_all:{
			while(offset!=ptr_end){
				if(strstr((char*)offset->data.question,match)){
					indeces[counter] = offset;
					indeces = realloc(indeces,(counter+2)*size);
					indeces[counter+1] = NULL;
					counter++;
				}
				else{
					for(i=0;i<offset->data.number;i++){
						if(strstr((char*)offset->data.choices[i].choice,match)){
							indeces[counter] = offset;
							indeces = realloc(indeces,(counter+2)*size);
							indeces[counter+1] = NULL;
							counter++;
							break;
						}
					}
				}
				offset = offset->next;
			}
			break;
		}
		default:
			free(indeces);
			indeces = NULL;
			break;
	}
	return indeces;
}
char *search(edit_list *node,char *match,int option){
	char *ptr = NULL;
	if(!node)goto ret;
	switch(option){
		case _search_question_only:{
			ptr = strstr((char*)node->data.question,match);
			break;
		}
		case _search_answer_only:{
			for(int i=0;i<node->data.number;i++){
				ptr = strstr((char*)node->data.choices[i].choice,match);
				if(ptr)
					break;
			}
			break;
		}
		case _search_question_answer:{
			ptr = strstr((char*)node->data.question,match);
			if(ptr){
				break;
			}
			else{
				for(int i=0;i<node->data.number;i++){
					ptr = strstr((char*)node->data.choices[i].choice,match);
					if(ptr)
						break;
				}
				break;
			}
		}
		default:
			break;
	}
	ret:return ptr;
}


// Debug test bellow
