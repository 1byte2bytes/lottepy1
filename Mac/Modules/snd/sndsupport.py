# This script generates the Sound interface for Python.
# It uses the "bgen" package to generate C code.
# It execs the file sndgen.py which contain the function definitions
# (sndgen.py was generated by sndscan.py, scanning the <Sound.h> header file).

from macsupport import *


# define our own function and module generators

class SndMixIn: pass

class SndFunction(SndMixIn, OSErrFunctionGenerator): pass
class SndMethod(SndMixIn, OSErrMethodGenerator): pass


# includestuff etc. are imported from macsupport

includestuff = includestuff + """
#include <Sound.h>

#ifndef HAVE_UNIVERSAL_HEADERS
#define SndCallBackUPP ProcPtr
#define NewSndCallBackProc(x) ((SndCallBackProcPtr)(x))
#define SndListHandle Handle
#endif
"""

initstuff = initstuff + """
"""


# define types used for arguments (in addition to standard and macsupport types)

class SndChannelPtrType(OpaqueByValueType):
	def declare(self, name):
		# Initializing all SndChannelPtr objects to 0 saves
		# special-casing NewSndChannel(), where it is formally an
		# input-output parameter but we treat it as output-only
		# (since Python users are not supposed to allocate memory)
		Output("SndChannelPtr %s = 0;", name)

SndChannelPtr = SndChannelPtrType('SndChannelPtr', 'SndCh')

SndCommand = OpaqueType('SndCommand', 'SndCmd')
SndCommand_ptr = OpaqueType('SndCommand', 'SndCmd')
SndListHandle = OpaqueByValueType("SndListHandle", "ResObj")
SPBPtr = OpaqueByValueType("SPBPtr", "SPBObj")

#
# NOTE: the following is pretty dangerous. For void pointers we pass buffer addresses
# but we have no way to check that the buffer is big enough. This is the same problem
# as in C, though (but Pythoneers may not be suspecting this...)
void_ptr = Type("void *", "w")

class SndCallBackType(InputOnlyType):
	def __init__(self):
		Type.__init__(self, 'PyObject*', 'O')
	def getargsCheck(self, name):
		Output("if (%s != Py_None && !PyCallable_Check(%s))", name, name)
		OutLbrace()
		Output('PyErr_SetString(PyExc_TypeError, "callback must be callable");')
		Output("goto %s__error__;", name)
		OutRbrace()
	def passInput(self, name):
		return "NewSndCallBackProc(SndCh_UserRoutine)"
	def cleanup(self, name):
		# XXX This knows it is executing inside the SndNewChannel wrapper
		Output("if (_res != NULL && %s != Py_None)", name)
		OutLbrace()
		Output("SndChannelObject *p = (SndChannelObject *)_res;")
		Output("p->ob_itself->userInfo = (long)p;")
		Output("Py_INCREF(%s);", name)
		Output("p->ob_callback = %s;", name)
		OutRbrace()
		DedentLevel()
		Output(" %s__error__: ;", name)
		IndentLevel()

SndCallBackProcPtr = SndCallBackType()
SndCallBackUPP = SndCallBackProcPtr

SndCompletionProcPtr = FakeType('(SndCompletionProcPtr)0') # XXX
SndCompletionUPP = SndCompletionProcPtr

##InOutBuf128 = FixedInputOutputBufferType(128)
StateBlock = StructInputOutputBufferType('StateBlock')

AudioSelectionPtr = FakeType('0') # XXX

ProcPtr = FakeType('0') # XXX
FilePlayCompletionUPP = FakeType('0') # XXX

SCStatus = StructOutputBufferType('SCStatus')
SMStatus = StructOutputBufferType('SMStatus')
CompressionInfo = StructOutputBufferType('CompressionInfo')

includestuff = includestuff + """
#include <OSUtils.h> /* for Set(Current)A5 */

/* Create a SndCommand object (an (int, int, int) tuple) */
static PyObject *
SndCmd_New(SndCommand *pc)
{
	return Py_BuildValue("hhl", pc->cmd, pc->param1, pc->param2);
}

/* Convert a SndCommand argument */
static int
SndCmd_Convert(PyObject *v, SndCommand *pc)
{
	int len;
	pc->param1 = 0;
	pc->param2 = 0;
	if (PyTuple_Check(v)) {
		if (PyArg_ParseTuple(v, "h|hl", &pc->cmd, &pc->param1, &pc->param2))
			return 1;
		PyErr_Clear();
		return PyArg_ParseTuple(v, "hhs#", &pc->cmd, &pc->param1, &pc->param2, &len);
	}
	return PyArg_Parse(v, "h", &pc->cmd);
}

static pascal void SndCh_UserRoutine(SndChannelPtr chan, SndCommand *cmd); /* Forward */
static pascal void SPB_completion(SPBPtr my_spb); /* Forward */
static pascal void SPB_interrupt(SPBPtr my_spb); /* Forward */
"""


finalstuff = finalstuff + """
/* Routine passed to Py_AddPendingCall -- call the Python callback */
static int
SndCh_CallCallBack(arg)
	void *arg;
{
	SndChannelObject *p = (SndChannelObject *)arg;
	PyObject *args;
	PyObject *res;
	args = Py_BuildValue("(O(hhl))",
	                     p, p->ob_cmd.cmd, p->ob_cmd.param1, p->ob_cmd.param2);
	res = PyEval_CallObject(p->ob_callback, args);
	Py_DECREF(args);
	if (res == NULL)
		return -1;
	Py_DECREF(res);
	return 0;
}

/* Routine passed to NewSndChannel -- schedule a call to SndCh_CallCallBack */
static pascal void
SndCh_UserRoutine(SndChannelPtr chan, SndCommand *cmd)
{
	SndChannelObject *p = (SndChannelObject *)(chan->userInfo);
	if (p->ob_callback != NULL) {
		long A5 = SetA5(p->ob_A5);
		p->ob_cmd = *cmd;
		Py_AddPendingCall(SndCh_CallCallBack, (void *)p);
		SetA5(A5);
	}
}

/* SPB callbacks - Schedule callbacks to Python */
static int
SPB_CallCallBack(arg)
	void *arg;
{
	SPBObject *p = (SPBObject *)arg;
	PyObject *args;
	PyObject *res;
	
	if ( p->ob_thiscallback == 0 ) return 0;
	args = Py_BuildValue("(O)", p);
	res = PyEval_CallObject(p->ob_thiscallback, args);
	p->ob_thiscallback = 0;
	Py_DECREF(args);
	if (res == NULL)
		return -1;
	Py_DECREF(res);
	return 0;
}

static pascal void
SPB_completion(SPBPtr my_spb)
{
	SPBObject *p = (SPBObject *)(my_spb->userLong);
	
	if (p && p->ob_completion) {
		long A5 = SetA5(p->ob_A5);
		p->ob_thiscallback = p->ob_completion;	/* Hope we cannot get two at the same time */
		Py_AddPendingCall(SPB_CallCallBack, (void *)p);
		SetA5(A5);
	}
}

static pascal void
SPB_interrupt(SPBPtr my_spb)
{
	SPBObject *p = (SPBObject *)(my_spb->userLong);
	
	if (p && p->ob_interrupt) {
		long A5 = SetA5(p->ob_A5);
		p->ob_thiscallback = p->ob_interrupt;	/* Hope we cannot get two at the same time */
		Py_AddPendingCall(SPB_CallCallBack, (void *)p);
		SetA5(A5);
	}
}
"""


# create the module and object definition and link them

class SndObjectDefinition(ObjectDefinition):

	def outputStructMembers(self):
		ObjectDefinition.outputStructMembers(self)
		Output("/* Members used to implement callbacks: */")
		Output("PyObject *ob_callback;")
		Output("long ob_A5;");
		Output("SndCommand ob_cmd;")

	def outputInitStructMembers(self):
		ObjectDefinition.outputInitStructMembers(self)
		Output("it->ob_callback = NULL;")
		Output("it->ob_A5 = SetCurrentA5();");

	def outputCleanupStructMembers(self):
		ObjectDefinition.outputCleanupStructMembers(self)
		Output("Py_XDECREF(self->ob_callback);")
	
	def outputFreeIt(self, itselfname):
		Output("SndDisposeChannel(%s, 1);", itselfname)
		
#

class SpbObjectDefinition(ObjectDefinition):

	def outputStructMembers(self):
		Output("/* Members used to implement callbacks: */")
		Output("PyObject *ob_completion;")
		Output("PyObject *ob_interrupt;")
		Output("PyObject *ob_thiscallback;");
		Output("long ob_A5;")
		Output("SPB ob_spb;")

	def outputNew(self):
		Output()
		Output("%sPyObject *%s_New()", self.static, self.prefix)
		OutLbrace()
		Output("%s *it;", self.objecttype)
		self.outputCheckNewArg()
		Output("it = PyObject_NEW(%s, &%s);", self.objecttype, self.typename)
		Output("if (it == NULL) return NULL;")
		self.outputInitStructMembers()
		Output("return (PyObject *)it;")
		OutRbrace()

	def outputInitStructMembers(self):
		Output("it->ob_completion = NULL;")
		Output("it->ob_interrupt = NULL;")
		Output("it->ob_thiscallback = NULL;")
		Output("it->ob_A5 = SetCurrentA5();")
		Output("memset((char *)&it->ob_spb, 0, sizeof(it->ob_spb));")
		Output("it->ob_spb.userLong = (long)it;")

	def outputCleanupStructMembers(self):
		ObjectDefinition.outputCleanupStructMembers(self)
		Output("self->ob_spb.userLong = 0;")
		Output("self->ob_thiscallback = 0;")
		Output("Py_XDECREF(self->ob_completion);")
		Output("Py_XDECREF(self->ob_interrupt);")
	
	def outputConvert(self):
		Output("%s%s_Convert(v, p_itself)", self.static, self.prefix)
		IndentLevel()
		Output("PyObject *v;")
		Output("%s *p_itself;", self.itselftype)
		DedentLevel()
		OutLbrace()
		self.outputCheckConvertArg()
		Output("if (!%s_Check(v))", self.prefix)
		OutLbrace()
		Output('PyErr_SetString(PyExc_TypeError, "%s required");', self.name)
		Output("return 0;")
		OutRbrace()
		Output("*p_itself = &((%s *)v)->ob_spb;", self.objecttype)
		Output("return 1;")
		OutRbrace()

	def outputSetattr(self):
		Output()
		Output("static int %s_setattr(self, name, value)", self.prefix)
		IndentLevel()
		Output("%s *self;", self.objecttype)
		Output("char *name;")
		Output("PyObject *value;")
		DedentLevel()
		OutLbrace()
		self.outputSetattrBody()
		OutRbrace()

	def outputSetattrBody(self):
		Output("""
	int rv = 0;
	
	if (strcmp(name, "inRefNum") == 0)
		rv = PyArg_Parse(value, "l", &self->ob_spb.inRefNum);
	else if (strcmp(name, "count") == 0)
		rv = PyArg_Parse(value, "l", &self->ob_spb.count);
	else if (strcmp(name, "milliseconds") == 0)
		rv = PyArg_Parse(value, "l", &self->ob_spb.milliseconds);
	else if (strcmp(name, "buffer") == 0)
		rv = PyArg_Parse(value, "w#", &self->ob_spb.bufferPtr, &self->ob_spb.bufferLength);
	else if (strcmp(name, "completionRoutine") == 0) {
		self->ob_spb.completionRoutine = NewSICompletionProc(SPB_completion);
		self->ob_completion = value;
		Py_INCREF(value);
		rv = 1;
	} else if (strcmp(name, "interruptRoutine") == 0) {
		self->ob_spb.completionRoutine = NewSIInterruptProc(SPB_interrupt);
		self->ob_interrupt = value;
		Py_INCREF(value);
		rv = 1;
	}
	if ( rv ) return 0;
	else return -1;""")
			
	def outputGetattrHook(self):
		Output("""
			if (strcmp(name, "inRefNum") == 0)
				return Py_BuildValue("l", self->ob_spb.inRefNum);
			else if (strcmp(name, "count") == 0)
				return Py_BuildValue("l", self->ob_spb.count);
			else if (strcmp(name, "milliseconds") == 0)
				return Py_BuildValue("l", self->ob_spb.milliseconds);
			else if (strcmp(name, "error") == 0)
				return Py_BuildValue("h", self->ob_spb.error);""")
		
					

sndobject = SndObjectDefinition('SndChannel', 'SndCh', 'SndChannelPtr')
spbobject = SpbObjectDefinition('SPB', 'SPBObj', 'SPBPtr')
spbgenerator = ManualGenerator("SPB", "return SPBObj_New();")
module = MacModule('Snd', 'Snd', includestuff, finalstuff, initstuff)
module.addobject(sndobject)
module.addobject(spbobject)
module.add(spbgenerator)


# create lists of functions and object methods

functions = []
sndmethods = []


# populate the lists

execfile('sndgen.py')


# add the functions and methods to the module and object, respectively

for f in functions: module.add(f)
for f in sndmethods: sndobject.add(f)


# generate output

SetOutputFileName('Sndmodule.c')
module.generate()
