f=open('2.txt','r')
g={}
for line in f:
	g=g.update(line)
print g
