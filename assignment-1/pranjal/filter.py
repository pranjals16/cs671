#!/usr/bin/python
# -*- coding: utf-8 -*-
import re
f=open('finalcorpus.txt','r')
for line in f:
	line=line.replace('\"','\n')
	line=line.replace('.','\n')
	line=line.replace('`','\n')
	line=line.replace('>','\n')
	line=line.replace('<','\n')
	line=line.replace('~','\n')
	line=line.replace(',','\n')
	line=line.replace('\'','\n')
	line=line.replace(')','\n')
	line=line.replace('[','\n')
	line=line.replace(']','\n')
	line=line.replace('\\','\n')
	line=line.replace('/','\n')
	line=line.replace('…','\n')
	line=line.replace('(','\n')
	line=line.replace(':','\n')
	line=line.replace(';','\n')
	line=line.replace('%','\n')
	line=line.replace('’','\n')
	line=line.replace('‘','\n')
	line=line.replace('!','\n')
	line=line.replace('“','\n')
	line=line.replace('*','\n')
	line=line.replace('?','\n')
	line=line.replace('।','\n')
	line=line.replace('-','\n')
	line=line.replace(' ','\n')
	line=line.replace('\t','\n')

	line=line.replace('१','\n')
	line=line.replace('२','\n')
	line=line.replace('३','\n')
	line=line.replace('४','\n')
	line=line.replace('५','\n')
	line=line.replace('६','\n')
	line=line.replace('७','\n')
	line=line.replace('८','\n')
	line=line.replace('९','\n')
	line=line.replace('०','\n')
	
	line = re.sub(r'[a-zA-Z0-9]', '\n', line) 
	print line
