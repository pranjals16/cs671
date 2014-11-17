f = open('corpus.txt').read()
f = f.decode('utf-8')
chars = open('disjoin.txt').read()
chars = chars.decode('utf-8')
for ch in chars:
	f = f.replace(ch,'\n')
f=f.split()
count = {}
for word in f:
	if word in count:
		count[word] += 1
	else:
		count[word] = 1
f = open('words.txt', 'w+')
for word in count:
	print >>f, count[word], word.encode('utf-8')
