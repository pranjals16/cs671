test = open("hi-wseg-gt.txt").read()
train = open("output.txt").read()


train = train.decode("utf-8")
test = test.decode("utf-8")
test = test.split('\n')
train = train.split('\n')

for j in range(0,len(train)-1):
	print 'a'.join([train[j].encode("utf-8"),test[j].encode("utf-8")])
	
				
		

