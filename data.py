
from ctypes import *

MEM_ALLOC_FAIL = 1
MAX_EXCEEDED = 2
INVALID_INDEX = 3
DEL_EMPTY_SET = 4
ASSEMBLE_ERROR = 5

C_INSERT = 0x10
C_INSERT_CHOICE = 0x11
C_DELETE = 0x12
C_DELETE_CHOICE = 0x13
C_DELETE_ALL = 0x14
C_EDIT = 0x15
C_EDIT_CHOICE = 0x16
C_READ = 0x17
C_WRITE = 0x18

# Search options
c_search_all_question = 0x19
c_search_all_answer = 0x1a
c_search_all = 0x1b
c_search_in_range_question = 0x1c
c_search_in_range_answer = 0x1d
c_search_in_range_all = 0x1e
c_search_question_only = 0x1f
c_search_answer_only = 0x20
c_search_question_answer = 0x21


class answer_t(Structure):
	_fields_ = [
		("choice",c_char*250),
		("link",c_uint),
	]

class task_t(Structure):
	_fields_ = [
		("question",c_char*2304),
		("number",c_uint),
		("choices",answer_t*7),
	]

class edt(Structure):
	pass
edt._fields_ = [("data",task_t),("next",POINTER(edt))]
api = cdll.LoadLibrary("core.dll")


def initialize_project(title="",file=""):
	ls = []
	ls.append(edt())
	ls.append(pointer(ls[0]))
	if file=="":
		if title=="":
			pass
		else:
			ls[0].data.question = title.encode(encoding="utf-8")
	else:
		api.edit_task(ls[1],C_READ,file.encode(encoding="utf-8"),0)
	return ls

def iterate_question(ptr):
	offset = ptr.contents.next
	while offset:
		print(offset.contents.data.question.decode(encoding="utf-8"))
		print()
		offset = offset.contents.next



project = initialize_project(file="b.gt")

iterate_question(project[1])
print(api.count(project[1]))

project[1].contents.data.question = b"This is a title"

api.edit_task(project[1],C_INSERT,b"Question 1: what is this question?",0)
api.edit_task(project[1],C_INSERT,b"Question 2: wtf?",0)
api.edit_task(project[1],C_INSERT,b"Question 3: did you answer the previous question?",0)
api.edit_task(project[1],C_INSERT,b"Question 4: what is the first question?",0)
api.edit_task(project[1],C_INSERT,b"Question 5: did you get it all?",0)

api.edit_task(project[1],C_WRITE,b"asdf.gt",0)
