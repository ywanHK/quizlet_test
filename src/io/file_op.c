#include <zip.h>
#include "file_op.h"
#include "../api.h"

// name must be added extension .gt and validated
// to be ASCII ONLY before passing tho this function
//
// Created file follows the format in format.zip
// If file already exists, override it
// Returns 0 upon success
int create(char *archive_name){
	zip_t *archive = NULL;
	zip_source_t *source;
	task title={0};
	int status = 0,index;

	// check if archive works properly
	if(!(archive=zip_open(archive_name,ZIP_CREATE,&status))){
		if(!(archive=zip_open(archive_name,ZIP_TRUNCATE,&status))){
			zip_close(archive);
			return status;
		}
		status = 0;
	}

	// create the proper formats
	zip_dir_add(archive,"i",0);
	source = zip_source_buffer(archive,&title,sizeof(title),0);
	index = zip_name_locate(archive,NAME,0);
	if(!source){
		zip_source_free(source);
		zip_close(archive);
		return 1;
	}
	if(index<0){
		if(zip_file_add(archive,NAME,source,ZIP_FL_ENC_UTF_8)<0){
			zip_source_free(source);
			status = 1;
		}
	}
	else{
		if(zip_file_replace(archive,index,source,ZIP_FL_ENC_UTF_8)<0){
			zip_source_free(source);
			status = 1;
		}
	}
	zip_close(archive);
	return status;
}


// This function extracts all bytes
// except the file named 00000000
//
// *name must be checked and filtered off from ".."
// Abort in case ".." is present in *name
struct file_info read_from_file(char *archive_name,char *name){
	zip_t *archive = NULL;
	zip_file_t *data = NULL;
	struct file_info info={0};
	struct zip_stat stat={0};
	int status = 0,data_len = sizeof(task);

	// Check if archive works properly and filter name
	if(strstr(name,"..")){
		info.error = 1;
		goto ret;
	}
	if(!(archive=zip_open(archive_name,0,&status))){
		zip_close(archive);
		info.error = status;
		goto ret;
	}
	if(zip_stat(archive,name,0,&stat)<0){
		zip_close(archive);
		info.error = 1;
		goto ret;
	}

	// Load data and verify type
	if(!strcmp(name,NAME)){
		info.length = stat.size<=(MAX_NUM+1)*data_len?stat.size:(MAX_NUM+1)*data_len;
		info.content = calloc(1,info.length);
		if(info.length<=0||!info.content){
			info.error = 1;
			goto ret;
		}
		info.type = 't';
		((task*)info.content)[0].number = (unsigned int)(info.length/data_len)-1;
	}
	else{
		info.length = stat.size;
		info.content = calloc(1,info.length);
		if(info.length<=0||!info.content){
			info.error = 1;
			goto ret;
		}
	}
	data = zip_fopen(archive,name,0);
	zip_fread(data,info.content,info.length);
	zip_fclose(data);
	zip_close(archive);
	ret:return info;
}



// When file does not exist, return FILE_NO_EXT
// Must check for .. in *name before processing it
// returns 0 upon success
int write_to_file(char *archive_name,char *name,void *data,unsigned long long length){
	zip_t *archive = NULL;
	zip_source_t *source;
	int index,status = 0;
	if(strstr(name,".."))
		return 1;
	if(!(archive=zip_open(archive_name,0,&status))){
		zip_close(archive);
		return status;
	}
	if(!(source=zip_source_buffer(archive,data,length,0))){
		zip_source_free(source);
		zip_close(archive);
		return 1;
	}
	if((index=zip_name_locate(archive,name,0))<0){
		if(zip_file_add(archive,name,source,ZIP_FL_ENC_UTF_8)<0){
			zip_source_free(source);
			status = 1;
		}
	}
	else{
		if(zip_file_replace(archive,index,source,ZIP_FL_ENC_UTF_8)<0){
			zip_source_free(source);
			status = 1;
		}
	}
	zip_close(archive);
	return status;
}


// only files in the i/ directory can be deleted
// *name must be filter off from ".."
// returns 0 upon success
int delete_file(char *archive_name,char *name){
	zip_t *archive = NULL;
	int index,status = 0;
	if(strstr(name,"..")||!strcmp(name,NAME)||!strcmp(name,"i/"))
		return INVALID_INDEX;
	if(!(archive=zip_open(archive_name,0,&status))){
		zip_close(archive);
		return status;
	}
	if((index=zip_name_locate(archive,name,0))<0){
		zip_close(archive);
		return 1;
	}
	status = zip_delete(archive,index);
	zip_close(archive);
	return status;
}

// Debug tests bellow
