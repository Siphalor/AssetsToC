from pygments.lexer import RegexLexer, bygroups
from pygments.token import *

class AtocLexer(RegexLexer):
	name = 'Atoc'
	aliases = ['atoc']
	filenames = ['*.atoc']

	tokens = {
		'root': [
			(r'\s*#.*$', Comment.Single),
			#(r'(\$|\$\$)(?:\s*([^:\s]+)\s*(:)\s*(?:([^;]*));?)*$', bygroups(Operator, Name.Variable, Operator, Literal))
			(r'^\s*(\$|\$\$)', Operator, 'config'),
			(r'^(\s*)([<>])(.*)$', bygroups(Text, Operator, Name.Namespace)),
			(r'^\s*;', Operator),
			(r'^\s*{', Operator, 'input-bundle'),
			(r'^\s*(?=[^<>\{\}])', Text, 'statement') 
		],
		'config': [
			(r'$', Text, 'root'),
			(r';', Operator),
			(r'([^:]+)(:)(\s*)(true|false)(?=$|;)', bygroups(Name.Attribute, Operator, Keyword.Constant, Text)),
			(r'([^:]+)(:)(\s*)([0-9]+)(?=$|;)', bygroups(Name.Attribute, Operator, Number.Integer, Text)),
			(r'([^:]+)(:)([^$;\n\r]*)(?=$|;)', bygroups(Name.Attribute, Operator, String))
		],
		'statement': [
			(r'[\t ]*[^\|>;\n\r]+', String),
			(r'(\s*)(\|)(\s*)([^\|>;\n\r]+)', bygroups(Text, Operator, Text, Name.Variable)),
			(r'(\s*)(>)(\s*)([^\|\{;\n\r]+)', bygroups(Text, Operator, Text, String), 'root'),
			(r'\s*>\s*\{', Operator, 'output-bundle'),
			(r'\s*(;|$)', Operator, 'root')
		],
		'input-bundle': [
			(r'\s*[^\|\}\n\r]+', String),
			(r'(\s*)(\|)(\s*)([^\|\}\n\r]+)(\s*)(;|$|(?=\}))', bygroups(Text, Operator, Text, Name.Variable, Text, Operator)),
			(r'\}', Operator, 'statement')
		],
		'output-bundle': [
			(r'\s*[^;\}\n\r]+', String),
			(r'\s*;', Operator),
			(r'\s*\}', Operator, 'root')
		]
	}
