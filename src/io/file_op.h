#ifndef FILE_OP
#define FILE_OP

#define NAME "00000000" // Name of the question file

// type describes the type of content ('t' for task)
// length describes the length of content in bytes
struct file_info{
	unsigned char type;
	int error;
	unsigned long long length;
	void *content;
};

int create(char *archive_name);
struct file_info read_from_file(char *archive_name,char *name);
int write_to_file(char *archive_name,char *name,void *data,unsigned long long length);
int delete_file(char *archive_name,char *name);

#endif
