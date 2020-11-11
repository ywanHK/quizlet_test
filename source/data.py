from ctypes import *

MAX_NUM = 1000000
END_EXEC = 0xffffff00

MULTIPLE_CHOICE = 0x06 # b'\x06'
FILL_BLANK = 0x04 # b'\x04'

MEM_ALLOC_FAIL = 1
MAX_EXCEEDED = 2
INVALID_INDEX = 3
EMPTY_SET = 4
INVALID_TYPE = 5

C_INSERT = 0x10
C_INSERT_CHOICE = 0x11
C_DELETE = 0x12
C_DELETE_CHOICE = 0x13
C_DELETE_ALL = 0x14
C_EDIT = 0x15
C_EDIT_CHOICE = 0x16
C_EDIT_ANSWER = 0x17

C_CHANGE_TYPE = 0x18
C_READ = 0x19
C_WRITE = 0x1a

c_search_all_question = 0x1b
c_search_all_answer = 0x1c
c_search_all = 0x1d
c_search_in_range_question = 0x1e
c_search_in_range_answer = 0x1f
c_search_in_range_all = 0x20
c_search_question_only = 0x21
c_search_answer_only = 0x22
c_search_question_answer = 0x23

class answer(Structure):
	_fields_ = [
		("choice",c_char*250),
		("link",c_uint),
	]

class keywd(Structure):
	_fields_ = [
		("word",c_char*1784),
		("correct",c_uint),
		("incorrect",c_uint),
	]

class internal(Union):
	_fields_ = [
		("choices",answer*7),
		("keyword",keywd),
	]

class task_t(Structure):
	_fields_ = [
		("question",c_char*2304),
		("type",c_ubyte),
		("number",c_uint),
		("answer",internal),
	]

class edt(Structure):
	pass
edt._fields_ = [("data",task_t),("next",POINTER(edt))]

api = cdll.LoadLibrary("core.dll")
api.seek.restype = POINTER(edt)
api.search_all.restype = POINTER(POINTER(edt))
api.search_in.restype = POINTER(POINTER(edt))
api.search.restype = POINTER(c_char)
api.read_from_file.restype = POINTER(task_t)
api.mem_convert.restype = POINTER(task_t)
api.edit_task.restype = c_int
api.run_task.restype = c_uint

class edit_quiz:
	def __init__(self,description="",file="",default=FILL_BLANK):
		# handler is a pointer to the first element of linked-list
		self.initial = edt()
		self.handler = pointer(self.initial)
		self.default_type = default
		if file=="":
			if description=="":
				pass
			else:
				self.initial.question = description.encode()
		else:
			api.edit_task(self.handler,C_READ,file.encode(),0)
	def count(self):
		return api.count(self.handler)
	def seek(self,index,mode="content"):
		r = api.seek(self.handler,index)
		if mode == "data":
			return r.contents.data
		if mode == "pointer":
			return r
		return r.contents
	def insert(self,question,index,qtype=0):
		q = question.encode()
		if qtype==0:
			status = api.edit_task(self.handler,C_INSERT,q,index,self.default_type)
		else:
			status = api.edit_task(self.handler,C_INSERT,q,index,qtype)
		return status
	def edit(self,question,index):
		q = question.encode()
		return api.edit_task(self.handler,C_EDIT,q,index)
	def delete(self,index):
		return api.edit_task(self.handler,C_DELETE,index)
	def delete_all(self):
		return api.edit_task(self.handler,C_DELETE_ALL)
	def insert_choice(self,choice,index,pos,link=0):
		ptr = pointer(answer())
		ptr.contents.choice = choice.encode()
		ptr.contents.link = link
		return api.edit_task(self.handler,C_INSERT_CHOICE,ptr,index,pos)
	def edit_choice(self,index,position,choice=None,link=-1):
		ptr = pointer(answer())
		org = api.seek(self.handler,index)
		if org:
			original = org.contents.data
		if original.type != MULTIPLE_CHOICE:
			return INVALID_TYPE
		if choice is not None:
			ptr.contents.choice = choice.encode()
		else:
			choice = original.answer.choices[position-1].choice
			ptr.contents.choice = choice
		if link == -1:
			lk = original.answer.choices[position-1].link
			ptr.contents.link = lk
		else:
			ptr.contents.link = link
		return api.edit_task(self.handler,C_EDIT_CHOICE,ptr,index,position)
	def delete_choice(self,index,pos):
		return api.edit_task(self.handler,C_DELETE_CHOICE,index,pos)
	def edit_answer(self,index,keyword=None,cor=-1,incor=-1):
		kw = pointer(keywd())
		original = api.seek(self.handler,index).contents.data
		if original.type !=FILL_BLANK:
			return INVALID_TYPE
		if keyword is not None:
			kw.contents.word = keyword.encode()
		else:
			w = original.answer.keyword.word
			kw.contents.word = w
		if cor != -1:
			kw.contents.correct = cor
		else:
			c = original.answer.keyword.correct
			kw.contents.correct = c
		if incor != -1:
			kw.contents.incorrect = incor
		else:
			i = original.answer.keyword.incorrect
			kw.contents.incorrect = i
		return api.edit_task(self.handler,C_EDIT_ANSWER,kw,index)
	def change_type(self,tp,index):
		return api.edit_task(self.handler,C_CHANGE_TYPE,index,tp)
	def serach_all(self,mt,option):
		match = mt.encode()
		match_list = []
		ptr = api.search_all(self.handler,match,option)
		i = 0
		while ptr.contents[i]:
			match_list.append(ptr.contents[i])
			i += 1
		return match_list
	def search_range(self,mt,option,start,end):
		match = mt.encode()
		match_list = []
		ptr = api.search_in(self.handler,match,option,start,end)
		i = 0
		while ptr.contents[i]:
			match_list.append(ptr.contents[i])
			i += 1
		return match_list
	def search_in(self,match,index,option):
		node = api.seek(self.handler,index)
		return api.search(node,match,option)
	def save(self,file,cmpl=0):
		if file=="":
			return 1
		f = file.encode()
		return api.edit_task(self.handler,C_WRITE,f,cmpl)


class run_quiz:
	def __init__(self,file="",task=None):
		# task is a the first element of a linked-list
		# __handler is an array allocated by calloc()
		self.positon = 1
		if file!="":
			self.handler = api.read_from_file(file.encode())
			if not self.handler:
				raise Exception("Invalid file format")
		elif task is not None:
			self.handler = api.mem_convert(task.handler)
			if not self.handler:
				raise Exception("Memory allocation failed")
		else:
			raise Exception("No parameters")
		self.number = self.handler[0].number
	@staticmethod
	def format(data):
		qtype = data.type
		entry = {}
		qn = data.question.decode()
		if qtype == FILL_BLANK:
			entry.update({qn:None})
		elif qtype == MULTIPLE_CHOICE:
			entry.update({qn:[]})
			for i in range(data.number):
				entry[qn].append(data.answer.choices[i].choice.decode())
		return entry
	def initialize(self):
		api.safe_check(self.handler,self.number,1)
		q = {}
		if self.number > 0:
			q.update(self.format(self.handler[self.positon]))
			q.update({"status":0})
		else:
			q.update({"status":EMPTY_SET})
		return q
	def nextq(self,ans):
		qtype = self.handler[self.positon].type
		ret = {}
		if qtype == FILL_BLANK:
			if isinstance(ans,str):
				ans = ans.encode()
			else:
				ans = str(ans)
		elif qtype == MULTIPLE_CHOICE:
			if isinstance(ans,int):
				pass
			else:
				try:
					ans = abs(int(ans) % MAX_NUM)
				except Exception:
					ret.update({"status":INVALID_TYPE})
					return ret
		else:
			ret.update({"status":INVALID_TYPE})
		self.positon = api.run_task(self.handler,self.positon,ans)
		if self.positon == END_EXEC:
			ret.update({"status":END_EXEC})
		else:
			ret = self.format(self.handler[self.positon])
			ret.update({"status":0})
		return ret
	def finalize(self):
		self.positon = 1
		api.finish(self.handler)



# DEBUG test below

def iterate(data):
	offset = data.handler.contents.next
	while offset:
		print(offset.contents.data.question.decode())
		if offset.contents.data.type == MULTIPLE_CHOICE:
			for i in range(offset.contents.data.number):
				link = offset.contents.data.answer.choices[i].link
				choice = offset.contents.data.answer.choices[i].choice.decode()
				print(i+1,"--> ",link,"--> ",end="")
				print(choice)
				i += 1
		else:
			correct = offset.contents.data.answer.keyword.correct
			incorrect = offset.contents.data.answer.keyword.incorrect
			keyword = offset.contents.data.answer.keyword.word.decode()
			print("correct = ",correct)
			print("incorrect = ",incorrect)
			print("keyword = ",keyword)
		print()
		offset = offset.contents.next
