train = open("FinalDistribution/Output/finalSegmentation.txt").read()
test = open("testcorp.txt").read()
train = train.decode("utf-8")
test = test.decode("utf-8")
train = train.replace(':','\n').split('\n')
test = test.split()
for i in range(0,len(test)):
	for j in range(0,len(train)-1):
		if (j%2==0):
			if  (train[j]==test[i]):
				print ':'.join([train[j].encode("utf=8"),train[j+1].encode("utf=8")])
