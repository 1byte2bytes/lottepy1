
/* =========================== Module Qd ============================ */

#include "Python.h"



#define SystemSevenOrLater 1

#include "macglue.h"
#include <Memory.h>
#include <Dialogs.h>
#include <Menus.h>
#include <Controls.h>

extern PyObject *ResObj_New(Handle);
extern int ResObj_Convert(PyObject *, Handle *);
extern PyObject *OptResObj_New(Handle);
extern int OptResObj_Convert(PyObject *, Handle *);

extern PyObject *WinObj_New(WindowPtr);
extern int WinObj_Convert(PyObject *, WindowPtr *);
extern PyTypeObject Window_Type;
#define WinObj_Check(x) ((x)->ob_type == &Window_Type)

extern PyObject *DlgObj_New(DialogPtr);
extern int DlgObj_Convert(PyObject *, DialogPtr *);
extern PyTypeObject Dialog_Type;
#define DlgObj_Check(x) ((x)->ob_type == &Dialog_Type)

extern PyObject *MenuObj_New(MenuHandle);
extern int MenuObj_Convert(PyObject *, MenuHandle *);

extern PyObject *CtlObj_New(ControlHandle);
extern int CtlObj_Convert(PyObject *, ControlHandle *);

extern PyObject *GrafObj_New(GrafPtr);
extern int GrafObj_Convert(PyObject *, GrafPtr *);

extern PyObject *BMObj_New(BitMapPtr);
extern int BMObj_Convert(PyObject *, BitMapPtr *);

extern PyObject *WinObj_WhichWindow(WindowPtr);

#include <QuickDraw.h>

#define resNotFound -192 /* Can't include <Errors.h> because of Python's "errors.h" */

/*
** Parse/generate RGB records
*/
PyObject *QdRGB_New(itself)
	RGBColorPtr itself;
{

	return Py_BuildValue("lll", (long)itself->red, (long)itself->green, (long)itself->blue);
}

QdRGB_Convert(v, p_itself)
	PyObject *v;
	RGBColorPtr p_itself;
{
	long red, green, blue;
	
	if( !PyArg_ParseTuple(v, "lll", &red, &green, &blue) )
		return 0;
	p_itself->red = (unsigned short)red;
	p_itself->green = (unsigned short)green;
	p_itself->blue = (unsigned short)blue;
	return 1;
}

/*
** Generate FontInfo records
*/
static
PyObject *QdFI_New(itself)
	FontInfo *itself;
{

	return Py_BuildValue("hhhh", itself->ascent, itself->descent,
			itself->widMax, itself->leading);
}



static PyObject *Qd_Error;

/* ---------------------- Object type GrafPort ---------------------- */

PyTypeObject GrafPort_Type;

#define GrafObj_Check(x) ((x)->ob_type == &GrafPort_Type)

typedef struct GrafPortObject {
	PyObject_HEAD
	GrafPtr ob_itself;
} GrafPortObject;

PyObject *GrafObj_New(itself)
	GrafPtr itself;
{
	GrafPortObject *it;
	if (itself == NULL) return PyMac_Error(resNotFound);
	it = PyObject_NEW(GrafPortObject, &GrafPort_Type);
	if (it == NULL) return NULL;
	it->ob_itself = itself;
	return (PyObject *)it;
}
GrafObj_Convert(v, p_itself)
	PyObject *v;
	GrafPtr *p_itself;
{
	if (DlgObj_Check(v) || WinObj_Check(v)) {
		*p_itself = ((GrafPortObject *)v)->ob_itself;
		return 1;
	}
	if (!GrafObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "GrafPort required");
		return 0;
	}
	*p_itself = ((GrafPortObject *)v)->ob_itself;
	return 1;
}

static void GrafObj_dealloc(self)
	GrafPortObject *self;
{
	/* Cleanup of self->ob_itself goes here */
	PyMem_DEL(self);
}

static PyMethodDef GrafObj_methods[] = {
	{NULL, NULL, 0}
};

PyMethodChain GrafObj_chain = { GrafObj_methods, NULL };

static PyObject *GrafObj_getattr(self, name)
	GrafPortObject *self;
	char *name;
{

			{	CGrafPtr itself_color = (CGrafPtr)self->ob_itself;
			
				if ( strcmp(name, "data") == 0 )
					return PyString_FromStringAndSize((char *)self->ob_itself, sizeof(GrafPort));
					
				if ( (itself_color->portVersion&0xc000) == 0xc000 ) {
					/* Color-only attributes */
				
					if ( strcmp(name, "portBits") == 0 )
						/* XXXX Do we need HLock() stuff here?? */
						return BMObj_New((BitMapPtr)*itself_color->portPixMap);
					if ( strcmp(name, "grafVars") == 0 )
						return Py_BuildValue("O&", ResObj_New, (Handle)itself_color->visRgn);
					if ( strcmp(name, "chExtra") == 0 )
						return Py_BuildValue("h", itself_color->chExtra);
					if ( strcmp(name, "pnLocHFrac") == 0 )
						return Py_BuildValue("h", itself_color->pnLocHFrac);
					if ( strcmp(name, "bkPixPat") == 0 )
						return Py_BuildValue("O&", ResObj_New, (Handle)itself_color->bkPixPat);
					if ( strcmp(name, "rgbFgColor") == 0 )
						return Py_BuildValue("O&", QdRGB_New, &itself_color->rgbFgColor);
					if ( strcmp(name, "rgbBkColor") == 0 )
						return Py_BuildValue("O&", QdRGB_New, &itself_color->rgbBkColor);
					if ( strcmp(name, "pnPixPat") == 0 )
						return Py_BuildValue("O&", ResObj_New, (Handle)itself_color->pnPixPat);
					if ( strcmp(name, "fillPixPat") == 0 )
						return Py_BuildValue("O&", ResObj_New, (Handle)itself_color->fillPixPat);
				} else {
					/* Mono-only attributes */
					if ( strcmp(name, "portBits") == 0 )
						return BMObj_New(&self->ob_itself->portBits);
					if ( strcmp(name, "bkPat") == 0 )
						return Py_BuildValue("s#", (char *)&self->ob_itself->bkPat, sizeof(Pattern));
					if ( strcmp(name, "fillPat") == 0 )
						return Py_BuildValue("s#", (char *)&self->ob_itself->fillPat, sizeof(Pattern));
					if ( strcmp(name, "pnPat") == 0 )
						return Py_BuildValue("s#", (char *)&self->ob_itself->pnPat, sizeof(Pattern));
				}
				/*
				** Accessible for both color/mono windows.
				** portVersion is really color-only, but we put it here
				** for convenience
				*/
				if ( strcmp(name, "portVersion") == 0 )
					return Py_BuildValue("h", itself_color->portVersion);
				if ( strcmp(name, "device") == 0 )
					return PyInt_FromLong((long)self->ob_itself->device);
				if ( strcmp(name, "portRect") == 0 )
					return Py_BuildValue("O&", PyMac_BuildRect, &self->ob_itself->portRect);
				if ( strcmp(name, "visRgn") == 0 )
					return Py_BuildValue("O&", ResObj_New, (Handle)self->ob_itself->visRgn);
				if ( strcmp(name, "clipRgn") == 0 )
					return Py_BuildValue("O&", ResObj_New, (Handle)self->ob_itself->clipRgn);
				if ( strcmp(name, "pnLoc") == 0 )
					return Py_BuildValue("O&", PyMac_BuildPoint, self->ob_itself->pnLoc);
				if ( strcmp(name, "pnSize") == 0 )
					return Py_BuildValue("O&", PyMac_BuildPoint, self->ob_itself->pnSize);
				if ( strcmp(name, "pnMode") == 0 )
					return Py_BuildValue("h", self->ob_itself->pnMode);
				if ( strcmp(name, "pnVis") == 0 )
					return Py_BuildValue("h", self->ob_itself->pnVis);
				if ( strcmp(name, "txFont") == 0 )
					return Py_BuildValue("h", self->ob_itself->txFont);
				if ( strcmp(name, "txFace") == 0 )
					return Py_BuildValue("h", (short)self->ob_itself->txFace);
				if ( strcmp(name, "txMode") == 0 )
					return Py_BuildValue("h", self->ob_itself->txMode);
				if ( strcmp(name, "txSize") == 0 )
					return Py_BuildValue("h", self->ob_itself->txSize);
				if ( strcmp(name, "spExtra") == 0 )
					return Py_BuildValue("O&", PyMac_BuildFixed, self->ob_itself->spExtra);
				/* XXXX Add more, as needed */
				/* This one is so we can compare grafports: */
				if ( strcmp(name, "_id") == 0 )
					return Py_BuildValue("l", (long)self->ob_itself);
			}
	return Py_FindMethodInChain(&GrafObj_chain, (PyObject *)self, name);
}

#define GrafObj_setattr NULL

PyTypeObject GrafPort_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0, /*ob_size*/
	"GrafPort", /*tp_name*/
	sizeof(GrafPortObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) GrafObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc) GrafObj_getattr, /*tp_getattr*/
	(setattrfunc) GrafObj_setattr, /*tp_setattr*/
};

/* -------------------- End object type GrafPort -------------------- */


/* ----------------------- Object type BitMap ----------------------- */

PyTypeObject BitMap_Type;

#define BMObj_Check(x) ((x)->ob_type == &BitMap_Type)

typedef struct BitMapObject {
	PyObject_HEAD
	BitMapPtr ob_itself;
	PyObject *referred_object;
	BitMap *referred_bitmap;
} BitMapObject;

PyObject *BMObj_New(itself)
	BitMapPtr itself;
{
	BitMapObject *it;
	if (itself == NULL) return PyMac_Error(resNotFound);
	it = PyObject_NEW(BitMapObject, &BitMap_Type);
	if (it == NULL) return NULL;
	it->ob_itself = itself;
	it->referred_object = NULL;
	it->referred_bitmap = NULL;
	return (PyObject *)it;
}
BMObj_Convert(v, p_itself)
	PyObject *v;
	BitMapPtr *p_itself;
{
	if (!BMObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "BitMap required");
		return 0;
	}
	*p_itself = ((BitMapObject *)v)->ob_itself;
	return 1;
}

static void BMObj_dealloc(self)
	BitMapObject *self;
{
	Py_XDECREF(self->referred_object);
	if (self->referred_bitmap) free(self->referred_bitmap);
	PyMem_DEL(self);
}

static PyMethodDef BMObj_methods[] = {
	{NULL, NULL, 0}
};

PyMethodChain BMObj_chain = { BMObj_methods, NULL };

static PyObject *BMObj_getattr(self, name)
	BitMapObject *self;
	char *name;
{
	if ( strcmp(name, "baseAddr") == 0 )
				return PyInt_FromLong((long)self->ob_itself->baseAddr);
			if ( strcmp(name, "rowBytes") == 0 )
				return PyInt_FromLong((long)self->ob_itself->rowBytes);
			if ( strcmp(name, "bounds") == 0 )
				return Py_BuildValue("O&", PyMac_BuildRect, &self->ob_itself->bounds);
			/* XXXX Add more, as needed */
			if ( strcmp(name, "bitmap_data") == 0 )
				return PyString_FromStringAndSize((char *)self->ob_itself, sizeof(BitMap));
			if ( strcmp(name, "pixmap_data") == 0 )
				return PyString_FromStringAndSize((char *)self->ob_itself, sizeof(PixMap));
			
	return Py_FindMethodInChain(&BMObj_chain, (PyObject *)self, name);
}

#define BMObj_setattr NULL

PyTypeObject BitMap_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0, /*ob_size*/
	"BitMap", /*tp_name*/
	sizeof(BitMapObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) BMObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc) BMObj_getattr, /*tp_getattr*/
	(setattrfunc) BMObj_setattr, /*tp_setattr*/
};

/* --------------------- End object type BitMap --------------------- */


/* ------------------ Object type QDGlobalsAccess ------------------- */

staticforward PyTypeObject QDGlobalsAccess_Type;

#define QDGA_Check(x) ((x)->ob_type == &QDGlobalsAccess_Type)

typedef struct QDGlobalsAccessObject {
	PyObject_HEAD
} QDGlobalsAccessObject;

static PyObject *QDGA_New()
{
	QDGlobalsAccessObject *it;
	it = PyObject_NEW(QDGlobalsAccessObject, &QDGlobalsAccess_Type);
	if (it == NULL) return NULL;
	return (PyObject *)it;
}

static void QDGA_dealloc(self)
	QDGlobalsAccessObject *self;
{
	PyMem_DEL(self);
}

static PyMethodDef QDGA_methods[] = {
	{NULL, NULL, 0}
};

static PyMethodChain QDGA_chain = { QDGA_methods, NULL };

static PyObject *QDGA_getattr(self, name)
	QDGlobalsAccessObject *self;
	char *name;
{

		if ( strcmp(name, "arrow") == 0 )
			return PyString_FromStringAndSize((char *)&qd.arrow, sizeof(qd.arrow));
		if ( strcmp(name, "black") == 0 ) 
			return PyString_FromStringAndSize((char *)&qd.black, sizeof(qd.black));
		if ( strcmp(name, "white") == 0 ) 
			return PyString_FromStringAndSize((char *)&qd.white, sizeof(qd.white));
		if ( strcmp(name, "gray") == 0 ) 
			return PyString_FromStringAndSize((char *)&qd.gray, sizeof(qd.gray));
		if ( strcmp(name, "ltGray") == 0 ) 
			return PyString_FromStringAndSize((char *)&qd.ltGray, sizeof(qd.ltGray));
		if ( strcmp(name, "dkGray") == 0 ) 
			return PyString_FromStringAndSize((char *)&qd.dkGray, sizeof(qd.dkGray));
		if ( strcmp(name, "screenBits") == 0 ) 
			return BMObj_New(&qd.screenBits);
		if ( strcmp(name, "thePort") == 0 ) 
			return GrafObj_New(qd.thePort);
		if ( strcmp(name, "randSeed") == 0 ) 
			return Py_BuildValue("l", &qd.randSeed);
			
	return Py_FindMethodInChain(&QDGA_chain, (PyObject *)self, name);
}

#define QDGA_setattr NULL

staticforward PyTypeObject QDGlobalsAccess_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0, /*ob_size*/
	"QDGlobalsAccess", /*tp_name*/
	sizeof(QDGlobalsAccessObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) QDGA_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc) QDGA_getattr, /*tp_getattr*/
	(setattrfunc) QDGA_setattr, /*tp_setattr*/
};

/* ---------------- End object type QDGlobalsAccess ----------------- */


static PyObject *Qd_MacSetPort(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	GrafPtr port;
	if (!PyArg_ParseTuple(_args, "O&",
	                      GrafObj_Convert, &port))
		return NULL;
	MacSetPort(port);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_GetPort(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	GrafPtr port;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	GetPort(&port);
	_res = Py_BuildValue("O&",
	                     GrafObj_New, port);
	return _res;
}

static PyObject *Qd_GrafDevice(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short device;
	if (!PyArg_ParseTuple(_args, "h",
	                      &device))
		return NULL;
	GrafDevice(device);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_SetPortBits(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	BitMapPtr bm;
	if (!PyArg_ParseTuple(_args, "O&",
	                      BMObj_Convert, &bm))
		return NULL;
	SetPortBits(bm);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_PortSize(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short width;
	short height;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &width,
	                      &height))
		return NULL;
	PortSize(width,
	         height);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MovePortTo(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short leftGlobal;
	short topGlobal;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &leftGlobal,
	                      &topGlobal))
		return NULL;
	MovePortTo(leftGlobal,
	           topGlobal);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_SetOrigin(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short h;
	short v;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &h,
	                      &v))
		return NULL;
	SetOrigin(h,
	          v);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_SetClip(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &rgn))
		return NULL;
	SetClip(rgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_GetClip(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &rgn))
		return NULL;
	GetClip(rgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_ClipRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetRect, &r))
		return NULL;
	ClipRect(&r);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_BackPat(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Pattern *pat__in__;
	int pat__in_len__;
	if (!PyArg_ParseTuple(_args, "s#",
	                      (char **)&pat__in__, &pat__in_len__))
		return NULL;
	if (pat__in_len__ != sizeof(Pattern))
	{
		PyErr_SetString(PyExc_TypeError, "buffer length should be sizeof(Pattern)");
		goto pat__error__;
	}
	BackPat(pat__in__);
	Py_INCREF(Py_None);
	_res = Py_None;
 pat__error__: ;
	return _res;
}

static PyObject *Qd_InitCursor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	InitCursor();
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MacSetCursor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Cursor *crsr__in__;
	int crsr__in_len__;
	if (!PyArg_ParseTuple(_args, "s#",
	                      (char **)&crsr__in__, &crsr__in_len__))
		return NULL;
	if (crsr__in_len__ != sizeof(Cursor))
	{
		PyErr_SetString(PyExc_TypeError, "buffer length should be sizeof(Cursor)");
		goto crsr__error__;
	}
	MacSetCursor(crsr__in__);
	Py_INCREF(Py_None);
	_res = Py_None;
 crsr__error__: ;
	return _res;
}

static PyObject *Qd_HideCursor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	HideCursor();
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MacShowCursor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	MacShowCursor();
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_ObscureCursor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	ObscureCursor();
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_HidePen(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	HidePen();
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_ShowPen(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	ShowPen();
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_GetPen(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Point pt;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	GetPen(&pt);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildPoint, pt);
	return _res;
}

static PyObject *Qd_GetPenState(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PenState pnState__out__;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	GetPenState(&pnState__out__);
	_res = Py_BuildValue("s#",
	                     (char *)&pnState__out__, (int)sizeof(PenState));
 pnState__error__: ;
	return _res;
}

static PyObject *Qd_SetPenState(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PenState *pnState__in__;
	int pnState__in_len__;
	if (!PyArg_ParseTuple(_args, "s#",
	                      (char **)&pnState__in__, &pnState__in_len__))
		return NULL;
	if (pnState__in_len__ != sizeof(PenState))
	{
		PyErr_SetString(PyExc_TypeError, "buffer length should be sizeof(PenState)");
		goto pnState__error__;
	}
	SetPenState(pnState__in__);
	Py_INCREF(Py_None);
	_res = Py_None;
 pnState__error__: ;
	return _res;
}

static PyObject *Qd_PenSize(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short width;
	short height;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &width,
	                      &height))
		return NULL;
	PenSize(width,
	        height);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_PenMode(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short mode;
	if (!PyArg_ParseTuple(_args, "h",
	                      &mode))
		return NULL;
	PenMode(mode);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_PenPat(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Pattern *pat__in__;
	int pat__in_len__;
	if (!PyArg_ParseTuple(_args, "s#",
	                      (char **)&pat__in__, &pat__in_len__))
		return NULL;
	if (pat__in_len__ != sizeof(Pattern))
	{
		PyErr_SetString(PyExc_TypeError, "buffer length should be sizeof(Pattern)");
		goto pat__error__;
	}
	PenPat(pat__in__);
	Py_INCREF(Py_None);
	_res = Py_None;
 pat__error__: ;
	return _res;
}

static PyObject *Qd_PenNormal(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PenNormal();
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MoveTo(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short h;
	short v;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &h,
	                      &v))
		return NULL;
	MoveTo(h,
	       v);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_Move(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short dh;
	short dv;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &dh,
	                      &dv))
		return NULL;
	Move(dh,
	     dv);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MacLineTo(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short h;
	short v;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &h,
	                      &v))
		return NULL;
	MacLineTo(h,
	          v);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_Line(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short dh;
	short dv;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &dh,
	                      &dv))
		return NULL;
	Line(dh,
	     dv);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_ForeColor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	long color;
	if (!PyArg_ParseTuple(_args, "l",
	                      &color))
		return NULL;
	ForeColor(color);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_BackColor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	long color;
	if (!PyArg_ParseTuple(_args, "l",
	                      &color))
		return NULL;
	BackColor(color);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_ColorBit(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short whichBit;
	if (!PyArg_ParseTuple(_args, "h",
	                      &whichBit))
		return NULL;
	ColorBit(whichBit);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MacSetRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short left;
	short top;
	short right;
	short bottom;
	if (!PyArg_ParseTuple(_args, "hhhh",
	                      &left,
	                      &top,
	                      &right,
	                      &bottom))
		return NULL;
	MacSetRect(&r,
	           left,
	           top,
	           right,
	           bottom);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildRect, &r);
	return _res;
}

static PyObject *Qd_MacOffsetRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short dh;
	short dv;
	if (!PyArg_ParseTuple(_args, "O&hh",
	                      PyMac_GetRect, &r,
	                      &dh,
	                      &dv))
		return NULL;
	MacOffsetRect(&r,
	              dh,
	              dv);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildRect, &r);
	return _res;
}

static PyObject *Qd_MacInsetRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short dh;
	short dv;
	if (!PyArg_ParseTuple(_args, "O&hh",
	                      PyMac_GetRect, &r,
	                      &dh,
	                      &dv))
		return NULL;
	MacInsetRect(&r,
	             dh,
	             dv);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildRect, &r);
	return _res;
}

static PyObject *Qd_SectRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	Rect src1;
	Rect src2;
	Rect dstRect;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetRect, &src1,
	                      PyMac_GetRect, &src2))
		return NULL;
	_rv = SectRect(&src1,
	               &src2,
	               &dstRect);
	_res = Py_BuildValue("bO&",
	                     _rv,
	                     PyMac_BuildRect, &dstRect);
	return _res;
}

static PyObject *Qd_MacUnionRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect src1;
	Rect src2;
	Rect dstRect;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetRect, &src1,
	                      PyMac_GetRect, &src2))
		return NULL;
	MacUnionRect(&src1,
	             &src2,
	             &dstRect);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildRect, &dstRect);
	return _res;
}

static PyObject *Qd_MacEqualRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	Rect rect1;
	Rect rect2;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetRect, &rect1,
	                      PyMac_GetRect, &rect2))
		return NULL;
	_rv = MacEqualRect(&rect1,
	                   &rect2);
	_res = Py_BuildValue("b",
	                     _rv);
	return _res;
}

static PyObject *Qd_EmptyRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	Rect r;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetRect, &r))
		return NULL;
	_rv = EmptyRect(&r);
	_res = Py_BuildValue("b",
	                     _rv);
	return _res;
}

static PyObject *Qd_MacFrameRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetRect, &r))
		return NULL;
	MacFrameRect(&r);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_PaintRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetRect, &r))
		return NULL;
	PaintRect(&r);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_EraseRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetRect, &r))
		return NULL;
	EraseRect(&r);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MacInvertRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetRect, &r))
		return NULL;
	MacInvertRect(&r);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MacFillRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	Pattern *pat__in__;
	int pat__in_len__;
	if (!PyArg_ParseTuple(_args, "O&s#",
	                      PyMac_GetRect, &r,
	                      (char **)&pat__in__, &pat__in_len__))
		return NULL;
	if (pat__in_len__ != sizeof(Pattern))
	{
		PyErr_SetString(PyExc_TypeError, "buffer length should be sizeof(Pattern)");
		goto pat__error__;
	}
	MacFillRect(&r,
	            pat__in__);
	Py_INCREF(Py_None);
	_res = Py_None;
 pat__error__: ;
	return _res;
}

static PyObject *Qd_FrameOval(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetRect, &r))
		return NULL;
	FrameOval(&r);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_PaintOval(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetRect, &r))
		return NULL;
	PaintOval(&r);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_EraseOval(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetRect, &r))
		return NULL;
	EraseOval(&r);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_InvertOval(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetRect, &r))
		return NULL;
	InvertOval(&r);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_FillOval(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	Pattern *pat__in__;
	int pat__in_len__;
	if (!PyArg_ParseTuple(_args, "O&s#",
	                      PyMac_GetRect, &r,
	                      (char **)&pat__in__, &pat__in_len__))
		return NULL;
	if (pat__in_len__ != sizeof(Pattern))
	{
		PyErr_SetString(PyExc_TypeError, "buffer length should be sizeof(Pattern)");
		goto pat__error__;
	}
	FillOval(&r,
	         pat__in__);
	Py_INCREF(Py_None);
	_res = Py_None;
 pat__error__: ;
	return _res;
}

static PyObject *Qd_FrameRoundRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short ovalWidth;
	short ovalHeight;
	if (!PyArg_ParseTuple(_args, "O&hh",
	                      PyMac_GetRect, &r,
	                      &ovalWidth,
	                      &ovalHeight))
		return NULL;
	FrameRoundRect(&r,
	               ovalWidth,
	               ovalHeight);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_PaintRoundRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short ovalWidth;
	short ovalHeight;
	if (!PyArg_ParseTuple(_args, "O&hh",
	                      PyMac_GetRect, &r,
	                      &ovalWidth,
	                      &ovalHeight))
		return NULL;
	PaintRoundRect(&r,
	               ovalWidth,
	               ovalHeight);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_EraseRoundRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short ovalWidth;
	short ovalHeight;
	if (!PyArg_ParseTuple(_args, "O&hh",
	                      PyMac_GetRect, &r,
	                      &ovalWidth,
	                      &ovalHeight))
		return NULL;
	EraseRoundRect(&r,
	               ovalWidth,
	               ovalHeight);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_InvertRoundRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short ovalWidth;
	short ovalHeight;
	if (!PyArg_ParseTuple(_args, "O&hh",
	                      PyMac_GetRect, &r,
	                      &ovalWidth,
	                      &ovalHeight))
		return NULL;
	InvertRoundRect(&r,
	                ovalWidth,
	                ovalHeight);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_FillRoundRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short ovalWidth;
	short ovalHeight;
	Pattern *pat__in__;
	int pat__in_len__;
	if (!PyArg_ParseTuple(_args, "O&hhs#",
	                      PyMac_GetRect, &r,
	                      &ovalWidth,
	                      &ovalHeight,
	                      (char **)&pat__in__, &pat__in_len__))
		return NULL;
	if (pat__in_len__ != sizeof(Pattern))
	{
		PyErr_SetString(PyExc_TypeError, "buffer length should be sizeof(Pattern)");
		goto pat__error__;
	}
	FillRoundRect(&r,
	              ovalWidth,
	              ovalHeight,
	              pat__in__);
	Py_INCREF(Py_None);
	_res = Py_None;
 pat__error__: ;
	return _res;
}

static PyObject *Qd_FrameArc(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short startAngle;
	short arcAngle;
	if (!PyArg_ParseTuple(_args, "O&hh",
	                      PyMac_GetRect, &r,
	                      &startAngle,
	                      &arcAngle))
		return NULL;
	FrameArc(&r,
	         startAngle,
	         arcAngle);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_PaintArc(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short startAngle;
	short arcAngle;
	if (!PyArg_ParseTuple(_args, "O&hh",
	                      PyMac_GetRect, &r,
	                      &startAngle,
	                      &arcAngle))
		return NULL;
	PaintArc(&r,
	         startAngle,
	         arcAngle);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_EraseArc(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short startAngle;
	short arcAngle;
	if (!PyArg_ParseTuple(_args, "O&hh",
	                      PyMac_GetRect, &r,
	                      &startAngle,
	                      &arcAngle))
		return NULL;
	EraseArc(&r,
	         startAngle,
	         arcAngle);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_InvertArc(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short startAngle;
	short arcAngle;
	if (!PyArg_ParseTuple(_args, "O&hh",
	                      PyMac_GetRect, &r,
	                      &startAngle,
	                      &arcAngle))
		return NULL;
	InvertArc(&r,
	          startAngle,
	          arcAngle);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_FillArc(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short startAngle;
	short arcAngle;
	Pattern *pat__in__;
	int pat__in_len__;
	if (!PyArg_ParseTuple(_args, "O&hhs#",
	                      PyMac_GetRect, &r,
	                      &startAngle,
	                      &arcAngle,
	                      (char **)&pat__in__, &pat__in_len__))
		return NULL;
	if (pat__in_len__ != sizeof(Pattern))
	{
		PyErr_SetString(PyExc_TypeError, "buffer length should be sizeof(Pattern)");
		goto pat__error__;
	}
	FillArc(&r,
	        startAngle,
	        arcAngle,
	        pat__in__);
	Py_INCREF(Py_None);
	_res = Py_None;
 pat__error__: ;
	return _res;
}

static PyObject *Qd_NewRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_rv = NewRgn();
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_OpenRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	OpenRgn();
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_CloseRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle dstRgn;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &dstRgn))
		return NULL;
	CloseRgn(dstRgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_BitMapToRegion(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	OSErr _err;
	RgnHandle region;
	BitMapPtr bMap;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      ResObj_Convert, &region,
	                      BMObj_Convert, &bMap))
		return NULL;
	_err = BitMapToRegion(region,
	                      bMap);
	if (_err != noErr) return PyMac_Error(_err);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_DisposeRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &rgn))
		return NULL;
	DisposeRgn(rgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MacCopyRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle srcRgn;
	RgnHandle dstRgn;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      ResObj_Convert, &srcRgn,
	                      ResObj_Convert, &dstRgn))
		return NULL;
	MacCopyRgn(srcRgn,
	           dstRgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_SetEmptyRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &rgn))
		return NULL;
	SetEmptyRgn(rgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MacSetRectRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	short left;
	short top;
	short right;
	short bottom;
	if (!PyArg_ParseTuple(_args, "O&hhhh",
	                      ResObj_Convert, &rgn,
	                      &left,
	                      &top,
	                      &right,
	                      &bottom))
		return NULL;
	MacSetRectRgn(rgn,
	              left,
	              top,
	              right,
	              bottom);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_RectRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	Rect r;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      ResObj_Convert, &rgn,
	                      PyMac_GetRect, &r))
		return NULL;
	RectRgn(rgn,
	        &r);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MacOffsetRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	short dh;
	short dv;
	if (!PyArg_ParseTuple(_args, "O&hh",
	                      ResObj_Convert, &rgn,
	                      &dh,
	                      &dv))
		return NULL;
	MacOffsetRgn(rgn,
	             dh,
	             dv);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_InsetRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	short dh;
	short dv;
	if (!PyArg_ParseTuple(_args, "O&hh",
	                      ResObj_Convert, &rgn,
	                      &dh,
	                      &dv))
		return NULL;
	InsetRgn(rgn,
	         dh,
	         dv);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_SectRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle srcRgnA;
	RgnHandle srcRgnB;
	RgnHandle dstRgn;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      ResObj_Convert, &srcRgnA,
	                      ResObj_Convert, &srcRgnB,
	                      ResObj_Convert, &dstRgn))
		return NULL;
	SectRgn(srcRgnA,
	        srcRgnB,
	        dstRgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MacUnionRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle srcRgnA;
	RgnHandle srcRgnB;
	RgnHandle dstRgn;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      ResObj_Convert, &srcRgnA,
	                      ResObj_Convert, &srcRgnB,
	                      ResObj_Convert, &dstRgn))
		return NULL;
	MacUnionRgn(srcRgnA,
	            srcRgnB,
	            dstRgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_DiffRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle srcRgnA;
	RgnHandle srcRgnB;
	RgnHandle dstRgn;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      ResObj_Convert, &srcRgnA,
	                      ResObj_Convert, &srcRgnB,
	                      ResObj_Convert, &dstRgn))
		return NULL;
	DiffRgn(srcRgnA,
	        srcRgnB,
	        dstRgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MacXorRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle srcRgnA;
	RgnHandle srcRgnB;
	RgnHandle dstRgn;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      ResObj_Convert, &srcRgnA,
	                      ResObj_Convert, &srcRgnB,
	                      ResObj_Convert, &dstRgn))
		return NULL;
	MacXorRgn(srcRgnA,
	          srcRgnB,
	          dstRgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_RectInRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	Rect r;
	RgnHandle rgn;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetRect, &r,
	                      ResObj_Convert, &rgn))
		return NULL;
	_rv = RectInRgn(&r,
	                rgn);
	_res = Py_BuildValue("b",
	                     _rv);
	return _res;
}

static PyObject *Qd_MacEqualRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	RgnHandle rgnA;
	RgnHandle rgnB;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      ResObj_Convert, &rgnA,
	                      ResObj_Convert, &rgnB))
		return NULL;
	_rv = MacEqualRgn(rgnA,
	                  rgnB);
	_res = Py_BuildValue("b",
	                     _rv);
	return _res;
}

static PyObject *Qd_EmptyRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	RgnHandle rgn;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &rgn))
		return NULL;
	_rv = EmptyRgn(rgn);
	_res = Py_BuildValue("b",
	                     _rv);
	return _res;
}

static PyObject *Qd_MacFrameRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &rgn))
		return NULL;
	MacFrameRgn(rgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MacPaintRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &rgn))
		return NULL;
	MacPaintRgn(rgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_EraseRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &rgn))
		return NULL;
	EraseRgn(rgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MacInvertRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &rgn))
		return NULL;
	MacInvertRgn(rgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MacFillRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	Pattern *pat__in__;
	int pat__in_len__;
	if (!PyArg_ParseTuple(_args, "O&s#",
	                      ResObj_Convert, &rgn,
	                      (char **)&pat__in__, &pat__in_len__))
		return NULL;
	if (pat__in_len__ != sizeof(Pattern))
	{
		PyErr_SetString(PyExc_TypeError, "buffer length should be sizeof(Pattern)");
		goto pat__error__;
	}
	MacFillRgn(rgn,
	           pat__in__);
	Py_INCREF(Py_None);
	_res = Py_None;
 pat__error__: ;
	return _res;
}

static PyObject *Qd_ScrollRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short dh;
	short dv;
	RgnHandle updateRgn;
	if (!PyArg_ParseTuple(_args, "O&hhO&",
	                      PyMac_GetRect, &r,
	                      &dh,
	                      &dv,
	                      ResObj_Convert, &updateRgn))
		return NULL;
	ScrollRect(&r,
	           dh,
	           dv,
	           updateRgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_CopyBits(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	BitMapPtr srcBits;
	BitMapPtr dstBits;
	Rect srcRect;
	Rect dstRect;
	short mode;
	RgnHandle maskRgn;
	if (!PyArg_ParseTuple(_args, "O&O&O&O&hO&",
	                      BMObj_Convert, &srcBits,
	                      BMObj_Convert, &dstBits,
	                      PyMac_GetRect, &srcRect,
	                      PyMac_GetRect, &dstRect,
	                      &mode,
	                      OptResObj_Convert, &maskRgn))
		return NULL;
	CopyBits(srcBits,
	         dstBits,
	         &srcRect,
	         &dstRect,
	         mode,
	         maskRgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_CopyMask(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	BitMapPtr srcBits;
	BitMapPtr maskBits;
	BitMapPtr dstBits;
	Rect srcRect;
	Rect maskRect;
	Rect dstRect;
	if (!PyArg_ParseTuple(_args, "O&O&O&O&O&O&",
	                      BMObj_Convert, &srcBits,
	                      BMObj_Convert, &maskBits,
	                      BMObj_Convert, &dstBits,
	                      PyMac_GetRect, &srcRect,
	                      PyMac_GetRect, &maskRect,
	                      PyMac_GetRect, &dstRect))
		return NULL;
	CopyMask(srcBits,
	         maskBits,
	         dstBits,
	         &srcRect,
	         &maskRect,
	         &dstRect);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_OpenPicture(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PicHandle _rv;
	Rect picFrame;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetRect, &picFrame))
		return NULL;
	_rv = OpenPicture(&picFrame);
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_PicComment(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short kind;
	short dataSize;
	Handle dataHandle;
	if (!PyArg_ParseTuple(_args, "hhO&",
	                      &kind,
	                      &dataSize,
	                      ResObj_Convert, &dataHandle))
		return NULL;
	PicComment(kind,
	           dataSize,
	           dataHandle);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_ClosePicture(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	ClosePicture();
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_DrawPicture(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PicHandle myPicture;
	Rect dstRect;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      ResObj_Convert, &myPicture,
	                      PyMac_GetRect, &dstRect))
		return NULL;
	DrawPicture(myPicture,
	            &dstRect);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_KillPicture(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PicHandle myPicture;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &myPicture))
		return NULL;
	KillPicture(myPicture);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_OpenPoly(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PolyHandle _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_rv = OpenPoly();
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_ClosePoly(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	ClosePoly();
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_KillPoly(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PolyHandle poly;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &poly))
		return NULL;
	KillPoly(poly);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_OffsetPoly(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PolyHandle poly;
	short dh;
	short dv;
	if (!PyArg_ParseTuple(_args, "O&hh",
	                      ResObj_Convert, &poly,
	                      &dh,
	                      &dv))
		return NULL;
	OffsetPoly(poly,
	           dh,
	           dv);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_FramePoly(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PolyHandle poly;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &poly))
		return NULL;
	FramePoly(poly);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_PaintPoly(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PolyHandle poly;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &poly))
		return NULL;
	PaintPoly(poly);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_ErasePoly(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PolyHandle poly;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &poly))
		return NULL;
	ErasePoly(poly);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_InvertPoly(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PolyHandle poly;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &poly))
		return NULL;
	InvertPoly(poly);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_FillPoly(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PolyHandle poly;
	Pattern *pat__in__;
	int pat__in_len__;
	if (!PyArg_ParseTuple(_args, "O&s#",
	                      ResObj_Convert, &poly,
	                      (char **)&pat__in__, &pat__in_len__))
		return NULL;
	if (pat__in_len__ != sizeof(Pattern))
	{
		PyErr_SetString(PyExc_TypeError, "buffer length should be sizeof(Pattern)");
		goto pat__error__;
	}
	FillPoly(poly,
	         pat__in__);
	Py_INCREF(Py_None);
	_res = Py_None;
 pat__error__: ;
	return _res;
}

static PyObject *Qd_SetPt(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Point pt;
	short h;
	short v;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &h,
	                      &v))
		return NULL;
	SetPt(&pt,
	      h,
	      v);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildPoint, pt);
	return _res;
}

static PyObject *Qd_LocalToGlobal(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Point pt;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetPoint, &pt))
		return NULL;
	LocalToGlobal(&pt);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildPoint, pt);
	return _res;
}

static PyObject *Qd_GlobalToLocal(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Point pt;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetPoint, &pt))
		return NULL;
	GlobalToLocal(&pt);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildPoint, pt);
	return _res;
}

static PyObject *Qd_Random(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_rv = Random();
	_res = Py_BuildValue("h",
	                     _rv);
	return _res;
}

static PyObject *Qd_MacGetPixel(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	short h;
	short v;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &h,
	                      &v))
		return NULL;
	_rv = MacGetPixel(h,
	                  v);
	_res = Py_BuildValue("b",
	                     _rv);
	return _res;
}

static PyObject *Qd_ScalePt(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Point pt;
	Rect srcRect;
	Rect dstRect;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      PyMac_GetPoint, &pt,
	                      PyMac_GetRect, &srcRect,
	                      PyMac_GetRect, &dstRect))
		return NULL;
	ScalePt(&pt,
	        &srcRect,
	        &dstRect);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildPoint, pt);
	return _res;
}

static PyObject *Qd_MapPt(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Point pt;
	Rect srcRect;
	Rect dstRect;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      PyMac_GetPoint, &pt,
	                      PyMac_GetRect, &srcRect,
	                      PyMac_GetRect, &dstRect))
		return NULL;
	MapPt(&pt,
	      &srcRect,
	      &dstRect);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildPoint, pt);
	return _res;
}

static PyObject *Qd_MapRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	Rect srcRect;
	Rect dstRect;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      PyMac_GetRect, &r,
	                      PyMac_GetRect, &srcRect,
	                      PyMac_GetRect, &dstRect))
		return NULL;
	MapRect(&r,
	        &srcRect,
	        &dstRect);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildRect, &r);
	return _res;
}

static PyObject *Qd_MapRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	Rect srcRect;
	Rect dstRect;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      ResObj_Convert, &rgn,
	                      PyMac_GetRect, &srcRect,
	                      PyMac_GetRect, &dstRect))
		return NULL;
	MapRgn(rgn,
	       &srcRect,
	       &dstRect);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MapPoly(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PolyHandle poly;
	Rect srcRect;
	Rect dstRect;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      ResObj_Convert, &poly,
	                      PyMac_GetRect, &srcRect,
	                      PyMac_GetRect, &dstRect))
		return NULL;
	MapPoly(poly,
	        &srcRect,
	        &dstRect);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_StdBits(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	BitMapPtr srcBits;
	Rect srcRect;
	Rect dstRect;
	short mode;
	RgnHandle maskRgn;
	if (!PyArg_ParseTuple(_args, "O&O&O&hO&",
	                      BMObj_Convert, &srcBits,
	                      PyMac_GetRect, &srcRect,
	                      PyMac_GetRect, &dstRect,
	                      &mode,
	                      OptResObj_Convert, &maskRgn))
		return NULL;
	StdBits(srcBits,
	        &srcRect,
	        &dstRect,
	        mode,
	        maskRgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_AddPt(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Point src;
	Point dst;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetPoint, &src,
	                      PyMac_GetPoint, &dst))
		return NULL;
	AddPt(src,
	      &dst);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildPoint, dst);
	return _res;
}

static PyObject *Qd_EqualPt(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	Point pt1;
	Point pt2;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetPoint, &pt1,
	                      PyMac_GetPoint, &pt2))
		return NULL;
	_rv = EqualPt(pt1,
	              pt2);
	_res = Py_BuildValue("b",
	                     _rv);
	return _res;
}

static PyObject *Qd_MacPtInRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	Point pt;
	Rect r;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetPoint, &pt,
	                      PyMac_GetRect, &r))
		return NULL;
	_rv = MacPtInRect(pt,
	                  &r);
	_res = Py_BuildValue("b",
	                     _rv);
	return _res;
}

static PyObject *Qd_Pt2Rect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Point pt1;
	Point pt2;
	Rect dstRect;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetPoint, &pt1,
	                      PyMac_GetPoint, &pt2))
		return NULL;
	Pt2Rect(pt1,
	        pt2,
	        &dstRect);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildRect, &dstRect);
	return _res;
}

static PyObject *Qd_PtToAngle(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	Point pt;
	short angle;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetRect, &r,
	                      PyMac_GetPoint, &pt))
		return NULL;
	PtToAngle(&r,
	          pt,
	          &angle);
	_res = Py_BuildValue("h",
	                     angle);
	return _res;
}

static PyObject *Qd_SubPt(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Point src;
	Point dst;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetPoint, &src,
	                      PyMac_GetPoint, &dst))
		return NULL;
	SubPt(src,
	      &dst);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildPoint, dst);
	return _res;
}

static PyObject *Qd_PtInRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	Point pt;
	RgnHandle rgn;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetPoint, &pt,
	                      ResObj_Convert, &rgn))
		return NULL;
	_rv = PtInRgn(pt,
	              rgn);
	_res = Py_BuildValue("b",
	                     _rv);
	return _res;
}

static PyObject *Qd_NewPixMap(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PixMapHandle _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_rv = NewPixMap();
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_DisposePixMap(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PixMapHandle pm;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &pm))
		return NULL;
	DisposePixMap(pm);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_CopyPixMap(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PixMapHandle srcPM;
	PixMapHandle dstPM;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      ResObj_Convert, &srcPM,
	                      ResObj_Convert, &dstPM))
		return NULL;
	CopyPixMap(srcPM,
	           dstPM);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_NewPixPat(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PixPatHandle _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_rv = NewPixPat();
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_DisposePixPat(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PixPatHandle pp;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &pp))
		return NULL;
	DisposePixPat(pp);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_CopyPixPat(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PixPatHandle srcPP;
	PixPatHandle dstPP;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      ResObj_Convert, &srcPP,
	                      ResObj_Convert, &dstPP))
		return NULL;
	CopyPixPat(srcPP,
	           dstPP);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_PenPixPat(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PixPatHandle pp;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &pp))
		return NULL;
	PenPixPat(pp);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_BackPixPat(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PixPatHandle pp;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &pp))
		return NULL;
	BackPixPat(pp);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_GetPixPat(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PixPatHandle _rv;
	short patID;
	if (!PyArg_ParseTuple(_args, "h",
	                      &patID))
		return NULL;
	_rv = GetPixPat(patID);
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_MakeRGBPat(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PixPatHandle pp;
	RGBColor myColor;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      ResObj_Convert, &pp,
	                      QdRGB_Convert, &myColor))
		return NULL;
	MakeRGBPat(pp,
	           &myColor);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_FillCRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	PixPatHandle pp;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetRect, &r,
	                      ResObj_Convert, &pp))
		return NULL;
	FillCRect(&r,
	          pp);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_FillCOval(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	PixPatHandle pp;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetRect, &r,
	                      ResObj_Convert, &pp))
		return NULL;
	FillCOval(&r,
	          pp);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_FillCRoundRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short ovalWidth;
	short ovalHeight;
	PixPatHandle pp;
	if (!PyArg_ParseTuple(_args, "O&hhO&",
	                      PyMac_GetRect, &r,
	                      &ovalWidth,
	                      &ovalHeight,
	                      ResObj_Convert, &pp))
		return NULL;
	FillCRoundRect(&r,
	               ovalWidth,
	               ovalHeight,
	               pp);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_FillCArc(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short startAngle;
	short arcAngle;
	PixPatHandle pp;
	if (!PyArg_ParseTuple(_args, "O&hhO&",
	                      PyMac_GetRect, &r,
	                      &startAngle,
	                      &arcAngle,
	                      ResObj_Convert, &pp))
		return NULL;
	FillCArc(&r,
	         startAngle,
	         arcAngle,
	         pp);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_FillCRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	PixPatHandle pp;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      ResObj_Convert, &rgn,
	                      ResObj_Convert, &pp))
		return NULL;
	FillCRgn(rgn,
	         pp);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_FillCPoly(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PolyHandle poly;
	PixPatHandle pp;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      ResObj_Convert, &poly,
	                      ResObj_Convert, &pp))
		return NULL;
	FillCPoly(poly,
	          pp);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_RGBForeColor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RGBColor color;
	if (!PyArg_ParseTuple(_args, "O&",
	                      QdRGB_Convert, &color))
		return NULL;
	RGBForeColor(&color);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_RGBBackColor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RGBColor color;
	if (!PyArg_ParseTuple(_args, "O&",
	                      QdRGB_Convert, &color))
		return NULL;
	RGBBackColor(&color);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_SetCPixel(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short h;
	short v;
	RGBColor cPix;
	if (!PyArg_ParseTuple(_args, "hhO&",
	                      &h,
	                      &v,
	                      QdRGB_Convert, &cPix))
		return NULL;
	SetCPixel(h,
	          v,
	          &cPix);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_SetPortPix(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PixMapHandle pm;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &pm))
		return NULL;
	SetPortPix(pm);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_GetCPixel(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short h;
	short v;
	RGBColor cPix;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &h,
	                      &v))
		return NULL;
	GetCPixel(h,
	          v,
	          &cPix);
	_res = Py_BuildValue("O&",
	                     QdRGB_New, &cPix);
	return _res;
}

static PyObject *Qd_GetForeColor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RGBColor color;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	GetForeColor(&color);
	_res = Py_BuildValue("O&",
	                     QdRGB_New, &color);
	return _res;
}

static PyObject *Qd_GetBackColor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RGBColor color;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	GetBackColor(&color);
	_res = Py_BuildValue("O&",
	                     QdRGB_New, &color);
	return _res;
}

static PyObject *Qd_OpColor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RGBColor color;
	if (!PyArg_ParseTuple(_args, "O&",
	                      QdRGB_Convert, &color))
		return NULL;
	OpColor(&color);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_HiliteColor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RGBColor color;
	if (!PyArg_ParseTuple(_args, "O&",
	                      QdRGB_Convert, &color))
		return NULL;
	HiliteColor(&color);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_DisposeCTable(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	CTabHandle cTable;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &cTable))
		return NULL;
	DisposeCTable(cTable);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_GetCTable(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	CTabHandle _rv;
	short ctID;
	if (!PyArg_ParseTuple(_args, "h",
	                      &ctID))
		return NULL;
	_rv = GetCTable(ctID);
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_GetCCursor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	CCrsrHandle _rv;
	short crsrID;
	if (!PyArg_ParseTuple(_args, "h",
	                      &crsrID))
		return NULL;
	_rv = GetCCursor(crsrID);
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_SetCCursor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	CCrsrHandle cCrsr;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &cCrsr))
		return NULL;
	SetCCursor(cCrsr);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_AllocCursor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	AllocCursor();
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_DisposeCCursor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	CCrsrHandle cCrsr;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &cCrsr))
		return NULL;
	DisposeCCursor(cCrsr);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_GetMaxDevice(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	GDHandle _rv;
	Rect globalRect;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetRect, &globalRect))
		return NULL;
	_rv = GetMaxDevice(&globalRect);
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_GetCTSeed(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	long _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_rv = GetCTSeed();
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *Qd_GetDeviceList(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	GDHandle _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_rv = GetDeviceList();
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_GetMainDevice(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	GDHandle _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_rv = GetMainDevice();
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_GetNextDevice(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	GDHandle _rv;
	GDHandle curDevice;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &curDevice))
		return NULL;
	_rv = GetNextDevice(curDevice);
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_TestDeviceAttribute(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	GDHandle gdh;
	short attribute;
	if (!PyArg_ParseTuple(_args, "O&h",
	                      ResObj_Convert, &gdh,
	                      &attribute))
		return NULL;
	_rv = TestDeviceAttribute(gdh,
	                          attribute);
	_res = Py_BuildValue("b",
	                     _rv);
	return _res;
}

static PyObject *Qd_SetDeviceAttribute(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	GDHandle gdh;
	short attribute;
	Boolean value;
	if (!PyArg_ParseTuple(_args, "O&hb",
	                      ResObj_Convert, &gdh,
	                      &attribute,
	                      &value))
		return NULL;
	SetDeviceAttribute(gdh,
	                   attribute,
	                   value);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_InitGDevice(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short qdRefNum;
	long mode;
	GDHandle gdh;
	if (!PyArg_ParseTuple(_args, "hlO&",
	                      &qdRefNum,
	                      &mode,
	                      ResObj_Convert, &gdh))
		return NULL;
	InitGDevice(qdRefNum,
	            mode,
	            gdh);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_NewGDevice(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	GDHandle _rv;
	short refNum;
	long mode;
	if (!PyArg_ParseTuple(_args, "hl",
	                      &refNum,
	                      &mode))
		return NULL;
	_rv = NewGDevice(refNum,
	                 mode);
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_DisposeGDevice(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	GDHandle gdh;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &gdh))
		return NULL;
	DisposeGDevice(gdh);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_SetGDevice(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	GDHandle gd;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &gd))
		return NULL;
	SetGDevice(gd);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_GetGDevice(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	GDHandle _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_rv = GetGDevice();
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_Color2Index(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	long _rv;
	RGBColor myColor;
	if (!PyArg_ParseTuple(_args, "O&",
	                      QdRGB_Convert, &myColor))
		return NULL;
	_rv = Color2Index(&myColor);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *Qd_Index2Color(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	long index;
	RGBColor aColor;
	if (!PyArg_ParseTuple(_args, "l",
	                      &index))
		return NULL;
	Index2Color(index,
	            &aColor);
	_res = Py_BuildValue("O&",
	                     QdRGB_New, &aColor);
	return _res;
}

static PyObject *Qd_InvertColor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RGBColor myColor;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	InvertColor(&myColor);
	_res = Py_BuildValue("O&",
	                     QdRGB_New, &myColor);
	return _res;
}

static PyObject *Qd_RealColor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	RGBColor color;
	if (!PyArg_ParseTuple(_args, "O&",
	                      QdRGB_Convert, &color))
		return NULL;
	_rv = RealColor(&color);
	_res = Py_BuildValue("b",
	                     _rv);
	return _res;
}

static PyObject *Qd_GetSubTable(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	CTabHandle myColors;
	short iTabRes;
	CTabHandle targetTbl;
	if (!PyArg_ParseTuple(_args, "O&hO&",
	                      ResObj_Convert, &myColors,
	                      &iTabRes,
	                      ResObj_Convert, &targetTbl))
		return NULL;
	GetSubTable(myColors,
	            iTabRes,
	            targetTbl);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MakeITable(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	CTabHandle cTabH;
	ITabHandle iTabH;
	short res;
	if (!PyArg_ParseTuple(_args, "O&O&h",
	                      ResObj_Convert, &cTabH,
	                      ResObj_Convert, &iTabH,
	                      &res))
		return NULL;
	MakeITable(cTabH,
	           iTabH,
	           res);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_SetClientID(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short id;
	if (!PyArg_ParseTuple(_args, "h",
	                      &id))
		return NULL;
	SetClientID(id);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_ProtectEntry(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short index;
	Boolean protect;
	if (!PyArg_ParseTuple(_args, "hb",
	                      &index,
	                      &protect))
		return NULL;
	ProtectEntry(index,
	             protect);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_ReserveEntry(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short index;
	Boolean reserve;
	if (!PyArg_ParseTuple(_args, "hb",
	                      &index,
	                      &reserve))
		return NULL;
	ReserveEntry(index,
	             reserve);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_QDError(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_rv = QDError();
	_res = Py_BuildValue("h",
	                     _rv);
	return _res;
}

static PyObject *Qd_CopyDeepMask(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	BitMapPtr srcBits;
	BitMapPtr maskBits;
	BitMapPtr dstBits;
	Rect srcRect;
	Rect maskRect;
	Rect dstRect;
	short mode;
	RgnHandle maskRgn;
	if (!PyArg_ParseTuple(_args, "O&O&O&O&O&O&hO&",
	                      BMObj_Convert, &srcBits,
	                      BMObj_Convert, &maskBits,
	                      BMObj_Convert, &dstBits,
	                      PyMac_GetRect, &srcRect,
	                      PyMac_GetRect, &maskRect,
	                      PyMac_GetRect, &dstRect,
	                      &mode,
	                      OptResObj_Convert, &maskRgn))
		return NULL;
	CopyDeepMask(srcBits,
	             maskBits,
	             dstBits,
	             &srcRect,
	             &maskRect,
	             &dstRect,
	             mode,
	             maskRgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_GetPattern(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PatHandle _rv;
	short patternID;
	if (!PyArg_ParseTuple(_args, "h",
	                      &patternID))
		return NULL;
	_rv = GetPattern(patternID);
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_MacGetCursor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	CursHandle _rv;
	short cursorID;
	if (!PyArg_ParseTuple(_args, "h",
	                      &cursorID))
		return NULL;
	_rv = MacGetCursor(cursorID);
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_GetPicture(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	PicHandle _rv;
	short pictureID;
	if (!PyArg_ParseTuple(_args, "h",
	                      &pictureID))
		return NULL;
	_rv = GetPicture(pictureID);
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_DeltaPoint(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	long _rv;
	Point ptA;
	Point ptB;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetPoint, &ptA,
	                      PyMac_GetPoint, &ptB))
		return NULL;
	_rv = DeltaPoint(ptA,
	                 ptB);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *Qd_ShieldCursor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect shieldRect;
	Point offsetPt;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetRect, &shieldRect,
	                      PyMac_GetPoint, &offsetPt))
		return NULL;
	ShieldCursor(&shieldRect,
	             offsetPt);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_ScreenRes(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short scrnHRes;
	short scrnVRes;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	ScreenRes(&scrnHRes,
	          &scrnVRes);
	_res = Py_BuildValue("hh",
	                     scrnHRes,
	                     scrnVRes);
	return _res;
}

static PyObject *Qd_GetIndPattern(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Pattern thePat__out__;
	short patternListID;
	short index;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &patternListID,
	                      &index))
		return NULL;
	GetIndPattern(&thePat__out__,
	              patternListID,
	              index);
	_res = Py_BuildValue("s#",
	                     (char *)&thePat__out__, (int)sizeof(Pattern));
 thePat__error__: ;
	return _res;
}

static PyObject *Qd_SlopeFromAngle(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Fixed _rv;
	short angle;
	if (!PyArg_ParseTuple(_args, "h",
	                      &angle))
		return NULL;
	_rv = SlopeFromAngle(angle);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildFixed, _rv);
	return _res;
}

static PyObject *Qd_AngleFromSlope(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short _rv;
	Fixed slope;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetFixed, &slope))
		return NULL;
	_rv = AngleFromSlope(slope);
	_res = Py_BuildValue("h",
	                     _rv);
	return _res;
}

static PyObject *Qd_TextFont(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short font;
	if (!PyArg_ParseTuple(_args, "h",
	                      &font))
		return NULL;
	TextFont(font);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_TextFace(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	StyleParameter face;
	if (!PyArg_ParseTuple(_args, "h",
	                      &face))
		return NULL;
	TextFace(face);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_TextMode(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short mode;
	if (!PyArg_ParseTuple(_args, "h",
	                      &mode))
		return NULL;
	TextMode(mode);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_TextSize(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short size;
	if (!PyArg_ParseTuple(_args, "h",
	                      &size))
		return NULL;
	TextSize(size);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_SpaceExtra(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Fixed extra;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetFixed, &extra))
		return NULL;
	SpaceExtra(extra);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_DrawChar(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	CharParameter ch;
	if (!PyArg_ParseTuple(_args, "h",
	                      &ch))
		return NULL;
	DrawChar(ch);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_DrawString(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Str255 s;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetStr255, s))
		return NULL;
	DrawString(s);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_MacDrawText(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	char *textBuf__in__;
	int textBuf__len__;
	int textBuf__in_len__;
	short firstByte;
	short byteCount;
	if (!PyArg_ParseTuple(_args, "s#hh",
	                      &textBuf__in__, &textBuf__in_len__,
	                      &firstByte,
	                      &byteCount))
		return NULL;
	MacDrawText(textBuf__in__,
	            firstByte,
	            byteCount);
	Py_INCREF(Py_None);
	_res = Py_None;
 textBuf__error__: ;
	return _res;
}

static PyObject *Qd_CharWidth(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short _rv;
	CharParameter ch;
	if (!PyArg_ParseTuple(_args, "h",
	                      &ch))
		return NULL;
	_rv = CharWidth(ch);
	_res = Py_BuildValue("h",
	                     _rv);
	return _res;
}

static PyObject *Qd_StringWidth(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short _rv;
	Str255 s;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetStr255, s))
		return NULL;
	_rv = StringWidth(s);
	_res = Py_BuildValue("h",
	                     _rv);
	return _res;
}

static PyObject *Qd_TextWidth(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short _rv;
	char *textBuf__in__;
	int textBuf__len__;
	int textBuf__in_len__;
	short firstByte;
	short byteCount;
	if (!PyArg_ParseTuple(_args, "s#hh",
	                      &textBuf__in__, &textBuf__in_len__,
	                      &firstByte,
	                      &byteCount))
		return NULL;
	_rv = TextWidth(textBuf__in__,
	                firstByte,
	                byteCount);
	_res = Py_BuildValue("h",
	                     _rv);
 textBuf__error__: ;
	return _res;
}

static PyObject *Qd_GetFontInfo(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	FontInfo info;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	GetFontInfo(&info);
	_res = Py_BuildValue("O&",
	                     QdFI_New, &info);
	return _res;
}

static PyObject *Qd_CharExtra(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Fixed extra;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetFixed, &extra))
		return NULL;
	CharExtra(extra);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_SetPort(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	GrafPtr thePort;
	if (!PyArg_ParseTuple(_args, "O&",
	                      GrafObj_Convert, &thePort))
		return NULL;
	SetPort(thePort);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_GetCursor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	CursHandle _rv;
	short cursorID;
	if (!PyArg_ParseTuple(_args, "h",
	                      &cursorID))
		return NULL;
	_rv = GetCursor(cursorID);
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *Qd_SetCursor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Cursor *crsr__in__;
	int crsr__in_len__;
	if (!PyArg_ParseTuple(_args, "s#",
	                      (char **)&crsr__in__, &crsr__in_len__))
		return NULL;
	if (crsr__in_len__ != sizeof(Cursor))
	{
		PyErr_SetString(PyExc_TypeError, "buffer length should be sizeof(Cursor)");
		goto crsr__error__;
	}
	SetCursor(crsr__in__);
	Py_INCREF(Py_None);
	_res = Py_None;
 crsr__error__: ;
	return _res;
}

static PyObject *Qd_ShowCursor(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	ShowCursor();
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_LineTo(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short h;
	short v;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &h,
	                      &v))
		return NULL;
	LineTo(h,
	       v);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_SetRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short left;
	short top;
	short right;
	short bottom;
	if (!PyArg_ParseTuple(_args, "hhhh",
	                      &left,
	                      &top,
	                      &right,
	                      &bottom))
		return NULL;
	SetRect(&r,
	        left,
	        top,
	        right,
	        bottom);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildRect, &r);
	return _res;
}

static PyObject *Qd_OffsetRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short dh;
	short dv;
	if (!PyArg_ParseTuple(_args, "O&hh",
	                      PyMac_GetRect, &r,
	                      &dh,
	                      &dv))
		return NULL;
	OffsetRect(&r,
	           dh,
	           dv);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildRect, &r);
	return _res;
}

static PyObject *Qd_InsetRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	short dh;
	short dv;
	if (!PyArg_ParseTuple(_args, "O&hh",
	                      PyMac_GetRect, &r,
	                      &dh,
	                      &dv))
		return NULL;
	InsetRect(&r,
	          dh,
	          dv);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildRect, &r);
	return _res;
}

static PyObject *Qd_UnionRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect src1;
	Rect src2;
	Rect dstRect;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetRect, &src1,
	                      PyMac_GetRect, &src2))
		return NULL;
	UnionRect(&src1,
	          &src2,
	          &dstRect);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildRect, &dstRect);
	return _res;
}

static PyObject *Qd_EqualRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	Rect rect1;
	Rect rect2;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetRect, &rect1,
	                      PyMac_GetRect, &rect2))
		return NULL;
	_rv = EqualRect(&rect1,
	                &rect2);
	_res = Py_BuildValue("b",
	                     _rv);
	return _res;
}

static PyObject *Qd_FrameRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetRect, &r))
		return NULL;
	FrameRect(&r);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_InvertRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetRect, &r))
		return NULL;
	InvertRect(&r);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_FillRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect r;
	Pattern *pat__in__;
	int pat__in_len__;
	if (!PyArg_ParseTuple(_args, "O&s#",
	                      PyMac_GetRect, &r,
	                      (char **)&pat__in__, &pat__in_len__))
		return NULL;
	if (pat__in_len__ != sizeof(Pattern))
	{
		PyErr_SetString(PyExc_TypeError, "buffer length should be sizeof(Pattern)");
		goto pat__error__;
	}
	FillRect(&r,
	         pat__in__);
	Py_INCREF(Py_None);
	_res = Py_None;
 pat__error__: ;
	return _res;
}

static PyObject *Qd_CopyRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle srcRgn;
	RgnHandle dstRgn;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      ResObj_Convert, &srcRgn,
	                      ResObj_Convert, &dstRgn))
		return NULL;
	CopyRgn(srcRgn,
	        dstRgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_SetRectRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	short left;
	short top;
	short right;
	short bottom;
	if (!PyArg_ParseTuple(_args, "O&hhhh",
	                      ResObj_Convert, &rgn,
	                      &left,
	                      &top,
	                      &right,
	                      &bottom))
		return NULL;
	SetRectRgn(rgn,
	           left,
	           top,
	           right,
	           bottom);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_OffsetRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	short dh;
	short dv;
	if (!PyArg_ParseTuple(_args, "O&hh",
	                      ResObj_Convert, &rgn,
	                      &dh,
	                      &dv))
		return NULL;
	OffsetRgn(rgn,
	          dh,
	          dv);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_UnionRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle srcRgnA;
	RgnHandle srcRgnB;
	RgnHandle dstRgn;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      ResObj_Convert, &srcRgnA,
	                      ResObj_Convert, &srcRgnB,
	                      ResObj_Convert, &dstRgn))
		return NULL;
	UnionRgn(srcRgnA,
	         srcRgnB,
	         dstRgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_XorRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle srcRgnA;
	RgnHandle srcRgnB;
	RgnHandle dstRgn;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      ResObj_Convert, &srcRgnA,
	                      ResObj_Convert, &srcRgnB,
	                      ResObj_Convert, &dstRgn))
		return NULL;
	XorRgn(srcRgnA,
	       srcRgnB,
	       dstRgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_EqualRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	RgnHandle rgnA;
	RgnHandle rgnB;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      ResObj_Convert, &rgnA,
	                      ResObj_Convert, &rgnB))
		return NULL;
	_rv = EqualRgn(rgnA,
	               rgnB);
	_res = Py_BuildValue("b",
	                     _rv);
	return _res;
}

static PyObject *Qd_FrameRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &rgn))
		return NULL;
	FrameRgn(rgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_PaintRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &rgn))
		return NULL;
	PaintRgn(rgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_InvertRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &rgn))
		return NULL;
	InvertRgn(rgn);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *Qd_FillRgn(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	RgnHandle rgn;
	Pattern *pat__in__;
	int pat__in_len__;
	if (!PyArg_ParseTuple(_args, "O&s#",
	                      ResObj_Convert, &rgn,
	                      (char **)&pat__in__, &pat__in_len__))
		return NULL;
	if (pat__in_len__ != sizeof(Pattern))
	{
		PyErr_SetString(PyExc_TypeError, "buffer length should be sizeof(Pattern)");
		goto pat__error__;
	}
	FillRgn(rgn,
	        pat__in__);
	Py_INCREF(Py_None);
	_res = Py_None;
 pat__error__: ;
	return _res;
}

static PyObject *Qd_GetPixel(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	short h;
	short v;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &h,
	                      &v))
		return NULL;
	_rv = GetPixel(h,
	               v);
	_res = Py_BuildValue("b",
	                     _rv);
	return _res;
}

static PyObject *Qd_PtInRect(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	Point pt;
	Rect r;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetPoint, &pt,
	                      PyMac_GetRect, &r))
		return NULL;
	_rv = PtInRect(pt,
	               &r);
	_res = Py_BuildValue("b",
	                     _rv);
	return _res;
}

static PyObject *Qd_DrawText(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	char *textBuf__in__;
	int textBuf__len__;
	int textBuf__in_len__;
	short firstByte;
	short byteCount;
	if (!PyArg_ParseTuple(_args, "s#hh",
	                      &textBuf__in__, &textBuf__in_len__,
	                      &firstByte,
	                      &byteCount))
		return NULL;
	DrawText(textBuf__in__,
	         firstByte,
	         byteCount);
	Py_INCREF(Py_None);
	_res = Py_None;
 textBuf__error__: ;
	return _res;
}

static PyObject *Qd_BitMap(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;

	BitMap *ptr;
	PyObject *source;
	Rect bounds;
	int rowbytes;
	char *data;

	if ( !PyArg_ParseTuple(_args, "O!iO&", &PyString_Type, &source, &rowbytes, PyMac_GetRect,
			&bounds) )
		return NULL;
	data = PyString_AsString(source);
	if ((ptr=(BitMap *)malloc(sizeof(BitMap))) == NULL )
		return PyErr_NoMemory();
	ptr->baseAddr = (Ptr)data;
	ptr->rowBytes = rowbytes;
	ptr->bounds = bounds;
	if ( (_res = BMObj_New(ptr)) == NULL ) {
		free(ptr);
		return NULL;
	}
	((BitMapObject *)_res)->referred_object = source;
	Py_INCREF(source);
	((BitMapObject *)_res)->referred_bitmap = ptr;
	return _res;

}

static PyObject *Qd_RawBitMap(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;

	BitMap *ptr;
	PyObject *source;

	if ( !PyArg_ParseTuple(_args, "O!", &PyString_Type, &source) )
		return NULL;
	if ( PyString_Size(source) != sizeof(BitMap) && PyString_Size(source) != sizeof(PixMap) ) {
		PyErr_BadArgument();
		return NULL;
	}
	ptr = (BitMapPtr)PyString_AsString(source);
	if ( (_res = BMObj_New(ptr)) == NULL ) {
		return NULL;
	}
	((BitMapObject *)_res)->referred_object = source;
	Py_INCREF(source);
	return _res;

}

static PyMethodDef Qd_methods[] = {
	{"MacSetPort", (PyCFunction)Qd_MacSetPort, 1,
	 "(GrafPtr port) -> None"},
	{"GetPort", (PyCFunction)Qd_GetPort, 1,
	 "() -> (GrafPtr port)"},
	{"GrafDevice", (PyCFunction)Qd_GrafDevice, 1,
	 "(short device) -> None"},
	{"SetPortBits", (PyCFunction)Qd_SetPortBits, 1,
	 "(BitMapPtr bm) -> None"},
	{"PortSize", (PyCFunction)Qd_PortSize, 1,
	 "(short width, short height) -> None"},
	{"MovePortTo", (PyCFunction)Qd_MovePortTo, 1,
	 "(short leftGlobal, short topGlobal) -> None"},
	{"SetOrigin", (PyCFunction)Qd_SetOrigin, 1,
	 "(short h, short v) -> None"},
	{"SetClip", (PyCFunction)Qd_SetClip, 1,
	 "(RgnHandle rgn) -> None"},
	{"GetClip", (PyCFunction)Qd_GetClip, 1,
	 "(RgnHandle rgn) -> None"},
	{"ClipRect", (PyCFunction)Qd_ClipRect, 1,
	 "(Rect r) -> None"},
	{"BackPat", (PyCFunction)Qd_BackPat, 1,
	 "(Pattern pat) -> None"},
	{"InitCursor", (PyCFunction)Qd_InitCursor, 1,
	 "() -> None"},
	{"MacSetCursor", (PyCFunction)Qd_MacSetCursor, 1,
	 "(Cursor crsr) -> None"},
	{"HideCursor", (PyCFunction)Qd_HideCursor, 1,
	 "() -> None"},
	{"MacShowCursor", (PyCFunction)Qd_MacShowCursor, 1,
	 "() -> None"},
	{"ObscureCursor", (PyCFunction)Qd_ObscureCursor, 1,
	 "() -> None"},
	{"HidePen", (PyCFunction)Qd_HidePen, 1,
	 "() -> None"},
	{"ShowPen", (PyCFunction)Qd_ShowPen, 1,
	 "() -> None"},
	{"GetPen", (PyCFunction)Qd_GetPen, 1,
	 "() -> (Point pt)"},
	{"GetPenState", (PyCFunction)Qd_GetPenState, 1,
	 "() -> (PenState pnState)"},
	{"SetPenState", (PyCFunction)Qd_SetPenState, 1,
	 "(PenState pnState) -> None"},
	{"PenSize", (PyCFunction)Qd_PenSize, 1,
	 "(short width, short height) -> None"},
	{"PenMode", (PyCFunction)Qd_PenMode, 1,
	 "(short mode) -> None"},
	{"PenPat", (PyCFunction)Qd_PenPat, 1,
	 "(Pattern pat) -> None"},
	{"PenNormal", (PyCFunction)Qd_PenNormal, 1,
	 "() -> None"},
	{"MoveTo", (PyCFunction)Qd_MoveTo, 1,
	 "(short h, short v) -> None"},
	{"Move", (PyCFunction)Qd_Move, 1,
	 "(short dh, short dv) -> None"},
	{"MacLineTo", (PyCFunction)Qd_MacLineTo, 1,
	 "(short h, short v) -> None"},
	{"Line", (PyCFunction)Qd_Line, 1,
	 "(short dh, short dv) -> None"},
	{"ForeColor", (PyCFunction)Qd_ForeColor, 1,
	 "(long color) -> None"},
	{"BackColor", (PyCFunction)Qd_BackColor, 1,
	 "(long color) -> None"},
	{"ColorBit", (PyCFunction)Qd_ColorBit, 1,
	 "(short whichBit) -> None"},
	{"MacSetRect", (PyCFunction)Qd_MacSetRect, 1,
	 "(short left, short top, short right, short bottom) -> (Rect r)"},
	{"MacOffsetRect", (PyCFunction)Qd_MacOffsetRect, 1,
	 "(Rect r, short dh, short dv) -> (Rect r)"},
	{"MacInsetRect", (PyCFunction)Qd_MacInsetRect, 1,
	 "(Rect r, short dh, short dv) -> (Rect r)"},
	{"SectRect", (PyCFunction)Qd_SectRect, 1,
	 "(Rect src1, Rect src2) -> (Boolean _rv, Rect dstRect)"},
	{"MacUnionRect", (PyCFunction)Qd_MacUnionRect, 1,
	 "(Rect src1, Rect src2) -> (Rect dstRect)"},
	{"MacEqualRect", (PyCFunction)Qd_MacEqualRect, 1,
	 "(Rect rect1, Rect rect2) -> (Boolean _rv)"},
	{"EmptyRect", (PyCFunction)Qd_EmptyRect, 1,
	 "(Rect r) -> (Boolean _rv)"},
	{"MacFrameRect", (PyCFunction)Qd_MacFrameRect, 1,
	 "(Rect r) -> None"},
	{"PaintRect", (PyCFunction)Qd_PaintRect, 1,
	 "(Rect r) -> None"},
	{"EraseRect", (PyCFunction)Qd_EraseRect, 1,
	 "(Rect r) -> None"},
	{"MacInvertRect", (PyCFunction)Qd_MacInvertRect, 1,
	 "(Rect r) -> None"},
	{"MacFillRect", (PyCFunction)Qd_MacFillRect, 1,
	 "(Rect r, Pattern pat) -> None"},
	{"FrameOval", (PyCFunction)Qd_FrameOval, 1,
	 "(Rect r) -> None"},
	{"PaintOval", (PyCFunction)Qd_PaintOval, 1,
	 "(Rect r) -> None"},
	{"EraseOval", (PyCFunction)Qd_EraseOval, 1,
	 "(Rect r) -> None"},
	{"InvertOval", (PyCFunction)Qd_InvertOval, 1,
	 "(Rect r) -> None"},
	{"FillOval", (PyCFunction)Qd_FillOval, 1,
	 "(Rect r, Pattern pat) -> None"},
	{"FrameRoundRect", (PyCFunction)Qd_FrameRoundRect, 1,
	 "(Rect r, short ovalWidth, short ovalHeight) -> None"},
	{"PaintRoundRect", (PyCFunction)Qd_PaintRoundRect, 1,
	 "(Rect r, short ovalWidth, short ovalHeight) -> None"},
	{"EraseRoundRect", (PyCFunction)Qd_EraseRoundRect, 1,
	 "(Rect r, short ovalWidth, short ovalHeight) -> None"},
	{"InvertRoundRect", (PyCFunction)Qd_InvertRoundRect, 1,
	 "(Rect r, short ovalWidth, short ovalHeight) -> None"},
	{"FillRoundRect", (PyCFunction)Qd_FillRoundRect, 1,
	 "(Rect r, short ovalWidth, short ovalHeight, Pattern pat) -> None"},
	{"FrameArc", (PyCFunction)Qd_FrameArc, 1,
	 "(Rect r, short startAngle, short arcAngle) -> None"},
	{"PaintArc", (PyCFunction)Qd_PaintArc, 1,
	 "(Rect r, short startAngle, short arcAngle) -> None"},
	{"EraseArc", (PyCFunction)Qd_EraseArc, 1,
	 "(Rect r, short startAngle, short arcAngle) -> None"},
	{"InvertArc", (PyCFunction)Qd_InvertArc, 1,
	 "(Rect r, short startAngle, short arcAngle) -> None"},
	{"FillArc", (PyCFunction)Qd_FillArc, 1,
	 "(Rect r, short startAngle, short arcAngle, Pattern pat) -> None"},
	{"NewRgn", (PyCFunction)Qd_NewRgn, 1,
	 "() -> (RgnHandle _rv)"},
	{"OpenRgn", (PyCFunction)Qd_OpenRgn, 1,
	 "() -> None"},
	{"CloseRgn", (PyCFunction)Qd_CloseRgn, 1,
	 "(RgnHandle dstRgn) -> None"},
	{"BitMapToRegion", (PyCFunction)Qd_BitMapToRegion, 1,
	 "(RgnHandle region, BitMapPtr bMap) -> None"},
	{"DisposeRgn", (PyCFunction)Qd_DisposeRgn, 1,
	 "(RgnHandle rgn) -> None"},
	{"MacCopyRgn", (PyCFunction)Qd_MacCopyRgn, 1,
	 "(RgnHandle srcRgn, RgnHandle dstRgn) -> None"},
	{"SetEmptyRgn", (PyCFunction)Qd_SetEmptyRgn, 1,
	 "(RgnHandle rgn) -> None"},
	{"MacSetRectRgn", (PyCFunction)Qd_MacSetRectRgn, 1,
	 "(RgnHandle rgn, short left, short top, short right, short bottom) -> None"},
	{"RectRgn", (PyCFunction)Qd_RectRgn, 1,
	 "(RgnHandle rgn, Rect r) -> None"},
	{"MacOffsetRgn", (PyCFunction)Qd_MacOffsetRgn, 1,
	 "(RgnHandle rgn, short dh, short dv) -> None"},
	{"InsetRgn", (PyCFunction)Qd_InsetRgn, 1,
	 "(RgnHandle rgn, short dh, short dv) -> None"},
	{"SectRgn", (PyCFunction)Qd_SectRgn, 1,
	 "(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn) -> None"},
	{"MacUnionRgn", (PyCFunction)Qd_MacUnionRgn, 1,
	 "(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn) -> None"},
	{"DiffRgn", (PyCFunction)Qd_DiffRgn, 1,
	 "(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn) -> None"},
	{"MacXorRgn", (PyCFunction)Qd_MacXorRgn, 1,
	 "(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn) -> None"},
	{"RectInRgn", (PyCFunction)Qd_RectInRgn, 1,
	 "(Rect r, RgnHandle rgn) -> (Boolean _rv)"},
	{"MacEqualRgn", (PyCFunction)Qd_MacEqualRgn, 1,
	 "(RgnHandle rgnA, RgnHandle rgnB) -> (Boolean _rv)"},
	{"EmptyRgn", (PyCFunction)Qd_EmptyRgn, 1,
	 "(RgnHandle rgn) -> (Boolean _rv)"},
	{"MacFrameRgn", (PyCFunction)Qd_MacFrameRgn, 1,
	 "(RgnHandle rgn) -> None"},
	{"MacPaintRgn", (PyCFunction)Qd_MacPaintRgn, 1,
	 "(RgnHandle rgn) -> None"},
	{"EraseRgn", (PyCFunction)Qd_EraseRgn, 1,
	 "(RgnHandle rgn) -> None"},
	{"MacInvertRgn", (PyCFunction)Qd_MacInvertRgn, 1,
	 "(RgnHandle rgn) -> None"},
	{"MacFillRgn", (PyCFunction)Qd_MacFillRgn, 1,
	 "(RgnHandle rgn, Pattern pat) -> None"},
	{"ScrollRect", (PyCFunction)Qd_ScrollRect, 1,
	 "(Rect r, short dh, short dv, RgnHandle updateRgn) -> None"},
	{"CopyBits", (PyCFunction)Qd_CopyBits, 1,
	 "(BitMapPtr srcBits, BitMapPtr dstBits, Rect srcRect, Rect dstRect, short mode, RgnHandle maskRgn) -> None"},
	{"CopyMask", (PyCFunction)Qd_CopyMask, 1,
	 "(BitMapPtr srcBits, BitMapPtr maskBits, BitMapPtr dstBits, Rect srcRect, Rect maskRect, Rect dstRect) -> None"},
	{"OpenPicture", (PyCFunction)Qd_OpenPicture, 1,
	 "(Rect picFrame) -> (PicHandle _rv)"},
	{"PicComment", (PyCFunction)Qd_PicComment, 1,
	 "(short kind, short dataSize, Handle dataHandle) -> None"},
	{"ClosePicture", (PyCFunction)Qd_ClosePicture, 1,
	 "() -> None"},
	{"DrawPicture", (PyCFunction)Qd_DrawPicture, 1,
	 "(PicHandle myPicture, Rect dstRect) -> None"},
	{"KillPicture", (PyCFunction)Qd_KillPicture, 1,
	 "(PicHandle myPicture) -> None"},
	{"OpenPoly", (PyCFunction)Qd_OpenPoly, 1,
	 "() -> (PolyHandle _rv)"},
	{"ClosePoly", (PyCFunction)Qd_ClosePoly, 1,
	 "() -> None"},
	{"KillPoly", (PyCFunction)Qd_KillPoly, 1,
	 "(PolyHandle poly) -> None"},
	{"OffsetPoly", (PyCFunction)Qd_OffsetPoly, 1,
	 "(PolyHandle poly, short dh, short dv) -> None"},
	{"FramePoly", (PyCFunction)Qd_FramePoly, 1,
	 "(PolyHandle poly) -> None"},
	{"PaintPoly", (PyCFunction)Qd_PaintPoly, 1,
	 "(PolyHandle poly) -> None"},
	{"ErasePoly", (PyCFunction)Qd_ErasePoly, 1,
	 "(PolyHandle poly) -> None"},
	{"InvertPoly", (PyCFunction)Qd_InvertPoly, 1,
	 "(PolyHandle poly) -> None"},
	{"FillPoly", (PyCFunction)Qd_FillPoly, 1,
	 "(PolyHandle poly, Pattern pat) -> None"},
	{"SetPt", (PyCFunction)Qd_SetPt, 1,
	 "(short h, short v) -> (Point pt)"},
	{"LocalToGlobal", (PyCFunction)Qd_LocalToGlobal, 1,
	 "(Point pt) -> (Point pt)"},
	{"GlobalToLocal", (PyCFunction)Qd_GlobalToLocal, 1,
	 "(Point pt) -> (Point pt)"},
	{"Random", (PyCFunction)Qd_Random, 1,
	 "() -> (short _rv)"},
	{"MacGetPixel", (PyCFunction)Qd_MacGetPixel, 1,
	 "(short h, short v) -> (Boolean _rv)"},
	{"ScalePt", (PyCFunction)Qd_ScalePt, 1,
	 "(Point pt, Rect srcRect, Rect dstRect) -> (Point pt)"},
	{"MapPt", (PyCFunction)Qd_MapPt, 1,
	 "(Point pt, Rect srcRect, Rect dstRect) -> (Point pt)"},
	{"MapRect", (PyCFunction)Qd_MapRect, 1,
	 "(Rect r, Rect srcRect, Rect dstRect) -> (Rect r)"},
	{"MapRgn", (PyCFunction)Qd_MapRgn, 1,
	 "(RgnHandle rgn, Rect srcRect, Rect dstRect) -> None"},
	{"MapPoly", (PyCFunction)Qd_MapPoly, 1,
	 "(PolyHandle poly, Rect srcRect, Rect dstRect) -> None"},
	{"StdBits", (PyCFunction)Qd_StdBits, 1,
	 "(BitMapPtr srcBits, Rect srcRect, Rect dstRect, short mode, RgnHandle maskRgn) -> None"},
	{"AddPt", (PyCFunction)Qd_AddPt, 1,
	 "(Point src, Point dst) -> (Point dst)"},
	{"EqualPt", (PyCFunction)Qd_EqualPt, 1,
	 "(Point pt1, Point pt2) -> (Boolean _rv)"},
	{"MacPtInRect", (PyCFunction)Qd_MacPtInRect, 1,
	 "(Point pt, Rect r) -> (Boolean _rv)"},
	{"Pt2Rect", (PyCFunction)Qd_Pt2Rect, 1,
	 "(Point pt1, Point pt2) -> (Rect dstRect)"},
	{"PtToAngle", (PyCFunction)Qd_PtToAngle, 1,
	 "(Rect r, Point pt) -> (short angle)"},
	{"SubPt", (PyCFunction)Qd_SubPt, 1,
	 "(Point src, Point dst) -> (Point dst)"},
	{"PtInRgn", (PyCFunction)Qd_PtInRgn, 1,
	 "(Point pt, RgnHandle rgn) -> (Boolean _rv)"},
	{"NewPixMap", (PyCFunction)Qd_NewPixMap, 1,
	 "() -> (PixMapHandle _rv)"},
	{"DisposePixMap", (PyCFunction)Qd_DisposePixMap, 1,
	 "(PixMapHandle pm) -> None"},
	{"CopyPixMap", (PyCFunction)Qd_CopyPixMap, 1,
	 "(PixMapHandle srcPM, PixMapHandle dstPM) -> None"},
	{"NewPixPat", (PyCFunction)Qd_NewPixPat, 1,
	 "() -> (PixPatHandle _rv)"},
	{"DisposePixPat", (PyCFunction)Qd_DisposePixPat, 1,
	 "(PixPatHandle pp) -> None"},
	{"CopyPixPat", (PyCFunction)Qd_CopyPixPat, 1,
	 "(PixPatHandle srcPP, PixPatHandle dstPP) -> None"},
	{"PenPixPat", (PyCFunction)Qd_PenPixPat, 1,
	 "(PixPatHandle pp) -> None"},
	{"BackPixPat", (PyCFunction)Qd_BackPixPat, 1,
	 "(PixPatHandle pp) -> None"},
	{"GetPixPat", (PyCFunction)Qd_GetPixPat, 1,
	 "(short patID) -> (PixPatHandle _rv)"},
	{"MakeRGBPat", (PyCFunction)Qd_MakeRGBPat, 1,
	 "(PixPatHandle pp, RGBColor myColor) -> None"},
	{"FillCRect", (PyCFunction)Qd_FillCRect, 1,
	 "(Rect r, PixPatHandle pp) -> None"},
	{"FillCOval", (PyCFunction)Qd_FillCOval, 1,
	 "(Rect r, PixPatHandle pp) -> None"},
	{"FillCRoundRect", (PyCFunction)Qd_FillCRoundRect, 1,
	 "(Rect r, short ovalWidth, short ovalHeight, PixPatHandle pp) -> None"},
	{"FillCArc", (PyCFunction)Qd_FillCArc, 1,
	 "(Rect r, short startAngle, short arcAngle, PixPatHandle pp) -> None"},
	{"FillCRgn", (PyCFunction)Qd_FillCRgn, 1,
	 "(RgnHandle rgn, PixPatHandle pp) -> None"},
	{"FillCPoly", (PyCFunction)Qd_FillCPoly, 1,
	 "(PolyHandle poly, PixPatHandle pp) -> None"},
	{"RGBForeColor", (PyCFunction)Qd_RGBForeColor, 1,
	 "(RGBColor color) -> None"},
	{"RGBBackColor", (PyCFunction)Qd_RGBBackColor, 1,
	 "(RGBColor color) -> None"},
	{"SetCPixel", (PyCFunction)Qd_SetCPixel, 1,
	 "(short h, short v, RGBColor cPix) -> None"},
	{"SetPortPix", (PyCFunction)Qd_SetPortPix, 1,
	 "(PixMapHandle pm) -> None"},
	{"GetCPixel", (PyCFunction)Qd_GetCPixel, 1,
	 "(short h, short v) -> (RGBColor cPix)"},
	{"GetForeColor", (PyCFunction)Qd_GetForeColor, 1,
	 "() -> (RGBColor color)"},
	{"GetBackColor", (PyCFunction)Qd_GetBackColor, 1,
	 "() -> (RGBColor color)"},
	{"OpColor", (PyCFunction)Qd_OpColor, 1,
	 "(RGBColor color) -> None"},
	{"HiliteColor", (PyCFunction)Qd_HiliteColor, 1,
	 "(RGBColor color) -> None"},
	{"DisposeCTable", (PyCFunction)Qd_DisposeCTable, 1,
	 "(CTabHandle cTable) -> None"},
	{"GetCTable", (PyCFunction)Qd_GetCTable, 1,
	 "(short ctID) -> (CTabHandle _rv)"},
	{"GetCCursor", (PyCFunction)Qd_GetCCursor, 1,
	 "(short crsrID) -> (CCrsrHandle _rv)"},
	{"SetCCursor", (PyCFunction)Qd_SetCCursor, 1,
	 "(CCrsrHandle cCrsr) -> None"},
	{"AllocCursor", (PyCFunction)Qd_AllocCursor, 1,
	 "() -> None"},
	{"DisposeCCursor", (PyCFunction)Qd_DisposeCCursor, 1,
	 "(CCrsrHandle cCrsr) -> None"},
	{"GetMaxDevice", (PyCFunction)Qd_GetMaxDevice, 1,
	 "(Rect globalRect) -> (GDHandle _rv)"},
	{"GetCTSeed", (PyCFunction)Qd_GetCTSeed, 1,
	 "() -> (long _rv)"},
	{"GetDeviceList", (PyCFunction)Qd_GetDeviceList, 1,
	 "() -> (GDHandle _rv)"},
	{"GetMainDevice", (PyCFunction)Qd_GetMainDevice, 1,
	 "() -> (GDHandle _rv)"},
	{"GetNextDevice", (PyCFunction)Qd_GetNextDevice, 1,
	 "(GDHandle curDevice) -> (GDHandle _rv)"},
	{"TestDeviceAttribute", (PyCFunction)Qd_TestDeviceAttribute, 1,
	 "(GDHandle gdh, short attribute) -> (Boolean _rv)"},
	{"SetDeviceAttribute", (PyCFunction)Qd_SetDeviceAttribute, 1,
	 "(GDHandle gdh, short attribute, Boolean value) -> None"},
	{"InitGDevice", (PyCFunction)Qd_InitGDevice, 1,
	 "(short qdRefNum, long mode, GDHandle gdh) -> None"},
	{"NewGDevice", (PyCFunction)Qd_NewGDevice, 1,
	 "(short refNum, long mode) -> (GDHandle _rv)"},
	{"DisposeGDevice", (PyCFunction)Qd_DisposeGDevice, 1,
	 "(GDHandle gdh) -> None"},
	{"SetGDevice", (PyCFunction)Qd_SetGDevice, 1,
	 "(GDHandle gd) -> None"},
	{"GetGDevice", (PyCFunction)Qd_GetGDevice, 1,
	 "() -> (GDHandle _rv)"},
	{"Color2Index", (PyCFunction)Qd_Color2Index, 1,
	 "(RGBColor myColor) -> (long _rv)"},
	{"Index2Color", (PyCFunction)Qd_Index2Color, 1,
	 "(long index) -> (RGBColor aColor)"},
	{"InvertColor", (PyCFunction)Qd_InvertColor, 1,
	 "() -> (RGBColor myColor)"},
	{"RealColor", (PyCFunction)Qd_RealColor, 1,
	 "(RGBColor color) -> (Boolean _rv)"},
	{"GetSubTable", (PyCFunction)Qd_GetSubTable, 1,
	 "(CTabHandle myColors, short iTabRes, CTabHandle targetTbl) -> None"},
	{"MakeITable", (PyCFunction)Qd_MakeITable, 1,
	 "(CTabHandle cTabH, ITabHandle iTabH, short res) -> None"},
	{"SetClientID", (PyCFunction)Qd_SetClientID, 1,
	 "(short id) -> None"},
	{"ProtectEntry", (PyCFunction)Qd_ProtectEntry, 1,
	 "(short index, Boolean protect) -> None"},
	{"ReserveEntry", (PyCFunction)Qd_ReserveEntry, 1,
	 "(short index, Boolean reserve) -> None"},
	{"QDError", (PyCFunction)Qd_QDError, 1,
	 "() -> (short _rv)"},
	{"CopyDeepMask", (PyCFunction)Qd_CopyDeepMask, 1,
	 "(BitMapPtr srcBits, BitMapPtr maskBits, BitMapPtr dstBits, Rect srcRect, Rect maskRect, Rect dstRect, short mode, RgnHandle maskRgn) -> None"},
	{"GetPattern", (PyCFunction)Qd_GetPattern, 1,
	 "(short patternID) -> (PatHandle _rv)"},
	{"MacGetCursor", (PyCFunction)Qd_MacGetCursor, 1,
	 "(short cursorID) -> (CursHandle _rv)"},
	{"GetPicture", (PyCFunction)Qd_GetPicture, 1,
	 "(short pictureID) -> (PicHandle _rv)"},
	{"DeltaPoint", (PyCFunction)Qd_DeltaPoint, 1,
	 "(Point ptA, Point ptB) -> (long _rv)"},
	{"ShieldCursor", (PyCFunction)Qd_ShieldCursor, 1,
	 "(Rect shieldRect, Point offsetPt) -> None"},
	{"ScreenRes", (PyCFunction)Qd_ScreenRes, 1,
	 "() -> (short scrnHRes, short scrnVRes)"},
	{"GetIndPattern", (PyCFunction)Qd_GetIndPattern, 1,
	 "(short patternListID, short index) -> (Pattern thePat)"},
	{"SlopeFromAngle", (PyCFunction)Qd_SlopeFromAngle, 1,
	 "(short angle) -> (Fixed _rv)"},
	{"AngleFromSlope", (PyCFunction)Qd_AngleFromSlope, 1,
	 "(Fixed slope) -> (short _rv)"},
	{"TextFont", (PyCFunction)Qd_TextFont, 1,
	 "(short font) -> None"},
	{"TextFace", (PyCFunction)Qd_TextFace, 1,
	 "(StyleParameter face) -> None"},
	{"TextMode", (PyCFunction)Qd_TextMode, 1,
	 "(short mode) -> None"},
	{"TextSize", (PyCFunction)Qd_TextSize, 1,
	 "(short size) -> None"},
	{"SpaceExtra", (PyCFunction)Qd_SpaceExtra, 1,
	 "(Fixed extra) -> None"},
	{"DrawChar", (PyCFunction)Qd_DrawChar, 1,
	 "(CharParameter ch) -> None"},
	{"DrawString", (PyCFunction)Qd_DrawString, 1,
	 "(Str255 s) -> None"},
	{"MacDrawText", (PyCFunction)Qd_MacDrawText, 1,
	 "(Buffer textBuf, short firstByte, short byteCount) -> None"},
	{"CharWidth", (PyCFunction)Qd_CharWidth, 1,
	 "(CharParameter ch) -> (short _rv)"},
	{"StringWidth", (PyCFunction)Qd_StringWidth, 1,
	 "(Str255 s) -> (short _rv)"},
	{"TextWidth", (PyCFunction)Qd_TextWidth, 1,
	 "(Buffer textBuf, short firstByte, short byteCount) -> (short _rv)"},
	{"GetFontInfo", (PyCFunction)Qd_GetFontInfo, 1,
	 "() -> (FontInfo info)"},
	{"CharExtra", (PyCFunction)Qd_CharExtra, 1,
	 "(Fixed extra) -> None"},
	{"SetPort", (PyCFunction)Qd_SetPort, 1,
	 "(GrafPtr thePort) -> None"},
	{"GetCursor", (PyCFunction)Qd_GetCursor, 1,
	 "(short cursorID) -> (CursHandle _rv)"},
	{"SetCursor", (PyCFunction)Qd_SetCursor, 1,
	 "(Cursor crsr) -> None"},
	{"ShowCursor", (PyCFunction)Qd_ShowCursor, 1,
	 "() -> None"},
	{"LineTo", (PyCFunction)Qd_LineTo, 1,
	 "(short h, short v) -> None"},
	{"SetRect", (PyCFunction)Qd_SetRect, 1,
	 "(short left, short top, short right, short bottom) -> (Rect r)"},
	{"OffsetRect", (PyCFunction)Qd_OffsetRect, 1,
	 "(Rect r, short dh, short dv) -> (Rect r)"},
	{"InsetRect", (PyCFunction)Qd_InsetRect, 1,
	 "(Rect r, short dh, short dv) -> (Rect r)"},
	{"UnionRect", (PyCFunction)Qd_UnionRect, 1,
	 "(Rect src1, Rect src2) -> (Rect dstRect)"},
	{"EqualRect", (PyCFunction)Qd_EqualRect, 1,
	 "(Rect rect1, Rect rect2) -> (Boolean _rv)"},
	{"FrameRect", (PyCFunction)Qd_FrameRect, 1,
	 "(Rect r) -> None"},
	{"InvertRect", (PyCFunction)Qd_InvertRect, 1,
	 "(Rect r) -> None"},
	{"FillRect", (PyCFunction)Qd_FillRect, 1,
	 "(Rect r, Pattern pat) -> None"},
	{"CopyRgn", (PyCFunction)Qd_CopyRgn, 1,
	 "(RgnHandle srcRgn, RgnHandle dstRgn) -> None"},
	{"SetRectRgn", (PyCFunction)Qd_SetRectRgn, 1,
	 "(RgnHandle rgn, short left, short top, short right, short bottom) -> None"},
	{"OffsetRgn", (PyCFunction)Qd_OffsetRgn, 1,
	 "(RgnHandle rgn, short dh, short dv) -> None"},
	{"UnionRgn", (PyCFunction)Qd_UnionRgn, 1,
	 "(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn) -> None"},
	{"XorRgn", (PyCFunction)Qd_XorRgn, 1,
	 "(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn) -> None"},
	{"EqualRgn", (PyCFunction)Qd_EqualRgn, 1,
	 "(RgnHandle rgnA, RgnHandle rgnB) -> (Boolean _rv)"},
	{"FrameRgn", (PyCFunction)Qd_FrameRgn, 1,
	 "(RgnHandle rgn) -> None"},
	{"PaintRgn", (PyCFunction)Qd_PaintRgn, 1,
	 "(RgnHandle rgn) -> None"},
	{"InvertRgn", (PyCFunction)Qd_InvertRgn, 1,
	 "(RgnHandle rgn) -> None"},
	{"FillRgn", (PyCFunction)Qd_FillRgn, 1,
	 "(RgnHandle rgn, Pattern pat) -> None"},
	{"GetPixel", (PyCFunction)Qd_GetPixel, 1,
	 "(short h, short v) -> (Boolean _rv)"},
	{"PtInRect", (PyCFunction)Qd_PtInRect, 1,
	 "(Point pt, Rect r) -> (Boolean _rv)"},
	{"DrawText", (PyCFunction)Qd_DrawText, 1,
	 "(Buffer textBuf, short firstByte, short byteCount) -> None"},
	{"BitMap", (PyCFunction)Qd_BitMap, 1,
	 "Take (string, int, Rect) argument and create BitMap"},
	{"RawBitMap", (PyCFunction)Qd_RawBitMap, 1,
	 "Take string BitMap and turn into BitMap object"},
	{NULL, NULL, 0}
};




void initQd()
{
	PyObject *m;
	PyObject *d;




	m = Py_InitModule("Qd", Qd_methods);
	d = PyModule_GetDict(m);
	Qd_Error = PyMac_GetOSErrException();
	if (Qd_Error == NULL ||
	    PyDict_SetItemString(d, "Error", Qd_Error) != 0)
		Py_FatalError("can't initialize Qd.Error");
	GrafPort_Type.ob_type = &PyType_Type;
	Py_INCREF(&GrafPort_Type);
	if (PyDict_SetItemString(d, "GrafPortType", (PyObject *)&GrafPort_Type) != 0)
		Py_FatalError("can't initialize GrafPortType");
	BitMap_Type.ob_type = &PyType_Type;
	Py_INCREF(&BitMap_Type);
	if (PyDict_SetItemString(d, "BitMapType", (PyObject *)&BitMap_Type) != 0)
		Py_FatalError("can't initialize BitMapType");
	QDGlobalsAccess_Type.ob_type = &PyType_Type;
	Py_INCREF(&QDGlobalsAccess_Type);
	if (PyDict_SetItemString(d, "QDGlobalsAccessType", (PyObject *)&QDGlobalsAccess_Type) != 0)
		Py_FatalError("can't initialize QDGlobalsAccessType");

	{
		PyObject *o;
	 	
		o = QDGA_New();
		if (o == NULL || PyDict_SetItemString(d, "qd", o) != 0)
			Py_FatalError("can't initialize Qd.qd");
	}


}

/* ========================= End module Qd ========================== */

