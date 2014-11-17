# -*- coding: utf-8 -*- 
f = open('words.txt').read()
g = open('filwords.txt', 'w+')
matra = open('matra1.txt').read()	 
f = f.decode('utf-8')
matra = matra.decode('utf-8')
matra = matra.split()
tf = 2
tc = 2
flag = 0
f = f.split('\n')
for line in f:
	if line == '':
		continue
	flag = 0
	count = 0
	num = int(line.split()[0])
	word = line.split()[1]
	if num >=tf:
		flag = 1
	for i in range(0,len(word)):
		if word[i] in matra:
			count += 1
		if word[i] == u'्':
			count += 0.5
	if (len(word) - count) >= tc:
		if flag == 1:
			print  >>g, num, word.encode('utf-8')
