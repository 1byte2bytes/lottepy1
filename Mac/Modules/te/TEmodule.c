
/* =========================== Module TE ============================ */

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

#include <TextEdit.h>

/* Exported by Qdmodule.c: */
extern PyObject *QdRGB_New(RGBColor *);
extern int QdRGB_Convert(PyObject *, RGBColor *);

/*
** Parse/generate TextStyle records
*/
PyObject *TextStyle_New(itself)
	TextStylePtr itself;
{

	return Py_BuildValue("lllO&", (long)itself->tsFont, (long)itself->tsFace, (long)itself->tsSize, QdRGB_New,
				&itself->tsColor);
}

TextStyle_Convert(v, p_itself)
	PyObject *v;
	TextStylePtr p_itself;
{
	long font, face, size;
	
	if( !PyArg_ParseTuple(v, "lllO&", &font, &face, &size, QdRGB_Convert, &p_itself->tsColor) )
		return 0;
	p_itself->tsFont = (short)font;
	p_itself->tsFace = (Style)face;
	p_itself->tsSize = (short)size;
	return 1;
}

static PyObject *TE_Error;

/* ------------------------- Object type TE ------------------------- */

PyTypeObject TE_Type;

#define TEObj_Check(x) ((x)->ob_type == &TE_Type)

typedef struct TEObject {
	PyObject_HEAD
	TEHandle ob_itself;
} TEObject;

PyObject *TEObj_New(itself)
	TEHandle itself;
{
	TEObject *it;
	if (itself == NULL) {
						PyErr_SetString(TE_Error,"Cannot create null TE");
						return NULL;
					}
	it = PyObject_NEW(TEObject, &TE_Type);
	if (it == NULL) return NULL;
	it->ob_itself = itself;
	return (PyObject *)it;
}
TEObj_Convert(v, p_itself)
	PyObject *v;
	TEHandle *p_itself;
{
	if (!TEObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "TE required");
		return 0;
	}
	*p_itself = ((TEObject *)v)->ob_itself;
	return 1;
}

static void TEObj_dealloc(self)
	TEObject *self;
{
	TEDispose(self->ob_itself);
	PyMem_DEL(self);
}

static PyObject *TEObj_TESetText(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	char *text__in__;
	long text__len__;
	int text__in_len__;
	if (!PyArg_ParseTuple(_args, "s#",
	                      &text__in__, &text__in_len__))
		return NULL;
	text__len__ = text__in_len__;
	TESetText(text__in__, text__len__,
	          _self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
 text__error__: ;
	return _res;
}

static PyObject *TEObj_TEGetText(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	CharsHandle _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_rv = TEGetText(_self->ob_itself);
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *TEObj_TEIdle(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	TEIdle(_self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TESetSelect(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	long selStart;
	long selEnd;
	if (!PyArg_ParseTuple(_args, "ll",
	                      &selStart,
	                      &selEnd))
		return NULL;
	TESetSelect(selStart,
	            selEnd,
	            _self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TEActivate(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	TEActivate(_self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TEDeactivate(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	TEDeactivate(_self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TEKey(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	CharParameter key;
	if (!PyArg_ParseTuple(_args, "h",
	                      &key))
		return NULL;
	TEKey(key,
	      _self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TECut(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	TECut(_self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TECopy(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	TECopy(_self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TEPaste(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	TEPaste(_self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TEDelete(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	TEDelete(_self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TEInsert(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	char *text__in__;
	long text__len__;
	int text__in_len__;
	if (!PyArg_ParseTuple(_args, "s#",
	                      &text__in__, &text__in_len__))
		return NULL;
	text__len__ = text__in_len__;
	TEInsert(text__in__, text__len__,
	         _self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
 text__error__: ;
	return _res;
}

static PyObject *TEObj_TESetAlignment(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short just;
	if (!PyArg_ParseTuple(_args, "h",
	                      &just))
		return NULL;
	TESetAlignment(just,
	               _self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TEUpdate(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Rect rUpdate;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetRect, &rUpdate))
		return NULL;
	TEUpdate(&rUpdate,
	         _self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TEScroll(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short dh;
	short dv;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &dh,
	                      &dv))
		return NULL;
	TEScroll(dh,
	         dv,
	         _self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TESelView(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	TESelView(_self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TEPinScroll(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short dh;
	short dv;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &dh,
	                      &dv))
		return NULL;
	TEPinScroll(dh,
	            dv,
	            _self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TEAutoView(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean fAuto;
	if (!PyArg_ParseTuple(_args, "b",
	                      &fAuto))
		return NULL;
	TEAutoView(fAuto,
	           _self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TECalText(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	TECalText(_self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TEGetOffset(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short _rv;
	Point pt;
	if (!PyArg_ParseTuple(_args, "O&",
	                      PyMac_GetPoint, &pt))
		return NULL;
	_rv = TEGetOffset(pt,
	                  _self->ob_itself);
	_res = Py_BuildValue("h",
	                     _rv);
	return _res;
}

static PyObject *TEObj_TEGetPoint(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Point _rv;
	short offset;
	if (!PyArg_ParseTuple(_args, "h",
	                      &offset))
		return NULL;
	_rv = TEGetPoint(offset,
	                 _self->ob_itself);
	_res = Py_BuildValue("O&",
	                     PyMac_BuildPoint, _rv);
	return _res;
}

static PyObject *TEObj_TEClick(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Point pt;
	Boolean fExtend;
	if (!PyArg_ParseTuple(_args, "O&b",
	                      PyMac_GetPoint, &pt,
	                      &fExtend))
		return NULL;
	TEClick(pt,
	        fExtend,
	        _self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TESetStyleHandle(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	TEStyleHandle theHandle;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ResObj_Convert, &theHandle))
		return NULL;
	TESetStyleHandle(theHandle,
	                 _self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TEGetStyleHandle(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	TEStyleHandle _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_rv = TEGetStyleHandle(_self->ob_itself);
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *TEObj_TEGetStyle(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short offset;
	TextStyle theStyle;
	short lineHeight;
	short fontAscent;
	if (!PyArg_ParseTuple(_args, "h",
	                      &offset))
		return NULL;
	TEGetStyle(offset,
	           &theStyle,
	           &lineHeight,
	           &fontAscent,
	           _self->ob_itself);
	_res = Py_BuildValue("O&hh",
	                     TextStyle_New, &theStyle,
	                     lineHeight,
	                     fontAscent);
	return _res;
}

static PyObject *TEObj_TEStylePaste(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	TEStylePaste(_self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TESetStyle(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short mode;
	TextStyle newStyle;
	Boolean fRedraw;
	if (!PyArg_ParseTuple(_args, "hO&b",
	                      &mode,
	                      TextStyle_Convert, &newStyle,
	                      &fRedraw))
		return NULL;
	TESetStyle(mode,
	           &newStyle,
	           fRedraw,
	           _self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TEReplaceStyle(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short mode;
	TextStyle oldStyle;
	TextStyle newStyle;
	Boolean fRedraw;
	if (!PyArg_ParseTuple(_args, "hO&O&b",
	                      &mode,
	                      TextStyle_Convert, &oldStyle,
	                      TextStyle_Convert, &newStyle,
	                      &fRedraw))
		return NULL;
	TEReplaceStyle(mode,
	               &oldStyle,
	               &newStyle,
	               fRedraw,
	               _self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TEGetStyleScrapHandle(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	StScrpHandle _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_rv = TEGetStyleScrapHandle(_self->ob_itself);
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *TEObj_TEStyleInsert(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	char *text__in__;
	long text__len__;
	int text__in_len__;
	StScrpHandle hST;
	if (!PyArg_ParseTuple(_args, "s#O&",
	                      &text__in__, &text__in_len__,
	                      ResObj_Convert, &hST))
		return NULL;
	text__len__ = text__in_len__;
	TEStyleInsert(text__in__, text__len__,
	              hST,
	              _self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
 text__error__: ;
	return _res;
}

static PyObject *TEObj_TEGetHeight(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	long _rv;
	long endLine;
	long startLine;
	if (!PyArg_ParseTuple(_args, "ll",
	                      &endLine,
	                      &startLine))
		return NULL;
	_rv = TEGetHeight(endLine,
	                  startLine,
	                  _self->ob_itself);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *TEObj_TEContinuousStyle(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Boolean _rv;
	short mode;
	TextStyle aStyle;
	if (!PyArg_ParseTuple(_args, "hO&",
	                      &mode,
	                      TextStyle_Convert, &aStyle))
		return NULL;
	_rv = TEContinuousStyle(&mode,
	                        &aStyle,
	                        _self->ob_itself);
	_res = Py_BuildValue("bhO&",
	                     _rv,
	                     mode,
	                     TextStyle_New, &aStyle);
	return _res;
}

static PyObject *TEObj_TEUseStyleScrap(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	long rangeStart;
	long rangeEnd;
	StScrpHandle newStyles;
	Boolean fRedraw;
	if (!PyArg_ParseTuple(_args, "llO&b",
	                      &rangeStart,
	                      &rangeEnd,
	                      ResObj_Convert, &newStyles,
	                      &fRedraw))
		return NULL;
	TEUseStyleScrap(rangeStart,
	                rangeEnd,
	                newStyles,
	                fRedraw,
	                _self->ob_itself);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TEObj_TENumStyles(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	long _rv;
	long rangeStart;
	long rangeEnd;
	if (!PyArg_ParseTuple(_args, "ll",
	                      &rangeStart,
	                      &rangeEnd))
		return NULL;
	_rv = TENumStyles(rangeStart,
	                  rangeEnd,
	                  _self->ob_itself);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *TEObj_TEFeatureFlag(_self, _args)
	TEObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	short _rv;
	short feature;
	short action;
	if (!PyArg_ParseTuple(_args, "hh",
	                      &feature,
	                      &action))
		return NULL;
	_rv = TEFeatureFlag(feature,
	                    action,
	                    _self->ob_itself);
	_res = Py_BuildValue("h",
	                     _rv);
	return _res;
}

static PyMethodDef TEObj_methods[] = {
	{"TESetText", (PyCFunction)TEObj_TESetText, 1,
	 "(Buffer text) -> None"},
	{"TEGetText", (PyCFunction)TEObj_TEGetText, 1,
	 "() -> (CharsHandle _rv)"},
	{"TEIdle", (PyCFunction)TEObj_TEIdle, 1,
	 "() -> None"},
	{"TESetSelect", (PyCFunction)TEObj_TESetSelect, 1,
	 "(long selStart, long selEnd) -> None"},
	{"TEActivate", (PyCFunction)TEObj_TEActivate, 1,
	 "() -> None"},
	{"TEDeactivate", (PyCFunction)TEObj_TEDeactivate, 1,
	 "() -> None"},
	{"TEKey", (PyCFunction)TEObj_TEKey, 1,
	 "(CharParameter key) -> None"},
	{"TECut", (PyCFunction)TEObj_TECut, 1,
	 "() -> None"},
	{"TECopy", (PyCFunction)TEObj_TECopy, 1,
	 "() -> None"},
	{"TEPaste", (PyCFunction)TEObj_TEPaste, 1,
	 "() -> None"},
	{"TEDelete", (PyCFunction)TEObj_TEDelete, 1,
	 "() -> None"},
	{"TEInsert", (PyCFunction)TEObj_TEInsert, 1,
	 "(Buffer text) -> None"},
	{"TESetAlignment", (PyCFunction)TEObj_TESetAlignment, 1,
	 "(short just) -> None"},
	{"TEUpdate", (PyCFunction)TEObj_TEUpdate, 1,
	 "(Rect rUpdate) -> None"},
	{"TEScroll", (PyCFunction)TEObj_TEScroll, 1,
	 "(short dh, short dv) -> None"},
	{"TESelView", (PyCFunction)TEObj_TESelView, 1,
	 "() -> None"},
	{"TEPinScroll", (PyCFunction)TEObj_TEPinScroll, 1,
	 "(short dh, short dv) -> None"},
	{"TEAutoView", (PyCFunction)TEObj_TEAutoView, 1,
	 "(Boolean fAuto) -> None"},
	{"TECalText", (PyCFunction)TEObj_TECalText, 1,
	 "() -> None"},
	{"TEGetOffset", (PyCFunction)TEObj_TEGetOffset, 1,
	 "(Point pt) -> (short _rv)"},
	{"TEGetPoint", (PyCFunction)TEObj_TEGetPoint, 1,
	 "(short offset) -> (Point _rv)"},
	{"TEClick", (PyCFunction)TEObj_TEClick, 1,
	 "(Point pt, Boolean fExtend) -> None"},
	{"TESetStyleHandle", (PyCFunction)TEObj_TESetStyleHandle, 1,
	 "(TEStyleHandle theHandle) -> None"},
	{"TEGetStyleHandle", (PyCFunction)TEObj_TEGetStyleHandle, 1,
	 "() -> (TEStyleHandle _rv)"},
	{"TEGetStyle", (PyCFunction)TEObj_TEGetStyle, 1,
	 "(short offset) -> (TextStyle theStyle, short lineHeight, short fontAscent)"},
	{"TEStylePaste", (PyCFunction)TEObj_TEStylePaste, 1,
	 "() -> None"},
	{"TESetStyle", (PyCFunction)TEObj_TESetStyle, 1,
	 "(short mode, TextStyle newStyle, Boolean fRedraw) -> None"},
	{"TEReplaceStyle", (PyCFunction)TEObj_TEReplaceStyle, 1,
	 "(short mode, TextStyle oldStyle, TextStyle newStyle, Boolean fRedraw) -> None"},
	{"TEGetStyleScrapHandle", (PyCFunction)TEObj_TEGetStyleScrapHandle, 1,
	 "() -> (StScrpHandle _rv)"},
	{"TEStyleInsert", (PyCFunction)TEObj_TEStyleInsert, 1,
	 "(Buffer text, StScrpHandle hST) -> None"},
	{"TEGetHeight", (PyCFunction)TEObj_TEGetHeight, 1,
	 "(long endLine, long startLine) -> (long _rv)"},
	{"TEContinuousStyle", (PyCFunction)TEObj_TEContinuousStyle, 1,
	 "(short mode, TextStyle aStyle) -> (Boolean _rv, short mode, TextStyle aStyle)"},
	{"TEUseStyleScrap", (PyCFunction)TEObj_TEUseStyleScrap, 1,
	 "(long rangeStart, long rangeEnd, StScrpHandle newStyles, Boolean fRedraw) -> None"},
	{"TENumStyles", (PyCFunction)TEObj_TENumStyles, 1,
	 "(long rangeStart, long rangeEnd) -> (long _rv)"},
	{"TEFeatureFlag", (PyCFunction)TEObj_TEFeatureFlag, 1,
	 "(short feature, short action) -> (short _rv)"},
	{NULL, NULL, 0}
};

PyMethodChain TEObj_chain = { TEObj_methods, NULL };

static PyObject *TEObj_getattr(self, name)
	TEObject *self;
	char *name;
{

				if( strcmp(name, "destRect") == 0 )
					return Py_BuildValue("O&", PyMac_BuildRect,
							&(*self->ob_itself)->destRect);
				if( strcmp(name, "viewRect") == 0 )
					return Py_BuildValue("O&", PyMac_BuildRect,
							&(*self->ob_itself)->viewRect);
				if( strcmp(name, "selRect") == 0 )
					return Py_BuildValue("O&", PyMac_BuildRect,
							&(*self->ob_itself)->selRect);
				if( strcmp(name, "lineHeight") == 0 )
					return Py_BuildValue("h", (*self->ob_itself)->lineHeight);
				if( strcmp(name, "fontAscent") == 0 )
					return Py_BuildValue("h", (*self->ob_itself)->fontAscent);
				if( strcmp(name, "selPoint") == 0 )
					return Py_BuildValue("O&", PyMac_BuildPoint,
							&(*self->ob_itself)->selPoint);
				if( strcmp(name, "selStart") == 0 )
					return Py_BuildValue("h", (*self->ob_itself)->selStart);
				if( strcmp(name, "selEnd") == 0 )
					return Py_BuildValue("h", (*self->ob_itself)->selEnd);
				if( strcmp(name, "active") == 0 )
					return Py_BuildValue("h", (*self->ob_itself)->active);
				if( strcmp(name, "just") == 0 )
					return Py_BuildValue("h", (*self->ob_itself)->just);
				if( strcmp(name, "teLength") == 0 )
					return Py_BuildValue("h", (*self->ob_itself)->teLength);
				if( strcmp(name, "txFont") == 0 )
					return Py_BuildValue("h", (*self->ob_itself)->txFont);
				if( strcmp(name, "txFace") == 0 )
					return Py_BuildValue("h", (*self->ob_itself)->txFace);
				if( strcmp(name, "txMode") == 0 )
					return Py_BuildValue("h", (*self->ob_itself)->txMode);
				if( strcmp(name, "txSize") == 0 )
					return Py_BuildValue("h", (*self->ob_itself)->txSize);
				if( strcmp(name, "nLines") == 0 )
					return Py_BuildValue("h", (*self->ob_itself)->nLines);
			
	return Py_FindMethodInChain(&TEObj_chain, (PyObject *)self, name);
}

#define TEObj_setattr NULL

PyTypeObject TE_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0, /*ob_size*/
	"TE", /*tp_name*/
	sizeof(TEObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) TEObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc) TEObj_getattr, /*tp_getattr*/
	(setattrfunc) TEObj_setattr, /*tp_setattr*/
};

/* ----------------------- End object type TE ----------------------- */


static PyObject *TE_TEScrapHandle(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	Handle _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_rv = TEScrapHandle();
	_res = Py_BuildValue("O&",
	                     ResObj_New, _rv);
	return _res;
}

static PyObject *TE_TEGetScrapLength(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	long _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_rv = TEGetScrapLength();
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *TE_TENew(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	TEHandle _rv;
	Rect destRect;
	Rect viewRect;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetRect, &destRect,
	                      PyMac_GetRect, &viewRect))
		return NULL;
	_rv = TENew(&destRect,
	            &viewRect);
	_res = Py_BuildValue("O&",
	                     TEObj_New, _rv);
	return _res;
}

static PyObject *TE_TETextBox(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	char *text__in__;
	long text__len__;
	int text__in_len__;
	Rect box;
	short just;
	if (!PyArg_ParseTuple(_args, "s#O&h",
	                      &text__in__, &text__in_len__,
	                      PyMac_GetRect, &box,
	                      &just))
		return NULL;
	text__len__ = text__in_len__;
	TETextBox(text__in__, text__len__,
	          &box,
	          just);
	Py_INCREF(Py_None);
	_res = Py_None;
 text__error__: ;
	return _res;
}

static PyObject *TE_TEStyleNew(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	TEHandle _rv;
	Rect destRect;
	Rect viewRect;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyMac_GetRect, &destRect,
	                      PyMac_GetRect, &viewRect))
		return NULL;
	_rv = TEStyleNew(&destRect,
	                 &viewRect);
	_res = Py_BuildValue("O&",
	                     TEObj_New, _rv);
	return _res;
}

static PyObject *TE_TESetScrapLength(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	long length;
	if (!PyArg_ParseTuple(_args, "l",
	                      &length))
		return NULL;
	TESetScrapLength(length);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TE_TEFromScrap(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	OSErr _err;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_err = TEFromScrap();
	if (_err != noErr) return PyMac_Error(_err);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *TE_TEToScrap(_self, _args)
	PyObject *_self;
	PyObject *_args;
{
	PyObject *_res = NULL;
	OSErr _err;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	_err = TEToScrap();
	if (_err != noErr) return PyMac_Error(_err);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef TE_methods[] = {
	{"TEScrapHandle", (PyCFunction)TE_TEScrapHandle, 1,
	 "() -> (Handle _rv)"},
	{"TEGetScrapLength", (PyCFunction)TE_TEGetScrapLength, 1,
	 "() -> (long _rv)"},
	{"TENew", (PyCFunction)TE_TENew, 1,
	 "(Rect destRect, Rect viewRect) -> (TEHandle _rv)"},
	{"TETextBox", (PyCFunction)TE_TETextBox, 1,
	 "(Buffer text, Rect box, short just) -> None"},
	{"TEStyleNew", (PyCFunction)TE_TEStyleNew, 1,
	 "(Rect destRect, Rect viewRect) -> (TEHandle _rv)"},
	{"TESetScrapLength", (PyCFunction)TE_TESetScrapLength, 1,
	 "(long length) -> None"},
	{"TEFromScrap", (PyCFunction)TE_TEFromScrap, 1,
	 "() -> None"},
	{"TEToScrap", (PyCFunction)TE_TEToScrap, 1,
	 "() -> None"},
	{NULL, NULL, 0}
};




void initTE()
{
	PyObject *m;
	PyObject *d;




	m = Py_InitModule("TE", TE_methods);
	d = PyModule_GetDict(m);
	TE_Error = PyMac_GetOSErrException();
	if (TE_Error == NULL ||
	    PyDict_SetItemString(d, "Error", TE_Error) != 0)
		Py_FatalError("can't initialize TE.Error");
	TE_Type.ob_type = &PyType_Type;
	Py_INCREF(&TE_Type);
	if (PyDict_SetItemString(d, "TEType", (PyObject *)&TE_Type) != 0)
		Py_FatalError("can't initialize TEType");
}

/* ========================= End module TE ========================== */

