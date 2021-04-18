from ctypes import *
import re

MAX_NUM = 400000
END_EXEC = 0xffffff00

MULTIPLE_CHOICE = 0x06
FILL_BLANK = 0x04

NAME = b"00000000"

MEM_ALLOC_FAIL = 1
MAX_EXCEEDED = 2
INVALID_INDEX = 3
EMPTY_SET = 4
INVALID_TYPE = 5
FILE_NO_EXT = 6

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

## options for the search functions
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
		("choice",c_char*480),
		("explanation",c_char*512),
		("link",c_uint),
	]

class keywd(Structure):
	_fields_ = [
		("word",c_char*1920),
		("explanation",c_char*2048),
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
		("question",c_char*3072),
		("type",c_ubyte),
		("number",c_uint),
		("answer",internal),
	]

class edt(Structure):
	pass
edt._fields_ = [("data",task_t),("next",POINTER(edt))]

class file_info(Structure):
	_fields_ = [
		("type",c_char),
		("error",c_int),
		("length",c_ulonglong),
		("content",c_void_p),
	]

class result(Structure):
	_fields_ = [
		("explanation",c_char_p),
		("next",c_uint),
	]

api = cdll.LoadLibrary("./core.dll")
api.seek.restype = POINTER(edt)
api.search_all.restype = POINTER(POINTER(edt))
api.search_in.restype = POINTER(POINTER(edt))
api.search.restype = POINTER(c_char)
api.read_from_file.restype = file_info
api.mem_convert.restype = POINTER(task_t)
api.edit_task.restype = c_int
api.run_task.restype = result


# self.handler is a pointer to the first element of linked-list
# self.name is the name of the .gt file
class edit_quiz:
	def __init__(self,description="",file="",default=FILL_BLANK):
		self.handler = pointer(edt())
		self.default_type = default
		self.name = b"" # Name of the archive
		if file=="":
			if description=="":
				pass
			else:
				self.handler.contents.data.question = description.encode()
		else:
			api.edit_task(self.handler,C_READ,file.encode(),0)
			self.name = file.encode()
		self.handler.contents.data.type = self.default_type
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
	def insert_choice(self,choice,explanation,index,position,link=0):
		ptr = pointer(answer())
		ptr.contents.choice = choice.encode()
		ptr.contents.explanation = explanation.encode()
		ptr.contents.link = link
		return api.edit_task(self.handler,C_INSERT_CHOICE,ptr,index,position)
	def edit_choice(self,index,position,choice=None,explanation=None,link=-1):
		ptr = pointer(answer())
		org = api.seek(self.handler,index)
		if org:
			original = org.contents.data
		else:
			return INVALID_INDEX
		if original.type != MULTIPLE_CHOICE:
			return INVALID_TYPE
		if choice is not None:
			ptr.contents.choice = choice.encode()
		else:
			choice = original.answer.choices[position-1].choice
			ptr.contents.choice = choice
		if explanation is not None:
			ptr.contents.explanation = explanation.encode()
		else:
			explanation = original.answer.choices[position-1].explanation
			ptr.contents.explanation = explanation
		if link < 0:
			lk = original.answer.choices[position-1].link
			ptr.contents.link = lk
		else:
			ptr.contents.link = link
		return api.edit_task(self.handler,C_EDIT_CHOICE,ptr,index,position)
	def delete_choice(self,index,pos):
		return api.edit_task(self.handler,C_DELETE_CHOICE,index,pos)
	def edit_answer(self,index,keyword=None,explanation=None,correct=-1,incorrect=-1):
		kw = pointer(keywd())
		org = api.seek(self.handler,index)
		if org:
			original = org.contents.data
		else:
			return INVALID_INDEX
		if original.type !=FILL_BLANK:
			return INVALID_TYPE
		if keyword is not None:
			kw.contents.word = keyword.encode()
		else:
			w = original.answer.keyword.word
			kw.contents.word = w
		if explanation is not None:
			kw.contents.explanation = explanation.encode()
		else:
			w = original.answer.keyword.explanation
			kw.contents.explanation = w
		if correct >= 0:
			kw.contents.correct = correct
		else:
			c = original.answer.keyword.correct
			kw.contents.correct = c
		if incorrect >= 0:
			kw.contents.incorrect = incorrect
		else:
			i = original.answer.keyword.incorrect
			kw.contents.incorrect = i
		return api.edit_task(self.handler,C_EDIT_ANSWER,kw,index)
	def change_type(self,index,tp):
		return api.edit_task(self.handler,C_CHANGE_TYPE,index,tp)
	def serach_all(self,match,option):
		# Option: c_search_all_question, c_search_all_answer, c_search_all
		match = match.encode()
		match_list = []
		ptr = api.search_all(self.handler,match,option)
		if ptr:
			i = 0
			while ptr.contents[i]:
				match_list.append(ptr.contents[i])
				i += 1
		return match_list
	def search_range(self,match,option,start,end):
		# Options: c_search_in_range_question, c_search_in_range_answer, c_search_in_range_all
		match = match.encode()
		match_list = []
		ptr = api.search_in(self.handler,match,option,start,end)
		if ptr:
			i = 0
			while ptr.contents[i]:
				match_list.append(ptr.contents[i])
				i += 1
		return match_list
	def search_in(self,match,index,option):
		# Options: c_search_question_only, c_search_answer_only, c_search_question_answer
		node = api.seek(self.handler,index)
		return api.search(node,match,option)
	def change_default_type(self,def_type):
		if def_type not in [MULTIPLE_CHOICE,FILL_BLANK]:
			return 1
		self.default_type = def_type
		self.handler.contents.data.type = self.default_type
		return 0
	def add_image(self,path,image_name):
		if self.name == b"":
			# project is not saved yet
			return 1
		with open(path,"rb") as fp:
			contents = fp.read()
		img_name = ("i/" + image_name).replace("..","").encode()
		return api.write_to_file(self.name,img_name,contents,len(contents))
	def delete_image(self,image_name):
		if self.name == b"":
			# project is not saved yet
			return 1
		img_name = ("i/" + image_name).replace("..","").encode()
		return api.delete_file(self.name,img_name)
	def save(self,name=None,cmpl=0):
		if name not in ("",None):
			self.name = str(name).encode()
		if self.name == b"":
			return 1
		if api.edit_task(self.handler,C_WRITE,self.name,cmpl):
			status = api.create(self.name)
			if status:
				return status
			return api.edit_task(self.handler,C_WRITE,self.name,cmpl)
		return 0



# run_task returns struct result
# task is a the first element of a linked-list
# handler is an array allocated by calloc()
class run_quiz:
	def __init__(self,file="",task=None,gui_enabled=False):
		self.positon = 1
		self.gui_enabled = gui_enabled
		if file!="":
			information = api.read_from_file(file.encode(),NAME)
			if information.error != 0:
				err = "Can't open file : {}".format(information.error)
				raise IOError(err)
			self.handler = cast(information.content,POINTER(task_t))
			if not self.handler: # maybe content can be NULL
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
	def nextq(self,response):
		if self.positon == END_EXEC or self.positon > self.number:
			return {"status":END_EXEC}
		qtype = self.handler[self.positon].type
		ret = {}
		if qtype == MULTIPLE_CHOICE:
			if isinstance(response,int):
				pass
			else:
				try:
					response = abs(int(response))
				except Exception:
					ret.update({"status":INVALID_TYPE})
					return ret
			outcome = api.run_task(self.handler,self.positon,response,b"")
		else:
			if isinstance(response,str):
				response = response.encode()
			else:
				response = str(response).encode()
			outcome = api.run_task(self.handler,self.positon,0,response)
		self.positon = outcome.next
		if self.positon == END_EXEC:
			ret.update({"status":END_EXEC})
		else:
			ret = self.format(self.handler[self.positon])
			ret.update({"status":0})
		ret.update({"explanation":outcome.explanation})
		return ret
	def finalize(self):
		self.positon = 1
		api.finish(self.handler)



def parse_image(text):
	image_list = [x.group().replace("..","") for x in re.finditer(r"\<i\/(.*?)\>",text)]
	temp = re.sub(r"\<i\/(.*?)\>","<img>",text.replace("<img>","")).split("<img>")
	final_list = []
	for i in range(len(temp)-1):
		final_list.append(temp[i])
		final_list.append(image_list[i])
	return final_list



# DEBUG tests below

def iterate(data):
	number = 1
	offset = data.handler.contents.next
	print("Title : [ {} ]\n".format(data.handler.contents.data.question.decode()))
	while offset:
		print("{}.".format(number),offset.contents.data.question.decode())
		if offset.contents.data.type == MULTIPLE_CHOICE:
			for i in range(offset.contents.data.number):
				link = offset.contents.data.answer.choices[i].link
				choice = offset.contents.data.answer.choices[i].choice.decode()
				explanation = offset.contents.data.answer.choices[i].explanation.decode()
				print("({}) \t[ {} ]\n\t[ {} ]\n".format(link,choice,explanation))
				i += 1
		else:
			correct = offset.contents.data.answer.keyword.correct
			incorrect = offset.contents.data.answer.keyword.incorrect
			keyword = offset.contents.data.answer.keyword.word.decode()
			explanation = offset.contents.data.answer.keyword.explanation.decode()
			print("({}) ({}) [ {} ]\n\t[ {} ]".format(correct,incorrect,keyword,explanation))
		print("\n")
		offset = offset.contents.next
		number += 1
