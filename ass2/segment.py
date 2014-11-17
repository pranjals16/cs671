#!/usr/bin/python
# -*- coding: utf-8 -*-
f=open('line.txt','r').read()
f=f.strip()
matra=open('matra.txt','r').read()
matra=matra.decode('utf-8')
f = f.decode('utf-8')
g=open('filwords.txt','r').read()
g = g.decode('utf-8')
g = g.split()
word = ''
temp = ''
temp2=''
j=0
while j in range(len(f)):
#	print "f=",f,j
	word = word+(f[j])
	j=j+1
#	print "word=",word
	for i in range(len(g)):
		if word == g[i]:
			freq=g[i-1]
			if int(freq)>50:
				print freq
				if j<len(f):
					if f[j] in matra:
						continue				
				temp = temp+word+' '
				temp2 = temp+word
				f=f[j:len(f)]
				j=0
				word=''
print temp.encode('utf-8')
