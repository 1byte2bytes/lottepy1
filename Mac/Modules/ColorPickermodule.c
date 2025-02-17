/******************************************************************
Copyright 1998 by Just van Rossum, Den Haag, The Netherlands.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Just van Rossum not be 
used in advertising or publicity pertaining to distribution of the 
software without specific, written prior permission.

JUST VAN ROSSUM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO 
EVENT SHALL JUST VAN ROSSUM BE LIABLE FOR ANY SPECIAL, INDIRECT OR 
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF 
USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR 
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

#include <ColorPicker.h>
#include "Python.h"
#include "macglue.h"

/* ----------------------------------------------------- */

extern QdRGB_Convert(PyObject *v, RGBColorPtr p_itself);
extern PyObject *QdRGB_New(RGBColorPtr itself);

static char cp_GetColor__doc__[] =
"GetColor(prompt, (r, g, b)) -> (r, g, b), ok"
;

static PyObject *
cp_GetColor(self, args)
	PyObject *self;	/* Not used */
	PyObject *args;
{
	RGBColor inColor, outColor;
	Boolean ok;
	Point where = {0, 0};
	Str255 prompt;
	
	if (!PyArg_ParseTuple(args, "O&O&", PyMac_GetStr255, prompt, QdRGB_Convert, &inColor))
		return NULL;
	
	ok = GetColor(where, prompt, &inColor, &outColor);
	
	return Py_BuildValue("O&h", QdRGB_New, &outColor, ok);
}

/* List of methods defined in the module */

static struct PyMethodDef cp_methods[] = {
	{"GetColor",	(PyCFunction)cp_GetColor,	METH_VARARGS,	cp_GetColor__doc__},
	{NULL,	 		(PyCFunction)NULL, 			0, 				NULL}		/* sentinel */
};


/* Initialization function for the module (*must* be called initColorPicker) */

static char cp_module_documentation[] = 
""
;

void initColorPicker();

void initColorPicker()
{
	PyObject *m;

	/* Create the module and add the functions */
	m = Py_InitModule4("ColorPicker", cp_methods,
		cp_module_documentation,
		(PyObject*)NULL,PYTHON_API_VERSION);

	/* Add symbolic constants to the module here */
	
	/* XXXX Add constants here */
	
	/* Check for errors */
	if (PyErr_Occurred())
		Py_FatalError("can't initialize module ColorPicker");
}

