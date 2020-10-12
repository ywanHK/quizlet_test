
from ctypes import *

MAX_NUM = 1000000
END_EXEC = 0xffffff00

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
api.seek.restype = POINTER(edt)
api.search_all.restype = POINTER(POINTER(edt))
api.search_in.restype = POINTER(POINTER(edt))
api.search.restype = POINTER(c_char)
api.run_task.restype = POINTER(task_t)



class start_project:
	def __init__(self,title="",file=""):
		self.initial = edt()
		self.handler = pointer(self.initial)
		if file=="":
			if title=="":
				pass
			else:
				self.initial.question = title.encode()
		else:
			api.edit_task(self.handler,C_READ,file.encode(),0)
	def count(self):
		return api.count(self.handler)
	def seek(self,index,mode="data"):
		r = api.seek(self.handler,index)
		if mode=="data":
			return r.contents.data
		return r.contents
	def insert(self,question,index):
		q = question.encode()
		return api.edit_task(self.handler,C_INSERT,q,index)
	def edit(self,question,index):
		q = question.encode()
		return api.edit_task(self.handler,C_EDIT,q,index)
	def delete(self,index):
		return api.edit_task(self.handler,C_DELETE,index)
	def delete_all(self):
		return api.edit_task(self.handler,C_DELETE_ALL)
	def insert_choice(self,choice,index,pos,link=0):
		ptr = pointer(answer_t())
		ptr.contents.choice = choice.encode()
		ptr.contents.link = link
		return api.edit_task(self.handler,C_INSERT_CHOICE,ptr,index,pos)
	def edit_choice(self,choice,index,pos,link=0):
		ptr = pointer(answer_t())
		ptr.contents.choice = choice.encode()
		ptr.contents.link = link
		return api.edit_task(self.handler,C_EDIT_CHOICE,ptr,index,pos)
	def delete_choice(self,index,pos):
		return api.edit_task(self.handler,C_DELETE_CHOICE,index,pos)

	def search_all(self,mt,option):
		match = mt.encode()
		match_list = []
		ptr = api.search_all(self.handler,match,option)
		i = 0
		while ptr[i]:
			match_list.append(ptr[i])
			i += 1
		return match_list
	def search_range(self,mt,option,start,end):
		match = mt.encode()
		match_list = []
		ptr = api.search_in(self.handler,match,option,start,end)
		i = 0
		while ptr[i]:
			match_list.append(ptr[i])
			i += 1
		return match_list
	def search_in(self,match,index,option):
		node = api.seek(self.handler,index)
		return api.search(node,match,option)

	def save(self,file,link=0):
		if file=="":
			return 1
		f = file.encode()
		return api.edit_task(self.handler,C_WRITE,f,link)




# DEBUG test below
