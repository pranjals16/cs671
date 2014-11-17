def precision(train,test):
	data_list=[]
	if len(train)!=len(test):
		print "Size Error"
		return -1
	else:
		for i in xrange(len(train)):
			if (train[i]==1 and train[i]==test[i]):
				data_list.append(1);
			else:
				data_list.append(0);
		return float(sum(data_list))/sum(train)		

def recall(train, test):
	data_list=[]	
	if len(train)!=len(test):
		print "Size Error"
		return -1
	else:
		for i in xrange(len(train)):
			if (train[i]==1 and train[i]==test[i]):
				data_list.append(1);
			else:
				data_list.append(0);
		return float(sum(data_list))/sum(test)

def fscore (precision, recall):
	if (precision+recall!=0):
		return 2*precision*recall/(precision+recall)
	else:
		return 0

def get_binary_list(word):
	bin_list=[]
	flag=0
	for i in xrange(len(word)):
		if(word[i]=='+'):
			bin_list.append(1);
			flag=1;
		else:
			if(flag==1):
				flag=0;
			else:
				bin_list.append(0);
	return bin_list

def compare_words(wtrain,wtest):
	train=get_binary_list(wtrain);
	test=get_binary_list(wtest);
	print precision(train,test), recall(train,test),fscore(precision(train,test), recall(train,test));
i=0
f="pranjal.txt"
big_train=[]
big_test=[]
with open(f,'r') as f:
	for line in f:
		[wtrain,wtest]=line.split(' ');
		wtest=wtest[:-1]
		if(len(get_binary_list(wtrain))!=len(get_binary_list(wtest))):
			continue
		else:
			i=i+1
			big_train.extend(get_binary_list(wtrain));
			big_test.extend(get_binary_list(wtest));
	prec=precision(big_train,big_test)
	rec=recall(big_train,big_test)
	print "precision=",prec,"\nrecall=",rec,"\nfscore=",fscore(prec,rec);
