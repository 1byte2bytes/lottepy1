"""Suite Standard URL suite: Mac URL standard, supported by many apps



Level 1, version 1

Generated from flap:Programma's:Netscape Navigator� Folder:Netscape Navigator� 3.01
AETE/AEUT resource version 1/0, language 0, script 0
"""

import aetools
import MacOS

_code = 'GURL'

class Standard_URL_suite:

	_argmap_GetURL = {
		'to' : 'dest',
		'inside' : 'HWIN',
		'_from' : 'refe',
	}

	def GetURL(self, _object, _attributes={}, **_arguments):
		"""GetURL: Loads the URL (optionaly to disk)

		Required argument: The url 
		Keyword argument to: file the URL should be loaded into  
		Keyword argument inside: Window the URL should be loaded to
		Keyword argument _from: Refererer, to be sent with the HTTP request
		Keyword argument _attributes: AppleEvent attribute dictionary
		"""
		_code = 'GURL'
		_subcode = 'GURL'

		aetools.keysubst(_arguments, self._argmap_GetURL)
		_arguments['----'] = _object


		_reply, _arguments, _attributes = self.send(_code, _subcode,
				_arguments, _attributes)
		if _arguments.has_key('errn'):
			raise aetools.Error, aetools.decodeerror(_arguments)
		# XXXX Optionally decode result
		if _arguments.has_key('----'):
			return _arguments['----']


#
# Indices of types declared in this module
#
_classdeclarations = {
}

_propdeclarations = {
}

_compdeclarations = {
}

_enumdeclarations = {
}
