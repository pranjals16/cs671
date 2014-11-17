#!/usr/bin/python
# -*- coding: utf-8 -*-
f=open('line.txt','r').read()
f = f.decode('utf-8')
g=open('filteredcorpus.txt','r').read()
g = g.decode('utf-8')
g = g.split()
word = 'वो'
for i in range(len(g)):
	print g[i].encode('utf-8')
	print word
	if word == g[i].encode('utf-8'):
		freq=g[i-1]
		print "yay"
