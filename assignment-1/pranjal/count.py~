import re
x=open('2.txt','r')
count={}
for word in x:
	if word in count:
		count[word]+=1
	else:
		count[word]=1
for word, times in count.items():
	if times > 1:
		print times,word
