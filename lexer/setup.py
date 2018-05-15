from setuptools import setup
setup(
	name="Pygments AssetsToC Plugin",
	version="1.0.0",
	entry_points="""
		[pygments.lexers]
		atoc_lexer = atoclexer.lexer:AtocLexer
	""",
	description="A Pygments AssetsToC lexer",
	author="Siphalor",
)
