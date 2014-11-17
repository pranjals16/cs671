import re
f=open('2.txt','r')
g=open('2.txt','r').read()
count={}
for word in f:
	if word in count:
		count[word]+=1
	else:
		count[word]=1
for word, times in count.items():
	print times,word
#	fr=re.findall(word,g)
#	print len(fr),word
