test = open("pranjalseg.txt").read()
train = open("pranjalresult.txt").read()


train = train.decode("utf-8")
test = test.decode("utf-8")
test = test.replace(':','\n').split('\n')
train = train.replace(':','\n').split('\n')

for j in range(0,len(train)-1):
	if (j%2!=0):
		print ' '.join([train[j].encode("utf-8"),test[j].encode("utf-8")])
	
				
		

