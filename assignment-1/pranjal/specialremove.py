text = open("3.txt").read()
special = open("special.txt").read()

text = text.decode("utf-8")
special = special.decode("utf-8")

special = special.split()

words = text.split()

for i in range(0,len(words)):
	if i%2 != 0:
		count = 0
		for j in range(0,len(words[i])):
			if words[i][j] in special:
				count = count + 1;
		
		if (len(words[i]) - count) >= 3:
			print ' '.join([words[i-1].encode('utf-8'),words[i].encode('utf-8')])
				
		

