The folder should contain: 
corpus.txt (Hindi data set)
FinalDistribution folder (Undivide++)
testcorp.txt (test corpus)
1)Run list.py: removes all english characters and hindi numeric characters along with all symbols and punctuations (all the undesirable characters (A-Z, a-z, 0-9 etc are stored in disjoin.txt); outputs words.txt which contains the frequency-word distribution from the data set:-
python list.py
2)Run filter.py: set tf (threshold freqency) and tc (minimum hindi-legnth of word) in the code; run it to get filwords.txt which is the result of applying these filters to words.txt; the code uses matra.txt to count hindi-word legnth:-
python filter.py
3)Switch to Undivide++ directory; compile and run undivide++ on filwords.txt with desired parameters:-
cd FinalDistribution
g++ UnsupervisedWordSegmentation.cpp
./a.out filwords.txt 1 1 1 0 0
cd ..
5)Run pick.py which picks and prints only those words which are in testcorp.txt from Output/finalSegmentation.txt:-
python pick.py > output.txt