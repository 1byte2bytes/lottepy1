"""Easy to use dialogs.

Message(msg) -- display a message and an OK button.
AskString(prompt, default) -- ask for a string, display OK and Cancel buttons.
AskPassword(prompt, default) -- like AskString(), but shows text as bullets.
AskYesNoCancel(question, default) -- display a question and Yes, No and Cancel buttons.
bar = Progress(label, maxvalue) -- Display a progress bar
bar.set(value) -- Set value
bar.inc( *amount ) -- increment value by amount (default=1)
bar.label( *newlabel ) -- get or set text label. 

More documentation in each function.
This module uses DLOG resources 256, 257 and 258.
Based upon STDWIN dialogs with the same names and functions.
"""

from Dlg import GetNewDialog, SetDialogItemText, GetDialogItemText, ModalDialog
import Qd
import QuickDraw
import Dialogs
import Windows
import Dlg,Win,Evt,Events # sdm7g
import MacOS
import string

def cr2lf(text):
	if '\r' in text:
		text = string.join(string.split(text, '\r'), '\n')
	return text

def lf2cr(text):
	if '\n' in text:
		text = string.join(string.split(text, '\n'), '\r')
	if len(text) > 253:
		text = text[:253] + '\311'
	return text

def Message(msg, id=256, ok=None):
	"""Display a MESSAGE string.
	
	Return when the user clicks the OK button or presses Return.
	
	The MESSAGE string can be at most 255 characters long.
	"""
	
	d = GetNewDialog(id, -1)
	if not d:
		print "Can't get DLOG resource with id =", id
		return
	tp, h, rect = d.GetDialogItem(2)
	SetDialogItemText(h, lf2cr(msg))
	if ok != None:
		tp, h, rect = d.GetDialogItem(1)
		h.as_Control().SetControlTitle(ok)
	d.SetDialogDefaultItem(1)
	while 1:
		n = ModalDialog(None)
		if n == 1:
			return


def AskString(prompt, default = "", id=257, ok=None, cancel=None):
	"""Display a PROMPT string and a text entry field with a DEFAULT string.
	
	Return the contents of the text entry field when the user clicks the
	OK button or presses Return.
	Return None when the user clicks the Cancel button.
	
	If omitted, DEFAULT is empty.
	
	The PROMPT and DEFAULT strings, as well as the return value,
	can be at most 255 characters long.
	"""
	
	id = 257
	d = GetNewDialog(id, -1)
	if not d:
		print "Can't get DLOG resource with id =", id
		return
	tp, h, rect = d.GetDialogItem(3)
	SetDialogItemText(h, lf2cr(prompt))
	tp, h, rect = d.GetDialogItem(4)
	SetDialogItemText(h, lf2cr(default))
	d.SelectDialogItemText(4, 0, 999)
#	d.SetDialogItem(4, 0, 255)
	if ok != None:
		tp, h, rect = d.GetDialogItem(1)
		h.as_Control().SetControlTitle(ok)
	if cancel != None:
		tp, h, rect = d.GetDialogItem(2)
		h.as_Control().SetControlTitle(cancel)
	d.SetDialogDefaultItem(1)
	d.SetDialogCancelItem(2)
	while 1:
		n = ModalDialog(None)
		if n == 1:
			tp, h, rect = d.GetDialogItem(4)
			return cr2lf(GetDialogItemText(h))
		if n == 2: return None

def AskPassword(prompt,	 default='', id=257):	
	"""Display a PROMPT string and a text entry field with a DEFAULT string.
	The string is displayed as bullets only.
	
	Return the contents of the text entry field when the user clicks the
	OK button or presses Return.
	Return None when the user clicks the Cancel button.
	
	If omitted, DEFAULT is empty.
	
	The PROMPT and DEFAULT strings, as well as the return value,
	can be at most 255 characters long.
	"""
	d = GetNewDialog(id, -1)
	if not d:
		print "Can't get DLOG resource with id =", id
		return
	tp, h, rect = d.GetDialogItem(3)	# STATIC TEXT ITEM <= prompt 
	SetDialogItemText(h, lf2cr(prompt))	
	tp, h, rect = d.GetDialogItem(4)	# EDIT TEXT ITEM 
	bullets = '\245'*len(default)
	SetDialogItemText(h, bullets )
	d.SelectDialogItemText(4, 999, 999)
	d.SetDialogDefaultItem(Dialogs.ok)
	d.SetDialogCancelItem(Dialogs.cancel)
	string = default
	oldschedparams = MacOS.SchedParams(0,0)
	while 1:
		ready,ev = Evt.WaitNextEvent(Events.everyEvent, 6)
		if not ready: continue
		what,msg,when,where,mod = ev
		if what == 0 : Dlg.DialogSelect(ev)	# for blinking caret
		elif Dlg.IsDialogEvent(ev):
			if what in (Events.keyDown, Events.autoKey):
				charcode = msg & Events.charCodeMask
				if ( mod & Events.cmdKey ):
					MacOS.SysBeep()
					continue	# don't do cut & paste commands
				else:	
					if charcode == Events.kReturnCharCode:
						break
					elif charcode == Events.kEscapeCharCode:
						string = None
						break
					elif charcode in (Events.kLeftArrowCharCode,
								Events.kBackspaceCharCode):
						string = string[:-1]
					else:
						string = string + chr(charcode)
						msg =  0245   # Octal code for bullet
						ev = (what,msg,when,where,mod)
			rs, win, item = Dlg.DialogSelect(ev)
			if item == Dialogs.ok :
				break	
			elif item == Dialogs.cancel :
				string = None
				break
		elif what == Events.mouseDown:
			part, win = Win.FindWindow(where)
			if part == Windows.inDrag and win:
				win.DragWindow(where, screenbounds)
			elif part == Windows.inMenuBar:
				MacOS.HandleEvent(ev)
			else:
				MacOS.SysBeep()		# Cannot handle selections, unfortunately
			
		elif what == Events.updateEvt: MacOS.HandleEvent(ev)
	apply(MacOS.SchedParams, oldschedparams)
	return string

def AskYesNoCancel(question, default = 0, yes=None, no=None, cancel=None, id=258):
	"""Display a QUESTION string which can be answered with Yes or No.
	
	Return 1 when the user clicks the Yes button.
	Return 0 when the user clicks the No button.
	Return -1 when the user clicks the Cancel button.
	
	When the user presses Return, the DEFAULT value is returned.
	If omitted, this is 0 (No).
	
	The QUESTION strign ca be at most 255 characters.
	"""
	
	d = GetNewDialog(id, -1)
	if not d:
		print "Can't get DLOG resource with id =", id
		return
	# Button assignments:
	# 1 = default (invisible)
	# 2 = Yes
	# 3 = No
	# 4 = Cancel
	# The question string is item 5
	tp, h, rect = d.GetDialogItem(5)
	SetDialogItemText(h, lf2cr(question))
	if yes != None:
		tp, h, rect = d.GetDialogItem(2)
		h.as_Control().SetControlTitle(yes)
	if no != None:
		tp, h, rect = d.GetDialogItem(3)
		h.as_Control().SetControlTitle(no)
	if cancel != None:
		if cancel == '':
			d.HideDialogItem(4)
		else:
			tp, h, rect = d.GetDialogItem(4)
			h.as_Control().SetControlTitle(cancel)
	d.SetDialogCancelItem(4)
	if default == 1:
		d.SetDialogDefaultItem(2)
	elif default == 0:
		d.SetDialogDefaultItem(3)
	elif default == -1:
		d.SetDialogDefaultItem(4)
	while 1:
		n = ModalDialog(None)
		if n == 1: return default
		if n == 2: return 1
		if n == 3: return 0
		if n == 4: return -1


		

screenbounds = Qd.qd.screenBits.bounds
screenbounds = screenbounds[0]+4, screenbounds[1]+4, \
	screenbounds[2]-4, screenbounds[3]-4

				
class ProgressBar:
	def __init__(self, title="Working...", maxval=100, label="", id=259):
		self.maxval = maxval
		self.curval = -1
		self.d = GetNewDialog(id, -1)
		self.title(title)
		self.label(label)
		self._update(0)

	def __del__( self ):
		self.d.BringToFront()
		self.d.HideWindow()
		del self.d
		
	def title(self, newstr=""):
		"""title(text) - Set title of progress window"""
		self.d.BringToFront()
		w = self.d.GetDialogWindow()
		w.SetWTitle(newstr)
		
	def label( self, *newstr ):
		"""label(text) - Set text in progress box"""
		self.d.BringToFront()
		if newstr:
			self._label = lf2cr(newstr[0])
		tp, text_h, rect = self.d.GetDialogItem(2)
		SetDialogItemText(text_h, self._label)		

				
	def _update(self, value):
		self.d.BringToFront()
		tp, h, bar_rect = self.d.GetDialogItem(3)
		Qd.SetPort(self.d)
		
		Qd.FrameRect(bar_rect)	# Draw outline
		
		inner_rect = Qd.InsetRect(bar_rect, 1, 1)
		l, t, r, b = inner_rect

		Qd.ForeColor(QuickDraw.blackColor)
		Qd.BackColor(QuickDraw.blackColor)
		Qd.PaintRect((l, t, int(l + (r-l)*value/self.maxval), b))	# Draw bar

		Qd.ForeColor(QuickDraw.whiteColor)
		Qd.BackColor(QuickDraw.whiteColor)
		Qd.PaintRect((int(l + (r-l)*value/self.maxval), t, r, b))	# Clear rest
				
		# Restore settings
		Qd.ForeColor(QuickDraw.blackColor)
		Qd.BackColor(QuickDraw.whiteColor)
		
		# Test for cancel button
		
		ready, ev = Evt.WaitNextEvent( Events.mDownMask, 1  )
		if ready : 
			what,msg,when,where,mod = ev
			part = Win.FindWindow(where)[0]
			if Dlg.IsDialogEvent(ev):
				ds = Dlg.DialogSelect(ev)
				if ds[0] and ds[1] == self.d and ds[-1] == 1:
					raise KeyboardInterrupt, ev
			else:
				if part == 4:	# inDrag 
					self.d.DragWindow(where, screenbounds)
				else:
					MacOS.HandleEvent(ev) 
			
			
	def set(self, value):
		"""set(value) - Set progress bar position"""
		if value < 0: value = 0
		if value > self.maxval: value = self.maxval
		self.curval = value
		self._update(value)

	def inc(self, n=1):
		"""inc(amt) - Increment progress bar position"""
		self.set(self.curval + n)

def test():
	import time

	Message("Testing EasyDialogs.")
	ok = AskYesNoCancel("Do you want to proceed?")
	ok = AskYesNoCancel("Do you want to identify?", yes="Indentify", no="Don't identify")
	if ok > 0:
		s = AskString("Enter your first name", "Joe")
		Message("Thank you,\n%s" % `s`)
	text = ( "Working Hard...", "Hardly Working..." , 
			"So far, so good!", "Keep on truckin'" )
	bar = ProgressBar("Progress, progress...", 100)
	try:
		appsw = MacOS.SchedParams(1, 0)
		for i in range(100):
			bar.set(i)
			time.sleep(0.1)
			if i % 10 == 0:
				bar.label(text[(i/10) % 4])
		bar.label("Done.")
		time.sleep(0.3) 	# give'em a chance to see the done.
	finally:
		del bar
		apply(MacOS.SchedParams, appsw)


	
	
if __name__ == '__main__':
	try:
		test()
	except KeyboardInterrupt:
		Message("Operation Canceled.")

