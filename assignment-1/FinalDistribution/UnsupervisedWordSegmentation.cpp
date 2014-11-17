#include<iostream>
#include<fstream>
#include<string>
#include<map>
#include<vector>
#include<cmath>
#include <cstdlib>
//#include<sys/stat.h>


#include"LinkList.h"

using namespace std;

#define MEMORY_MAX_VOCABULARY_SIZE 500000
#define MEMORY_AFFIX_ROOT_RATIO 10000


typedef map<string,int> Map;
typedef map<string,string> Map2;
typedef map<string,float> Map3;
typedef map<string,char> Map4;
Map vocabulary;
Map reverseVocabulary;
Map roots;
Map originalSuffixes;
Map smallRoots;
vector<string> lowFreqWords;
Map countStringPrefix;
Map countStringSuffix;
Map countStringPrefixFirst;
Map countStringSuffixFirst;
Map newMorphemesPrefix;
Map newMorphemesSuffix;
Map rankPrefix;
Map rankSuffix;
Map tempVocabulary1;
Map tempVocabulary2;
//vector<string> relationsPrefix[3000];
//vector<string> relationsSuffix[3000];
Map relationsPrefix[MEMORY_AFFIX_ROOT_RATIO];
Map relationsSuffix[MEMORY_AFFIX_ROOT_RATIO];
Map finalSuffix;
Map finalPrefix;
Map suffixCountOfRoot;
Map prefixCountOfRoot;
Map actualSuffixCountOfRoot;
Map actualPrefixCountOfRoot;
Map actualMorphemeCountOfRoot1;
Map actualMorphemeCountOfRoot2;
string inputs[MEMORY_MAX_VOCABULARY_SIZE];
vector<string> outputs[MEMORY_MAX_VOCABULARY_SIZE];
Map nouns;
Map verbs;
Map adjectives;
int testCount;
Map2 similarSuffixes;	//similarSuffixes[ers]=er
Map2 similarSuffixesOpposite;	//similarSuffixesOpposite[er]=ers
Map deletedSuffixes;			//it contains similar suffixes like ers and er so they cant be used in allomoprhic rule learning.
Map2 conditionalSimilarSuffixes;
Map4 conditionOfSimilarSuffix;
Map2 allomorphRoots;
Map allomorphRootsCount;
Map3 freqThreshold;
Map3 freqThresholdPrefix;
Map selectedForClustering;
Map clusterFeatureValue[3];
Map totallyDeleted;	//deleted suffixes like e, ...
Map lowFreqRatioPrefix;
Map suffixesForSmallRoot;
Map prefixesForSmallRoot;
ofstream freqCheckFile;
ofstream testMultipleOutputFile;
ofstream typeFile;
Map smallRootsSelected;
string suffixLevelSimilarityString[MEMORY_AFFIX_ROOT_RATIO][10];
float suffixLevelSimilarityWeight[MEMORY_AFFIX_ROOT_RATIO][10];
ofstream finalOutputFile;


//The following parameters need to be changed depending on different language's morphological complexity.
#define SMALL_ROOT_LENGTH 3
#define LOW_FREQUENCY_DROPOUTS 1
#define LOW_FREQUENCY_DROPOUTS_LEARNING 1
#define SUFFIX_CUTOFF_THRESHOLD 70	//CRITICAL	
#define PREFIX_CUTOFF_THRESHOLD 60	//CRITICAL
#define COMPOSITE_SUFFIX_THRESHOLD 0.65
#define WRFR_SUFFIX_THRESHOLD 10
#define WRFR_PREFIX_THRESHOLD 1.5
#define SLS_NORMALIZATION_CONSTANT 5
#define ALLOMORPH_REPLACEMENT_THRESHOLD 3
#define ALLOMORPH_DELETION_THRESHOLD 3	
#define ALLOMORPH_ADDITION_THRESHOLD 3 
#define PROMOTE_LONG_SEGMENTATION 1
#define PROMOTE_LONG_SEGMENTATION_LENGTH 15
#define INDUCE_OUTOFVOCABULARY_ROOTS 0
#define INDUCE_OUTOFVOCABULARY_ROOTS_THRESHOLD 5

int ifFoundRoot2(string output);
int checkConstraints2(string output);
int noOfDiv(string str);
void getType(string myParse, vector<string>& typeStr);


class Info
{
	public:
	char ch;
	int finalOrNot;
	int derivedOrNot;	
	Info *sibling;
	Info *child;
	int count;
	Info()
	{
		finalOrNot=0;
		derivedOrNot=0;
		ch=0;
		sibling=NULL;
		child=NULL;
		count=0;
	}
};

class Trie
{
	Info *root;
	Info *reversedRoot;
	int threshold1;
	int threshold2;
	int prefixCount;
	int suffixCount;
	ofstream featureFile;
	ofstream cooccurFile;	
	ofstream clusterFile;
	ofstream freqFile;
	ofstream rulesFile;
	ofstream compoAllomorphFile;
	ofstream conditionalSimilarSuffixFile;
	ofstream checkFile;
	ofstream rootsFile;
	ofstream weightFile;
	ofstream vocabularyFile;
	ofstream goldsmithFile;	
	ofstream topSuffixes;
	ofstream topPrefixes;
	ofstream outOfVocabularyRoots;
	int allomorphReplacementThreshold;
	int allomorphAdditionThreshold;
	int allomorphDeletionThreshold;
	int	step2OnOff;
	int step3OnOff;
	int step4OnOff;
	int step5OnOff;

	
	public:
	Trie()
	{
		root=new Info;
		reversedRoot=new Info;		
		prefixCount=0;
		suffixCount=0;
		
		//mkdir("Output", 0777);
		
		featureFile.open("Output/featureFile.txt");
		cooccurFile.open("Output/coccurFile.txt");
		clusterFile.open("Output/clusterFile.txt");
		freqFile.open("Output/freqFile.txt");
		rulesFile.open("Output/rulesFile.txt");
		compoAllomorphFile.open("Output/compositeAllomorph.txt");
		conditionalSimilarSuffixFile.open("Output/conditionalSimilarSuffix.txt");
		rootsFile.open("Output/rootsFile.txt");
		weightFile.open("Output/weightFile.txt");
		vocabularyFile.open("Output/vocabulary.txt");
		finalOutputFile.open("Output/finalSegmentation.txt");
		goldsmithFile.open("Output/goldsmithFile.txt");	
		topPrefixes.open("Output/topPrefixes.txt");	
		topSuffixes.open("Output/topSuffixes.txt");	
		outOfVocabularyRoots.open("Output/outOfVocabularyRoots.txt");	
		
		//default steps
		step2OnOff=1;
		step3OnOff=0;
		step4OnOff=1;
		step5OnOff=1;
	}

	void setThreshold(int th1, int th2){threshold1=th1; threshold2=th2;}
	void setAllomorphThreshold(int th1, int th2, int th3){ allomorphReplacementThreshold=th1; allomorphDeletionThreshold=th2; allomorphAdditionThreshold=th3;}
	
	//void reverse(string& str);
	void insert(int forwardOrBackward, int first, string str, int derivedOrNot);
	int find(string str);
	int findCount(int forwardOrBackward, string str);
	void getNewMorphemes(int);
	void divideAndConquer();	
	void stripRoot(Info *node, string str, vector<string> prev, int foundFirstBlack, string countStr);
	void stripRoot2(Info *node, string str, vector<string> prev, int foundFirstBlack, string countStr);
	void stripRootNew(Info *node, string str, vector<string> prev, int foundFirstBlack, string countStr);
	void stripRoot2New(Info *node, string str, vector<string> prev, int foundFirstBlack, string countStr);
	void printVocabulary();
	void insertVocabularyForward();
	void DFSSearch(int, Info *node, vector<string> str, int foundFirstBlack, string countStr);
	void insertVocabularyBackward();
	void resetRoot();
	void printCountString(int);
	void addCountString(int, string countStr);
	int addCountStringToVocabulary(string countStr);
	void multiplyCountString(int suffixOrPrefix);
	void printRoots();
	void insertRootForward();	
	void insertRootBackward();
	void printRelationStrings();
	void getFinalMorphemes();
	void getActualRootCount();
	void addPrefixCountOfRoot(string str);
	void addSuffixCountOfRoot(string str);
	void getDivisionsOfSuffixes();
	void getDivisionsOfSuffix(string str, vector<string>& options);
	void printSimilarSuffixes();
	void showDivisionCountOfSuffix(string suffix, string parseStr, float& prev);
	float compareRelationStrings(Map v1, Map v2);
	void getSmallRootsFromVocabulary();
	void checkWithOriginalSuffixes();
	void filterRelationString(string, int, vector<string>& deleted, int &suffixDeleted);
	void filterByFrequency(string current, int index);
	void filterByFrequencyPrefix(string current, int index);
	int freqMatch(string root, string inflection,string suffix);
	int freqMatchPrefix(string root, string inflection,string prefix);
	int freqMatchSmallRoot(string root, string inflection,string suffix);
	int freqMatchPrefixSmallRoot(string root, string inflection,string prefix);	
	void filterRelationStrings1();
	void filterRelationStrings2();
	void cluster();
	int getRange(int x);
	void findAllomorph();
	void findAllomorphByDeletion();
	void findAllomorphByAddition();
	//int ifVowel(char ch);
	int bothConsonent(string str);
	int findSimilar(string word);
	char learnLeft(vector<string> words);
	void findSimilar(string word, vector<char>& similar);
	int findSimilarByDeletion(string word);
	int findSimilarByAddition(string word);
	void findSimilarByDeletion(string word, vector<char>& similar);
	void findSimilarByAddition(string word, vector<char>& similar);
	void removeSimilarSuffixes();
	void printUncomparables(Map v1, Map v2);
	void printMorphemes();
	int compareAllomorph(string inflection, string old, string newRoot, string newSuffix, int newCount);
	int compareCompositeAllomorph(string old, string newSuff1, string newSuff2);
	float lastCharOfWords(Map relationString, char char1);	
	char allomorphicCompositeFirst(string suffix);
	int compare(char,char);
	void processSuffixesAndRoots();
	void getClustersAndMorphemeInformation();
	void getDivisionsOfRootsByRoot(string str, vector<string>& options);
	void getDivisionsOfRootsByRoots();
	int ifSmallOrMorpheme(string sub);
	void getDivisionsOfSmallRoot(string str, vector<string>& options);
	void getDivisionsOfSmallRoots();
	void getRootMorpheme(string output, string& root, string& morpheme, int& suffixOrPrefix);
	void deletePrefixSuffixesFromSmallRoots();
	void getSuffixesPrefixesForSmallRoots();
	void getDivisionOfSuffixByRoot(string suffix, vector<string>& options);
	void getDivisionOfSuffixesByRoot();
	void printAllIntoFile();
	void insertWithCount(int forwardOrBackward, string str);
	void getRootCount();
	int getCount(int forwardOrBackward, string str);
	void deletePrefixSuffixes();
	void showHighFrequencyWords();
	void findAllomorph2();
	int findSimilar2(string word);
	void findSimilar2(string word, vector<char>& similar, vector<int>& pos);
	void getNewRoots();
	void getNewRoots2();	
	void getNewRootsFromRoots();
	float checkSuffixLevelSimilarity(string current, Map suffixList);
	void copyWeight(int i, int j);
	void getSuffixLevelSimilarityWeightsForAll();
	void getSuffixLevelSimilarityWeights(string current);
	int findSimilarAtBeginning(string word);
	void findSimilarAtBeginning(string word, vector<char>& similar);
	int compareAllomorph2(string old, string oldSeg, string newRoot, string newMorpheme, int newCount);	
	void findAllomorphAtBeginning();
	void findAllomorphAtBeginning2();
	char learnRight(vector<string> words);
	int freqMatchMultipleRoot(string inflection, string root1, string root2);
	int freqMatchMultipleRootSmall(string inflection, string root1, string root2);
	void copyRelationString(string suffix1, string suffix2);
	void copySimilarSuffixesToRelationString();
	int ifRootPlusSuffix(string inflection, string& otherRoot, string& otherSuffix);
	void copyAllomorphsToRelationString();
	void deleteRootsFromPrefixesAndSuffixes();
	void setSteps(int step2, int step3, int step4, int step5);
};

float sigmoid(int x)
{	
	float hyp=x*0.02;
	hyp= (exp(hyp) - exp(-hyp)) / (exp(hyp) + exp(-hyp));
	return 10*hyp;
}

void reverse(string& str)
{
	int i=0;
	int j=str.length()-1;
	char temp;
	while(i<j)
	{
		temp=str[i];
		str[i]=str[j];
		str[j]=temp;
		i++;
		j--;
	}
}

int ifVowel(char ch)
{
	if (ch=='A') return 1;
	if (ch=='E') return 1;
	if (ch=='I') return 1;
	if (ch=='O') return 1;
	if (ch=='U') return 1;
	if(ch=='V') return 1;
	if(ch=='*') return 1;
	if(ch=='!') return 1;
	if(ch=='?') return 1;

	if (ch=='a') return 1;
	if (ch=='@') return 1;
	if (ch=='i') return 1;
	if (ch=='#') return 1;
	if (ch=='u') return 1;
	if(ch=='$') return 1;
	if(ch=='e') return 1;
	if(ch=='^') return 1;
	if(ch=='o') return 1;
	if(ch=='&') return 1;

	return 0;
}

int getLengthWithoutHallant(string str)
{
	int len=0;
	for(int i=0;i<str.length();++i)
	{
		if(str.at(i)=='~') continue;
		len++;
	}
	return len;
}


void tokenize(const string& str,
                      vector<string>& tokens,
                      const string punctuation, const string& delimiters=" ")
{

    string delPunc=delimiters+punctuation;     

    // Find First delimiter
    string::size_type lastPos = str.find_first_not_of(delPunc, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delPunc, 0);

    //if there is delimeters at the beginning    
    while((pos<lastPos) && (pos!=string::npos))
    {
	//cout<<"POS:"<<pos<<" ";
	if(delimiters.find(str[pos])==string::npos) 	//if delimiters not found
	{
		//cout<<"Add:"<<str.substr(pos, 1)<<" ";
		tokens.push_back(str.substr(pos, 1));	
	}
	pos = str.find_first_of(delPunc, pos+1);	
    }
	

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        
	tokens.push_back(str.substr(lastPos, pos - lastPos));

	if(pos!=string::npos)
	{
	    if(delimiters.find(str[pos])==string::npos) //if delimiters not found
		tokens.push_back(str.substr(pos, 1));
	}

        // find next non delimeter
        lastPos = str.find_first_not_of(delPunc, pos);
        // Find next delimiter"
        if(pos!=string::npos) pos = str.find_first_of(delPunc, pos+1);

	while((pos<lastPos) && (pos!=string::npos))
    	{
		//if the delimiters not found push that puncDel
		if(delimiters.find(str[pos])==string::npos) 
			tokens.push_back(str.substr(pos, 1));	
		pos = str.find_first_of(delPunc, pos+1);
    	}
    }
}

//Set the steps here.
void Trie::setSteps(int step2, int step3, int step4, int step5)
{
	step2OnOff=step2;
	step3OnOff=step3;
	step4OnOff=step4;
	step5OnOff=step5;	
}


void Trie::addSuffixCountOfRoot(string str)
{
	if(suffixCountOfRoot.find(str)!=suffixCountOfRoot.end())
		suffixCountOfRoot[str]+=1;
	else suffixCountOfRoot[str]=1;

}

void Trie::addPrefixCountOfRoot(string str)
{
	if(prefixCountOfRoot.find(str)!=prefixCountOfRoot.end())
		prefixCountOfRoot[str]+=1;
	else prefixCountOfRoot[str]=1;

}


void Trie::insert(int forwardOrBackward, int first, string str, int derivedOrNot=0)
{
	Info *prev;
	Info *current;

	if(forwardOrBackward) prev=root;
	else prev=reversedRoot;

	current=prev->child;

	int i=0;
	int len=str.length();
	int childOrSibling=1;
	
	string sub="";
	while(current!=NULL)
	{
		if(current->ch==str[i])
		{
			if(i==(len-1)) 
			{
				current->finalOrNot=1;
				current->derivedOrNot=derivedOrNot;
				return;
			}			
			
			sub+=str[i];
			if(first)
			{
			if(current->finalOrNot==1)//if you get some root in the middle of the path
			{
				if(forwardOrBackward) addSuffixCountOfRoot(sub);
				else addPrefixCountOfRoot(sub);
			}
			}

			i++;
			prev=current;
			childOrSibling=1;
			current=current->child;
		}
		else
		{
			prev=current;
			childOrSibling=0;
			current=current->sibling;
		}
	}

	if(childOrSibling)
	{
		prev->child=new Info;
		current=prev->child;
	}
	else 
	{
		prev->sibling=new Info;
		current=prev->sibling;
	}

	current->ch=str[i];
	i++;

	while(true)
	{
		if(i==len)
		{
			current->finalOrNot=1;
			current->derivedOrNot=derivedOrNot;
			return;
		}

		current->child=new Info;
		current=current->child;
		current->ch=str[i];		
		i++;
	}

}

void Trie::insertWithCount(int forwardOrBackward, string str)
{
	Info *prev;
	Info *current;

	if(forwardOrBackward) prev=root;
	else prev=reversedRoot;

	current=prev->child;

	int i=0;
	int len=str.length();
	int childOrSibling=1;
	
	string sub="";
	string rest;
	string revWhole=str;
	reverse(revWhole);	
	string revSub;
	int index;

	while(current!=NULL)
	{
		if(current->ch==str[i])
		{
			if(i==(len-1)) 
			{
				current->finalOrNot=1;
				return;
			}			
			
			sub+=str[i];
			
			if(current->finalOrNot==1)//if you get some root in the middle of the path
			{
				rest=str.substr(i+1);
				if(forwardOrBackward) 
				{
 				   if(newMorphemesSuffix.find(rest) != newMorphemesSuffix.end())
				   {
					index=newMorphemesSuffix[rest];

					//if found in relation string 
					//i.e. it matches frequency filtering and Suffix Level Sim
					if(relationsSuffix[index].find(sub) != relationsSuffix[index].end())
						current->count++;
				   }
				   //else if(roots.find(rest) != roots.end())
				   else if(vocabulary.find(rest) != vocabulary.end())	
				   {
					if(freqMatchMultipleRoot(str, sub, rest))
						current->count++;
				   }
				}
				else
				{
 				   if(newMorphemesPrefix.find(rest) != newMorphemesPrefix.end())
				   {
					index=newMorphemesPrefix[rest];

					//if found in relation string 
					//i.e. it matches frequency filtering and Suffix Level Sim
					if(relationsPrefix[index].find(sub) != relationsPrefix[index].end())
						current->count++;
				   }
				   else 
				   {
					reverse(rest);
					//if(roots.find(rest) != roots.end())
					if(vocabulary.find(rest) != vocabulary.end())
					{
						revSub=sub;
						reverse(revSub);
						
						if(freqMatchMultipleRoot(revWhole, revSub, rest))
							current->count++;
					}
				   }
				}
			}
			
			i++;
			prev=current;
			childOrSibling=1;
			current=current->child;
		}
		else
		{
			prev=current;
			childOrSibling=0;
			current=current->sibling;
		}
	}

	if(childOrSibling)
	{
		prev->child=new Info;
		current=prev->child;
	}
	else 
	{
		prev->sibling=new Info;
		current=prev->sibling;
	}

	current->ch=str[i];
	i++;

	while(true)
	{
		if(i==len)
		{
			current->finalOrNot=1;			
			return;
		}

		current->child=new Info;
		current=current->child;
		current->ch=str[i];		
		i++;
	}	
}

int Trie::find(string str)
{
	Info *current=root->child;
	int i=0;
	int len=str.length();
	
	while(current!=NULL)
	{
		if(current->ch==str[i])
		{
			if(i==(len-1))
			{
				//cout<<i<<endl;
				if(current->finalOrNot==1) return 1;
				else return 0;
			}
			
			i++;
			current=current->child;
		}
		else
		{
			current=current->sibling;
		}
	}
	//cout<<i<<endl;
	return 0;
}

//it considers only valid suffixes to get the root count
int Trie::getCount(int forwardOrBackward, string str)
{
	Info *current;
	if(forwardOrBackward)
		current=root->child;
	else current=reversedRoot->child;;

	int i=0;
	int len=str.length();
	while(current!=NULL)
	{
		if(current->ch==str[i])
		{
			if(i==(len-1))
			{
				//cout<<i<<endl;
				if(current->finalOrNot==1) return current->count;
				else return 0;
			}
			
			i++;
			current=current->child;
		}
		else
		{
			current=current->sibling;
		}
	}
	//cout<<i<<endl;
	return 0;
}

//it was done previously but it considers all candidate suffixes to get the root count
int Trie::findCount(int forwardOrBackward, string str)
{
	/*
	Info *current;
	if(forwardOrBackward) current=root->child;
	else current=reversedRoot->child;
	int i=0;
	int len=str.length();
	
	while(current!=NULL)
	{
		if(current->ch==str[i])
		{
			if(i==(len-1))
			{
				//cout<<i<<endl;
				if(current->finalOrNot==1) return current->count;
				else return 0;
			}
			
			i++;
			current=current->child;
		}
		else
		{
			current=current->sibling;
		}
	}
	//cout<<i<<endl;
	return 0;
	*/

	if(forwardOrBackward)
	{
		if(actualSuffixCountOfRoot.find(str)!=actualSuffixCountOfRoot.end())
			return actualSuffixCountOfRoot[str];
		else return 0;
	}

	else
	{
		if(actualPrefixCountOfRoot.find(str)!=actualPrefixCountOfRoot.end())
			return actualPrefixCountOfRoot[str];
		else return 0;

	}
}


void Trie::addCountString(int forwardOrBackward, string countStr)
{
	if(countStr!="") 
	{	
		if(forwardOrBackward)
		{
			if(countStringSuffix.find(countStr) == countStringSuffix.end()) 
				countStringSuffix[countStr]=1;		
			else countStringSuffix[countStr]=countStringSuffix[countStr]+1;
		}
		else 
		{
			if(countStringPrefix.find(countStr) == countStringPrefix.end()) 
				countStringPrefix[countStr]=1;		
			else countStringPrefix[countStr]=countStringPrefix[countStr]+1;
		}
	}
}

void Trie::DFSSearch(int forwardOrBackward, Info *node, vector<string> prev, int foundFirstBlack, string countStr)
{
	if(node==NULL) return;
	if(node->ch!=0) //if not root
	{
		//str+=node->ch;	//if not root
		if(foundFirstBlack) countStr+=node->ch;
	}

	vector<string> prevNew;
	if(node->finalOrNot)
	{					
		if(countStr!="") prevNew.push_back(countStr);
		addCountString(forwardOrBackward, countStr);		
		
		for(int i=0;i<prev.size();++i)
		{			
			prevNew.push_back(prev.at(i)+countStr);
			addCountString(forwardOrBackward, prev.at(i)+countStr);
		}
		
		//str="";
		countStr="";
		foundFirstBlack=1;		
		
	}	

	Info *current=node->child;
	while(current!=NULL)
	{
		if(node->finalOrNot) 
			DFSSearch(forwardOrBackward, current,prevNew,foundFirstBlack,countStr);
		else DFSSearch(forwardOrBackward, current,prev,foundFirstBlack,countStr);
		current=current->sibling;
	}
}


void Trie::stripRoot(Info *node, string str, vector<string> prev, int foundFirstBlack, string countStr)
{
	if(node==NULL) return;
	if(node->ch!=0) //if not root
	{
		str+=node->ch;	//if not root
		if(foundFirstBlack) countStr+=node->ch;
	}

	int x,flag=0;
	vector<string> prevNew;

	int index;
	string newStr;
	if(node->finalOrNot)
	{		
		if(!foundFirstBlack)
		{			
			if(str!="") 
			{
				roots[str]=1;
			}
		}
		
		else
		{
			if(countStr!="") prevNew.push_back(countStr);
			if( (countStr!="") 
				&& (newMorphemesSuffix.find(countStr) != newMorphemesSuffix.end()))
				{					
					flag=1;	
					index=newMorphemesSuffix[countStr];
					newStr=str.substr(0,str.length()-countStr.length());
					relationsSuffix[index][newStr]=1;
				}
	
			for(int i=0;i<prev.size();++i)
			{			
				prevNew.push_back(prev.at(i)+countStr);
				//if((prev.at(i)+countStr)=="ality") cout<<"\nXX:"<<str;

				if( (countStr!="") 
				&& (newMorphemesSuffix.find(prev.at(i)+countStr) 
					!= newMorphemesSuffix.end()))
				{
					flag=1;	
					index=newMorphemesSuffix[prev.at(i)+countStr];
					newStr=str.substr(0,str.length()-(prev.at(i)+countStr).length());
					relationsSuffix[index][newStr]=1;
					
				}	
			}			
			if(flag==0) 
			{
				if(str!="")
				{
					//if(tempVocabulary2.find(str)!=tempVocabulary2.end());	
					//else	
						
						roots[str]=1;
				}			
			}
			else
				tempVocabulary1[str]=1;//the words that has already been splitted
				//roots.erase(str);			
		}
		
		//str="";
		countStr="";
		foundFirstBlack=1;
	}	
	
	Info *current=node->child;
	while(current!=NULL)
	{
		if(node->finalOrNot) 
			stripRoot(current,str,prevNew,foundFirstBlack,countStr);
		else
			stripRoot(current,str,prev,foundFirstBlack,countStr);

		current=current->sibling;
	}
}

void Trie::stripRoot2(Info *node, string str, vector<string> prev, int foundFirstBlack, string countStr)
{
	if(node==NULL) return;
	if(node->ch!=0) //if not root
	{
		str+=node->ch;	//if not root
		if(foundFirstBlack) countStr+=node->ch;
	}

	int x,flag=0;
	vector<string> prevNew;

	string temp;
	int index;
	string newStr;
	if(node->finalOrNot)
	{		
		if(!foundFirstBlack)
		{			
			if(str!="")
			{
				temp=str;
				reverse(temp);

				//if already splitted out 
				if(tempVocabulary1.find(temp)!=tempVocabulary1.end());	
				else
					roots[temp]=1;
			}
		}
		
		else
		{
			if(countStr!="") prevNew.push_back(countStr);
			if( (countStr!="") 
				&& (newMorphemesPrefix.find(countStr) != newMorphemesPrefix.end()))
				{
					flag=1;	
					index=newMorphemesPrefix[countStr];
					newStr=str.substr(0,str.length()-countStr.length());
					relationsPrefix[index][newStr]=1;
				}

			for(int i=0;i<prev.size();++i)
			{			
				prevNew.push_back(prev.at(i)+countStr);
				if( (countStr!="") 
				&& (newMorphemesPrefix.find(prev.at(i)+countStr) 
					!= newMorphemesPrefix.end()))
				{
					flag=1;	
					index=newMorphemesPrefix[prev.at(i)+countStr];
					newStr=str.substr(0,str.length()-(prev.at(i)+countStr).length());
					relationsPrefix[index][newStr]=1;
				}
			}			
			
			if(flag==0) 
			{
				if(str!="")
				{
					temp=str;
					reverse(temp);

					//if already splitted out
					if(tempVocabulary1.find(temp)!=tempVocabulary1.end());
					else
						roots[temp]=1;
				}
			}
			else	
			{		
				temp=str;
				reverse(temp);				
				roots.erase(temp); //the words that has already been splitted
			}


		}
		
		//str="";
		countStr="";
		foundFirstBlack=1;	
	}	
	
	Info *current=node->child;
	while(current!=NULL)
	{
		if(node->finalOrNot) 
			stripRoot2(current,str,prevNew,foundFirstBlack,countStr);
		else
			stripRoot2(current,str,prev,foundFirstBlack,countStr);
		
		current=current->sibling;
	}
	
}


void Trie::stripRootNew(Info *node, string str, vector<string> prev, int foundFirstBlack, string countStr)
{
	if(node==NULL) return;
	if(node->ch!=0) //if not root
	{
		str+=node->ch;	//if not root
		if(foundFirstBlack) countStr+=node->ch;
	}

	int x,flag=0;
	vector<string> prevNew;

	int index;
	string newStr;
	if(node->finalOrNot)
	{		
		if(!foundFirstBlack)
		{			
			if(str!="") 
			{
				roots[str]=1;
			}
		}
		
		else
		{
			if(countStr!="") prevNew.push_back(countStr);
			if( (countStr!="") 
				&& (newMorphemesSuffix.find(countStr) != newMorphemesSuffix.end()))
				{	
					index=newMorphemesSuffix[countStr];						
					newStr=str.substr(0,str.length()-countStr.length());

					if( relationsSuffix[index].find(newStr) != relationsSuffix[index].end() ) 
						flag=1;
				}
	
			for(int i=0;i<prev.size();++i)
			{			
				prevNew.push_back(prev.at(i)+countStr);
				//if((prev.at(i)+countStr)=="ality") cout<<"\nXX:"<<str;

				if( (countStr!="") 
				&& (newMorphemesSuffix.find(prev.at(i)+countStr) 
					!= newMorphemesSuffix.end()))
				{					
					index=newMorphemesSuffix[prev.at(i)+countStr];
					newStr=str.substr(0,str.length()-(prev.at(i)+countStr).length());
					
					if( relationsSuffix[index].find(newStr) != relationsSuffix[index].end() ) 
						flag=1;
				}	
			}			
			if(flag==0) 
			{
				if(str!="")
				{
					//if(tempVocabulary2.find(str)!=tempVocabulary2.end());	
					//else	
						
						roots[str]=1;
				}			
			}
			else
				tempVocabulary1[str]=1;//the words that has already been splitted
				//roots.erase(str);			
		}
		
		//str="";
		countStr="";
		foundFirstBlack=1;
	}	
	
	Info *current=node->child;
	while(current!=NULL)
	{
		if(node->finalOrNot) 
			stripRootNew(current,str,prevNew,foundFirstBlack,countStr);
		else
			stripRootNew(current,str,prev,foundFirstBlack,countStr);

		current=current->sibling;
	}
}

void Trie::stripRoot2New(Info *node, string str, vector<string> prev, int foundFirstBlack, string countStr)
{
	if(node==NULL) return;
	if(node->ch!=0) //if not root
	{
		str+=node->ch;	//if not root
		if(foundFirstBlack) countStr+=node->ch;
	}

	int x,flag=0;
	vector<string> prevNew;

	string temp;
	int index;
	string newStr;
	if(node->finalOrNot)
	{		
		if(!foundFirstBlack)
		{			
			if(str!="")
			{
				temp=str;
				reverse(temp);

				//if already splitted out 
				if(tempVocabulary1.find(temp)!=tempVocabulary1.end());	
				else
					roots[temp]=1;
			}
		}
		
		else
		{
			if(countStr!="") prevNew.push_back(countStr);
			if( (countStr!="") 
				&& (newMorphemesPrefix.find(countStr) != newMorphemesPrefix.end()))
				{					
					index=newMorphemesPrefix[countStr];
					newStr=str.substr(0,str.length()-countStr.length());
					
					if( relationsPrefix[index].find(newStr) != relationsPrefix[index].end() ) 
						flag=1;
				}

			for(int i=0;i<prev.size();++i)
			{			
				prevNew.push_back(prev.at(i)+countStr);
				if( (countStr!="") 
				&& (newMorphemesPrefix.find(prev.at(i)+countStr) 
					!= newMorphemesPrefix.end()))
				{					
					index=newMorphemesPrefix[prev.at(i)+countStr];
					newStr=str.substr(0,str.length()-(prev.at(i)+countStr).length());
					if( relationsPrefix[index].find(newStr) != relationsPrefix[index].end() ) 
						flag=1;
				}
			}			
			
			if(flag==0) 
			{
				if(str!="")
				{
					temp=str;
					reverse(temp);

					//if already splitted out
					if(tempVocabulary1.find(temp)!=tempVocabulary1.end());
					else
						roots[temp]=1;
				}
			}
			else	
			{		
				temp=str;
				reverse(temp);				
				roots.erase(temp); //the words that has already been splitted
			}


		}
		
		//str="";
		countStr="";
		foundFirstBlack=1;	
	}	
	
	Info *current=node->child;
	while(current!=NULL)
	{
		if(node->finalOrNot) 
			stripRoot2New(current,str,prevNew,foundFirstBlack,countStr);
		else
			stripRoot2New(current,str,prev,foundFirstBlack,countStr);
		
		current=current->sibling;
	}
	
}


//Extract the top Suffixes and Prefixes
void Trie::getNewMorphemes(int suffixOrPrefix)
{

	Map::const_iterator pos;
	int len;
	string temp;
	int threshold;
	LinkList l1;
	l1.setMax(1000);	
	
	if(suffixOrPrefix)
	{
		topSuffixes<<"Top Suffixes Learned:"<<endl;	
		for (pos = countStringSuffix.begin(); pos != countStringSuffix.end(); ++pos)
		{		
			len=(pos->first).length();

			//if there is '~' at the end
			if(pos->first.at(len-1) == '~') continue;				
			if(pos->first.at(0) == '~') continue;				

			//if(len<2) continue;
			threshold=(len<=2)? (4-len)*threshold1 : threshold1;
			if(countStringSuffix[pos->first]>=threshold)
			{			
				//if the new morpheme is root itself then
				//if( vocabulary.find(pos->first) != vocabulary.end() )
				//	continue;
				
				newMorphemesSuffix[pos->first]=suffixCount+1;
				suffixCount++;
				l1.add(pos->first, countStringSuffix[pos->first]);				
			}
		}		
		
	}

	else
	{
		topPrefixes<<"Top Prefixes Learned:"<<endl;
		for ( pos = countStringPrefix.begin(); pos != countStringPrefix.end(); ++pos )
		{		
			len=(pos->first).length();

			if(pos->first.at(len-1) == '~') continue;				
			if(pos->first.at(0) == '~') continue;

			if(len==1) continue;
			threshold=(len<=2)? (4-len)*threshold2 : threshold2;
			if(countStringPrefix[pos->first]>=threshold)
			{			
				temp=pos->first;
				reverse(temp);

				//if the new morpheme is root itself then 
				//if( vocabulary.find(temp) != vocabulary.end() )
				//	continue;
								
				newMorphemesPrefix[pos->first]=prefixCount+1;
				prefixCount++;
				
				l1.add(temp, countStringPrefix[pos->first]);				
			}
		
		}
		
	}

	if(suffixOrPrefix) newMorphemesSuffix["'s"]=++suffixCount;
	if(suffixOrPrefix) newMorphemesSuffix["'"]=++suffixCount;

	//Print to a file
	l1.setIterator();
	string next;
	int count;
	int rank=1;
	while(true)
	{
		count=(int)l1.getNext(next);	
		if(count==0) break;
		if(suffixOrPrefix)
		{
			topSuffixes<<next<<endl;				
			rankSuffix[next]=rank;
			rank+=1;
		}
		else 
		{
			topPrefixes<<next<<endl;				
			rankPrefix[next]=rank;
			rank+=1;
		}
	}
	
	if(suffixOrPrefix) topSuffixes<<"\nTotal Suffixes Selected:"<<suffixCount<<endl;
	else topPrefixes<<"\nTotal Prefixes Selected:"<<prefixCount<<endl;		
	
	cout<< "Size=" <<prefixCount <<" " <<suffixCount<<endl;
	
}


void Trie::getRootCount()
{
	Map::const_iterator pos;
	for (pos = vocabulary.begin(); pos != vocabulary.end(); ++pos) 
	{			
		insertWithCount(1,pos->first);
	}
	
    	for (pos = reverseVocabulary.begin(); pos != reverseVocabulary.end(); ++pos) 
	{
		insertWithCount(0,pos->first);
	}

	for (pos = roots.begin(); pos != roots.end(); ++pos) 	
	{
		actualMorphemeCountOfRoot1[pos->first]=getCount(1,pos->first);
	}

	string temp;
	for (pos = roots.begin(); pos != roots.end(); ++pos) 	
	{
		temp=pos->first;
		reverse(temp);
		actualMorphemeCountOfRoot2[pos->first]=getCount(0,temp);
	}
}

void Trie::insertVocabularyBackward()
{
	Map::const_iterator pos;
	string rev;
    	for (pos = reverseVocabulary.begin(); pos != reverseVocabulary.end(); ++pos) 
	{
		//rev=pos->first;
		//reverse(rev);
		insert(0,1,pos->first);
	}	
}

void Trie::resetRoot()
{
	root->child=NULL;
	root->sibling=NULL;
	reversedRoot->child=NULL;
	reversedRoot->sibling=NULL;
}

void Trie::insertVocabularyForward()
{
	Map::const_iterator pos;	
	for (pos = vocabulary.begin(); pos != vocabulary.end(); ++pos) 
	{			
		insert(1,1,pos->first);
	}
	
	//cout<<"Count:"<<findCount(1,"saji")<<endl;
}

void Trie::insertRootBackward()
{
	Map::const_iterator pos;
	string rev;
    	for (pos = roots.begin(); pos != roots.end(); ++pos) 
	{
		rev=pos->first;
		reverse(rev);
		insert(0,0,rev);
	}
}

void Trie::insertRootForward()
{
	Map::const_iterator pos;	
	for (pos = roots.begin(); pos != roots.end(); ++pos) 
		insert(1,0,pos->first);
	
}


void Trie::printCountString(int forward)
{
	LinkList l1;
	l1.setMax(1000);	
	Map::const_iterator pos;	
	if(forward)
	{
		for (pos = countStringSuffix.begin(); pos != countStringSuffix.end(); ++pos) 
			l1.add(pos->first,pos->second);
	}
	else
	{
		for (pos = countStringPrefix.begin(); pos != countStringPrefix.end(); ++pos) 
			l1.add(pos->first,pos->second);
	}

	l1.setIterator();
	string next;
	int count;
	
	if(forward) cout<<"\nPrinting Top Suffixes:\n";
	else cout<<"\nPrinting Top Prefixes:\n";
	
	int rank=0;
	while(true)
	{
		count=(int)l1.getNext(next);	
		if(!forward) reverse(next);	
		if(count==0) break;
		cout<<next<<":"<<count<<" "<<rank<<endl;		
		rank++;
	}
}


void Trie::printRoots()
{
	rootsFile<<roots.size()<<endl;
	Map::const_iterator pos;
	for (pos = roots.begin(); pos != roots.end(); ++pos) 
	{		
		if(allomorphRoots.find(pos->first) != allomorphRoots.end()) 
			continue;
		rootsFile<<pos->first<<endl;
	}

}

void Trie::getSmallRootsFromVocabulary()
{
	Map::const_iterator pos;	
	string temp;
	for(pos=smallRoots.begin(); pos!=smallRoots.end(); ++pos)
	{
		//if *not* found in a selected list of k.
		if( smallRootsSelected.find(pos->first) == smallRootsSelected.end() )
			continue;
		
		roots[pos->first]=1;
		vocabulary[pos->first]=pos->second;

		temp=pos->first;
		reverse(temp);
		reverseVocabulary[temp]=1;
	}

}

void Trie::multiplyCountString(int suffixOrPrefix)
{
	Map::const_iterator pos;	
	string rev;
	
	if(suffixOrPrefix)
	{
		cout<<"Special Countstring Suffix:\n";
		for (pos = countStringSuffix.begin(); pos != countStringSuffix.end(); ++pos) 
		{
			//if( (pos->first).length() < 3)
			//countStringSuffix[pos->first]=(pos->second) / (4-(pos->first).length());

			//countStringSuffix[pos->first]=(pos->second) * (pos->first).length();
			countStringSuffix[pos->first]=(pos->second) * getLengthWithoutHallant(pos->first);

		}
	}
	else 
	{
		cout<<"Special Countstring Prefix:\n";
		for ( pos = countStringPrefix.begin(); pos != countStringPrefix.end(); ++pos ) 
		{	
			rev=pos->first;
			reverse(rev);

			//if( (pos->first).length() >= 5)
			//	if( (pos->second<10) && (pos->second>5) )
			//		cout<<rev<<" "<<pos->second<<endl;

			//countStringPrefix[pos->first]=(pos->second) * (pos->first).length();
			countStringPrefix[pos->first]=(pos->second) * getLengthWithoutHallant(pos->first);
		}
	}
	
}

void Trie::checkWithOriginalSuffixes()
{
	Map::const_iterator pos;
	int no=0, count=0;
	cout<<"Checking with Original Suffixes\n";
	for (pos = originalSuffixes.begin(); pos != originalSuffixes.end(); ++pos) 		
	{
		if( newMorphemesSuffix.find(pos->first) != newMorphemesSuffix.end() ) 
			count++;
		else cout<<pos->first<<endl;

		no++;
	}

	cout<<"\nSuffix Accuracy Label:"<<(float)count/no<<endl;

}

void Trie::printMorphemes()
{

	Map::const_iterator pos;	
	cout<<"Printing Suffixes\n";
	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos) 		
		cout<<pos->first<<endl;		

}

void Trie::divideAndConquer()
{
	/////////////////////////////////////////
	///STEP-1: Induce Prefixes/Suffixes/Roots 

	vector<string> prev;
	cout<<"going in forward direction\n";
	insertVocabularyForward();	
	DFSSearch(1,root,prev,0,"");
	multiplyCountString(1);
	printCountString(1);
	newMorphemesSuffix.clear();
	getNewMorphemes(1);
	checkWithOriginalSuffixes();

	//reverse and add to TRIE
	cout<<"going in reverse direction\n";	
	insertVocabularyBackward();		
	DFSSearch(0,reversedRoot,prev,0,"");	
	multiplyCountString(0);	
	printCountString(0);
	newMorphemesPrefix.clear();
	getNewMorphemes(0);

	cout<<"Striping roots:"<<endl;
	int prevSize=-1;
	for(int i=0;i<2;++i)
	{
		stripRoot(root,"",prev,0,"");	
		stripRoot2(reversedRoot,"",prev,0,"");		
	}
		
	//getActualRootCount();
	//getFinalMorphemes();
	
	//Identify and remove composite suffixes. 	
	getDivisionsOfSuffixes();
	removeSimilarSuffixes();

	
	
	/////////////////////////////////////////
	///STEP-2: Detecting Incorrect Attachments Using Relative Frequency
	
	//Filter according to WRFR -- word root frequency ratio
	if(step2OnOff)
		filterRelationStrings1();
		
	

	/////////////////////////////////////////
	///STEP-2: Applying Suffix Level Similarity
	
	//filter by suffix level similarity.
	if(step3OnOff)
		filterRelationStrings2();	
	
	//delete the prefixes and suffixes which went far below after filtering
	//deletePrefixSuffixes();	
	
	
	
	/////////////////////////////////////////
	///This is a necessary step part of Step-1.
	
	//Induce roots according to the prefixes and suffixes as learned before.
	cout<<"Striping roots:"<<endl;
	prevSize=-1;
	roots.clear();
	tempVocabulary1.clear();
	
	for(int i=0;i<2;++i)
	{
		stripRootNew(root,"",prev,0,"");	
		stripRoot2New(reversedRoot,"",prev,0,"");

		cout<<"\nPrinting Roots:"<<endl;
		//printRoots();
		
		if(roots.size()==prevSize) break;
		else prevSize=roots.size();

		resetRoot();
		insertRootForward();
		insertRootBackward();
		tempVocabulary1.clear();				
	}

	//printRelationStrings();

	resetRoot();
	getRootCount();	

	
	/////////////////////////////////////////
	///Step-4: Inducing Orthographic Rules and Allomorphs	
	if(step4OnOff)
	{	
		//Orthographic rule induction step.
		findAllomorph();
		findAllomorphByDeletion();
		findAllomorphByAddition();				
	}
	

	/*
	//handle small roots specially	
	getSmallRootsFromVocabulary();	
	
	findAllomorph();
	findAllomorphByDeletion();
	findAllomorphByAddition();


	//filter by weight
	filterRelationStrings2();	

	//again stripping roots
	cout<<"Striping roots:"<<endl;
	prevSize=-1;
	roots.clear();
	tempVocabulary1.clear();
	for(int i=0;i<5;++i)
	{
		stripRootNew(root,"",prev,0,"");	
		stripRoot2New(reversedRoot,"",prev,0,"");

		
		cout<<"\nPrinting Roots:"<<endl;
		//printRoots();
		
		if(roots.size()==prevSize) break;
		else prevSize=roots.size();

		resetRoot();
		insertRootForward();
		insertRootBackward();
		tempVocabulary1.clear();		
	}
	*/	
	
	
	getDivisionOfSuffixesByRoot();
	getDivisionsOfRootsByRoots();
	
	//Learn a set of new roots
	//It works well for some languages (e.g., Turkish), where roots often don't appear in the corpus individually
	if(INDUCE_OUTOFVOCABULARY_ROOTS)
		getNewRoots2();	//newRoot is induced using suffixal attachemnts
		//getNewRoot();	//newRoot is induced using suffixal attachemnts, and suffix level similarity constraints are imposed.
	
	
	/*
	//Small roots typically misbehave severly during the learning phase. 
	//Handle small roots specially: smallroot+{a selected set of suffixes and prefixes}	
	//Idea is to learn the segmentations rules from non-small segmentations (i.e., where root length>3), 
	//and then apply the rules to handle small-root (length 3) separately. 
	*/
	
	if(step5OnOff)
	{
		cout<<"\nHandling Small Roots Separately:"<<endl;
		deletePrefixSuffixesFromSmallRoots();
		getSuffixesPrefixesForSmallRoots();
		getDivisionsOfSmallRoots();	
	}
	
	
	//resetRoot();
	//getRootCount();

	printRoots();		
	copyAllomorphsToRelationString();
	copySimilarSuffixesToRelationString();

	//not working
	//deleteRootsFromPrefixesAndSuffixes();

	getSmallRootsFromVocabulary();
	
}

void Trie::deleteRootsFromPrefixesAndSuffixes()
{
	Map deleted;
	Map::const_iterator pos;
	for(pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos)
	{
		if(pos->first.length()<=3) continue;
		if(vocabulary.find(pos->first)!=vocabulary.end())
			deleted[pos->first]=1;
	}

	for(pos = deleted.begin(); pos != deleted.end(); ++pos)
		newMorphemesSuffix.erase(pos->first);

	string rev;
	deleted.clear();
	for(pos = newMorphemesPrefix.begin(); pos != newMorphemesPrefix.end(); ++pos)
	{

		if(pos->first.length()<=3) continue;

		rev=pos->first;
		reverse(rev);
		
		if(vocabulary.find(rev)!=vocabulary.end())
			deleted[pos->first]=1;		
	}

	for(pos = deleted.begin(); pos != deleted.end(); ++pos)
		newMorphemesPrefix.erase(pos->first);

}

//copy suffix1(ers) entries to the suffix2(er)
void Trie::copyRelationString(string suffix1, string suffix2)
{
	int index1=newMorphemesSuffix[suffix1];
	int index2=newMorphemesSuffix[suffix2];

	Map::const_iterator pos;
	for(pos = relationsSuffix[index1].begin(); pos != relationsSuffix[index1].end(); ++pos) 
	{
		relationsSuffix[index2][pos->first]=1;
	}
	
}

void Trie::copySimilarSuffixesToRelationString()
{
	Map2::const_iterator pos;
	int plus;
	string div;
	for(pos = similarSuffixes.begin(); pos != similarSuffixes.end(); ++pos) 
	{	
		div=pos->second;	
		plus=div.find_first_of('+');
		string sub=div.substr(0,plus);
		copyRelationString(pos->first,sub);
	}
}

void Trie::copyAllomorphsToRelationString()
{
	
	Map2::const_iterator pos;
	int plus, index;
	string div, root, suffix;
	for(pos = allomorphRoots.begin(); pos != allomorphRoots.end(); ++pos) 
	{	
		div=pos->second;	
		plus=div.find_first_of('+');
		root=div.substr(0,plus);
		suffix=div.substr(plus+1);
		index=newMorphemesSuffix[suffix];
		relationsSuffix[index][root]=1;
		cout<<"Adding "<<root<<" to "<<suffix;
	}
}

void Trie::deletePrefixSuffixes()
{
	cout<<"Deleting Affixes after freq filtering:\n";
	Map::const_iterator pos;
	int len;
	Map deleted;	
	int threshold;

	/*
	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos) 
	{	
		len=getLengthWithoutHallant(pos->first);
		threshold=(len<=2)? (4-len)*threshold1 : threshold1;
		if(countStringSuffix[pos->first]<threshold)
		{
			deleted[pos->first]=1;		
		}
	}
	for (pos = deleted.begin(); pos != deleted.end(); ++pos)
	{
		newMorphemesSuffix.erase(pos->first);
		cout<<"Deleted suffix:"<<pos->first<<" "<<countStringSuffix[pos->first]<<endl;
	}

	deleted.clear();
	for (pos = newMorphemesPrefix.begin(); pos != newMorphemesPrefix.end(); ++pos) 
	{	
		len=getLengthWithoutHallant(pos->first);
		threshold=(len<=2)? (4-len)*threshold2 : threshold2;
		if(countStringPrefix[pos->first]<threshold)
		{
			deleted[pos->first]=1;		
		}
	}
	for (pos = deleted.begin(); pos != deleted.end(); ++pos)
	{
		newMorphemesPrefix.erase(pos->first);
		cout<<"Deleted prefix:"<<pos->first<<" "<<countStringSuffix[pos->first]<<endl;
	}
	*/

	int prevCount, newCount;
	float ratio;
	int index;
	cout<<"Reduced Suffixes Percentage:"<<endl;
	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos) 
	{		
		prevCount=countStringSuffixFirst[pos->first];
		newCount=countStringSuffix[pos->first];
		
		if(prevCount-newCount==0) continue;		
		ratio=(float)(prevCount-newCount)/prevCount;

		if(newCount>100) continue;		

		if(ratio<0.2) continue;

		cout<<pos->first<<" Prev:"<<prevCount<<" New:"<<newCount<<" Percentage:"<<ratio<<endl;

		deleted[pos->first]=1;
		
	}
	for (pos = deleted.begin(); pos != deleted.end(); ++pos)
	{
		//newMorphemesSuffix.erase(pos->first);
		index=newMorphemesSuffix[pos->first];
		relationsSuffix[index].clear();
		cout<<"Deleted suffix:"<<pos->first<<" "<<countStringSuffix[pos->first]<<endl;
	}

	deleted.clear();
	string rev;
	cout<<"Reduced Prefixes Percentage:"<<endl;
	for (pos = newMorphemesPrefix.begin(); pos != newMorphemesPrefix.end(); ++pos) 
	{	
		prevCount=countStringPrefixFirst[pos->first];
		newCount=countStringPrefix[pos->first];		

		if(prevCount-newCount==0) continue;

		ratio=(float)(prevCount-newCount)/prevCount;
		if(ratio<0.2) continue;

		deleted[pos->first]=1;
		
		rev=pos->first;
		reverse(rev);
		cout<<rev<<" Prev:"<<prevCount<<" New:"<<newCount<<" Percentage:"<<ratio<<endl;
		
	}
	for (pos = deleted.begin(); pos != deleted.end(); ++pos)
	{
		//newMorphemesPrefix.erase(pos->first);
		index=newMorphemesPrefix[pos->first];
		relationsPrefix[index].clear();
		cout<<"Deleted prefix:"<<pos->first<<" "<<countStringPrefix[pos->first]<<endl;
	}
}

void Trie::deletePrefixSuffixesFromSmallRoots()
{

	Map::const_iterator pos;
	string temp;
	for (pos = smallRoots.begin(); pos != smallRoots.end(); ++pos) 
	{
		//smallRoots.erase(pos->first);
		//if smallroots is low frequency
		if(pos->second<=5) continue;

		//if suffix is found in small roots		
		if(newMorphemesSuffix.find(pos->first) != newMorphemesSuffix.end()) continue;

		temp=pos->first;
		reverse(temp);
		//smallRoots.erase(temp);

		if(newMorphemesPrefix.find(temp) != newMorphemesPrefix.end()) continue;

		smallRootsSelected[pos->first]=1;
	}	
}

void Trie::getSuffixesPrefixesForSmallRoots()
{	
	Map::const_iterator pos;
	Map2::const_iterator pos2;
	string sub;
	int plus;
	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos) 
	{
		//if(countStringSuffix[pos->first]>9000)
		//if(countStringSuffix[pos->first]>800)
		if(rankSuffix[pos->first] < 20)	//consider only top 20 suffixes for small roots
		{
			suffixesForSmallRoot[pos->first]=1;
			for (pos2 = similarSuffixes.begin(); pos2 != similarSuffixes.end();++pos2) 
			{
				plus=(pos2->second).find_first_of('+');	//ers=er+s
				sub=(pos2->second).substr(0,plus);	//er
				if(sub==pos->first)
					suffixesForSmallRoot[pos2->first]=1;	//add ers
			}
			for (pos2 = conditionalSimilarSuffixes.begin(); 
				pos2 != conditionalSimilarSuffixes.end(); ++pos2) 
			{				
				if(pos2->second==pos->first)	//if d==ed or ting=ing
					suffixesForSmallRoot[pos2->first]=1;	//add d
			}
			

		}
	}

	string temp;
	for (pos = newMorphemesPrefix.begin(); pos != newMorphemesPrefix.end(); ++pos) 
	{
		//if(countStringPrefix[pos->first]>900)
		//if(countStringPrefix[pos->first]>200)
		if(rankPrefix[pos->first] <= 20)	//consider only top 20 prefixes for small roots
		{			
			prefixesForSmallRoot[pos->first]=1;
		}
	}

	cout<<"Selected Suffixes For Small Roots\n";
	for (pos = suffixesForSmallRoot.begin(); pos != suffixesForSmallRoot.end(); ++pos)
		cout<<pos->first<<" "<<rankSuffix[pos->first]<<endl;

}

int Trie::ifSmallOrMorpheme(string sub)
{
	if(smallRootsSelected.find(sub) != smallRootsSelected.end())
	{
		return 1;
	}
	
	if(vocabulary.find(sub) != vocabulary.end())
	{
		if(getLengthWithoutHallant(sub)>=5)
			return 1;
	}
	
	if(newMorphemesSuffix.find(sub) != newMorphemesSuffix.end())
	//if(suffixesForSmallRoot.find(sub) != suffixesForSmallRoot.end())
	{		
		if( getLengthWithoutHallant(sub) == 1 )
		{
			if(rankSuffix[sub]<10) return 1;
			else return 0;
		}
		else return 1;
	}
	
	reverse(sub);
	//if(prefixesForSmallRoot.find(sub) != prefixesForSmallRoot.end())
	if(newMorphemesPrefix.find(sub) != newMorphemesPrefix.end())
		return 1;
	
	return 0;	

}

void Trie::getDivisionsOfSmallRoot(string str, vector<string>& options)
{		
		int len=str.length();
		
		string sub,rest,outputStr;		
		vector<string> newOptions;

		for(int i=0;i<len;++i)
		{
			newOptions.clear();
			sub=str.substr(0,i+1);
			
			if( ifSmallOrMorpheme(sub) )
			{				
				if(i!=(len-1))
				{
					rest=str.substr(i+1,len-i-1);
					getDivisionsOfSmallRoot( rest, newOptions);

					if(newOptions.size()>0)
					{						
						for(int v=0;v<newOptions.size();++v)
						{
							outputStr=sub+"+";
							outputStr+=newOptions.at(v);
							options.push_back(outputStr);
						}
						
					}
				}
				else 
				{					
					options.push_back(str);
				}
				
			}
		}		

}


void Trie::getRootMorpheme(string output, string& root, string& morpheme, int& suffixOrPrefix)
{
	vector<string> tokens;
	tokenize(output,tokens,".","+");	

	string rev=tokens.at(0);
	reverse(rev);

	//if first one is root
	if(smallRootsSelected.find(tokens.at(0)) != smallRootsSelected.end())
	{
		//if(smallRoots[tokens.at(0)] <= 10) { suffixOrPrefix=3; return; }
		
		if(suffixesForSmallRoot.find(tokens.at(1)) != suffixesForSmallRoot.end())
		//if(newMorphemesSuffix.find(tokens.at(1)) != newMorphemesSuffix.end())
		{
			root=tokens.at(0);
			morpheme=tokens.at(1);
			suffixOrPrefix=1;
			return;
		}

		//if both are small roots then problem
		if(smallRootsSelected.find(tokens.at(1)) != smallRootsSelected.end())
		{			
			suffixOrPrefix=3;
			return;
		}

		if(vocabulary.find(tokens.at(1)) != vocabulary.end())
		{			
			suffixOrPrefix=2;
			root=tokens.at(0);
			morpheme=tokens.at(1);
			return;
		}
	}
	else if(prefixesForSmallRoot.find(rev) != prefixesForSmallRoot.end())
	{
		if(smallRootsSelected.find(tokens.at(1)) != smallRootsSelected.end())
		{
			//if(smallRoots[tokens.at(1)] > 10)
			//{
				root=tokens.at(1);						
				morpheme=tokens.at(0);
				suffixOrPrefix=0;
				return;
			//}
		}		
	}
	else if (vocabulary.find(tokens.at(0)) != vocabulary.end())
	{
		if(smallRootsSelected.find(tokens.at(1)) != smallRootsSelected.end())
		{			
			suffixOrPrefix=2;
			root=tokens.at(1);
			morpheme=tokens.at(0);
			return;
		}
	}
	
	suffixOrPrefix=3;

}

//Get divisions of small roots using small roots.
void Trie::getDivisionsOfSmallRoots()
{
	Map::const_iterator pos;
	vector<string> options;
	cout<<"Getting divisions using small roots"<<endl;
	string root, morpheme;
	int suffixOrPrefix;
	Map deleted;
	int index;
	string temp;
	for (pos = roots.begin(); pos != roots.end(); ++pos) 
	{
		if(allomorphRoots.find(pos->first) != allomorphRoots.end()) continue;
		
		//cout<<"SMALL:"<<pos->first<<" ";
		
		options.clear();
		getDivisionsOfSmallRoot(pos->first,options);
		if(options.size()>=1)
		{		
			//cout<<options.size()<<endl;
			for(int i=0;i<options.size();++i)
			{
				if(options.at(i) == pos->first) continue;

				//cout<<endl;
				//cout<<pos->first<<" "<<options.at(i)<<"===";

				if(checkConstraints2(options.at(i)))
				{					
					if( noOfDiv(options.at(i)) <= 1 )
					{
						getRootMorpheme(options.at(i), root, morpheme,
							 suffixOrPrefix);

						//cout<<suffixOrPrefix<<" ";

						if(suffixOrPrefix==1)
						{

							if(freqMatchSmallRoot(root, root+morpheme, morpheme))
							{
								cout<<pos->first<<"="<<options.at(i)<<
									" Root+Suffix Freq matched"<<endl;
								deleted[pos->first]=1;

								//add to the relation string
								index=newMorphemesSuffix[morpheme];
								relationsSuffix[index][root]=1;
							}
							else cout<<pos->first<<"="<<options.at(i)<<
									" Root+Suffix Not Freq matched"<<endl;

						}
						else if(suffixOrPrefix==0)
						{
							if(freqMatchPrefixSmallRoot(root, morpheme+root, morpheme))
							{
								cout<<pos->first<<"="<<options.at(i)<<
									" Prefix+Root Freq matched"<<endl;
								deleted[pos->first]=1;

								//		
								//add to the relation string	
								temp=morpheme;
								reverse(temp);
								index=newMorphemesPrefix[temp];
								temp=root;
								reverse(temp);
								relationsPrefix[index][temp]=1;

							}
							else cout<<pos->first<<"="<<options.at(i)<<
									" Prefix+Root Not Freq matched"<<endl;
						}
						else if(suffixOrPrefix==2)
						{
							//double root
							if(freqMatchMultipleRootSmall(pos->first,root,morpheme))
							{
								cout<<pos->first<<"="<<options.at(i)<<endl;
								deleted[pos->first]=1;
							}
							else
								cout<<pos->first<<" Compound Freq Not matched"<<endl;
						}
						
					}
					
				}
			}
			
		}

	}
	
	for (pos = deleted.begin(); pos != deleted.end(); ++pos)
		roots.erase(pos->first);
	

}

int checkType2(string token)
{
	string temp=token;
	reverse(temp);

	if(smallRootsSelected.find(token) != smallRootsSelected.end())
		return 1;

	if(vocabulary.find(token)!=vocabulary.end())
		return 4;

	if(newMorphemesSuffix.find(token) != newMorphemesSuffix.end())
	{
		
		if(newMorphemesPrefix.find(temp) != newMorphemesPrefix.end())
			return 5;	//both suffix and prefix
		else return 2;
	}
	
	else if(newMorphemesPrefix.find(temp) != newMorphemesPrefix.end())
		return 3;	//if prefix only

	/*
	else 
		return 4;	//vocabulary
	*/
}

int ifFoundRoot2(string output)
{
	vector<string> tokens;
	tokenize(output,tokens,".","+");
	for(int i=0;i<tokens.size();++i)
	{
		if( smallRootsSelected.find(tokens.at(i)) != smallRootsSelected.end() )
			return 1;
	}
	return 0;
}

int checkConstraints2(string output)
{
	if(!ifFoundRoot2(output)) return 0;
	
	vector<string> tokens;
	tokenize(output,tokens,".","+");
	int type;
	string temp;
	for(int i=0;i<tokens.size();++i)
	{
		type=checkType2(tokens.at(i));

		if((type==1)||(type==4)) continue;
		
		if( (type==2) || (type==5) )//Suffix
		{
			if (i!=0)
			{

			   if(smallRootsSelected.find(tokens.at(i-1)) != smallRootsSelected.end())
					continue;
			   if(newMorphemesSuffix.find(tokens.at(i-1)) != newMorphemesSuffix.end())
					continue;
			}

			//return 0;
			
		}

		if( (type ==3) || (type==5)  )//prefix
		{
			if(i!=tokens.size()-1)
			{
		 	    if(smallRootsSelected.find(tokens.at(i+1)) != smallRootsSelected.end())
				continue;

			    temp=tokens.at(i+1);
			    reverse(temp);
			    if(newMorphemesPrefix.find(temp) != newMorphemesPrefix.end())
				continue;
			}
		}

		return 0;
	}
	
	return 1;
}




int Trie::findSimilar(string word)
{	
	string sub=word.substr(0, word.length()-1);
	string newS;
	for(char ch=(char)33;ch<=(char)126;++ch)
	{
		//if(!ifVowel(ch)) continue;
		newS=sub+ch;

		//if(vocabulary.find(newS) != vocabulary.end())
		if(roots.find(newS) != roots.end())
			return 1;
	}

	return 0;	
}

//say "@dr" is the replacement of "Adr"
int Trie::findSimilarAtBeginning(string word)
{	
	string sub=word.substr(1);
	string newS;
	for(char ch=(char)33;ch<=(char)126;++ch)
	{
		if(!ifVowel(ch)) continue;
		newS=ch+sub;

		//if(vocabulary.find(newS) != vocabulary.end())
		if(roots.find(newS) != roots.end())
			return 1;
	}
	return 0;	
}


int Trie::findSimilar2(string word)
{
	string sub1,sub2;
	string newS;

	for(int i=1;i<(word.length()-1);++i)
	{
		if( !ifVowel(word.at(i)) ) continue;
		sub1=word.substr(0,i);
		sub2=word.substr(i+1);
			
		for(char ch=(char)33;ch<=(char)126;++ch)
		{
			if(!ifVowel(ch)) continue;	//only vowel to vowel allowed
			if(ch==word.at(i)) continue;

			newS=sub1+ch;
			newS+=sub2;
	
			//if(vocabulary.find(newS) != vocabulary.end())
			if(roots.find(newS) != roots.end())
				return 1;
		}
	}
	return 0;	
}


int Trie::findSimilarByDeletion(string word)
{
	string sub=word;
	string newS;
	for(char ch=(char)33;ch<=(char)126;++ch)
	{
		//if(!ifVowel(ch)) continue;
		newS=sub+ch;

		//if(vocabulary.find(newS) != vocabulary.end())
		if(roots.find(newS) != roots.end())
			return 1;
	}

	return 0;	
}

int Trie::findSimilarByAddition(string word)
{		
	string newS=word.substr(0,word.length()-1);	//get len-1 chars
	if(roots.find(newS) != roots.end())
		return 1;
	return 0;
}

int Trie::bothConsonent(string str)
{
	if( ifVowel(str.at(0)) ) return 0;
	if( ifVowel(str.at(1)) ) return 0;
	return 1;
}

void Trie::findSimilar(string word, vector<char>& similar)
{
	//find similar by replacement
	string sub=word.substr(0, word.length()-1);
	string newS;
	for(char ch=(char)33;ch<=(char)126;++ch)
	{
		//if(!ifVowel(ch)) continue;
		newS=sub+ch;

		//if(vocabulary.find(newS) != vocabulary.end())
		if(roots.find(newS) != roots.end())
		{
			similar.push_back(ch);
			//return 1;
		}
	}
	
}

void Trie::findSimilarAtBeginning(string word, vector<char>& similar)
{
	//find similar by replacement
	string sub=word.substr(1);
	string newS;
	for(char ch=(char)33;ch<=(char)126;++ch)
	{
		if(!ifVowel(ch)) continue;
		newS=ch+sub;

		//if(vocabulary.find(newS) != vocabulary.end())
		if(roots.find(newS) != roots.end())
		{
			similar.push_back(ch);
			//return 1;
		}
	}
	
}


void Trie::findSimilar2(string word, vector<char>& similar, vector<int>& pos)
{
	//find similar by replacement in the middle
	string sub1,sub2;
	string newS;

	for(int i=1;i< (word.length()-1);++i)
	{
		if( !ifVowel(word.at(i)) ) continue;
		sub1=word.substr(0,i);
		sub2=word.substr(i+1);
			
		for(char ch=(char)33;ch<=(char)126;++ch)
		{
			if(!ifVowel(ch)) continue;
			if(ch==word.at(i)) continue;

			newS=sub1+ch;
			newS+=sub2;
	
			//if(vocabulary.find(newS) != vocabulary.end())
			if(roots.find(newS) != roots.end())
			{
				similar.push_back(ch);
				pos.push_back(i);				
			}
		}
	}	
}

void Trie::findSimilarByDeletion(string word, vector<char>& similar)
{
	//find similar by replacement
	string sub=word;
	string newS;
	for(char ch=(char)33;ch<=(char)126;++ch)
	{
		//if(!ifVowel(ch)) continue;
		newS=sub+ch;

		//if(vocabulary.find(newS) != vocabulary.end())
		if(roots.find(newS) != roots.end())
		{
			similar.push_back(ch);
			//return 1;
		}
	}
	
}

void Trie::findSimilarByAddition(string word, vector<char>& similar)
{
	//find similar by replacement
	string sub=word;
	int len=word.length();
	string newS=word.substr(0,len-1);
	if(roots.find(newS) != roots.end())
	{
		similar.push_back(word.at(len-1));	//push the last char
		//return 1;
	}		
}



char Trie::learnLeft(vector<string> words)
{
	map<char,int> lastCharSame;
	int lastCharVowel=0;
	int lastCharCons=0;
	char ch;
	string word;
	for(int i=0;i<words.size();++i)
	{
		
		word=words.at(i);
		rulesFile<<word<<" ";
		ch=word.at(word.length() -1);
		if(lastCharSame.find(ch) != lastCharSame.end())
			lastCharSame[ch] += 1;
		else lastCharSame[ch] =1;

		if(ifVowel(ch))
			lastCharVowel++;
		else lastCharCons++;
	}


	int max=-1;
	char maxChar;

	map<char,int>::const_iterator pos;
	for (pos = lastCharSame.begin(); pos != lastCharSame.end(); ++pos) 
		if(pos->second > max) 
		{
			max=pos->second;
			maxChar=pos->first;
		}

	rulesFile<<"\nLast Char: "<<maxChar<<":"<<(float)max/words.size()<<endl;
	rulesFile<<"Last Char vowel:"<<(float)lastCharVowel/words.size()<<endl;
	rulesFile<<"Last Char consonent:"<<(float)lastCharCons/words.size()<<endl;	

	if( (float)max/words.size() > .9 )
		return maxChar;
	if ( (float)lastCharVowel/words.size() == 1)
		return 'V';
	if ( (float)lastCharCons/words.size() == 1)
		return 'C';
	return '@';

}

char Trie::learnRight(vector<string> words)
{
	map<char,int> lastCharSame;
	int lastCharVowel=0;
	int lastCharCons=0;
	char ch;
	string word;
	for(int i=0;i<words.size();++i)
	{
		
		word=words.at(i);
		rulesFile<<word<<" ";
		ch=word.at(1);
		if(lastCharSame.find(ch) != lastCharSame.end())
			lastCharSame[ch] += 1;
		else lastCharSame[ch] =1;

		if(ifVowel(ch))
			lastCharVowel++;
		else lastCharCons++;
	}


	int max=-1;
	char maxChar;

	map<char,int>::const_iterator pos;
	for (pos = lastCharSame.begin(); pos != lastCharSame.end(); ++pos) 
		if(pos->second > max) 
		{
			max=pos->second;
			maxChar=pos->first;
		}

	rulesFile<<"\nLast Char: "<<maxChar<<":"<<(float)max/words.size()<<endl;
	rulesFile<<"Last Char vowel:"<<(float)lastCharVowel/words.size()<<endl;
	rulesFile<<"Last Char consonent:"<<(float)lastCharCons/words.size()<<endl;	

	if( (float)max/words.size() > .9 )
		return maxChar;
	if ( (float)lastCharVowel/words.size() == 1)
		return 'V';
	if ( (float)lastCharCons/words.size() == 1)
		return 'C';
	return '@';

}



void Trie::findAllomorph2()
{	
	Map::const_iterator pos;
	Map::const_iterator pos2;
	Map::const_iterator pos3;
	
	//finding root allomorphy
	cout<<"Printing allomorphs for replacement:"<<endl;
	rulesFile<<"Replacement Character Change Rules:"<<endl;
	rulesFile<<"******************************************************************************\n\n\n"<<endl;
	
	Map count;
	int len1, len2;
	string sub;
	
	Map relationAllomorph[10000];

	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos)
	//for al, er, ...
	{
		if(deletedSuffixes.find(pos->first) != deletedSuffixes.end()) continue;
		if(pos->first.length()==1) continue;

		len1=pos->first.length();		
		for (pos2 = vocabulary.begin(); pos2 != vocabulary.end(); ++pos2)
		//for denial, ..
		{			
			len2=pos2->first.length();
			if(len2<len1) continue;
			sub=(pos2->first).substr(len2-len1, len1);
		
			//see if there is "al" at the end of the current vocabulary
			if(sub!=pos->first) continue;			

			//sub="deni" from vocabulary "denial"
			sub=(pos2->first).substr(0,len2-len1);	

			//see the substring is actually in the voca, like work+er
			if(vocabulary.find(sub)!=vocabulary.end()) continue;

			if(sub.length()<3) continue;

			//if last char of sub is vowel continue;
			//if( !ifVowel( sub.at(sub.length()-1) ) ) continue;

			//if the remaining sub has some similar words in the voca..
			if(!findSimilar2(sub)) continue; //deni and deny
		
			if(count.find(sub) == count.end())
				count[sub]=1;
			else count[sub]= count[sub]+1;

			//relation allomorph contains the containts the allomorps per suffix
			relationAllomorph[pos->second][sub]=1;
		}
		
	}

	//allomorphs contains deni, refi, ....with their index
	//similar is a vector which contains the similar characters to form allomorphs say: y:i
	Map allomorphs;
	int current=0;
	vector<char> similar[50000];
	vector<int> position[50000];
	Map deleted;
	LinkList list;
	list.setMax(50);
	for (pos = count.begin(); pos != count.end(); ++pos)
	{
		if(pos->second >=1) 
		{
			list.add(pos->first,pos->second);
			allomorphs[pos->first]=current;
			findSimilar2(pos->first, similar[current], position[current]);
			current++;
		}
		else deleted[pos->first]=1;
			
	}

	cout<<"Printing Best allomorphs for replacement:";
	string next;
	int countW;
	list.setIterator();
	while(true)
	{
		countW=(int)list.getNext(next);
		if(countW==0) break;
		cout<<next<<":"<<countW<<endl;
	}
	
	string temp, pattern;
	Map patterns;
	Map halfPatterns;	
	float strength;
	string halfPattern;
	Map selected;
	char char1, char2;
	vector<string> leftWords;
	string root,inflection;
	int p;
	string sub1, sub2;

	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos)
	{
		if(deletedSuffixes.find(pos->first) != deletedSuffixes.end()) continue;
		if(pos->first.length()==1) continue;

		for (pos2 = deleted.begin(); pos2 != deleted.end(); ++pos2)
		{
			relationAllomorph[pos->second].erase(pos2->first);
		}
		cout<<pos->first<<":"<<endl;

		//find the best pattern under the current suffix
		//pattern contains the count of different possible replacements for current suffix: y:i, a:i, ....
		//half pattern contains the count of replacements options starting with y like y:i, y:s, ....
		patterns.clear();
		halfPatterns.clear();
		for (pos2 = relationAllomorph[pos->second].begin(); pos2 != relationAllomorph[pos->second].end(); ++pos2)
		{
			cout<<pos2->first<<"(";			
			
			current=allomorphs[pos2->first];

			if(similar[current].size() > 1) { cout<<") "; continue;	} //if same i comes from y, a, ...

			for(int i=0;i<similar[current].size();++i)
			{
				p=position[current].at(i);
				sub1=pos2->first.substr(0,p);	//h
				sub2=pos2->first.substr(p+1);   //t
				
				root=sub1 + similar[current].at(i) + sub2; //h+A+t
				inflection=pos2->first+pos->first;	//hEt+ECI

				if(!freqMatch(root,inflection, pos->first))
				{
					cout<<" Not matched "<<root<<" "<<inflection<<" ) ";
					continue;
				}

				temp=pos2->first.at(p);	//E

				pattern = temp + similar[current].at(i); //EA = A is replaced by E
				cout<<pattern<<" ";
				if( patterns.find(pattern) != patterns.end() )
					patterns[pattern]+=1;
				else patterns[pattern]=1;

				halfPattern=similar[current].at(i); //"A"

				if( halfPatterns.find(halfPattern) != halfPatterns.end() )
					halfPatterns[halfPattern]+=1;
				else halfPatterns[halfPattern]=1;

			}
			cout<<") ";
		}
		cout<<endl;

		LinkList ob;
		ob.setMax(5);
		for (pos2 = patterns.begin(); pos2 != patterns.end(); ++pos2)
			ob.add(pos2->first, pos2->second);

		cout<<"Learned Patterns:";
		ob.setIterator();
		selected.clear();
		while(true)
		{
			countW=(int) ob.getNext(next);
			if(countW==0) break;
			halfPattern=next.substr(1,1); //y
			if(bothConsonent(next)) continue;
			strength = (float)countW/halfPatterns[halfPattern];
			cout<<next<<":"<<countW<<" "<<strength<<" ";
			//if(strength==0) cout<< halfPatterns[halfPattern]<<" ";
			if( (countW * strength) >=6)
				selected[next]=1;
		}
		cout<<endl<<endl;
		

		char ch;
		int diff=0;
		for (pos3 = selected.begin(); pos3 != selected.end(); ++pos3)
		{
			//pos3->first=EA
			char1=pos3->first.at(0);	//E
			char2=pos3->first.at(1);	//A
			rulesFile<<char2<<":"<<char1<<"<=> @* _ @* +:0 "<<pos->first<<endl;
			
			leftWords.clear();
			diff=0;
			for (pos2 = relationAllomorph[pos->second].begin(); pos2 != relationAllomorph[pos->second].end(); ++pos2)
			{
				current=allomorphs[pos2->first];	//index of deni
				len2=pos2->first.length();		//len of deni
									
				for(int i=0;i<similar[current].size();++i)
				{					
					p=position[current].at(i);
					ch=pos2->first.at(p);	//E in hAt

					if(ch==char1)
					{
						if(similar[current].at(i) == char2)	//see different options for hEt....if it is A
						{
							sub1=pos2->first.substr(0,p);	//h
							sub2=pos2->first.substr(p+1);   //t
				
							root=sub1 + similar[current].at(i) + sub2; //h+A+t
							inflection=pos2->first+pos->first;

							if(!freqMatch(root,inflection, pos->first))
							{
								//cout<<"Not matched"<<root<<" "<<inflection<<endl;
								break;
							}
							diff++;

						}
					}
				}
			}

			for (pos2 = relationAllomorph[pos->second].begin(); pos2 != relationAllomorph[pos->second].end(); ++pos2)
			{
				current=allomorphs[pos2->first];	//index of deni
				len2=pos2->first.length();		//len of deni
									
				for(int i=0;i<similar[current].size();++i)
				{
					
					p=position[current].at(i);
					ch=pos2->first.at(p);	//E in hAt
					if(ch==char1)
					{
						if(similar[current].at(i) == char2)	//see different options for hEt....if it is A
						{
							sub1=pos2->first.substr(0,p);	//h
							sub2=pos2->first.substr(p+1);   //t
				
							root=sub1 + similar[current].at(i) + sub2; //h+A+t
							inflection=pos2->first+pos->first;

							if(!freqMatch(root,inflection, pos->first))
							{
								//cout<<"Not matched"<<root<<" "<<inflection<<endl;
								break;
							}			

							leftWords.push_back(pos2->first);
							//rightWords.push_back(sub2);

							if( allomorphRoots.find(inflection) != allomorphRoots.end() )
							{
								cout<<"Allomorph Twice:"<<inflection<<":"<<allomorphRoots[inflection]<<", "<<root+"+"<<pos->first;
								if( !compareAllomorph(inflection, allomorphRoots[inflection], root, pos->first, diff) )
								{
									cout<<" Selected first"<<endl; 
									continue;
								}
								else cout<<" Selected second"<<endl; 
							}

							allomorphRoots[inflection] = sub1 + char2 + sub2 + "+" +pos->first;
							allomorphRootsCount[inflection]=diff;


							//if deni+er is possible then make it deni+ers
							/*
							//similarSuffixesOpposite[er]=ers
							if(similarSuffixesOpposite.find(pos->first) != similarSuffixesOpposite.end())
							{
								temp=similarSuffixesOpposite[pos->first];
								allomorphRoots[pos2->first + temp] =  sub1 + char2 + sub2 + "+" +temp;
								allomorphRootsCount[pos2->first+temp]=diff;
							}
							*/
							break;
						}

					}
					
				}
			}

			ch=learnLeft(leftWords);
			//ch2=learnRight(rightWords);
			rulesFile<<char2<<":"<<char1<<" <=> @* _ +:0 @* "<<pos->first<<endl<<endl;

			//Use this rule to get composite but allomophed suffixes
			//like liness, ies etc ...			

		}
	}

	cout<<"The New Guys for replacement:"<<endl<<endl;
	Map2::const_iterator posx;
	for (posx = allomorphRoots.begin(); posx != allomorphRoots.end(); ++posx)
		cout<<posx->first<<":"<<posx->second<<endl;
	
}


void Trie::findAllomorph()
{
	Map::const_iterator pos;
	Map::const_iterator pos2;
	Map::const_iterator pos3;
	
	//finding root allomorphy
	cout<<"Printing allomorphs for replacement:"<<endl;
	rulesFile<<"Replacement Character Change Rules:"<<endl;
	rulesFile<<"******************************************************************************\n\n\n"<<endl;
	
	Map count;
	int len1, len2;
	string sub;
	
	Map relationAllomorph[10000];

	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos)
	//for al, er, ...
	{
		//if composite suffixes continue
		if(deletedSuffixes.find(pos->first) != deletedSuffixes.end()) continue;
		if(pos->first.length()==1) continue;

		len1=pos->first.length();		
		for (pos2 = vocabulary.begin(); pos2 != vocabulary.end(); ++pos2)
		//for denial, ..
		{			
			len2=pos2->first.length();
			if(len2<len1) continue;
			sub=(pos2->first).substr(len2-len1, len1);

		
			//see if there is "al" at the end of the current vocabulary
			if(sub!=pos->first) continue;			

			//sub="deni" from vocabulary "denial"
			sub=(pos2->first).substr(0,len2-len1);	

			//see the substring is actually in the voca, like work+er
			if(vocabulary.find(sub)!=vocabulary.end()) continue;

			if(sub.length()<=3) continue;

			//if last char of sub is vowel continue;
			//if( !ifVowel( sub.at(sub.length()-1) ) ) continue;

			//if the remaining sub has some similar words in the voca..
			if(!findSimilar(sub)) continue; //deni and deny
		
			if(count.find(sub) == count.end())
				count[sub]=1;
			else count[sub]= count[sub]+1;

			//relation allomorph contains the containts the allomorps per suffix
			relationAllomorph[pos->second][sub]=1;
		}
		
	}

	//allomorphs contains deni, refi, ....with their index
	//similar is a vector which contains the similar characters to form allomorphs say: y:i
	Map allomorphs;
	int current=0;
	vector<char> similar[50000];
	Map deleted;
	LinkList list;
	list.setMax(20);
	for (pos = count.begin(); pos != count.end(); ++pos)
	{
		if(pos->second >= 3) 
		{
			list.add(pos->first,pos->second);
			allomorphs[pos->first]=current;
			findSimilar(pos->first, similar[current]);
			current++;
		}		
		else deleted[pos->first]=1;
			
	}

	cout<<"Printing Best allomorphs for replacement:";
	string next;
	int countW;
	list.setIterator();
	while(true)
	{
		countW=(int)list.getNext(next);
		if(countW==0) break;
		cout<<next<<":"<<countW<<endl;
	}	
	
	string temp, pattern;
	Map patterns;
	Map halfPatterns;	
	float strength;
	string halfPattern;
	Map selected;
	char char1, char2;
	vector<string> leftWords;
	string root,inflection;
	string otherRoot, otherSuffix;

	//learn rules for each suffix separately
	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos)
	{
		if( deletedSuffixes.find(pos->first) != deletedSuffixes.end() ) continue;
		if( pos->first.length()==1 ) continue;
	
		for (pos2 = deleted.begin(); pos2 != deleted.end(); ++pos2)
		{
			relationAllomorph[pos->second].erase(pos2->first);
		}
		cout<<pos->first<<":"<<endl;

		//find the best pattern under the current suffix
		//pattern contains the count of different possible replacements for current suffix: y:i, a:i, ....
		//half pattern contains the count of replacements options starting with y like y:i, y:s, ....
		patterns.clear();
		halfPatterns.clear();

		//for dri, fri ....
		for (pos2 = relationAllomorph[pos->second].begin(); pos2 != relationAllomorph[pos->second].end(); ++pos2)
		{
			cout<<pos2->first<<"(";
			temp=pos2->first.at(pos2->first.length() - 1);	//i
			
			current=allomorphs[pos2->first];

			if(similar[current].size() > 1) { cout<<") "; continue;	} //if same i comes from y, a, ...

			for(int i=0;i<similar[current].size();++i)
			{
				root=pos2->first.substr( 0, pos2->first.length() - 1 ) + similar[current].at(i);
				inflection=pos2->first+pos->first;

				if( !freqMatch(root, inflection, pos->first) )
				{
					cout<<" Not matched "<<root<<" "<<inflection<<" ) ";
					continue;
				}

				//if there is other way to segment this
				if(ifRootPlusSuffix(inflection, otherRoot, otherSuffix))
				{
					if( countStringSuffix[otherSuffix] > countStringSuffix[pos->first] )				
					{	
						cout<<" other "<<otherRoot <<"+"<<otherSuffix<<" ) ";	
						continue;						
					}
				}

				pattern = temp + similar[current].at(i); //iy = y is replaced by i
				cout<<pattern<<" ";
				
				if( patterns.find(pattern) != patterns.end() )
					patterns[pattern]+=1;
				else patterns[pattern]=1;

				halfPattern=similar[current].at(i); //"y"

				if( halfPatterns.find(halfPattern) != halfPatterns.end() )
					halfPatterns[halfPattern]+=1;
				else halfPatterns[halfPattern]=1;

			}
			cout<<") ";
		}
		cout<<endl;

		LinkList ob;
		ob.setMax(5);
		for (pos2 = patterns.begin(); pos2 != patterns.end(); ++pos2)
			ob.add(pos2->first, pos2->second);

		cout<<"Learned Patterns:";
		ob.setIterator();
		selected.clear();
		while(true)
		{
			countW=(int) ob.getNext(next);
			if(countW==0) break;
			halfPattern=next.substr(1,1); //get y from iy
			if(bothConsonent(next)) continue;
			strength = (float)countW/halfPatterns[halfPattern];
			cout<<next<<":"<<countW<<" "<<strength<<" ";
			//if(strength==0) cout<< halfPatterns[halfPattern]<<" ";
			
			//if( (countW * strength) >=20) 
			if( (countW * strength) >= allomorphReplacementThreshold ) 				
				selected[next]=(int)countW*strength;
		}
		cout<<endl<<endl;
		

		char ch;
		int diff=0;
		for (pos3 = selected.begin(); pos3 != selected.end(); ++pos3)
		{
			//pos->first=iy
			char1=pos3->first.at(0);	//i
			char2=pos3->first.at(1);	//y
			rulesFile<<char2<<":"<<char1<<"<=> _ +:0 "<<pos->first<<endl;
			rulesFile<<"=============================="<<endl;
			rulesFile<<"Weighted Count:"<<pos3->second<<endl;
			
			leftWords.clear();
			diff=0;
			for (pos2 = relationAllomorph[pos->second].begin(); pos2 != relationAllomorph[pos->second].end(); ++pos2)
			{
				len2=pos2->first.length();	//len of deni

				ch=pos2->first.at(pos2->first.length() - 1);	//i in deni
				if(ch==char1)
				{
					current=allomorphs[pos2->first];	//index of deni
					for(int i=0;i<similar[current].size();++i)
					{
						if(similar[current].at(i) == char2)	//see different options for deni....
						{
							root=pos2->first.substr(0,pos2->first.length() - 1) + char2;
							inflection=pos2->first+pos->first;

							if(!freqMatch(root,inflection, pos->first))
							{
								//cout<<"Not matched"<<root<<" "<<inflection<<endl;
								break;
							}
							diff++;

						}
					}
				}
			}

			for (pos2 = relationAllomorph[pos->second].begin(); pos2 != relationAllomorph[pos->second].end(); ++pos2)
			{
				len2=pos2->first.length();		//len of deni

				ch=pos2->first.at(pos2->first.length() - 1);	//i in deni
				if(ch==char1)
				{
					current=allomorphs[pos2->first];	//index of deni
					for(int i=0;i<similar[current].size();++i)
					{
						if(similar[current].at(i) == char2)	//see different options for deni....
						{
							root=pos2->first.substr(0,pos2->first.length() - 1) + char2;
							inflection=pos2->first+pos->first;

							if(!freqMatch(root,inflection, pos->first))
							{
								//cout<<"Not matched"<<root<<" "<<inflection<<endl;
								break;
							}			

							leftWords.push_back( pos2->first.substr(0, len2-1) );

							if( allomorphRoots.find(inflection) != allomorphRoots.end() )
							{
								cout<<"Allomorph Twice:"<<inflection<<":"<<allomorphRoots[inflection]<<", "<<root+"+"<<pos->first;
								if( !compareAllomorph(inflection, allomorphRoots[inflection], root, pos->first, diff) )
								{
									cout<<" Selected first"<<endl; 
									continue;
								}
								else cout<<" Selected second"<<endl; 
							}

							allomorphRoots[inflection] = pos2->first.substr(0,len2-1) + char2 + "+" + pos->first;
							allomorphRootsCount[inflection]=diff;

							//if deni+er is possible then make it deni+ers
							//similarSuffixesOpposite[er]=ers
							if(similarSuffixesOpposite.find(pos->first) != similarSuffixesOpposite.end())
							{
								temp=similarSuffixesOpposite[pos->first];
								allomorphRoots[pos2->first + temp] = pos2->first.substr(0,len2-1) + char2 + "+" + temp;
								allomorphRootsCount[pos2->first+temp]=diff;
							}
							break;
						}

					}
					
				}
			}

			ch=learnLeft(leftWords);
			rulesFile<<char2<<":"<<char1<<" <=> "<<ch<<" _ +:0 "<<pos->first<<endl<<endl;

			//Use this rule to get composite but allomophed suffixes
			//like liness, ies etc ...
			string str;
			for (pos2 = newMorphemesSuffix.begin(); pos2 != newMorphemesSuffix.end(); ++pos2)
			{
				if(deletedSuffixes.find(pos2->first) != deletedSuffixes.end()) continue;
				temp=char1 + pos->first;	//i+es
				len1=pos2->first.length();
				len2=temp.length();

				if(len1<len2) continue;
				str=pos2->first.substr(len1-len2, len2);	//get the last len2 chars of current suffix

				if(str == temp)
				{
					if(len1==len2)
						str= char2;	//y
					else
						str= pos2->first.substr(0, len1-len2) + char2;	//ly
					
					if(newMorphemesSuffix.find(str) != newMorphemesSuffix.end())
					{
						int ind1=newMorphemesSuffix[str];	//ly
						int ind2=pos2->second;			//liness
						float x= compareRelationStrings(relationsSuffix[ind1], relationsSuffix[ind2]);						
						if(x>0.65)
						{							
							compoAllomorphFile<<pos2->first<<"="<<str<<"+"<<pos->first<<" strngth:"<<x<<endl;
							//deletedSuffixes[pos2->first]=1;
							similarSuffixes[pos2->first]=str+ "+" + pos->first;
						}

					}

				}

			}


		}
	}

	cout<<"The New Guys for replacement:"<<endl<<endl;
	Map2::const_iterator posx;
	for (posx = allomorphRoots.begin(); posx != allomorphRoots.end(); ++posx)
		cout<<posx->first<<":"<<posx->second<<endl;
	
}


float Trie::lastCharOfWords(Map relationString, char char1)
{
	Map::const_iterator pos;
	int count=0;
	int len;
	
	for(pos=relationString.begin(); pos != relationString.end(); ++pos)
	{
		len=pos->first.length();
		if(pos->first.at(len-1) == char1)
			count++;
	}

	return (float)count/relationString.size();

}

int Trie::compareAllomorph(string inflection, string old, string newRoot, string newSuffix, int newCount)
{
	int i=old.find_first_of('+');
	string oldRoot=old.substr(0,i);
	string oldSuffix=old.substr(i+1);

	if(oldSuffix!=newSuffix)	//is suffix is different
	{
		if(countStringSuffix[newSuffix] > countStringSuffix[oldSuffix])
			return 1;
		else return 0;
	}
	else	//suffix is equal
	{
		if( allomorphRootsCount[old] > newCount)
			return 0;
		else return 1;
	}
}

int Trie::compareAllomorph2(string inflection, string old, string newRoot, string newMorpheme, int newCount)
{
	int i=old.find_first_of('+');
	string oldMorpheme=old.substr(0,i);
	string oldRoot=old.substr(i+1);

	if(oldMorpheme!=newMorpheme)	//is prefix different
	{
		string temp1=oldMorpheme;
		string temp2=newMorpheme;
		reverse(temp1);
		reverse(temp2);
		if(newMorphemesPrefix.find(temp2) != newMorphemesPrefix.end())
		{
			//if both are prefix
			if(newMorphemesPrefix.find(temp1) != newMorphemesPrefix.end())
			{
				if(countStringPrefix[temp2] > countStringSuffix[temp1])
					return 1;
				else return 0;
			}
			else return 1;	//if newone is a prefix old one root
		}
		else //if new one is a root
		{
			//if first one is a prefix
			if(newMorphemesPrefix.find(temp1) != newMorphemesPrefix.end()) return 0;
			else	//if both are root
			{
				if(actualMorphemeCountOfRoot1[newMorpheme] >= 	
					actualMorphemeCountOfRoot1[oldMorpheme])
						return 1;
				else return 0;
			}
		}
	}
	else	//prefix or root is equal
	{
		if( allomorphRootsCount[inflection] > newCount)
			return 0;
		else return 1;
	}
}


int Trie::compareCompositeAllomorph(string old, string newSuff1, string newSuff2)
{
	int i=old.find_first_of('+');
	string oldSuff1=old.substr(0,i);
	string oldSuff2=old.substr(i+1);

	int count1=countStringSuffix[newSuff1]+countStringSuffix[newSuff2];
	int count2=countStringSuffix[oldSuff1]+countStringSuffix[oldSuff2];

	if(count1>count2)
		return 1;
	else return 0;
}

//see whether suffix is composite one (say ies=y+es), return first suffix(y) if it is of length 1
char Trie::allomorphicCompositeFirst(string suffix)
{
	if(similarSuffixes.find(suffix) != similarSuffixes.end())
	{
		string str=similarSuffixes[suffix];
		int i=str.find_first_of('+');
		string str1=str.substr(0,i);

		if(str1.length()==1)
		{
			//rulesFile<<"Ha Ha "<<str1.at(0)<<endl;
			return str1.at(0);
		}
	}
	return 0;
}


int Trie::ifRootPlusSuffix(string inflection, string& otherRoot, string& otherSuffix)
{	
	int len;
	string sub;
	
	int best=0;
	int flag=0;
	for(int i=0; i<inflection.length()-1;++i)
	{
		sub=inflection.substr(0, i+1);
		if(roots.find(sub) == roots.end()) continue;
		sub=inflection.substr(i+1);
		if(newMorphemesSuffix.find(sub) == newMorphemesSuffix.end()) continue;

		if(countStringSuffix[sub]>best)
		{
			otherRoot=inflection.substr(0,i+1);
			otherSuffix=sub;
			best=countStringSuffix[sub];
			flag=1;
		}
	}
	return flag;
}

//take+er = e:0 <=> _ +:0 er
void Trie::findAllomorphByDeletion()
{	
	Map::const_iterator pos;
	Map::const_iterator pos2;
	Map::const_iterator pos3;

	//finding root allomorphy
	cout<<"Printing allomorphs for Deletions:"<<endl;
	rulesFile<<"\nDeletion Character Change Rules:"<<endl;
	rulesFile<<"******************************************************************************\n\n\n"<<endl;
	
	Map count;
	int len1, len2;
	string sub;
	
	Map relationAllomorph[10000];

	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos)
	//for al, er, ...
	{
		if(deletedSuffixes.find(pos->first) != deletedSuffixes.end()) continue;
		if(pos->first.length()==1) continue;

		len1=pos->first.length();		
		for (pos2 = vocabulary.begin(); pos2 != vocabulary.end(); ++pos2)
		//for denial, ..
		{			
			len2=pos2->first.length();
			if(len2<len1) continue;
			sub=(pos2->first).substr(len2-len1, len1);
		
			//see if there is "al" at the end
			if(sub!=pos->first) continue;
			

			sub=(pos2->first).substr(0,len2-len1);	

			//see the substring is actually in the voca, like work+er
			if(vocabulary.find(sub)!=vocabulary.end()) continue;

			if(sub.length()<=3) continue;

			//if last char of sub is vowel continue;
			//if( !ifVowel( sub.at(sub.length()-1) ) ) continue;

			//if the remaining sub has some similar words in the voca..
			if(!findSimilarByDeletion(sub)) continue; //driv and drive
		
			if(count.find(sub) == count.end())
				count[sub]=1;
			else count[sub]= count[sub]+1;

			relationAllomorph[pos->second][sub]=1;
		}
		
	}


	//allomorphs contains driv, wav, ....with their index
	Map allomorphs;
	int current=0;
	vector<char> similar[50000];
	Map deleted;
	LinkList list;
	list.setMax(20);
	for (pos = count.begin(); pos != count.end(); ++pos)
	{
		if(pos->second >= 3) 
		{
			list.add(pos->first,pos->second);
			//cout<<pos->first<<":"<<pos->second<<endl;
			allomorphs[pos->first]=current;
			findSimilarByDeletion(pos->first, similar[current]);
			current++;
		}
		else deleted[pos->first]=1;
			
	}
	
	//printing the best allomorphs
	cout<<"Printing the best allomorphs by Deletions"<<endl;
	list.setIterator();
	string next;
	int countW;
	while(true)
	{
		countW=(int) list.getNext(next);
		if(countW==0) break;
		cout<<next<<" "<<countW<<endl;
	}


	string temp, pattern;
	Map patterns;
	Map halfPatterns;		
	float strength;
	string halfPattern;
	Map selected;
	char char1;
	vector<string> leftWords;
	string root,inflection;
	Map realCount;
	string changed;
	string otherRoot, otherSuffix;


	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos)
	{
		if(deletedSuffixes.find(pos->first) != deletedSuffixes.end()) continue;
		if(pos->first.length()==1) continue;
		for (pos2 = deleted.begin(); pos2 != deleted.end(); ++pos2)
		{
			relationAllomorph[pos->second].erase(pos2->first);
		}
		cout<<pos->first<<":"<<endl;
		cout<<"====================="<<endl;

		//find the best pattern under the current suffix
		//pattern contains the count of different possible deletions for current suffix: e:0, ....
		//half pattern contains the count of replacements options starting with y like y:i, y:s, ....
		patterns.clear();
		halfPatterns.clear();
		for (pos2 = relationAllomorph[pos->second].begin(); pos2 != relationAllomorph[pos->second].end(); ++pos2)
		{
			cout<<pos2->first<<"(";
			
			temp=pos2->first.at(pos2->first.length() - 1);	//v dont need
			
			current=allomorphs[pos2->first];
			if(similar[current].size() > 1) { cout<<") "; continue;	} //if same i comes from y, a, ...

			for(int i=0;i<similar[current].size();++i)
			{

				root=pos2->first + similar[current].at(i);	//driv+e
				inflection=pos2->first+pos->first;		//driv+er

				if(!freqMatch(root,inflection, pos->first))
				{
					cout<<" Not matched "<<root<<" "<<inflection<<" ) ";
					continue;
				}
			
				//if there is other way to segment this 
				if(ifRootPlusSuffix(inflection, otherRoot, otherSuffix))
				{
					if( countStringSuffix[otherSuffix] > countStringSuffix[pos->first] )				
					{	
						cout<<" other "<<otherRoot <<"+"<<otherSuffix<<" ) ";	
						continue;						
					}						
				}

				pattern = similar[current].at(i); //e = e is deleted by 0
				cout<<pattern<<" ";
				if( patterns.find(pattern) != patterns.end() )
					patterns[pattern]+=1;
				else patterns[pattern]=1;

				/*
				halfPattern=similar[current].at(i); //"y"

				if( halfPatterns.find(halfPattern) != halfPatterns.end() )
					halfPatterns[halfPattern]+=1;
				else halfPatterns[halfPattern]=1;
				*/
			}
			cout<<") ";
		}
		cout<<endl;

		LinkList ob;
		ob.setMax(5);
		for (pos2 = patterns.begin(); pos2 != patterns.end(); ++pos2)
			ob.add(pos2->first, pos2->second);

		cout<<"Learned Patterns:";
		ob.setIterator();
		selected.clear();
		while(true)
		{
			countW=(int) ob.getNext(next);
			if(countW==0) break;
			//halfPattern=next.substr(1,1); //y
			//if(bothConsonent(next)) continue;
			//strength = (float)countW/halfPatterns[halfPattern];
			cout<<next<<":"<<countW<<" ";
			//if(strength==0) cout<< halfPatterns[halfPattern]<<" ";
			
			//if(countW>=20)
			if( countW >= allomorphDeletionThreshold )
				selected[next]=1;
			
		}
		cout<<endl;


		char ch;
		int diff=0;
		char allChar=allomorphicCompositeFirst(pos->first);
		for (pos3 = selected.begin(); pos3 != selected.end(); ++pos3)
		{
			//pos->first=iy
			char1=pos3->first.at(0);	//e of e:0
			//char2=pos3->first.at(1);	

			//SAJIB:This condition is questionable
			//if(pos->first.at(0)==char1) continue;
			
			diff=0;
			for (pos2 = relationAllomorph[pos->second].begin(); pos2 != relationAllomorph[pos->second].end(); ++pos2)
			{
					len2=pos2->first.length();		//len of driv

					current=allomorphs[pos2->first];	//index of driv
					for(int i=0;i<similar[current].size();++i)
					{
						if(similar[current].at(i) == char1)	//see different options for driv
						{
							root=pos2->first + char1;
							inflection=pos2->first+pos->first;

							if(!freqMatch(root,inflection, pos->first))
							{
								//cout<<"Not matched"<<root<<" "<<inflection<<endl;
								break;
							}
							diff++;
						}
					}
			}

			cout<<"Percentage of "<<pos3->first<<":"<<(float)diff/relationAllomorph[pos->second].size()<<endl<<endl;

			//if the rule percentage is less than .15 dont take that.
			if( (float)diff / relationAllomorph[pos->second].size() <.15 ) continue; 

			rulesFile<<char1<<":0"<<"<=> _ +:0 "<<pos->first<<endl;	
			rulesFile<<"====================================="<<endl;
			rulesFile<<"count:"<<patterns[pos3->first]<<endl;		
			leftWords.clear();
			for (pos2 = relationAllomorph[pos->second].begin(); pos2 != relationAllomorph[pos->second].end(); ++pos2)
			{
					len2=pos2->first.length();		//len of driv

					current=allomorphs[pos2->first];	//index of driv
					for(int i=0;i<similar[current].size();++i)
					{
						if(similar[current].at(i) == char1)	//see different options for driv
						{
							root=pos2->first + char1;
							inflection=pos2->first+pos->first;

							if(!freqMatch(root,inflection, pos->first))
							{
								//cout<<"Not matched"<<root<<" "<<inflection<<endl;
								break;
							}			

							leftWords.push_back( pos2->first);
							//diff++;

							inflection=pos2->first + pos->first;					
							if( allomorphRoots.find(inflection) != allomorphRoots.end() )
							{
								cout<<"Allomorph Twice:"<<inflection<<":"<<allomorphRoots[inflection]<<", "<<root+"+"<<pos->first;
								if( !compareAllomorph(inflection,allomorphRoots[inflection], root, pos->first, diff) )
								{
									cout<<" Selected first"<<endl; 
									continue;
								}
								else cout<<" Selected second"<<endl; 
							}

							if(allChar!=0)
							{
								if(char1==allChar) 
								{
									changed=pos->first.substr(1);	//deny+ies	and char1=y
									rulesFile<<"ha ha "<<pos->first<<" changed to "<<changed<<endl;
									
								}
								else changed=pos->first;
							}
							else changed=pos->first;

							allomorphRoots[inflection] = pos2->first + char1 + "+" + changed;
							allomorphRootsCount[inflection]=diff;

							//if deni+er is possible then make it deni+ers
							//similarSuffixesOpposite[er]=ers
							if(similarSuffixesOpposite.find(changed) != similarSuffixesOpposite.end())
							{
								temp=similarSuffixesOpposite[changed];
								allomorphRoots[pos2->first + temp] = pos2->first + char1 + "+" + temp;
								allomorphRootsCount[pos2->first + temp]=diff;
							}
							break;
						}

					}					
				
			}


			ch=learnLeft(leftWords);
			rulesFile<<char1<<":0"<<" <=> "<<ch<<" _ +:0 "<<pos->first<<endl<<endl;
			rulesFile<<"Percentage:"<<(float) diff/relationAllomorph[pos->second].size()<<endl<<endl;
			cout<<char1<<":0"<<" <=> "<<ch<<" _ +:0 "<<pos->first<<endl<<endl;
			

			//get those suffix which are allomoprhically similar er and r
			if(pos->first.at(0) == char1)	//if e:0 and er
			{
				temp=pos->first.substr(1);	//r
				
				if(newMorphemesSuffix.find(temp) != newMorphemesSuffix.end())	//if r is found in the list
				{
				
				//if count[er]>count[r] by distance 10
				int ab=abs(countStringSuffix[pos->first] - countStringSuffix[temp]);
				if(  (ab< 30) || (countStringSuffix[pos->first] >countStringSuffix[temp]) )
				{				
					cout<<"Similarity checking Suffix:"<<pos->first<<":"<<temp;
					strength= lastCharOfWords(relationsSuffix[newMorphemesSuffix[temp]], char1);
					cout<<" "<<strength<<endl;
					if(strength>.1)	//if its less than .1 then it might be noise
					{
						conditionalSimilarSuffixes[temp]=pos->first;
						cout<<"similar Suffix:"<<pos->first<<":"<<temp<<endl;
						conditionalSimilarSuffixFile<<"similar Suffix:"<<pos->first<<":"<<temp<<endl;
						conditionOfSimilarSuffix[temp]=char1;
					}
					
				}
				}
			}
				


			//Use this rule to get composite but allomophed suffixes
			//like ating, izness etc ...
			
			string str;
			for (pos2 = newMorphemesSuffix.begin(); pos2 != newMorphemesSuffix.end(); ++pos2)
			{
				if(deletedSuffixes.find(pos2->first) != deletedSuffixes.end()) continue;
				len1=pos2->first.length();	//ation
				len2=pos->first.length();	//ion

				if(len1<=len2) continue;
				str=pos2->first.substr(len1-len2, len2);	//get the last len2 chars of current suffix

				if(str == pos->first)	//ation has ion
				{
					str= pos2->first.substr(0, len1-len2) + char1;	//at+e
					
					if(newMorphemesSuffix.find(str) != newMorphemesSuffix.end())
					{
						int ind1=newMorphemesSuffix[str];	//ate
						int ind2=pos2->second;			//ation
						float x= compareRelationStrings(relationsSuffix[ind1], relationsSuffix[ind2]);	
						cout<<"Possible allomorphic composite:"<<pos2->first<<":"<<x<<endl;			
								
						if(x>0.65)
						{							
							
							//deletedSuffixes[pos2->first]=1;
							if(similarSuffixes.find(pos2->first) != similarSuffixes.end())
							{
								cout<<"Composite Twice:"<<pos2->first<<":"<<similarSuffixes[pos2->first]<<" "<<str+ "+" + pos->first;
								compoAllomorphFile<<"Composite Twice:"<<pos2->first<<":"<<similarSuffixes[pos2->first]<<" "<<str+ "+" + pos->first;

								if( !compareCompositeAllomorph(similarSuffixes[pos2->first], str, pos->first) )
								{
									cout<<" Selected first"<<endl; 
									compoAllomorphFile<<" Selected first"<<endl;
									continue;
								}
								else
								{
									cout<<" Selected second"<<endl; 
									compoAllomorphFile<<" Selected second"<<endl; 
								}
							}
							similarSuffixes[pos2->first]=str+ "+" + pos->first;
							compoAllomorphFile<<pos2->first<<"="<<str<<"+"<<pos->first<<" strngth:"<<x<<endl;
							cout<<"Composite in Delete:"<<pos2->first<<"="<<str<<"+"<<pos->first<<" strngth:"<<x<<endl;

						}

					}

				}

			}

		}
	}

	cout<<"The New Guys for deletions:"<<endl<<endl;
	Map2::const_iterator posx;
	for (posx = allomorphRoots.begin(); posx != allomorphRoots.end(); ++posx)
		cout<<posx->first<<":"<<posx->second<<endl;

}


//bat+er =batter 0:t <=> t _ +:0 er
void Trie::findAllomorphByAddition()
{	
	Map::const_iterator pos;
	Map::const_iterator pos2;
	Map::const_iterator pos3;

	//finding root allomorphy
	cout<<"Printing allomorphs for Deletions:"<<endl;
	rulesFile<<"\nAddition Character Change Rules:"<<endl;
	rulesFile<<"******************************************************************************\n\n\n"<<endl;
	
	Map count;
	int len1, len2;
	string sub;
	
	Map relationAllomorph[10000];

	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos)
	//for al, er, ...
	{
		if(deletedSuffixes.find(pos->first) != deletedSuffixes.end()) continue;
		if(pos->first.length()==1) continue;

		len1=pos->first.length();		
		for (pos2 = vocabulary.begin(); pos2 != vocabulary.end(); ++pos2)
		//for denial, ..
		{			
			len2=pos2->first.length();
			if(len2<len1) continue;
			sub=(pos2->first).substr(len2-len1, len1);
		
			//see if there is "al" at the end
			if(sub!=pos->first) continue;
			
			sub=(pos2->first).substr(0,len2-len1);	

			//see the substring is actually in the voca, like work+er
			if(vocabulary.find(sub)!=vocabulary.end()) continue;

			//because gett has to be 4 to see 
			if(sub.length()<=4) continue;

			//if last char of sub is vowel continue;
			if( ifVowel( sub.at(sub.length()-1)) )continue;

			//if gett does not have same character at the end
			if( sub.at(sub.length()-1) != sub.at(sub.length()-2) ) continue;

			//if the remaining sub has some similar words in the voca..
			if(!findSimilarByAddition(sub)) continue; //batt and bat
		
			if(count.find(sub) == count.end())
				count[sub]=1;
			else count[sub]= count[sub]+1;

			relationAllomorph[pos->second][sub]=1;
		}		
	}

	//allomorphs contains deni, refi, ....with their index
	Map allomorphs;
	int current=0;
	vector<char> similar[50000];
	Map deleted;
	LinkList list;
	list.setMax(20);
	for (pos = count.begin(); pos != count.end(); ++pos)
	{
		if(pos->second >= 3) 
		{
			list.add(pos->first,pos->second);
			//cout<<pos->first<<":"<<pos->second<<endl;
			allomorphs[pos->first]=current;
			findSimilarByAddition(pos->first, similar[current]);
			current++;
		}
		else deleted[pos->first]=1;
			
	}
	
	//printing the best allomorphs
	cout<<"Printing the best allomorphs by Deletions"<<endl;
	list.setIterator();
	string next;
	int countW;
	while(true)
	{
		countW=(int) list.getNext(next);
		if(countW==0) break;
		cout<<next<<" "<<countW<<endl;
	}


	string temp, pattern;
	Map patterns;
	Map halfPatterns;		
	float strength;
	string halfPattern;
	Map selected;
	char char1;
	vector<string> leftWords;
	string root,inflection;
	Map realCount;


	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos)
	{
		if(deletedSuffixes.find(pos->first) != deletedSuffixes.end()) continue;
		for (pos2 = deleted.begin(); pos2 != deleted.end(); ++pos2)
		{
			relationAllomorph[pos->second].erase(pos2->first);

		}
		cout<<pos->first<<":"<<endl;


		//find the best pattern under the current suffix
		//pattern contains the count of different possible deletions for current suffix: 0:t, ....
		//half pattern contains the count of replacements options starting with y like y:i, y:s, ....
		patterns.clear();
		halfPatterns.clear();
		for (pos2 = relationAllomorph[pos->second].begin(); pos2 != relationAllomorph[pos->second].end(); ++pos2)
		{
			cout<<pos2->first<<"(";
			//temp=pos2->first.at(pos2->first.length() - 1);	//v dont need
			
			current=allomorphs[pos2->first];

			if(similar[current].size() > 1) { cout<<") "; continue;	} //if same i comes from y, a, ...

			for(int i=0;i<similar[current].size();++i)
			{
				root=pos2->first.substr(0,pos2->first.length()-1);	//get
				inflection=pos2->first+pos->first;			//gett+ing

				if(!freqMatch(root,inflection, pos->first))
				{
					cout<<" Not matched "<<root<<" "<<inflection<<" ) ";
					continue;
				}

				pattern = similar[current].at(i); //t = t is added to get
				cout<<pattern<<" ";
				if( patterns.find(pattern) != patterns.end() )
					patterns[pattern]+=1;
				else patterns[pattern]=1;

				/*
				halfPattern=similar[current].at(i); //"y"

				if( halfPatterns.find(halfPattern) != halfPatterns.end() )
					halfPatterns[halfPattern]+=1;
				else halfPatterns[halfPattern]=1;
				*/

			}
			cout<<") ";
		}
		cout<<endl;

		LinkList ob;
		ob.setMax(5);
		for (pos2 = patterns.begin(); pos2 != patterns.end(); ++pos2)
			ob.add(pos2->first, pos2->second);

		cout<<"Learned Patterns:";
		ob.setIterator();
		selected.clear();
		while(true)
		{
		
			countW=(int) ob.getNext(next);
			
			if(countW==0) break;
			
			//halfPattern=next.substr(1,1); //y
			//if(bothConsonent(next)) continue;
			//strength = (float)countW/halfPatterns[halfPattern];
			cout<<next<<":"<<countW<<" ";
			//if(strength==0) cout<< halfPatterns[halfPattern]<<" ";
			
			//if(countW >=20) 
			if( countW >= allomorphAdditionThreshold )
				selected[next]=countW;
				
		}
		cout<<endl<<endl;

		char ch;
		int diff=0;
		for (pos3 = selected.begin(); pos3 != selected.end(); ++pos3)
		{
			//pos->first=iy
			char1=pos3->first.at(0);	//t
			//char2=pos3->first.at(1);	
			
			diff=0;
			for (pos2 = relationAllomorph[pos->second].begin(); pos2 != relationAllomorph[pos->second].end(); ++pos2)
			{
					len2=pos2->first.length();		//len of gett

					current=allomorphs[pos2->first];	//index of gett
					for(int i=0;i<similar[current].size();++i)
					{
						if(similar[current].at(i) == char1)	//see different options for gett
						{
							root=pos2->first.substr(0,len2-1);	//gett-t
							inflection=pos2->first+pos->first;

							if(!freqMatch(root,inflection, pos->first))
							{
								//cout<<"Not matched"<<root<<" "<<inflection<<endl;
								break;
							}
							diff++;
						}
					}
			}

			cout<<char1<<" percentage:"<<(float)diff / relationAllomorph[pos->second].size()<<endl;
			//if the rule percentage is less than .15 dont take that.
			if( (float)diff / relationAllomorph[pos->second].size() <.15 ) continue; 

			rulesFile<<"0:"<<char1<<"<=> _ +:0 "<<pos->first<<endl;			
			rulesFile<<"=============================="<<endl;
			rulesFile<<"Count:"<<pos3->second<<endl;
			leftWords.clear();
			for (pos2 = relationAllomorph[pos->second].begin(); pos2 != relationAllomorph[pos->second].end(); ++pos2)
			{
					len2=pos2->first.length();		//len of gett

					current=allomorphs[pos2->first];	//index of gett
					for(int i=0;i<similar[current].size();++i)
					{
						if(similar[current].at(i) == char1)	//gett has a t
						{
							root=pos2->first.substr(0,len2-1);	//gett-t
							inflection=pos2->first+pos->first;

							if(!freqMatch(root,inflection, pos->first))
							{
								//cout<<"Not matched"<<root<<" "<<inflection<<endl;
								break;
							}
			

							leftWords.push_back(root);
							//diff++;
							
							if( allomorphRoots.find(inflection) != allomorphRoots.end() )
							{
								cout<<"Allomorph Twice:"<<inflection<<":"<<allomorphRoots[inflection]<<", "<<root+"+"<<pos->first;
								if( !compareAllomorph(inflection,allomorphRoots[inflection], root, pos->first, diff) )
								{
									cout<<" Selected first"<<endl; 
									continue;
								}
								else cout<<" Selected second"<<endl; 
							}

							allomorphRoots[inflection] = root + "+" + pos->first;
							allomorphRootsCount[inflection]=diff;

							//if deni+er is possible then make it deni+ers
							//similarSuffixesOpposite[er]=ers
							if(similarSuffixesOpposite.find(pos->first) != similarSuffixesOpposite.end())
							{
								temp=similarSuffixesOpposite[pos->first];
								allomorphRoots[pos2->first + temp] = root + "+" + temp;
								allomorphRootsCount[pos2->first + temp]=diff;
							}
							break;
						}

					}					
				
			}


			ch=learnLeft(leftWords);
			rulesFile<<"0:"<<char1<<" <=> "<<ch<<" _ +:0 "<<pos->first<<endl<<endl;
			rulesFile<<"Percentage:"<<(float) diff/relationAllomorph[pos->second].size()<<endl;
			cout<<"0:"<<char1<<" <=> "<<ch<<" _ +:0 "<<pos->first<<endl<<endl;
			cout<<"Percentage:"<<(float) diff/relationAllomorph[pos->second].size()<<endl;


			//get those suffix which are allomoprhically similar ting and ing
			temp=char1+pos->first;	//ting

			if(newMorphemesSuffix.find(temp) != newMorphemesSuffix.end())	//if temp is found in the list
			{				
				//if count[ing]>count[ting] by distance 10
				int ab=abs(countStringSuffix[pos->first] - countStringSuffix[temp]);
				if(  (ab< 30) || (countStringSuffix[pos->first] >countStringSuffix[temp]) )
				{
				
					cout<<"Similarity checking Suffix:"<<pos->first<<":"<<temp;
					strength= lastCharOfWords(relationsSuffix[newMorphemesSuffix[temp]], ch);
					cout<<" "<<strength<<endl;
					if(strength>.1)	//if its less than .1 then it might be noise
					{
						conditionalSimilarSuffixes[temp]=pos->first;
						conditionOfSimilarSuffix[temp]=ch;
						cout<<"similar Suffix:"<<pos->first<<":"<<temp<<endl;
						conditionalSimilarSuffixFile<<"similar Suffix:"<<pos->first<<":"<<temp<<endl;
					}
					
				}
			}		


			//Use this rule to get composite but allomophed suffixes
			//like ating, izness etc ...
			/*			
			string str;
			for (pos2 = newMorphemesSuffix.begin(); pos2 != newMorphemesSuffix.end(); ++pos2)
			{
				if(deletedSuffixes.find(pos2->first) != deletedSuffixes.end()) continue;
				len1=pos2->first.length();	//ation
				len2=pos->first.length();	//ion

				if(len1<=len2) continue;
				str=pos2->first.substr(len1-len2, len2);	//get the last len2 chars of current suffix

				if(str == pos->first)	//ation has ion
				{
					str= pos2->first.substr(0, len1-len2) + char1;	//at+e
					
					if(newMorphemesSuffix.find(str) != newMorphemesSuffix.end())
					{
						int ind1=newMorphemesSuffix[str];	//ate
						int ind2=pos2->second;			//ation
						float x= compareRelationStrings(relationsSuffix[ind1], relationsSuffix[ind2]);				
								
						if(x>.65)
						{							
							
							//deletedSuffixes[pos2->first]=1;
							if(similarSuffixes.find(pos2->first) != similarSuffixes.end())
							{
								cout<<"Composite Twice:"<<pos2->first<<":"<<similarSuffixes[pos2->first]<<str+ "+" + pos->first;
								compoAllomorphFile<<"Composite Twice:"<<pos2->first<<":"<<similarSuffixes[pos2->first]<<str+ "+" + pos->first;

								if( !compareCompositeAllomorph(similarSuffixes[pos2->first], str, pos->first) )
								{
									cout<<" Selected first"<<endl; 
									compoAllomorphFile<<" Selected first"<<endl;
									continue;
								}
								else
								{
									cout<<" Selected second"<<endl; 
									compoAllomorphFile<<" Selected second"<<endl; 
								}
							}
							similarSuffixes[pos2->first]=str+ "+" + pos->first;
							compoAllomorphFile<<pos2->first<<"="<<str<<"+"<<pos->first<<" strngth:"<<x<<endl;
							cout<<"Composite in Delete:"<<pos2->first<<"="<<str<<"+"<<pos->first<<" strngth:"<<x<<endl;

						}

					}

				}

			}
			*/
			


		}
	}

	cout<<"The New Guys for deletions:"<<endl<<endl;
	Map2::const_iterator posx;
	for (posx = allomorphRoots.begin(); posx != allomorphRoots.end(); ++posx)
		cout<<posx->first<<":"<<posx->second<<endl;

}

void Trie::findAllomorphAtBeginning()
{	
	Map::const_iterator pos;
	Map::const_iterator pos2;
	Map::const_iterator pos3;
	
	//finding root allomorphy
	cout<<"Printing allomorphs for replacement:"<<endl;
	int len1, len2;
	string sub;
	
	Map morphemesForAllomorph;
	string rev;
	for (pos = newMorphemesPrefix.begin(); pos != newMorphemesPrefix.end(); ++pos)
	{
		rev=pos->first;
		reverse(rev);
		morphemesForAllomorph[rev]=1;
	}
	for (pos = roots.begin(); pos != roots.end(); ++pos)
		morphemesForAllomorph[pos->first]=1;	

	
	vector<char> similar;
	string root, inflection;
	string temp, pattern, halfPattern;
	Map patterns;
	Map halfPatterns;		
	int current=0;
	int currentPattern;
	Map patternIndex;
	vector<string> vocabularyList[5000];
	vector<string> morphemeList[5000];

	for (pos = morphemesForAllomorph.begin(); pos != morphemesForAllomorph.end(); ++pos)
	//for al, er, ...
	{				
		if(pos->first.length()==1) continue;

		rev=pos->first;
		reverse(rev);

		len1=pos->first.length();		
		for (pos2 = roots.begin(); pos2 != roots.end(); ++pos2)
		//for denial, ..
		{			
			len2=pos2->first.length();
			if(len2<len1) continue;

			sub=(pos2->first).substr(0, len1);
		
			//see if there is "an" at the beginning of the current vocabulary
			if(sub!=pos->first) continue;			

			//sub="Adr" from vocabulary "anAdr"
			sub=(pos2->first).substr(len1);	

			//see the substring is actually in the voca, like work+er
			if(vocabulary.find(sub)!=vocabulary.end()) continue;

			if(sub.length()<3) continue;

			//if first char of sub is not vowel continue;
			if( !ifVowel( sub.at(0) ) ) continue;

			//if the remaining sub has some similar words in the voca..
			if(!findSimilarAtBeginning(sub)) continue; //deni and deny

			similar.clear();
			findSimilarAtBeginning(sub, similar);			

			for(int i=0;i<similar.size();++i)
			{
				root=similar.at(i) + sub.substr(1); //@+dr
				inflection=pos2->first;		//an+Adr

				if(newMorphemesPrefix.find(rev) != newMorphemesPrefix.end())
				{
					if(!freqMatchPrefix(root,inflection, rev))
					{
						cout<<" Not matched "<<root<<" "<<inflection<<" ) ";
						continue;
					}
				}
				else
				{
					if(!freqMatchMultipleRoot(inflection, pos->first, root))
					{
						cout<<" Not Multi matched "<<inflection<<" "<<root;
						continue;
					}
				}
				
				

				pattern = sub.at(0);
				pattern+= similar.at(i); //A@ = @ is replaced by A
				
				if( patterns.find(pattern) != patterns.end() )
				{
					//increase count only when option is 1 i.e A:E I:E then dont count
					if(similar.size()==1) 
						patterns[pattern]+=1;

					currentPattern=patternIndex[pattern];	//add it into the list anyway					
				}
				else	//new pattern
				{
					patterns[pattern]=1;
					patternIndex[pattern]=current;
					currentPattern=current++;
				}				

				//save the inf0 (vocabulary,prefix) against pattern
				vocabularyList[currentPattern].push_back(pos2->first);
				morphemeList[currentPattern].push_back(pos->first);

				halfPattern=similar.at(i); //"@"

				if( halfPatterns.find(halfPattern) != halfPatterns.end() )
				{
					if(similar.size()==1) 
						halfPatterns[halfPattern]+=1;
				}
				else halfPatterns[halfPattern]=1;
			}

		}
		
	}

	LinkList ob;
	ob.setMax(50);
	for (pos2 = patterns.begin(); pos2 != patterns.end(); ++pos2)
		ob.add(pos2->first, pos2->second);

	cout<<"Learned Patterns:";
	ob.setIterator();
	Map selected;
	selected.clear();
	float strength;
	int countW;
	string next;
	while(true)
	{
		countW=(int) ob.getNext(next);
		if(countW==0) break;
		halfPattern=next.substr(1,1); //get y from iy
		strength = (float)countW/halfPatterns[halfPattern];		
		
		if( (countW * strength) >=10 )
		{
			cout<<"Selected "<<next;
			cout<<" count:"<<countW<<" strength:"<<strength<<" ";
			selected[next]=(int)countW*strength;
		}
	}
	cout<<endl<<endl;

	string morpheme;
	for (pos3 = selected.begin(); pos3 != selected.end(); ++pos3)
	{

		currentPattern=patternIndex[pos3->first];

		cout<<"\n\nPattern:"<<pos3->first<<" Strength:"<<pos3->second<<" Size:"<<
				vocabularyList[currentPattern].size()<<endl;
		rulesFile<<pos3->first.at(1)<<":"<<pos3->first.at(0)<<" <=> @ +:0 _ @"<<endl;
		cout<<"====================\n";		

		for(int i=0;i<vocabularyList[currentPattern].size();++i)
		{

		cout<<"["<<vocabularyList[currentPattern].at(i)<<" "<<morphemeList[currentPattern].at(i)<<"]"<<" ";

		inflection=vocabularyList[currentPattern].at(i);
		morpheme=morphemeList[currentPattern].at(i);
		root=inflection.substr(morpheme.length());

		if( allomorphRoots.find(inflection) != allomorphRoots.end() )
		{
			cout<<"\nAllomorph Twice:"<<inflection<<":"<<allomorphRoots[inflection]<<", "<<morpheme+"+"<<root;
			if( !compareAllomorph2(inflection, allomorphRoots[inflection], root, morpheme, vocabularyList[currentPattern].size()) )	
			{
				cout<<" Selected first"<<endl; 
				continue;
			}
			else cout<<" Selected second"<<endl; 
		}

						//an + @ dr
		allomorphRoots[inflection] = morpheme+"+"+pos3->first.at(1)+root.substr(1); 
		allomorphRootsCount[inflection]=vocabularyList[currentPattern].size();

		}
	
	}

	cout<<"The New Guys for replacement at the beginning:"<<endl<<endl;
	Map2::const_iterator posx;
	for (posx = allomorphRoots.begin(); posx != allomorphRoots.end(); ++posx)
		cout<<posx->first<<":"<<posx->second<<endl;		
	
}






float setRange(int range1, int range2, float minWeight, float maxWeight, float currentWeight)
{
	float ans= range1 + ((float)(range2-range1)/(maxWeight-minWeight))*(currentWeight-minWeight);
	return ans;
}


//copy similar suffix weight from j to i
void Trie::copyWeight(int i, int j)
{
	for(int k=0;k<10;++k)
	{
		suffixLevelSimilarityString[i][k]=suffixLevelSimilarityString[j][k];
		suffixLevelSimilarityWeight[i][k]=suffixLevelSimilarityWeight[j][k];
	}
}

void Trie::getSuffixLevelSimilarityWeightsForAll()
{
	int plus;
	string sub;
	Map::const_iterator pos;
	cout<<"Getting similarity value for all"<<endl;
	for(pos=newMorphemesSuffix.begin();pos!=newMorphemesSuffix.end();++pos)
	{
		//if composite
		if(similarSuffixes.find(pos->first) != similarSuffixes.end())
		{			
			plus=similarSuffixes[pos->first].find_first_of('+');
			sub=similarSuffixes[pos->first].substr(0,plus);
			copyWeight(pos->second,newMorphemesSuffix[sub]);			
			continue;				
		}
		//if found in vocabulary continue
		//if(vocabulary.find(pos->first) != vocabulary.end())
		//	continue;		
		getSuffixLevelSimilarityWeights(pos->first);
	}
}




void Trie::getSuffixLevelSimilarityWeights(string current)
{		
	int index=newMorphemesSuffix[current];	
	Map::const_iterator pos;		
	Map::const_iterator pos2;
	string whole;
	Map count;
	int index2;
	int plus;
	string subAllomorph;
	for (pos = relationsSuffix[index].begin(); pos != relationsSuffix[index].end(); ++pos)
	// pos->first= sing, ring, ....
	{
		//look the string in other suffix list
		for (pos2 = newMorphemesSuffix.begin(); pos2 != newMorphemesSuffix.end(); ++pos2)
		{			

			//if its ers=er+s			
			if( similarSuffixes.find(pos2->first) != similarSuffixes.end() )
			{
				plus=similarSuffixes[pos2->first].find_first_of('+');
				subAllomorph=similarSuffixes[pos2->first].substr(0,plus);
				if(subAllomorph==current) continue;				
			}
						

			index2=pos2->second;
			if (index2==index) continue;
			//if(similarSuffixes)			
		    if(relationsSuffix[index2].find(pos->first)!= relationsSuffix[index2].end())
			{
				//if the suffix is found before
				if(count.find(pos2->first) != count.end())
					count[pos2->first]+=1;
				else count[pos2->first]=1;
			}			
		}
		
		//work with whole string
		whole=pos->first+current;	//sing+er				
	}

	//listing the best suffixes which are good for current suffix.
	LinkList bestSuffixes;
	bestSuffixes.setMax(10);	

	float weight;
	float minWeight=1;
	float maxWeight=0;
	for (pos = count.begin(); pos != count.end(); ++pos)
	{
		//cout<<"Count:"<<pos->first<<"="<<pos->second<<endl;
		index2=newMorphemesSuffix[pos->first];
		if(pos->second<=2) continue;
		weight=(float) pos->second * pos->second 
				/ ( relationsSuffix[index].size() 
					* relationsSuffix[index2].size() );		
		if( weight<minWeight )
			minWeight=weight;
		if(weight>maxWeight)
			maxWeight=weight;
		bestSuffixes.add(pos->first, weight);
	}

	//cout<<"Showing the bests\n";
	bestSuffixes.setIterator();
	string next;
	string bests[10];
	int i=0;
	float weight2;
	float finalWeight=0;

	cout<<"Suffix:"<<current<<endl;
	while(true)
	{
		weight=bestSuffixes.getNext(next);
		if(weight==0) break;
		bests[i]=next;
		i++;
		weight2=setRange(1,10,minWeight, maxWeight, weight);	
		cout<<next<<" ("<<weight2<<") ";

		suffixLevelSimilarityString[index][i]=next;
		suffixLevelSimilarityWeight[index][i]=weight2;
	}
	cout<<endl<<endl;
}

//get the suffix level similarity value to see whether a root("bAq") attaches with ("er") when "bAq" attaches with suffixes in the "suffixList".
float Trie::checkSuffixLevelSimilarity(string current, Map suffixList)
{
	float totalWeight=0;
	int index=newMorphemesSuffix[current];
	for(int i=0;i<10;++i)
	{
		if(suffixList.find(suffixLevelSimilarityString[index][i]) != suffixList.end())
			totalWeight+=suffixLevelSimilarityWeight[index][i];
	}
	return totalWeight/3;
}

//add candidate allomorphs as new roots.
void Trie::getNewRoots()
{
	Map::const_iterator pos; 
	Map::const_iterator pos2;
	int len1, len2;
	string sub,temp;
	Map count;
	
	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos)
	//for al, er, ...
	{
		//if(deletedSuffixes.find(pos->first) != deletedSuffixes.end()) continue;
		if(pos->first.length()==1) continue;

		len1=pos->first.length();		
		for (pos2 = roots.begin(); pos2 != roots.end(); ++pos2)
		//for (pos2 = vocabulary.begin(); pos2 != vocabulary.end(); ++pos2)		
		{			
			len2=pos2->first.length();
			if(len2<len1) continue;
			sub=(pos2->first).substr(len2-len1, len1);
		
			//see if there is "al" at the end
			if(sub!=pos->first) continue;			

			sub=(pos2->first).substr(0,len2-len1);	

			//see the substring is actually in the voca, like work+er
			//i.e sub is a found root
			if(vocabulary.find(sub)!=vocabulary.end()) continue;
			if(smallRoots.find(sub)!=smallRoots.end()) continue;

			if(getLengthWithoutHallant(sub)<3) continue;
			
			//check that generated root is not suffix or prefix
			if(newMorphemesSuffix.find(sub)!=newMorphemesSuffix.end())
				continue;

			temp=sub;
			reverse(temp);
			if(newMorphemesPrefix.find(temp)!=newMorphemesPrefix.end())
				continue;			
			
			if(count.find(sub) == count.end())
				count[sub]=1;
			else count[sub]= count[sub]+1;
			
		}
		
	}
	

	/*
	//get the roots from compound as well
	//root=newRoot+vocabulary
	for (pos = vocabulary.begin(); pos != vocabulary.end(); ++pos)
	//for al, er, ...
	{
		if(deletedSuffixes.find(pos->first) != deletedSuffixes.end()) continue;
		//if(pos->first.length()==1) continue;

		len1=pos->first.length();		

		//see which roots can be divided into
		for (pos2 = roots.begin(); pos2 != roots.end(); ++pos2)
		//for denial, ..
		{			
			len2=pos2->first.length();
			if(len2<len1) continue;
			sub=(pos2->first).substr(len2-len1, len1);
		
			//see if there is "al" at the end
			if(sub!=pos->first) continue;			

			sub=(pos2->first).substr(0,len2-len1);	

			//see the substring is actually in the voca, like work+er
			//i.e sub is a found root
			if(vocabulary.find(sub)!=vocabulary.end()) continue;

			if(getLengthWithoutHallant(sub)<3) continue;
			
			//if the new one is already a prefix or suffix
			if(newMorphemesSuffix.find(sub)!=newMorphemesSuffix.end())
				continue;

			temp=sub;
			reverse(temp);
			if(newMorphemesPrefix.find(temp)!=newMorphemesPrefix.end())
				continue;
						
			if(count.find(sub) == count.end())
				count[sub]=1;
			else count[sub]= count[sub]+1;
			
		}
		
	}
	*/	

	//allomorphs contains driv, wav, ....with their index
	LinkList list;
	list.setMax(5000);
	for (pos = count.begin(); pos != count.end(); ++pos)
	{
		if(pos->second>=INDUCE_OUTOFVOCABULARY_ROOTS_THRESHOLD) 
		{
			list.add(pos->first,pos->second);			
		}	
	}

	/*
	cout<<"New roots"<<endl;
	int countW;
	string next;
	list.setIterator();
	while(true)
	{
		countW=(int) list.getNext(next);
		if(countW==0) break;
		cout<<next<<" "<<countW<<endl;		
	}
	*/

	
	//before you do that check suffix level similarity value as well
	getSuffixLevelSimilarityWeightsForAll();
	
	//printing the best allomorphs
	cout<<"Printing the new roots:"<<endl;
	list.setIterator();
	string next;
	int countW, index;
	Map deleted;
	Map suffixList;
	float weight;

	//for each new root
	while(true)
	{
		countW=(int) list.getNext(next);
		if(countW==0) break;
		cout<<next<<" "<<countW<<" ";

		//cout<<"Deleting:";
		
		suffixList.clear();

		//see which of the root=newRoot+suffix
		for(pos = roots.begin(); pos != roots.end(); ++pos)
		{		
			len1=next.length();		
			len2=pos->first.length();

			if(len2<=len1) continue;

			//get sing from singer
			sub=(pos->first).substr(0, len1);
		
			//see if there is "sing" at the beginning
			if(sub!=next) continue;			

			//get er
			sub=(pos->first).substr(len1, len2-len1);	

			if(sub.length()==1) continue;							
			
			//if er is found in suffix list
			if(newMorphemesSuffix.find(sub) != newMorphemesSuffix.end())
			{
				if(vocabulary[pos->first]<350) 
				//high frequency one remain unsegmented
				{
					//cout<<pos->first<<"("<<vocabulary[pos->first]<<") ";
					suffixList[sub]=1;
				}
				//else cout<<pos->first<<"(xx "<<vocabulary[pos->first]<<") ";
			}

		}
				
		int count=0;
		//suffixList contains the suffixes that are attached with current NewRoot
		for(pos = suffixList.begin(); pos != suffixList.end(); ++pos)
		{
			weight=checkSuffixLevelSimilarity(pos->first,suffixList);
			//cout<<pos->first<<"("<<weight<<") ";
			if(weight>=2) count++;
		}
		
		cout<<"(";
		if(count>=INDUCE_OUTOFVOCABULARY_ROOTS_THRESHOLD)
		{
			outOfVocabularyRoots<<next<<" "<<countW<<endl;
			
			//add all suffixes to the new root and delete those from root list
			for(pos = suffixList.begin(); pos != suffixList.end(); ++pos)
			{
			    cout<<pos->first<<", ";
				deleted[next+pos->first]=1;
				index=newMorphemesSuffix[pos->first];
				relationsSuffix[index][next]=1;
			}
				
			roots[next]=1;
			vocabulary[next]=1;
		}

		cout<<")"<<endl;
	}
	
	for(pos = deleted.begin(); pos != deleted.end(); ++pos)
	{
		roots.erase(pos->first);
	}
	
}


//add a set of new roots. (a simpler version -- not using suffix level similarity)
void Trie::getNewRoots2()
{
	Map::const_iterator pos; 
	Map::const_iterator pos2;
	int len1, len2;
	string sub,temp;
	Map count;
	
	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos)
	//for al, er, ...
	{
		//if(deletedSuffixes.find(pos->first) != deletedSuffixes.end()) continue;
		
		//drop length 1 suffixes
		if(pos->first.length()==1) continue;

		len1=pos->first.length();		
		for (pos2 = roots.begin(); pos2 != roots.end(); ++pos2)
		//for (pos2 = vocabulary.begin(); pos2 != vocabulary.end(); ++pos2)		
		{			
			//if found in allomorphic segmentation already leave them.
			if(allomorphRoots.find(pos2->first) != allomorphRoots.end()) continue;
			
			len2=pos2->first.length();
			if(len2<len1) continue;
			sub=(pos2->first).substr(len2-len1, len1);
		
			//see if there is "al" at the end
			if(sub!=pos->first) continue;			

			sub=(pos2->first).substr(0,len2-len1);	

			//see the substring is actually in the voca, like work+er
			//i.e sub is a found root
			if(vocabulary.find(sub)!=vocabulary.end()) continue;
			if(smallRoots.find(sub)!=smallRoots.end()) continue;


			if(getLengthWithoutHallant(sub)<3) continue;
			
			//check that generated root is not suffix or prefix
			if(newMorphemesSuffix.find(sub)!=newMorphemesSuffix.end())
				continue;

			temp=sub;
			reverse(temp);
			if(newMorphemesPrefix.find(temp)!=newMorphemesPrefix.end())
				continue;			
			
			if(count.find(sub) == count.end())
				count[sub]=1;
			else count[sub]= count[sub]+1;
			
		}
		
	}
	

	/*
	//get the roots from compound as well
	//root=newRoot+vocabulary
	for (pos = vocabulary.begin(); pos != vocabulary.end(); ++pos)
	//for al, er, ...
	{
		if(deletedSuffixes.find(pos->first) != deletedSuffixes.end()) continue;
		//if(pos->first.length()==1) continue;

		len1=pos->first.length();		

		//see which roots can be divided into
		for (pos2 = roots.begin(); pos2 != roots.end(); ++pos2)
		//for denial, ..
		{			
			len2=pos2->first.length();
			if(len2<len1) continue;
			sub=(pos2->first).substr(len2-len1, len1);
		
			//see if there is "al" at the end
			if(sub!=pos->first) continue;			

			sub=(pos2->first).substr(0,len2-len1);	

			//see the substring is actually in the voca, like work+er
			//i.e sub is a found root
			if(vocabulary.find(sub)!=vocabulary.end()) continue;

			if(getLengthWithoutHallant(sub)<3) continue;
			
			//if the new one is already a prefix or suffix
			if(newMorphemesSuffix.find(sub)!=newMorphemesSuffix.end())
				continue;

			temp=sub;
			reverse(temp);
			if(newMorphemesPrefix.find(temp)!=newMorphemesPrefix.end())
				continue;
						
			if(count.find(sub) == count.end())
				count[sub]=1;
			else count[sub]= count[sub]+1;
			
		}
		
	}
	*/	

	//allomorphs contains driv, wav, ....with their index
	LinkList list;
	list.setMax(5000);
	for (pos = count.begin(); pos != count.end(); ++pos)
	{
		if(pos->second>=INDUCE_OUTOFVOCABULARY_ROOTS_THRESHOLD) 
		{
			list.add(pos->first,pos->second);			
		}	
	}

	/*
	cout<<"New roots"<<endl;
	int countW;
	string next;
	list.setIterator();
	while(true)
	{
		countW=(int) list.getNext(next);
		if(countW==0) break;
		cout<<next<<" "<<countW<<endl;		
	}
	*/

	
	//before you do that check suffix level similarity value as well
	//getSuffixLevelSimilarityWeightsForAll();
	
	//printing the best allomorphs
	cout<<"Printing the new roots:"<<endl;
	list.setIterator();
	string next;
	int countW, index;
	Map deleted;
	Map suffixList;
	float weight;

	//for each new root
	while(true)
	{
		countW=(int) list.getNext(next);
		if(countW==0) break;
		cout<<next<<" "<<countW<<" ";
		outOfVocabularyRoots<<next<<" "<<countW<<endl;
		
		suffixList.clear();

		//see which of the root=newRoot+suffix
		for(pos = roots.begin(); pos != roots.end(); ++pos)
		{		
			//if found in allomorphic segmentation leave them already.
			if(allomorphRoots.find(pos->first) != allomorphRoots.end()) continue;			
			
			len1=next.length();		
			len2=pos->first.length();

			if(len2<=len1) continue;

			//get sing from singer
			sub=(pos->first).substr(0, len1);
		
			//see if there is "sing" at the beginning
			if(sub!=next) continue;			

			//get er
			sub=(pos->first).substr(len1, len2-len1);	

			if(sub.length()==1) continue;				
			
			//if er is found in suffix list
			if(newMorphemesSuffix.find(sub) != newMorphemesSuffix.end())
			{				
				if(vocabulary[pos->first]<350) 
				//high frequency one remain unsegmented
				{
					suffixList[sub]=1;
				}
				
			}
		}
				
		cout<<"(";
		//add all suffixes to the new root and delete those from root list
		for(pos = suffixList.begin(); pos != suffixList.end(); ++pos)
		{
				deleted[next+pos->first]=1;
				index=newMorphemesSuffix[pos->first];
				relationsSuffix[index][next]=1;
				cout<<pos->first<<", ";
		}
		cout<<")"<<endl;
				
		//add the  newRoots to the root list and vocabulary.		
		roots[next]=1;
		vocabulary[next]=1;
	}
	
	for(pos = deleted.begin(); pos != deleted.end(); ++pos)
	{
		roots.erase(pos->first);
	}		
	
}

//get the newRoots from other root+root pattern
void Trie::getNewRootsFromRoots()
{
	Map::const_iterator pos; 
	Map::const_iterator pos2;
	int len1, len2;
	string sub,temp;
	Map count;	
	
	cout<<"New roots by roots"<<endl;	
	
	//get the roots from compound as well
	//root=newRoot+vocabulary
	
	for (pos = vocabulary.begin(); pos != vocabulary.end(); ++pos)
	//sing
	{
		
		len1=pos->first.length();		

		//see which roots can be divided into i.e. roots=newRoot+vocabulary
		for (pos2 = roots.begin(); pos2 != roots.end(); ++pos2)
		//for bingsing
		{			
			//if found in allomorphic segmentation already leave them.
			if(allomorphRoots.find(pos2->first) != allomorphRoots.end()) continue;			
			
			len2=pos2->first.length();
			if(len2<len1) continue;
			
			sub=(pos2->first).substr(len2-len1, len1);
		
			//see if there is "sing" at the end
			if(sub!=pos->first) continue;			

			//get root - vocabulary=newRoot
			sub=(pos2->first).substr(0,len2-len1);	

			//see the substring is actually in the voca, like work+er
			//i.e sub is a found root
			if(vocabulary.find(sub)!=vocabulary.end()) continue;

			if(getLengthWithoutHallant(sub)<3) continue;
			
			//if the new one is already a prefix or suffix
			if(newMorphemesSuffix.find(sub)!=newMorphemesSuffix.end())
				continue;

			temp=sub;
			reverse(temp);
			if(newMorphemesPrefix.find(temp)!=newMorphemesPrefix.end())
				continue;
						
			if(count.find(sub) == count.end())
				count[sub]=1;
			else count[sub]= count[sub]+1;
			
		}
		
	}
	
	//root=vocabulary+newRoot
	for (pos = vocabulary.begin(); pos != vocabulary.end(); ++pos)
	{		
		len1=pos->first.length();		

		//see which roots can be divided into i.e. 	//root=vocabulary+newRoot
		for (pos2 = roots.begin(); pos2 != roots.end(); ++pos2)
		//for denial, ..
		{			
			//if found in allomorphic segmentation already leave them.
			if(allomorphRoots.find(pos2->first) != allomorphRoots.end()) continue;			
			
			len2=pos2->first.length();
			if(len2<len1) continue;
			
			sub=(pos2->first).substr(0, len1);
		
			//see if there is vocabulary at the beginning
			if(sub!=pos->first) continue;			
			
			//root=vocabulary+newRoot
			//get root - vocabulary=newRoot, 	
			sub=(pos2->first).substr(len1);	//from len1 to endOfstring
		
			//see the substring is actually in the voca, like work+er
			//i.e sub is a found root
			if(vocabulary.find(sub)!=vocabulary.end()) continue;

			if(getLengthWithoutHallant(sub)<3) continue;
			
			//if the new one is already a prefix or suffix
			if(newMorphemesSuffix.find(sub)!=newMorphemesSuffix.end())
				continue;

			temp=sub;
			reverse(temp);
			if(newMorphemesPrefix.find(temp)!=newMorphemesPrefix.end())
				continue;
						
			if(count.find(sub) == count.end())
				count[sub]=1;
			else count[sub]= count[sub]+1;
			
		}
		
	}	

	//allomorphs contains driv, wav, ....with their index
	LinkList list;
	list.setMax(5000);
	for (pos = count.begin(); pos != count.end(); ++pos)
	{
		if(pos->second>=5) 
		{
			list.add(pos->first,pos->second);			
		}	
	}

	
	int countW;
	string next;
	list.setIterator();
	while(true)
	{
		countW=(int) list.getNext(next);
		if(countW==0) break;
		cout<<next<<" "<<countW<<endl;		
	}
	

	/*
	//before you do that check suffix level similarity value as well
	//getSuffixLevelSimilarityWeightsForAll();
	
	//printing the best allomorphs
	cout<<"Printing the new roots:"<<endl;
	list.setIterator();
	string next;
	int countW, index;
	Map deleted;
	Map suffixList;
	float weight;

	//for each new root
	while(true)
	{
		countW=(int) list.getNext(next);
		if(countW==0) break;
		cout<<next<<" "<<countW<<" ";
		
		suffixList.clear();

		//see which of the root=newRoot+suffix
		for(pos = roots.begin(); pos != roots.end(); ++pos)
		{		
			//if found in allomorphic segmentation leave them already.
			if(allomorphRoots.find(pos->first) != allomorphRoots.end()) continue;
			
			
			len1=next.length();		
			len2=pos->first.length();

			if(len2<=len1) continue;

			//get sing from singer
			sub=(pos->first).substr(0, len1);
		
			//see if there is "sing" at the beginning
			if(sub!=next) continue;			

			//get er
			sub=(pos->first).substr(len1, len2-len1);	

			if(sub.length()==1) continue;				
			
			//if er is found in suffix list
			if(newMorphemesSuffix.find(sub) != newMorphemesSuffix.end())
			{				
				if(vocabulary[pos->first]<350) 
				//high frequency one remain unsegmented
				{
					suffixList[sub]=1;
				}
				
			}
		}
				
		cout<<"(";
		//add all suffixes to the new root and delete those from root list
		for(pos = suffixList.begin(); pos != suffixList.end(); ++pos)
		{
				deleted[next+pos->first]=1;
				index=newMorphemesSuffix[pos->first];
				relationsSuffix[index][next]=1;
				cout<<pos->first<<", ";
		}
		cout<<")"<<endl;
				
		//add the  newRoots to the root list and vocabulary.		
		roots[next]=1;
		vocabulary[next]=1;
	}
	
	for(pos = deleted.begin(); pos != deleted.end(); ++pos)
	{
		roots.erase(pos->first);
	}		
	*/
}


int Trie::compare(char ch1, char ch2)
{
	if(ch2=='V')
		return ifVowel(ch1);
	if(ch2=='C')
		return !ifVowel(ch1);
	else return ch1==ch2;
}


void Trie::processSuffixesAndRoots()
{
	Map2::const_iterator pos;
	Map::const_iterator pos2;
	string suffix2;
	int i, index, index2;
	string newRoot;
	for (pos = allomorphRoots.begin(); pos != allomorphRoots.end(); ++pos) 
	{
		i=(pos->second).find_first_of('+');	//countries=country+es
		newRoot=(pos->second).substr(0,i);	//country
		suffix2=(pos->second).substr(i+1);	//es
		index=newMorphemesSuffix[suffix2];	//es
		
		relationsSuffix[index][newRoot]=1;		
	}
	
	for (pos = similarSuffixes.begin(); pos != similarSuffixes.end(); ++pos) 
	{
		i=(pos->second).find_first_of('+');	//ers=er+s
		suffix2=(pos->second).substr(0,i);
		index=newMorphemesSuffix[pos->first];	//ers
		index2=newMorphemesSuffix[suffix2];	//er

	
		for (pos2 = relationsSuffix[index].begin(); pos2 != relationsSuffix[index].end(); 
					++pos2) 
			relationsSuffix[index2][pos2->first]=1;		
	}

	int len;
	char ch1, ch2;
	for(pos = conditionalSimilarSuffixes.begin(); pos != conditionalSimilarSuffixes.end(); 
			++pos) 
	{		
		index=newMorphemesSuffix[pos->first];	//r
		index2=newMorphemesSuffix[pos->second];	//er
		for (pos2 = relationsSuffix[index].begin(); pos2 != relationsSuffix[index].end(); 
					++pos2) 
		{
			len=(pos2->first).length();	//drive
			ch1=(pos2->first).at(len-1);	//get the last char of drive
			ch2=conditionOfSimilarSuffix[pos->first];		//get e

			if( compare(ch1,ch2) )
				relationsSuffix[index2][pos2->first]=1;
		}
		
	}
	
	cout<<"printing new relation strings for clustering:"<<endl<<endl;
	//printRelationStrings();	

}

/*
void Trie::getClusterOfSuffix(string suffix, int from, int& to)
{
	Map::const_iterator pos;
	//if er is found in current cluster
	if(clusterFeatureValue[from].find(suffix) == clusterFeatureValue[from].end())
	{
		to=-1;
		return;
	}

	//find those words of this cluster which are added with er
	int index=newMorphemesSuffix[suffix];	
	Map *currentCluster;
	if(from==0)
		currentCluster=&nouns;	
	else if(from==1)
		currentCluster=&verbs;	
	else 
		currentCluster=&adjectives;	
	
	for (pos = currentCluster->begin(); pos != currentCluster->end(); ++pos)
	{
		if(relationsSuffix[index].find(pos->first) != relationsSuffix[index].end())
		{
			//see sing+er is attached with which other suffixes;

		}
			
	}	
	

}
*/

void Trie::getClustersAndMorphemeInformation()
{
	
	nouns.clear();
	adjectives.clear();
	verbs.clear();

	ifstream f("noun2.txt");
	char temp[500];
  	if (f.is_open())
  	{	    		
		//get the file data
		while (! f.eof() )
  		{				
	    		f.getline(temp,10000);
			nouns[temp]=1;

		}
	}
	else exit(0);
	f.close();
	
	ifstream f2("adjective2.txt");	
  	if (f2.is_open())
  	{	    		
		//get the file data
		while (! f2.eof() )
  		{				
	    		f2.getline(temp,10000);
			adjectives[temp]=1;

		}
	}
	else exit(0);
	f2.close();

	ifstream f3("verb2.txt");
  	if (f3.is_open())
  	{	    		
		//get the file data
		while (! f3.eof() )
  		{				
	    		f3.getline(temp,10000);
			verbs[temp]=1;

		}
	}
	else exit(0);
	f3.close();
	

	Map::const_iterator pos;
	Map::const_iterator pos2;
	int count=0;
	int index;
	cout<<"Printing Feature Value for Cluster:Noun"<<endl;
	for (pos2 = selectedForClustering.begin(); pos2 != selectedForClustering.end(); ++pos2)
	//for er
	{
		index=newMorphemesSuffix[pos2->first];

		count=0;
		for (pos = nouns.begin(); pos != nouns.end(); ++pos)
		{
			if(relationsSuffix[index].find(pos->first) != relationsSuffix[index].end())
				count++;			
		}
		if(count!=0)
			clusterFeatureValue[0][pos2->first]=count;
		else clusterFeatureValue[0][pos2->first]=1;

		cout<<pos2->first<<":"<<count<<endl;
	}
	
	cout<<"Printing Feature Value for Cluster:Adjective"<<endl;
	for (pos2 = selectedForClustering.begin(); pos2 != selectedForClustering.end(); ++pos2)
	//for er
	{
		index=newMorphemesSuffix[pos2->first];

		count=0;
		for (pos = adjectives.begin(); pos != adjectives.end(); ++pos)
		{
			if(relationsSuffix[index].find(pos->first) != relationsSuffix[index].end())
				count++;			
		}
		if(count!=0)
			clusterFeatureValue[1][pos2->first]=count;
		else clusterFeatureValue[1][pos2->first]=1;

		cout<<pos2->first<<":"<<count<<endl;
	}

	cout<<"Printing Feature Value for Cluster:Verb"<<endl;
	for (pos2 = selectedForClustering.begin(); pos2 != selectedForClustering.end(); ++pos2)
	//for er
	{
		index=newMorphemesSuffix[pos2->first];
		count=0;
		for (pos = verbs.begin(); pos != verbs.end(); ++pos)
		{
			if(relationsSuffix[index].find(pos->first) != relationsSuffix[index].end())
				count++;			
		}
		if(count!=0)
			clusterFeatureValue[2][pos2->first]=count;
		else clusterFeatureValue[2][pos2->first]=1;

		cout<<pos2->first<<":"<<count<<endl;
	}
	
}


void Trie::cluster()
{	
	
	//cluster all suffixes together:
	Map::const_iterator pos;
	Map::const_iterator pos2;
	int index;
	Map words;
	int suffixCount=0;
	/*
	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos) 
	{
		if(countStringSuffix[pos->first]>200)
		{			
			index=pos->second;
			suffixCount++;
			
	 	   	for (pos2 = relationsSuffix[index].begin(); 
				pos2 != relationsSuffix[index].end(); ++pos2)			
			{
				if(vocabulary[pos2->first]>20)
					words[pos2->first]=1;
			}
		}

	}
	*/

	processSuffixesAndRoots();

	cout<<"Suffixes Selected for clustering:"<<endl;
	for (pos2 = newMorphemesSuffix.begin(); pos2 != newMorphemesSuffix.end(); ++pos2) 
	{	
		
		if(similarSuffixes.find(pos2->first)!=similarSuffixes.end()) continue;

		if(conditionalSimilarSuffixes.find(pos2->first)
			!=conditionalSimilarSuffixes.end()) continue;

		if(pos2->first.length()>=5) continue;

		if(countStringSuffix[pos2->first]>2000)
		{
			suffixCount++;
			cout<<pos2->first<<" ";
			selectedForClustering[pos2->first]=1;
		}
	}
	cout<<endl;
		
	
	//words have the count of each root, i.e. how many suffixes root is attched to.	
	for (pos = roots.begin(); pos != roots.end(); ++pos)
	{
		if(vocabulary[pos->first]<=10) continue;

		for (pos2 = newMorphemesSuffix.begin(); pos2 != newMorphemesSuffix.end(); ++pos2) 
		{
			
			if(similarSuffixes.find(pos2->first)!=similarSuffixes.end()) continue;
			if(conditionalSimilarSuffixes.find(pos2->first)
				!=conditionalSimilarSuffixes.end()) continue;
			if(pos2->first.length()>=5) continue;
			if(countStringSuffix[pos2->first]>2000)
			{			
				index=pos2->second;				
		 	   	if(relationsSuffix[index].find(pos->first) != 
					relationsSuffix[index].end())
				{
					
					if(words.find(pos->first) !=  words.end())
						words[pos->first]+=1;
					else words[pos->first]=1;
				}
			}
		}	
		
	}

	/*
	int wordCount=0;
	cout<<"Root size:"<<roots.size()<<endl;
	for (pos = words.begin(); pos != words.end(); ++pos)
	{
		if(pos->second>=2)
			wordCount++;
	}
	cout<<"Clustering word count:"<<wordCount<<endl;
	*/
	
	
	featureFile<<words.size()<<" "<<suffixCount<<endl;
	//int index;
	string temp;
	for (pos = words.begin(); pos != words.end(); ++pos) 
	{
		//if(pos->second < 2) continue;

		featureFile<<pos->first<<endl;
		for (pos2 = newMorphemesSuffix.begin(); pos2 != newMorphemesSuffix.end(); ++pos2) 
		{			
			if(similarSuffixes.find(pos2->first)!=similarSuffixes.end()) continue;
			if(conditionalSimilarSuffixes.find(pos2->first)
				!=conditionalSimilarSuffixes.end()) continue;
			if(pos2->first.length()>=5) continue;
			if(countStringSuffix[pos2->first]>2000)
			{			
				index=pos2->second;
				if(relationsSuffix[index].find(pos->first) 
					!= relationsSuffix[index].end())
				{
					featureFile<<"1 ";
					//temp=pos->first + pos2->first;
					//featureFile<<(int)sigmoid(vocabulary[temp])+1<<" ";
				}
				else featureFile<<"0 ";
			}
		}
		featureFile<<endl;
	}
	
}


void Trie::printVocabulary()
{
	Map::const_iterator pos;
	for (pos = vocabulary.begin(); pos != vocabulary.end(); ++pos) 
	{
		//cout<<pos->first<<endl;		
		//vocabularyFile<<pos->second<<" "<<pos->first<<endl;		
		vocabularyFile<<"10 "<<pos->first<<endl;		
	}

	for (pos = smallRoots.begin(); pos != smallRoots.end(); ++pos) 
	{
		//cout<<pos->first<<endl;		
		//vocabularyFile<<pos->second<<" "<<pos->first<<endl;		
		vocabularyFile<<"10 "<<pos->first<<endl;
	}
}

void Trie::printAllIntoFile()
{
	Map::const_iterator pos;
	int count=0;
	int last;
	for (pos = vocabulary.begin(); pos != vocabulary.end(); ++pos) 
	{
		//cout<<pos->first<<endl;
		last= (int)( ((float)pos->second/1.2) + 1);
		for(int i=0;i<last;i++)
		{			
			goldsmithFile<<pos->first<<" ";
		}
		
		goldsmithFile<<endl;
		count+=last;
	}
	
	for (pos = smallRoots.begin(); pos != smallRoots.end(); ++pos) 
	{
		//cout<<pos->first<<endl;
		last= (int)( ((float)pos->second/1.2) + 1);
		for(int i=0;i<last;++i)
			goldsmithFile<<pos->first<<" ";
		goldsmithFile<<endl;
		count+=last;
	}
	cout<<"Count:"<<count<<endl;
	
}


int findInPreSuffRoot(string str)
{
	string temp=str;
	reverse(temp);

	if(newMorphemesPrefix.find(temp)!=newMorphemesPrefix.end())
		return 1;
	if(newMorphemesSuffix.find(str)!=newMorphemesSuffix.end())
		return 1;
	if(roots.find(str)!=roots.end())
		return 1;

	return 0;
		
}

int ifAllomorphOrComposite(string str, string& str1, string& str2)
{
	int i;
	if( similarSuffixes.find(str) != similarSuffixes.end() )
	{
		str=similarSuffixes[str];
		i=str.find_first_of('+');
		str1=str.substr(0,i);
		str2=str.substr(i+1);
		//cout<<str<<endl;
		return 1;
	}

	if( allomorphRoots.find(str) != allomorphRoots.end() )
	{
		str=allomorphRoots[str];
		i=str.find_first_of('+');
		str1=str.substr(0,i);
		str2=str.substr(i+1);
		return 1;
	}
	return 0;
}

void joinOptions(vector<string> options1, vector<string> options2, vector<string>& options)
{
	for(int i=0;i<options1.size();++i)
		for(int j=0;j<options2.size();++j)
		{
			//cout<<options1.at(i)<<"+"<<options2.at(j)<<endl;
			options.push_back(options1.at(i)+"+"+options2.at(j));
		}
}

void getDivisions(string str, vector<string>& options)
{
		int len=str.length();
		
		string sub,rest,outputStr;		
		vector<string> newOptions, newOptions1, newOptions2, subOptions;
		string str1, str2;

		for(int i=0;i<len;++i)
		{
			newOptions.clear();
			sub=str.substr(0,i+1);
			
			if( findInPreSuffRoot(sub) )  
			{
				//if(sub=="un") cout<<"found";				

				if(i!=(len-1))
				{
					//say driesfull... sub=dries rest=full
					//see whether sub is composite or allomorph
					subOptions.clear();
					if(ifAllomorphOrComposite(sub, str1, str2))
					{
						newOptions1.clear();
						newOptions2.clear();						
						getDivisions( str1, newOptions1);
						getDivisions( str2, newOptions2);
						joinOptions(newOptions1, newOptions2, subOptions);
					}
					else subOptions.push_back(sub);

					rest=str.substr(i+1,len-i-1);
					getDivisions( rest, newOptions);

					if(newOptions.size()>0)
					{	
						/*
						for(int v=0;v<newOptions.size();++v)
						{
							outputStr=sub+"+";
							outputStr+=newOptions.at(v);
							options.push_back(outputStr);
						}
						*/
						joinOptions(subOptions,newOptions,options);						
					}
				}
				else 
				{
					
					if(ifAllomorphOrComposite(str, str1, str2))
					{
						newOptions1.clear();
						newOptions2.clear();
						getDivisions( str1, newOptions1);
						getDivisions( str2, newOptions2);
						joinOptions(newOptions1, newOptions2, newOptions);

						for(int v=0;v<newOptions.size();++v)
							options.push_back(newOptions.at(v));					
					}

					else					
						options.push_back(str);
				}
				
			}
		}

}

void Trie::getDivisionsOfSuffix(string str, vector<string>& options)
{
		int len=str.length();
		
		string sub,rest,outputStr;		
		vector<string> newOptions;

		for(int i=0;i<len;++i)
		{
			newOptions.clear();
			sub=str.substr(0,i+1);
			
			if( newMorphemesSuffix.find(sub)!=newMorphemesSuffix.end() )
			{
				
				if(i!=(len-1))
				{
					rest=str.substr(i+1,len-i-1);
					getDivisionsOfSuffix( rest, newOptions);

					if(newOptions.size()>0)
					{						
						for(int v=0;v<newOptions.size();++v)
						{
							outputStr=sub+"+";
							outputStr+=newOptions.at(v);
							options.push_back(outputStr);
						}
						
					}
				}
				else 
				{					
					options.push_back(str);
				}
				
			}
		}
}

void Trie::getDivisionsOfRootsByRoots()
{
	Map::const_iterator pos;	
	int x=0;
	vector<string> options;
	cout<<"Dividing multiple roots\n";
	string str1, str2;
	int min;
	Map deleted;
	for (pos = roots.begin(); pos != roots.end(); ++pos)
	{
		options.clear();
		//if(vocabulary[pos->first]<10) continue;

		//if already a allomorphic root
		if(allomorphRoots.find(pos->first) != allomorphRoots.end())
			continue;
		
		getDivisionsOfRootsByRoot(pos->first,options);		

		if(options.size()>1)
		{
			//deleted[pos->first]=1;			
			
			if(getLengthWithoutHallant(pos->first)>10)  
			{
				deleted[pos->first]=1;	
				//continue;			
			}			
			else
			for(int i=0;i<options.size();++i)
			{
				if(options.at(i) == pos->first) continue;
				if( noOfDiv(options.at(i)) > 1 )
				{
					deleted[pos->first]=1;
					break;
				}		
		
				x=options.at(i).find_first_of('+');
				str1=options.at(i).substr(0,x);
				str2=options.at(i).substr(x+1);			

				cout<<pos->first<<":"<<endl;
				cout<<options.at(i)<<" ";
				cout<<"Freq:"<<vocabulary[pos->first]<<" "<<vocabulary[str1]
					<<" "<<vocabulary[str2]<<endl;

				
				//character=char+acter
				//if the freq of the compound is greater than both the elements, the division is not ok.				
				if( (vocabulary[pos->first] > vocabulary[str1]) && (vocabulary[pos->first] > vocabulary[str2]) )
				{
					cout<<" Special";
					continue;
				}

				else deleted[pos->first]=1;
				

				
				/*
				//if its less than both then the division is ok
				//ultraviolet=ultra+violet
				if( (vocabulary[pos->first] < vocabulary[str1]) && (vocabulary[pos->first] < vocabulary[str2]) )
				{
					deleted[pos->first]=1;
					break;		
				}
				*/	
				
				
				/*
				if(vocabulary[pos->first] > 500 ) 
				{
					if(vocabulary[str1]<vocabulary[str2])
						min=vocabulary[str1];
					else min=vocabulary[str2];

					//say moderate(1000) = mode(5) + rate(2000)
					if((float)vocabulary[pos->first]/min > 5)
					{
						cout<<" special2";
						break;
					}					
				}

				deleted[pos->first]=1;
				*/

			}
			cout<<endl;			
			
		}

	}

	
	//This is to promote segmentation of high-length words 
	//suggesting that any root which is > length 15 should be always segmented.
	if(PROMOTE_LONG_SEGMENTATION)
	{
	
		cout<<"\nPromoting Long Words' Segmentation"<<endl;
		
		for (pos = roots.begin(); pos != roots.end(); ++pos)
		{
			//if found in allomorphRoots spare them.
			if( allomorphRoots.find(pos->first) != allomorphRoots.end() ) continue;
			
			//We delete the high length words from the root list (roots're the unsegmented ones).
			if(pos->first.length()>=PROMOTE_LONG_SEGMENTATION_LENGTH) deleted[pos->first]=1;
		}
	
	}	

	for (pos = deleted.begin(); pos != deleted.end(); ++pos)
	{		
		roots.erase(pos->first);
		cout<<"Deleting:"<<pos->first<<endl;
	}
}

int allomorphOrRoot(string sub)
{
	//if(newMorphemesSuffix.find(sub) != newMorphemesSuffix.end()) return 0;

	string temp=sub;
	reverse(temp);
	//if(newMorphemesPrefix.find(temp) != newMorphemesPrefix.end()) return 0;

	if(vocabulary.find(sub) != vocabulary.end()) 
	{
		if(sub.length()>=3)
			return 1;
	}


	//if( roots.find(sub)!= roots.end() ) return 1;
			
	//if( allomorphRoots.find(sub) != allomorphRoots.end() ) return 1;

	return 0;

}

void Trie::getDivisionsOfRootsByRoot(string str, vector<string>& options)
{
		int len=str.length();
		
		string sub,rest,outputStr;		
		vector<string> newOptions;

		for(int i=0;i<len;++i)
		{
			newOptions.clear();
			sub=str.substr(0,i+1);			
			
			if(allomorphOrRoot(sub))
			{				
				if(i!=(len-1))
				{
					rest=str.substr(i+1,len-i-1);
					getDivisionsOfRootsByRoot( rest, newOptions);

					if(newOptions.size()>0)
					{						
						for(int v=0;v<newOptions.size();++v)
						{
							outputStr=sub+"+";
							outputStr+=newOptions.at(v);
							options.push_back(outputStr);
						}						
					}
				}
				else 
				{					
					options.push_back(str);
				}
				
			}
		}

}

//v2 is "ers", v1 is "er"
float Trie::compareRelationStrings(Map v1, Map v2)
{
	Map::const_iterator pos1;
	Map::const_iterator pos2;
	int x=0;
	for (pos2 = v2.begin(); pos2 != v2.end(); ++pos2)
	{	
		/*
		for (pos1 = v1.begin(); pos1 != v1.end(); ++pos1)
		{
			if(pos2->first==pos1->first)
			{	
				x++;
				break;
			}
		}
		*/

		if( v1.find(pos2->first) != v1.end() )
			x++;
		
	}

	return (float)x/v2.size();
}


//v1 is "er", v2 is "ers"
void Trie::printUncomparables(Map v1, Map v2)
{
	Map::const_iterator pos1;
	Map::const_iterator pos2;
	int x=0;
	
	for (pos2 = v2.begin(); pos2 != v2.end(); ++pos2)
	{	
		/*
		for (pos1 = v1.begin(); pos1 != v1.end(); ++pos1)
		{
			if(pos2->first==pos1->first)
			{	
				x++;
				break;
			}
		}
		*/

		if( v1.find(pos2->first) != v1.end() )
			x++;
		else cout<<pos2->first<<" ";
	}
	cout<<endl;
	
}


void Trie::showDivisionCountOfSuffix(string suffix, string parseStr, float& prev)
{
	vector<string> tokens;
	tokenize(parseStr,tokens,".","+");

	int ind1=newMorphemesSuffix[tokens.at(0)];
	int ind2=newMorphemesSuffix[suffix];
	float x= compareRelationStrings(relationsSuffix[ind1], relationsSuffix[ind2]);	

	//if( (x>=0.55) && (x<0.65) );
	//else return;					

	int len;
	if (tokens.size()>2) return;
	for(int i=0;i<tokens.size();++i)
	{
		len=getLengthWithoutHallant(tokens.at(i));
		cout<<tokens.at(i)<<"("<<countStringSuffix[tokens.at(i)]/len<<")+";
	}

	cout<<"  Strength:"<<x;

	if((x>=COMPOSITE_SUFFIX_THRESHOLD) && (tokens.size()>1))
	{
		if( x<=prev )
			return;
		else prev=x;

		int count= countStringSuffix[suffix]/getLengthWithoutHallant(suffix);
		int count1= countStringSuffix[tokens.at(0)]/getLengthWithoutHallant(tokens.at(0));
		int count2= countStringSuffix[tokens.at(1)]/getLengthWithoutHallant(tokens.at(1));


		if( (count < count1) && 
			(count < count2) );
		else { cout<<" Special "; return;}


		float y= (float)count1/count2;
		//if count1 is too big for count2 then count 1 is inflectional suffix.

		if(y<100)
		{
			similarSuffixes[suffix]=tokens.at(0)+"+"+tokens.at(1);
			similarSuffixesOpposite[tokens.at(0)]=suffix;
		}
	}

	cout<<"     , ";
}

void Trie::printSimilarSuffixes()
{
	Map2::const_iterator pos;
	cout<<"\nPrinting Similar Suffixes:"<<endl;	
	for (pos = similarSuffixes.begin(); pos != similarSuffixes.end(); ++pos)
	{
		cout<<pos->first<<" "<<pos->second<<endl;
	}
	
}

void Trie::getDivisionOfSuffixByRoot(string suffix, vector<string>& options)
{
	string sub, sub2;
	options.clear();
	int best=0;
	for(int i=0;i<(suffix.length()-1);++i)
	{		
		sub=suffix.substr(0,i+1);
		if( vocabulary.find(sub)!= vocabulary.end() )
		{
			sub2=suffix.substr(i+1);
			if(newMorphemesSuffix.find(sub2) !=  newMorphemesSuffix.end())
			{
				//check freq 				
				//check whether suffix is within 430

				if(countStringSuffix[sub2]<6000) continue;
				if(!freqMatch(sub,suffix,sub2))	continue;		
			
				if(countStringSuffix[sub2]>best)
				{
					options.clear();
					options.push_back(sub+"+"+sub2);
					best=countStringSuffix[sub2];			
				}
			}
		}
	}
		
}

void Trie::getDivisionOfSuffixesByRoot()
{
	Map::const_iterator pos;
	cout<<"\nPrinting Suffix Divisions by roots:"<<endl;
	int count;
	vector<string> options;
	int bestI;
	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos) 
	{
		if(similarSuffixes.find(pos->first) != similarSuffixes.end()) continue;
			
		//see whether its a root+suffix
		options.clear();
		getDivisionOfSuffixByRoot(pos->first,options);
		if(options.size()>=1)
		{
			bestI=0;
			/*
			for(int i=0;i<options.size();++i)
			{				

			}
			*/
			similarSuffixes[pos->first]=options.at(0);
			cout<<"Similar Suffix By Root:"<<pos->first<<" "<<options.at(0)
							<<countStringSuffix[pos->first]<<endl;
		}
		//else cout<<"Not:"<<pos->first<<endl;
	}
}

void Trie::getDivisionsOfSuffixes()
{
	vector<string> options;
	Map::const_iterator pos;
	cout<<"\nPrinting Suffix Divisions:"<<endl;
	int count;
	int len;
	float prev;
	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos) 
	{
		//if(pos->second<threshold1) continue;
		options.clear();
		getDivisionsOfSuffix(pos->first,options);
		if(options.size()<=1)
		{
			/*
			//see whether its a root+suffix
			options.clear();
			getDivisionOfSuffixByRoot(pos->first,options);
			if(options.size()>=1)
			{
				similarSuffixes[pos->first]=options.at(0);
				cout<<"Similar Suffix By Root:"<<pos->first<<" "<<options.at(0)<<countStringSuffix[pos->first]<<endl;
			}
			*/			

		}
		else 
		{
			len=getLengthWithoutHallant(pos->first);
			cout<<pos->first<<"("<<countStringSuffix[pos->first]/len<<")=";
			prev=0;
			for(int v=0;v<options.size();++v)
			{
				showDivisionCountOfSuffix(pos->first, options.at(v), prev);	
			}

			//if( similarSuffixes.find(pos->first) == similarSuffixes.end() )
			//	cout<<" Special2 ";
				

			//if( (prev>=0.55) && (prev<.65) )
			//	cout<<" Special";
			cout<<endl;			
		}
	}

	printSimilarSuffixes();
}

void Trie::removeSimilarSuffixes()
{
	Map2::const_iterator pos;
	Map::const_iterator pos2;
	int ind1, ind2, ind3;
	string last;

	//similar["ers"]="er";
	for (pos = similarSuffixes.begin(); pos != similarSuffixes.end(); ++pos) 
	{
		/*
		ind1=newMorphemesSuffix[pos->first]; //ers
		ind2=newMorphemesSuffix[pos->second]; //er
		last=pos->first.substr(pos->second.length(), pos->first.length() - pos->second.length());
		cout<<pos->first<<" "<<pos->second<<" "<<last<<endl;
		ind3=newMorphemesSuffix[last];		//s
		
		//cout<<pos->first<<":"<<pos->second<<":"<<endl;
		//printUncomparables(relationsSuffix[ind2], relationsSuffix[ind1]);

		//copy the relation string of pos->first to similar(pos->first)			
		for (pos2 = relationsSuffix[ind1].begin(); pos2 != relationsSuffix[ind1].end(); ++pos2)
		{
			//if not found in ind2 "er"
			if( relationsSuffix[ind2].find(pos2->first) == relationsSuffix[ind2].end() )
			{
				relationsSuffix[ind2][pos2->first]=1;
				//vocabulary[pos2->first+pos->second]=vocabulary[pos2->first+pos->first];
			}

			/*
			//if not found in ind3 "s"
			if( relationsSuffix[ind3].find(pos2->first + pos->second) == relationsSuffix[ind3].end() )
			{
				relationsSuffix[ind3][pos2->first + pos->second]=1;
				vocabulary[pos2->first+pos->second]=vocabulary[pos2->first+pos->first];
			}
			*/
				
		//}
		

		cout<<"Deleting Suffix "<<pos->first<<endl;
		//newMorphemesSuffix.erase(pos->first);
		deletedSuffixes[pos->first]=1;
	}
}


void Trie::printRelationStrings()
{
	Map::const_iterator pos;
	Map::const_iterator pos2;
	int index;
	string temp;
	
	for ( pos = newMorphemesPrefix.begin(); pos != newMorphemesPrefix.end(); ++pos ) 
	{
		temp=pos->first;
		reverse(temp);
		index=pos->second;
		if(relationsPrefix[index].size()>1000) continue;
		cout<<"\nPrefix:"<<temp<<endl;
		cout<<"======\n";		
		
  	    for ( pos2 = relationsPrefix[index].begin(); pos2 != relationsPrefix[index].end(); ++pos2 )
		{
			temp=pos2->first;
			reverse(temp);
			cout<<temp<<" ";
		}

		cout<<endl;
	}

	for ( pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos ) 
	{
		index=pos->second;
		//if(relationsSuffix[index].size()>1000) continue;
		cout<<"\nSuffix:"<<pos->first<<endl;
		cout<<"======\n";
		
     	for (pos2 = relationsSuffix[index].begin(); pos2 != relationsSuffix[index].end(); 
				++pos2)
			cout<<pos2->first<<" ";
		
		cout<<endl;
	}
	
}

//It only calculates affixes into its count ....
//getRootCount() calculates both affixes and roots into the root count
void Trie::getActualRootCount()
{
	//getactualrootcount
	Map countPrefix,countSuffix;
	Map::const_iterator pos;
	Map::const_iterator pos2;
	int index;
	string temp;
	int count;
	Map selected;

	for (pos = newMorphemesPrefix.begin(); pos != newMorphemesPrefix.end(); ++pos) 
	{		
		index=pos->second;		
  	    for (pos2 = relationsPrefix[index].begin(); pos2 != relationsPrefix[index].end(); ++pos2)
			selected[pos2->first]=1;

	}

	for (pos = selected.begin(); pos != selected.end(); ++pos) 
	{	
		actualPrefixCountOfRoot[pos->first]=0;
  	    for (pos2 = newMorphemesPrefix.begin(); pos2 != newMorphemesPrefix.end(); ++pos2) 
		{
			if(relationsPrefix[pos2->second].find(pos->first) !=relationsPrefix[pos2->second].end())
				actualPrefixCountOfRoot[pos->first]+=1;
		}
	}

	//do it for suffixes
	selected.clear();
	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos) 
	{
		index=pos->second;		
  	    for (pos2 = relationsSuffix[index].begin(); pos2 != relationsSuffix[index].end(); ++pos2)
			selected[pos2->first]=1;
	}

	for (pos = selected.begin(); pos != selected.end(); ++pos) 
	{	
		actualSuffixCountOfRoot[pos->first]=0;
  	    for (pos2 = newMorphemesSuffix.begin(); pos2 != newMorphemesSuffix.end(); ++pos2) 
		{
			if(relationsSuffix[pos2->second].find(pos->first) !=relationsSuffix[pos2->second].end())
				actualSuffixCountOfRoot[pos->first]+=1;
		}
	}

}

void Trie::getFinalMorphemes()
{
	Map countPrefix,countSuffix;
	Map::const_iterator pos;
	Map::const_iterator pos2;
	int index;
	string temp;
	int count;
	for (pos = newMorphemesPrefix.begin(); pos != newMorphemesPrefix.end(); ++pos) 
	{
		//temp=pos->first;
		//reverse(temp);
		index=pos->second;
		
		count=0;
  	    for (pos2 = relationsPrefix[index].begin(); pos2 != relationsPrefix[index].end(); ++pos2)
		{
			count+=findCount(0,pos2->first);			
		}
		countPrefix[pos->first]=count*getLengthWithoutHallant(pos->first);
	}

	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos) 
	{
		index=pos->second;
		count=0;

	    for (pos2 = relationsSuffix[index].begin(); pos2 != relationsSuffix[index].end(); ++pos2)
		{
			count+=findCount(1,pos2->first);			
		}
	  	countSuffix[pos->first]=count*getLengthWithoutHallant(pos->first);
	}

	//string temp;
	
	LinkList l1;
	l1.setMax(800);	
	for (pos = countPrefix.begin(); pos != countPrefix.end(); ++pos)
	{
		if(pos->second>10) 
		{
			finalPrefix[pos->first]= 1;			
			l1.add(pos->first,pos->second);
		}
	}

	
	LinkList l2;
	l2.setMax(800);
	for (pos = countSuffix.begin(); pos != countSuffix.end(); ++pos)
	{
		if(pos->second>10) 
		{
			finalSuffix[pos->first]= 1;
			l2.add(pos->first,pos->second);
		}
	}

	
	l1.setIterator();
	string next;	
	cout<<"\nFinal Prefixes:\n";
	int rank=0;
	while(true)
	{
		count=(int)l1.getNext(next);	
		reverse(next);	
		if(count==0) break;
		cout<<next<<":"<<count<<" ratio: "<<(float)count/countStringPrefix[next]<<" rank:"<<rank<<endl;		
		rank++;
	}

	l2.setIterator();	
	cout<<"\nFinal Suffixes\n";
	rank=0;
	while(true)
	{
		count=(int)l2.getNext(next);			
		if(count==0) break;
		cout<<next<<":"<<count<<" ratio: "<<(float)count/countStringSuffix[next]<<" rank:"<<rank<<endl;
		rank++;

	}
}

int Trie::getRange(int x)
{
	int y=x/10;
	if(y>4) return 4;
	if(y<1) return 1;
	return y;
}

int Trie::freqMatch(string root, string inflection, string suffix)
{
	//return 1;
	if(vocabulary[root] < vocabulary[inflection])
	{
			float ratio=(float)vocabulary[inflection]/vocabulary[root];
			if(ratio> WRFR_SUFFIX_THRESHOLD)
				return 0;
	}
	return 1;
		
}

int Trie::freqMatchMultipleRoot(string inflection, string root1, string root2)
{

	if( (vocabulary[inflection]>vocabulary[root1]) && (vocabulary[inflection]>vocabulary[root2]) )
		return 0;
	return 1;
		
}


int Trie::freqMatchPrefix(string root, string inflection, string prefix)
{
	//return 1;
	if(lowFreqRatioPrefix.find(prefix) != lowFreqRatioPrefix.end())
		return 1;

	if(vocabulary[root] < vocabulary[inflection])
	{
			float ratio=(float)vocabulary[inflection]/vocabulary[root];
			if(ratio> WRFR_PREFIX_THRESHOLD)
				return 0;
	}
	return 1;
		
}

int Trie::freqMatchSmallRoot(string root, string inflection, string suffix)
{
	//return 1;
	if(vocabulary[inflection] > smallRoots[root])
	{
			float ratio=(float)vocabulary[inflection]/smallRoots[root];
			//cout<<""<<ratio<<" ";
			if(ratio> WRFR_SUFFIX_THRESHOLD)
				return 0;
	}
	return 1;
		
}

int Trie::freqMatchPrefixSmallRoot(string root, string inflection, string prefix)
{
	//return 1;
	reverse(prefix);
	if(lowFreqRatioPrefix.find(prefix) != lowFreqRatioPrefix.end())
		return 1;

	if(smallRoots[root] < vocabulary[inflection])
	{
			float ratio=(float)vocabulary[inflection]/smallRoots[root];
			//cout<<" "<<ratio<<" ";			
			if(ratio> WRFR_PREFIX_THRESHOLD)
				return 0;
	}
	return 1;
		
}

int Trie::freqMatchMultipleRootSmall(string inflection, string root1, string root2)
{

	//if first one is small root
	if(smallRoots.find(root1)!=smallRoots.end())
	{	
		if( (vocabulary[inflection]>smallRoots[root1]) || (vocabulary[inflection]>vocabulary[root2]) )
			return 0;

	}
	else
	{
		if( (vocabulary[inflection]>smallRoots[root2]) || (vocabulary[inflection]>vocabulary[root1]) )
			return 0;

	}	
	return 1;
		
}


void Trie::showHighFrequencyWords()
{
	Map::const_iterator pos;
	Map::const_iterator pos2;
	freqFile<<"\n\nShowing high frequency ratios:"<<endl;
	int index;
	string current;
	for (pos2 = newMorphemesSuffix.begin(); pos2 != newMorphemesSuffix.end(); ++pos2)
	{
		current=pos2->first;
		freqFile<<"\n\nSuffix: "<<current<<endl;
		index=pos2->second;

		for (pos = relationsSuffix[index].begin(); pos != relationsSuffix[index].end(); ++pos)
		{		
			if(vocabulary[pos->first] > vocabulary[pos->first+current])
			{
			
				freqFile<<pos->first<<":"<<vocabulary[pos->first]<<" "
					<<pos->first+current<<":"<<vocabulary[pos->first+current]
					<<" -- ratio:"<<(float)vocabulary[pos->first]/vocabulary[pos->first+current]<<endl;
			}
		}
	}
}


void Trie::filterByFrequency(string current, int index)
{
	//start filtering ....
	//filter by frequency
	Map::const_iterator pos;
	//Map::const_iterator pos2;
	//string whole;
	//Map count;
	//int index2;
	freqFile<<"\n\nSuffix: "<<current<<endl;
	int lowFreqCount=0;	
	for (pos = relationsSuffix[index].begin(); pos != relationsSuffix[index].end(); ++pos)
	//pos->first= sing, ring, ....
	{		
		if(vocabulary[pos->first] < vocabulary[pos->first+current])
		{
		
			freqFile<<pos->first<<":"<<vocabulary[pos->first]<<" "
				<<pos->first+current<<":"<<vocabulary[pos->first+current]
				<<" -- ratio:"<<(float)vocabulary[pos->first+current]/vocabulary[pos->first]<<endl;
		
			lowFreqCount++;
		}
	}
	
	float ratio, threshold;
	int mult;
	if(lowFreqCount>0)
	{
		ratio=(float)lowFreqCount*100/relationsSuffix[index].size();
		//freqFile<<"Low Freq Percentage:"<<ratio<<endl;
		mult=getRange((int)ratio);
		threshold=(1+0.5*mult)*ratio;		
		
		if(current.length()<3)
		{
			//if the ratio is too low dont divide it by length
			//threshold=threshold/(4-current.length());			
			threshold=threshold/2;			
		}
		
		if(threshold<5)
		{
			freqFile<<"Low frequency:"<<endl;
			threshold=5;
		}		

		//freqFile<<"Threshold:"<<threshold<<endl;
	}
	
	threshold+= (threshold/10)*2;	//if the difference is in between some range then pick it up.
	freqThreshold[current]=threshold;	

	//threshold= (countStringSuffix[current]>2000)?10:3;
	
	
	//SO the previous threshold doesn't work. We adhered to a simple single point thresholding scheme here.
	threshold=WRFR_SUFFIX_THRESHOLD;		

	
	Map removed;
	
	//leave those words whose freq is greater than some threshold;
	freqFile<<"\nIncorrect Attachments:"<<endl;
	for (pos = relationsSuffix[index].begin(); pos != relationsSuffix[index].end(); ++pos)
	//pos->first= sing, ring, ....
	{
		//leave high length words. Those will be almost always segmented.
		if(getLengthWithoutHallant(pos->first) > 8) continue;

		//see the negative threshold	
		if(vocabulary[pos->first] < vocabulary[pos->first+current])
		{
			ratio=(float)vocabulary[pos->first+current]/vocabulary[pos->first];
			
			if(ratio>=threshold)	
			{
				freqFile<<"Removing:"<<pos->first<<":"<<ratio<<endl;
				removed[pos->first]=1; 
			}
		}
		/*
		else
		{
			ratio=(float)vocabulary[pos->first]/vocabulary[pos->first+current];
			if(ratio> 200)
				freqFile<<"Special Removing:"<<pos->first<<":"<<current<<endl;
		}
		*/
	}


	//delete the removed ones from the corresponding suffix
	//if(current!="s")
	for (pos = removed.begin(); pos != removed.end(); ++pos)
	//pos->first= sing, ring, ....
	{
		relationsSuffix[index].erase(pos->first);
	}

	/*
	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos)
	{
		countStringSuffix[pos->first]=relationsSuffix[pos->second].size() * getLengthWithoutHallant(pos->first);
	}
	*/
}


void Trie::filterByFrequencyPrefix(string current, int index)
{
	//start filtering ....
	//filter by frequency

	Map::const_iterator pos;
	string rev=current;
	reverse(rev);
	
	freqFile<<"\n\nPrefix: "<<rev<<endl;
	freqFile<<"Prefix Strength:"<<countStringPrefix[current]<<endl;
	int lowFreqCount=0;	
	string tempRoot;
	for (pos = relationsPrefix[index].begin(); pos != relationsPrefix[index].end(); ++pos)
	//pos->first= sing, ring, ....
	{		
		tempRoot=pos->first;
		reverse(tempRoot);

		//if un+friendly and friendly is not found in root
		//if un+real and real is found in root then check on it
		if(roots.find(tempRoot) == roots.end()) continue;
		
		if(vocabulary[tempRoot] < vocabulary[rev+tempRoot])
		{
		
			freqFile<<tempRoot<<":"<<vocabulary[tempRoot]<<" "
				<<rev+tempRoot<<":"<<vocabulary[rev+tempRoot]
				<<" -- ratio:"<<(float)vocabulary[rev+tempRoot]/vocabulary[tempRoot]
				<<endl;
			if(vocabulary[rev+tempRoot]>20000) freqFile<<"Special:"<<tempRoot<<endl;
			lowFreqCount++;
		}
	}
	
	float ratio, threshold;
	int mult;
	if(lowFreqCount>0)
	{
		ratio=(float)lowFreqCount*100/relationsPrefix[index].size();
		//freqFile<<"\nLow Freq Percentage:"<<ratio<<endl;

		if(ratio<5) 
		{
			lowFreqRatioPrefix[current]=1;
			return;		
		}


		mult=getRange((int)ratio);
		threshold=(1+.5*mult)*ratio;
		
		if(current.length()<3)
		{
			//if the ratio is too low dont divide it by length
			//if(ratio>9) threshold=threshold/(4-current.length());			
			if(ratio>9) threshold=threshold/2;			
		}

		if(threshold<8)
		{
			freqFile<<"Low frequency:"<<endl;
			threshold=8;
		}

		//freqFile<<"Threshold:"<<threshold<<endl;
	}
		
	threshold += (threshold/10)*2;	//if the difference is in between some range then pick it up.
	freqThresholdPrefix[current] = threshold;
	
	
	//Set threshold to a specific value.
	threshold=WRFR_PREFIX_THRESHOLD;	
	
	
	freqFile<<"\nIncorrect Attachments:"<<endl;
	Map removed;
	//leave those words whose freq is greater than some threshold;
	for (pos = relationsPrefix[index].begin(); pos != relationsPrefix[index].end(); ++pos)
	//pos->first= sing, ring, ....
	{	
		tempRoot=pos->first;
		reverse(tempRoot);

		if(roots.find(tempRoot) == roots.end()) continue;

		if(vocabulary[tempRoot] < vocabulary[rev+tempRoot])
		{
			ratio=(float)vocabulary[rev+tempRoot]/vocabulary[tempRoot];
			if(ratio>=threshold)
			{
				freqFile<<"Removing:"<<tempRoot<<":"<<ratio<<endl;
				removed[pos->first]=1;
			}
		}
	}


	//delete the removed ones from the corresponding prefix	
	for (pos = removed.begin(); pos != removed.end(); ++pos)
	//pos->first= sing, ring, ....
	{
		relationsPrefix[index].erase(pos->first);
	}

	/*
	for (pos = newMorphemesPrefix.begin(); pos != newMorphemesPrefix.end(); ++pos)
	{
		countStringPrefix[pos->first]=relationsPrefix[pos->second].size() * pos->first.length();
	}
	*/
}


void Trie::filterRelationString(string current, int index, vector<string>& deleted, int &suffixDeleted)
{
	cout<<"\n\nCurrent:"<<current<<" Size:"<<relationsSuffix[index].size()<<endl;
	suffixDeleted=0;

	//leave it if its inflectional suffixes
	//if(countStringSuffix[current]>=5000)	//leave the good ones
	//	return;
	if(vocabulary.find(current) != vocabulary.end())
		return;

	string sub1, sub2;
	int p;
	if(similarSuffixes.find(current) != similarSuffixes.end())
	{
		//if its ers=er+s
		p=similarSuffixes[current].find_first_of('+');
		sub1=similarSuffixes[current].substr(0,p);
		sub2=similarSuffixes[current].substr(p+1);
		if( (sub1+sub2)==current)
			return;				
	}
	
	Map::const_iterator pos;
		
	Map::const_iterator pos2;
	string whole;
	Map count;
	int index2;
	int plus;
	string subAllomorph;
	for (pos = relationsSuffix[index].begin(); pos != relationsSuffix[index].end(); ++pos)
	// pos->first= sing, ring, ....
	{
		//look the string in other suffix list
		for (pos2 = newMorphemesSuffix.begin(); pos2 != newMorphemesSuffix.end(); ++pos2)
		{	

			//if its ers=er+s			
			if( similarSuffixes.find(pos2->first) != similarSuffixes.end() )
			{
				plus=similarSuffixes[pos2->first].find_first_of('+');
				subAllomorph=similarSuffixes[pos2->first].substr(0,plus);
				if(subAllomorph==current) continue;				
			}						

			index2=pos2->second;
			if (index2==index) continue;
			//if(similarSuffixes)			
		    if(relationsSuffix[index2].find(pos->first)!= relationsSuffix[index2].end())
			{
				//if the suffix is found before
				if(count.find(pos2->first) != count.end())
					count[pos2->first]+=1;
				else count[pos2->first]=1;
			}			
		}
		
		//work with whole string
		whole=pos->first+current;	//sing+er				
	}

	//listing the 10 best suffixes which are good for current suffix.
	LinkList bestSuffixes;
	bestSuffixes.setMax(10);	

	float weight;
	float minWeight=1;
	float maxWeight=0;
	for (pos = count.begin(); pos != count.end(); ++pos)
	{
		//cout<<"Count:"<<pos->first<<"="<<pos->second<<endl;
		index2=newMorphemesSuffix[pos->first];
		if(pos->second<=2) continue;
		weight=(float) pos->second * pos->second 
				/ ( relationsSuffix[index].size() 
					* relationsSuffix[index2].size() );		

		//weight=(float) pos->second / ( relationsSuffix[index].size() 
		//			+ relationsSuffix[index2].size() - pos->second);		

		if( weight<minWeight )
			minWeight=weight;
		if(weight>maxWeight)
			maxWeight=weight;
		bestSuffixes.add(pos->first, weight);
	}

	cout<<"Showing the bests\n";
	bestSuffixes.setIterator();
	string next;
	string bests[10];
	int i=0;
	float weight2;	
	while(true)
	{
		weight=bestSuffixes.getNext(next);
		if(weight==0) break;
		bests[i]=next;
		i++;
		weight2=setRange(1,10,minWeight, maxWeight, weight);	
		cout<<next<<" weight="<<weight<<" New weight:"<<weight2<<endl;		
	}

	/*
	if(i<=4) 
	{
		
		suffixDeleted=1;
		for (pos = relationsSuffix[index].begin(); pos != relationsSuffix[index].end(); ++pos)
		{
			deleted.push_back(pos->first);
		}
		
		cout<<"Special Suffix Deletion:"<<endl;
		//return;
	}	
	*/

	//filter by weight
	float  totalWeight;
	float avgRatio1=0, avgRatio2=0;
	int count1=0,count2=0,count3=0;
	float ratio;
	//featureFile<<relationsSuffix[index].size()<<" 20\n";
	cout<<endl;
	weightFile<<"\n\nCurrent Suffix:"<<current<<endl;
	
	for (pos = relationsSuffix[index].begin(); pos != relationsSuffix[index].end(); ++pos)
	//pos->first= sing, ring, ....
	{
		//calculate score for sing, ring .....

		//dont need to check the high length words 
		if(getLengthWithoutHallant(pos->first) >= 8)
			continue;
				
		//see the average negative frequency ratio and positive frequency ratio
		if(vocabulary[pos->first] > vocabulary[pos->first+current])
		{
			count1++;
			avgRatio1+=(float)vocabulary[pos->first]/vocabulary[pos->first+current];

			/*
			if( (float)vocabulary[pos->first]/vocabulary[pos->first+current] > 2 )
				continue;
			*/
			cout<<pos->first<<":";
			cout<<" freq ratio:"<<(float)vocabulary[pos->first]/vocabulary[pos->first+current] <<"     ";
			
		}
		else	//if -
		{			
			avgRatio2+=(float)vocabulary[pos->first+current]/vocabulary[pos->first];
			count2++;
			
			cout<<pos->first<<":";
			cout<<"freq ratio:-"<<(float)vocabulary[pos->first+current]/vocabulary[pos->first] <<"      ";
			
		}		
					
		//featureFile<<pos->first<<endl;
		totalWeight=0;
		bestSuffixes.setIterator();		
		int x=0;
		float averageCoOccur=0;
		int noOfCombinations=0;		
		while(true)
		{
			weight=bestSuffixes.getNext(next);
			if(weight==0) break;

			weight=setRange(1,10,minWeight, maxWeight, weight);		

			/*
			//check any of the ranked suffixes are "ers" when considering "er".
			if( similarSuffixes.find(next) != similarSuffixes.end() )
			{
				plus=similarSuffixes[next].find_first_of('+');
				subAllomorph=similarSuffixes[next].substr(0,plus);
				if(subAllomorph==current);
				else weight=weight*2;
			}
			else weight*=2;
			*/


			index2=newMorphemesSuffix[next];
			if(relationsSuffix[index2].find(pos->first)!=relationsSuffix[index2].end())
			{
				totalWeight+=weight;
				cout<<next<<" ";
				//featureFile<<"1 ";
				
				/*			
				for(int k=x-1;k>=0;--k)
				{
					index2=newMorphemesSuffix[bests[k]];
					if(relationsSuffix[index2].find(pos->first)
						!=relationsSuffix[index2].end())
						{
							averageCoOccur+=suffixCooccurance[k][x];
							noOfCombinations++;
						}
				}
				*/
			
			}
			//else featureFile<<"0 ";			
			x++;
		}
		
		totalWeight=totalWeight/SLS_NORMALIZATION_CONSTANT;

		//featureFile<<endl;
		//totalWeight=totalWeight/3;
		
		/*
		if(totalWeight==0)
		{
			count3++;
			cout<<"Total Weight is "<<totalWeight<<endl;		
			if( ((float)vocabulary[pos->first]/vocabulary[pos->first+current]) <= 1.5)
			{
				weightFile<<"Deleting:"<<pos->first+current<<" weight:0 freq ratio:"<<(float)vocabulary[pos->first]/vocabulary[pos->first+current]<<endl;
				deleted.push_back(pos->first);
			}
			continue;
		}
		*/
		//else
		//{
			/*
			if(totalWeight<7) totalWeight*=1.5;
			else totalWeight*=2;
			*/
			cout<<"Total Weight is "<<totalWeight<<endl;
			if(vocabulary[pos->first] < vocabulary[pos->first+current])
			{
				ratio=(float)vocabulary[pos->first+current]/vocabulary[pos->first];
				
				//join WLLR and SLS together.
				totalWeight-=ratio;				
				
				
				if(totalWeight<=0) 
				{
					weightFile<<"Incorrect Attachment: "<<pos->first+current<<" weight:"<<totalWeight+ratio<<" ratio:-"<<ratio<<endl;
					deleted.push_back(pos->first);
				}
				continue;
			}
		
			//if freq ratio is positive
			/*			
			ratio=(float)vocabulary[pos->first]/vocabulary[pos->first+current];
			if( (totalWeight<0.5) && (vocabulary[pos->first]>=20) )
			{
				cout<<"Deleting "<<pos->first<<" from suffix "<<current<<endl;
				//relationsSuffix[index].erase(pos->first);
				deleted.push_back(pos->first);
			}
			*/
			
			
		//}
		//cout<<"Average Similarity:"<<averageCoOccur/noOfCombinations<<endl;
	}
	cout<<"Average Ratio: "<<avgRatio1/count1<<" "<<avgRatio2/count2<<endl;
	cout<<"low freq percentage: "<<(float)count2/relationsSuffix[index].size()<<endl;	
	weightFile<<"low freq percentage: "<<(float)count2/relationsSuffix[index].size()<<endl;
	weightFile<<"weight 0 percentage: "<<(float)count3/relationsSuffix[index].size()<<endl;
	if((float)count3/relationsSuffix[index].size() > 0.45)
	{
		weightFile<<"Special"<<endl;
		deleted.clear();
	}		
}


void Trie::filterRelationStrings1()
{	
	cout<<"Step2: Detecting Incorrect Attachments Using Relative Frequency:\n";	
	
	Map::const_iterator pos;
	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos) 
	{
		countStringSuffixFirst[pos->first]=countStringSuffix[pos->first];

		filterByFrequency( pos->first, pos->second );
		//filterRelationString(pos->first, pos->second);
		
		countStringSuffix[pos->first]=relationsSuffix[pos->second].size()	
				* getLengthWithoutHallant( pos->first );	
	}	
	//filterRelationString("al", newMorphemesSuffix["al"]);

	for (pos = newMorphemesPrefix.begin(); pos != newMorphemesPrefix.end(); ++pos) 
	{
		countStringPrefixFirst[pos->first]=countStringPrefix[pos->first];
		
		filterByFrequencyPrefix(pos->first, pos->second);
		//filterRelationString(pos->first, pos->second);
		
		countStringPrefix[pos->first]=relationsPrefix[pos->second].size()	
				* getLengthWithoutHallant(pos->first);
	}
}


void Trie::filterRelationStrings2()
{	
	cout<<"Step3: Detecting Incorrect Attachments Using Suffix Level Similarity:\n";	
	Map::const_iterator pos;
	vector<string> deleted;
	int i, deletedOrNot;
	Map deletedSuf;
	for (pos = newMorphemesSuffix.begin(); pos != newMorphemesSuffix.end(); ++pos) 
	{		
		deleted.clear();
		deletedOrNot=0;
		filterRelationString(pos->first, pos->second, deleted, deletedOrNot);
		for(i=0;i<deleted.size();++i)
			relationsSuffix[pos->second].erase(deleted.at(i));		
		countStringSuffix[pos->first]=relationsSuffix[pos->second].size()	
				* getLengthWithoutHallant(pos->first);	
	}
	
	//printRelationStrings();
}



void getOriginalSuffixes()
{
	ifstream f("pureSuffixes.txt");

	char temp[10000];
	string str;
	
  	if (f.is_open())
  	{
		//get the file data
		while (! f.eof() )
  		{				
	    	f.getline(temp,10000);
			str=temp;
			originalSuffixes[str]=1;			
		}
		
	}
	
}

int checkAlphaNumeric(string str)
{
	for(int i=0;i<str.length();++i)
	{
		if( (str[i]>='A') && (str[i]<='Z') ) return 1;
		if( (str[i]>='a') && (str[i]<='z') ) return 1;
	}
	return 0;	
}

//Print top 100 highest frequency words. 
void getHighFreqWords()
{
	LinkList list;
	list.setMax(100);

	Map::const_iterator pos;
	for (pos = vocabulary.begin(); pos != vocabulary.end(); ++pos) 
	{
		list.add(pos->first,pos->second);				
	}

	string next;
	int freq;
	cout<<"High Freq Words:\n";
	list.setIterator();
	while(true)
	{
		freq=(int) list.getNext(next);
		if(freq==0) break;
		cout<<next<<" "<<freq<<endl;				
	}

}

int checkNull(string str, int i)
{
	if( (str[i]=='n') && (str[i+1]=='u') && (str[i+2]=='l') && (str[i+3]=='l') )
		return 1;
	return 0;
}

int checkNull(string str)
{
	for(int i=0;i<str.size();++i)
	{
		 if( (i+3) >= str.size() )  return 0;
		 if(checkNull(str,i)) return 1;
	}
	return 0;
}



void getVocabulary(string vocabularyFile)
{	
	//ifstream f("freq-wsj.out");
	//ifstream f("in.txt");
	//ifstream f("wordlist.eng");
	//ifstream f("wordlist.tur");
	ifstream f(vocabularyFile.c_str());

	char temp[10000];
	string str, sub, sub1, sub2;
	string::size_type pos;     
	int len;

	//cout<<"Getting Data From File:"<<file<<"\n";
	vector<string> tokens;
	string temp2;
  	if (f.is_open())
  	{	    		
		//get the file data
		while (! f.eof() )
  		{				
		    f.getline(temp,10000);			
			str=temp;
			if(str=="") continue;
			len=str.length();			
			pos = str.find_first_of(" ", 0);
			sub=str.substr(0,pos);
			int freq=atoi(sub.c_str());

			//We drop freq 1 words
			if(freq<=LOW_FREQUENCY_DROPOUTS_LEARNING) continue;

			sub=str.substr(pos+1,len-pos-1);
			//cout<<sub;
			
			tokens.clear();
			tokenize(sub,tokens,"","-");			
						
			for(int i=0;i<tokens.size();++i)
			{
				//if( !checkAlphaNumeric(tokens.at(i)) ) continue;
				
				//if( (freq<=1) && (tokens.size()==1) )
				//{
				//	lowFreqWords.push_back(tokens.at(i));
				//	continue;
				//}

				//cout<<"checking "<<tokens.at(i)<<endl;

				if(checkNull(tokens.at(i))) continue;

				len=getLengthWithoutHallant(tokens.at(i));

				if(len<SMALL_ROOT_LENGTH) continue;

				if(len==SMALL_ROOT_LENGTH) 
				{
					
					if(smallRoots.find(tokens.at(i)) != smallRoots.end())
								smallRoots[tokens.at(i)]+=freq;
					else smallRoots[tokens.at(i)]=freq;					
					continue;	
				}
				

				//cout<<""

				//cout<<"inserting:"<<tokens.at(i)<<endl;
				if(vocabulary.find(tokens.at(i)) != vocabulary.end())
					vocabulary[tokens.at(i)]+=freq;
				else vocabulary[tokens.at(i)]=freq;

				temp2=tokens.at(i);
				reverse(temp2);
				
				if(reverseVocabulary.find(temp2) != reverseVocabulary.end())
					reverseVocabulary[temp2]+=freq;				
				else
					reverseVocabulary[temp2]=freq;
			}
		}
	}
	else 
	{
		cout<<"Vocabulary File Not Found"<<endl;
		exit(0);
	}

	f.close();

	/*
	ifstream f2("bangla2.txt");	
  	if (f2.is_open())
  	{	    		
		//get the file data
		while (! f2.eof() )
  		{				
	    	f2.getline(temp,10000);
			str=temp;
			if(str=="") continue;
			len=str.length();			
			pos = str.find_first_of(" ", 0);
			sub1=str.substr(0,pos);			
			sub2=str.substr(pos+1,len-pos-1);
			int freq=atoi(sub2.c_str());
			//cout<<sub;
			
			//if(freq<=1) continue;

			tokens.clear();
			tokenize(sub1,tokens,"","-");
			
			for(int i=0;i<tokens.size();++i)
			{
				
				//if( !checkAlphaNumeric(tokens.at(i)) ) continue;
				if(checkNull(tokens.at(i))) continue;

				len=getLengthWithoutHallant(tokens.at(i));

				if(len==1) continue;

				if(len<=2) 
				{
					if(smallRoots.find(tokens.at(i)) != smallRoots.end())
								smallRoots[tokens.at(i)]+=freq;
					else smallRoots[tokens.at(i)]=freq;					
					continue;
				}
				

				if(vocabulary.find(tokens.at(i)) != vocabulary.end())
					vocabulary[tokens.at(i)]+=freq;
				else vocabulary[tokens.at(i)]=freq;
				
				temp2=tokens.at(i);				
				reverse(temp2);

				if(reverseVocabulary.find(temp2) != reverseVocabulary.end())
					reverseVocabulary[temp2]+=freq;				
				else 
					reverseVocabulary[temp2]=freq;
			}
		}
	}	
	else exit(0);
	f2.close();
	

	ifstream f3("bangla4.txt");	
  	if (f3.is_open())
  	{	    		
		//get the file data
		while (! f3.eof() )
  		{				
	    	f3.getline(temp,10000);			
			str=temp;
			if(str=="") continue;
			len=str.length();			
			pos = str.find_first_of(" ", 0);
			sub1=str.substr(0,pos);			
			sub2=str.substr(pos+1,len-pos-1);
			int freq=atoi(sub2.c_str());
			//cout<<sub;

			//if(freq<=1) continue;
			
			tokens.clear();
			tokenize(sub1,tokens,"","-");
			
			for(int i=0;i<tokens.size();++i)
			{
				//if( !checkAlphaNumeric(tokens.at(i)) ) continue;
				if(checkNull(tokens.at(i))) continue;

				len=getLengthWithoutHallant(tokens.at(i));

				if(len==1) continue;
				
				if(len<=2) 
				{
					if(smallRoots.find(tokens.at(i)) != smallRoots.end())
								smallRoots[tokens.at(i)]+=freq;
					else smallRoots[tokens.at(i)]=freq;
					
					continue;
				}

				if(vocabulary.find(tokens.at(i)) != vocabulary.end())
					vocabulary[tokens.at(i)]+=freq;
				else vocabulary[tokens.at(i)]=freq;
				
				temp2=tokens.at(i);				
				reverse(temp2);

				if(reverseVocabulary.find(temp2) != reverseVocabulary.end())
					reverseVocabulary[temp2]+=freq;				
				else 
					reverseVocabulary[temp2]=freq;
			}
		}
	}	
	else exit(0);
	f3.close();
	

	ifstream f4("bangla9.txt");	
  	if (f4.is_open())
  	{	    		
		//get the file data
		while (! f4.eof() )
  		{				
	    		f4.getline(temp,10000);			
			str=temp;
			if(str=="") continue;
			len=str.length();			
			pos = str.find_first_of(" ", 0);
			sub1=str.substr(0,pos);			
			sub2=str.substr(pos+1,len-pos-1);
			int freq=atoi(sub2.c_str());
			//cout<<sub;

			//if(freq<=1) continue;
			
			tokens.clear();
			tokenize(sub1,tokens,"","-");
			
			for(int i=0;i<tokens.size();++i)
			{
				//if( !checkAlphaNumeric(tokens.at(i)) ) continue;
				if(checkNull(tokens.at(i))) continue;

				len=getLengthWithoutHallant(tokens.at(i));

				if(len==1) continue;

				if(len<=2) 
				{
					if(smallRoots.find(tokens.at(i)) != smallRoots.end())
								smallRoots[tokens.at(i)]+=freq;
					else smallRoots[tokens.at(i)]=freq;					
					continue;
				}

				if(vocabulary.find(tokens.at(i)) != vocabulary.end())
					vocabulary[tokens.at(i)]+=freq;
				else vocabulary[tokens.at(i)]=freq;
				
				temp2=tokens.at(i);				
				reverse(temp2);

				if(reverseVocabulary.find(temp2) != reverseVocabulary.end())
					reverseVocabulary[temp2]+=freq;				
				else 
					reverseVocabulary[temp2]=freq;
			}
		}
	}	
	else exit(0);
	f4.close();
	*/
}

void getClusters()
{
	ifstream f("noun.txt");
	char temp[500];
  	if (f.is_open())
  	{	    		
		//get the file data
		while (! f.eof() )
  		{				
	    		f.getline(temp,10000);
			nouns[temp]=1;

		}
	}
	else exit(0);
	f.close();
	
	ifstream f2("adjective.txt");	
  	if (f2.is_open())
  	{	    		
		//get the file data
		while (! f2.eof() )
  		{				
	    		f2.getline(temp,10000);
			adjectives[temp]=1;

		}
	}
	else exit(0);
	f2.close();

	ifstream f3("verb.txt");
  	if (f3.is_open())
  	{	    		
		//get the file data
		while (! f3.eof() )
  		{				
	    		f3.getline(temp,10000);
			verbs[temp]=1;

		}
	}
	else exit(0);
	f3.close();
}

int checkType(string token)
{
	if(roots.find(token) != roots.end())
		return 1;

	if(newMorphemesSuffix.find(token) != newMorphemesSuffix.end())
	{
		reverse(token);
		if(newMorphemesPrefix.find(token) != newMorphemesPrefix.end())
			return 5;	//both suffix and prefix
		else return 2;
	}
	return 3;	//prefix
}

int ifFoundRoot(string output)
{
	vector<string> tokens;
	tokenize(output,tokens,".","+");
	for(int i=0;i<tokens.size();++i)
	{
		if( roots.find(tokens.at(i)) != roots.end() )
			return 1;
	}
	return 0;
}

int noOfDiv(string str)
{
	vector<string> tokens;
	tokenize(str,tokens,"","+");
	if(tokens.size()>1)
		return tokens.size()-1;
	else return 1;
}

int noOfDiv2(string str)
{
	vector<string> tokens;
	tokenize(str,tokens,"","+");	
	return tokens.size()-1;	
}



int checkConstraints(string output)
{
	/*
	if(!ifFoundRoot(output)) return 0;

	
	vector<string> tokens;
	tokenize(output,tokens,".","+");
	int type;
	string temp;
	for(int i=0;i<tokens.size();++i)
	{
		type=checkType(tokens.at(i));

		if(type==1) continue;
		
		if( (type==2) || (type==5) )//Suffix
		{
			if (i!=0)
			{

				if(roots.find(tokens.at(i-1)) != roots.end())
					continue;

				if(newMorphemesSuffix.find(tokens.at(i-1)) != newMorphemesSuffix.end())
					continue;
			}

			//return 0;
			
		}

		if( (type ==3) || (type==5)  )//prefix
		{
			if(i!=tokens.size()-1)
			{
				if(roots.find(tokens.at(i+1)) != roots.end())
					continue;

				temp=tokens.at(i+1);
				reverse(temp);
				if(newMorphemesPrefix.find(temp) != newMorphemesPrefix.end())
					continue;
			}
		}

		return 0;
	}
	
	return 1;
	*/

	vector<string> typeStr;
	getType(output,typeStr);
	if(typeStr.size()==1) 
	{
		if(typeStr.at(0)=="R")
		{
			if( roots.find(output) == roots.end())
				return 0;			
		}
	}
	
	return 1;	
}


void getTestCases(string fileName)
{
	ifstream f(fileName.c_str());	
	
	char temp[10000];
	string str, sub, sub1, sub2;
	string::size_type pos;     
	int len;
	int count=0;
	
	//cout<<"Getting Data From File:"<<file<<"\n";
	vector<string> tokens;
	string temp2;
  	if (f.is_open())
  	{
		//get the file data
		while (! f.eof() )
  		{				
		    f.getline(temp,10000);			
			str=temp;
			if(str=="") continue;
			len=str.length();			
			pos = str.find_first_of(" ", 0);
			sub=str.substr(0,pos);
			int freq=atoi(sub.c_str());

			if(freq<=LOW_FREQUENCY_DROPOUTS) continue;

			sub=str.substr(pos+1,len-pos-1);
			
			inputs[count++]=sub;			
		}
		
		testCount=count;
	
	}
	else exit(0);

	f.close();
}


void getTestCases2()
{
	ifstream f("goldstdsample.tur");
	//ifstream f("testFile.txt");
	//freqCheckFile.open("freqCheckFile.txt");

	vector<string> tokens;
	char temp[10000];
	string str, result;
	int count=0;
	int len;
	int i;

  	if (f.is_open())
  	{	    		
		//get the file data
		while (! f.eof() )
  		{	
    		f.getline(temp,10000);			
			str=temp;
			if(str=="") continue;
			tokens.clear();
			tokenize(str,tokens,".","\t ,\n\r");
			//if(freq<=10) continue;
			
			len=getLengthWithoutHallant(tokens[0]);
			//if(len<=8) continue;
			//if( len>8) continue;

			inputs[count]=tokens[0];
			cout<<"word:"<<tokens[0]<<":";
			
			char ch=tokens[1].at(0);
			if((ch=='0')||(ch=='1'))
				i=2;
			else i=1;

			for(;i<tokens.size();++i)
			{
				outputs[count].push_back(tokens.at(i));		
				cout<<tokens.at(i)<<", ";
				//if( noOfDiv2(tokens.at(i)) == 1)
				//	freqCheckFile<<tokens.at(0)<<" "<<tokens.at(i)<<endl;
			}
			cout<<endl;
			
			count++;
		}
		testCount=count;
	}
	else exit(0);
}

void removePlus(string str, string& temp)
{
	temp="";
	for(int i=0;i<str.size();++i)
		if(str.at(i) != '+')
			temp+=str.at(i);
}

//tempActual is the actual without plus
int getAllomorphicError(string tempActual, string tempMy, string& my)
{
	string changed;
	int i=0, j=0, k=0;		
	char ch1;
	char ch2;
	char ch3;

	string myChanged="";
	int error=0;
	int len1=tempActual.length();
	int len2=tempMy.length();

	while(i<tempActual.size()) // || (j<tempMy.size()))
	{
		ch1=tempActual.at(i);
		ch2=tempMy.at(j);
		ch3=my.at(k);

		if( (ch3=='+') )
		{
			/*
			if(ch1==ch2)
			{
				myChanged+="+";
				k++;
				continue;
			}
			else	
				//healthyness 
				//healthiness
				//health+i+ness
			if(tempActual.at(i+1) == tempMy.at(j+1))	//replacement
			{
				myChanged+="+";
				k++;
				continue;

			}
			*/
			myChanged+="+";
			k++;
		}

		if(ch1==ch2)
		{
			myChanged+=ch1;
			i++;
			j++;
			k++;
		}
		else
		{
			if((i!=len1-1) && (j!=len2-1))
			{
			if(	tempActual.at(i+1) == tempMy.at(j+1))  
			{
				//replacement Error;
				error++;
				myChanged+=ch1;
				i++;
				j++;
				k++;				
				continue;
			}
			}

			
				if(j!=len2-1)
				if( tempMy.at(j+1) == ch1)	
				//shining
				//shineing
				//shine+ing
				{
					error++;
					j++;
					k++;
					continue;
				}
			
							
				if(i!=len1-1)
				if( tempActual.at(i+1) == ch2)	
				//getting
				//geting
				//get+ing
				{
					error++;
					myChanged+=tempActual.at(i);
					i++;
					continue;
				}

				error++;
				myChanged+=ch1;
				i++;
				j++;
			

		}
	}

	my=myChanged;
	return error;

}

//get preision, recall
float getPrecisionRecall(string actual, string my, int& H, int& I, int& D, int& A)
{
	string tempActual, tempMy;
	removePlus(actual, tempActual);
	removePlus(my, tempMy);

	int allomorphicError;	

	if(tempActual==tempMy) allomorphicError=0;
	else allomorphicError=getAllomorphicError(tempActual, tempMy, my);

	int i=0, j=0, k=0;		
	char ch1;
	char ch2;
	char ch3;	
	H=0, D=0, I=0;
	while(i<actual.size())
	{
			ch1=actual.at(i);
			ch2=my.at(j);

			if(ch1==ch2)
			{
				if(ch1=='+')
					H+=1;
				i++;
				j++;
			}

			else 
			{
				if(ch1=='+')
				//sing+er =actual
				//singer
				{
					D++;
					i++;
				}

				else if(ch2=='+')
				//singer
				//sing+er
				{
					I++;
					j++;
				}

			}

	}
	A=allomorphicError;

	//cout<<"H:I:D:"<<H<<" "<<I<<" "<<D<<endl;
	//return (float)2*H/ (2*H+I+D+A);
	return (float)2*H/ (2*H+I+D);

}

void getProbableType(string myParse, vector<string>& typeStr)
{
	int plus=myParse.find_first_of('+');
	string first, rest;

	//if no plus found
	if(plus==string::npos)
	{
		first=myParse;
		if(roots.find(first) != roots.end())
			typeStr.push_back("R");
		if(newMorphemesSuffix.find(first) != newMorphemesSuffix.end())
			typeStr.push_back("S");
		reverse(first);
		if(newMorphemesPrefix.find(first) != newMorphemesPrefix.end())
			typeStr.push_back("P");
		
		if(typeStr.size()==0) typeStr.push_back("R");

		return;
	}

	else
	{
		first=myParse.substr(0,plus);
		rest=myParse.substr(plus+1);
	}

	vector<string> restTypeStr;
	getProbableType(rest, restTypeStr);
	
	vector<string> firstTypeStr;
	if(roots.find(first) != roots.end())
		firstTypeStr.push_back("R");
	if(newMorphemesSuffix.find(first) != newMorphemesSuffix.end())
		firstTypeStr.push_back("S");
	reverse(first);
	if(newMorphemesPrefix.find(first) != newMorphemesPrefix.end())
		firstTypeStr.push_back("P");
		
	for(int i=0;i<firstTypeStr.size();++i)
		for(int j=0;j<restTypeStr.size();++j)
			typeStr.push_back(firstTypeStr.at(i)+"+"+restTypeStr.at(j));
	
}

int checkConstraints3(string typeStr)
{
	vector<string> tokens;
	tokenize(typeStr,tokens,"","+");

	string temp;
	for(int i=0;i<tokens.size();++i)
	{		
		if(tokens.at(i)=="R") continue;
		
		else if(tokens.at(i)=="S")	//Suffix
		{
			if (i!=0)
			{

				if(tokens.at(i-1) == "R")
					continue;

				if(tokens.at(i-1) == "S")
					continue;
			}

			//return 0;			
		}

		else if(tokens.at(i)=="P")	//Suffix
		{
			if ( i!=(tokens.size()-1) )
			{
				if(tokens.at(i+1) == "R")
					continue;

				if(tokens.at(i+1) == "P")
					continue;
			}

			//return 0;
			
		}

		return 0;
	}	
	return 1;
}

int ifFoundRoot3(string output)
{
	vector<string> tokens;
	tokenize(output,tokens,".","+");
	for(int i=0;i<tokens.size();++i)
	{
		if(tokens.at(i)=="R")
			return 1;
	}
	return 0;
}



// type of call+ing = R+S
void getType(string myParse, vector<string>& typeStr)
{
	vector<string> probableTypeStr;
	getProbableType(myParse, probableTypeStr);
	
	typeStr.clear();
	typeFile<<myParse<<":";
	for(int i=0;i<probableTypeStr.size();++i)
	{

		//typeFile<<"Probable:"<<probableTypeStr.at(i)<<" ";
		if(ifFoundRoot3(probableTypeStr.at(i)));
		else continue;

		if(checkConstraints3(probableTypeStr.at(i)))
		{
			typeStr.push_back(probableTypeStr.at(i));
			typeFile<<"Probable:"<<probableTypeStr.at(i)<<" ";
		}
	}
	
	if(typeStr.size()==0) typeStr.push_back("R");

	typeFile<<endl;
}

int zeroCount(string str)
{
	for(int i=0;i<str.length();++i)
		if(str.at(i)=='+')
			return 0;
	//if no plus
	return 1;
}

int getScoreOfSegmentation(string myParse, string typeStr)
{
	vector<string> tokens;
	tokenize(myParse, tokens, "", "+");
	
	vector<string> types;
	tokenize(typeStr, types, "", "+");

	if(types.size() != tokens.size()) 
	{
		cout<<"Problem:";
		return 0;
	}

	//if no plus
	if(tokens.size()==1) return 1;	

	int count=0;
	int len;
	string temp;
	for(int i=0;i<tokens.size();++i)
	{
		len=getLengthWithoutHallant(tokens.at(i));
		if(types.at(i) == "R")
		{
			if(i==0)
				count+=actualMorphemeCountOfRoot1[tokens.at(i)] * len;		
			else if (i == (tokens.size()-1) )
				count+=actualMorphemeCountOfRoot2[tokens.at(i)] * len;
			else count+= ( actualMorphemeCountOfRoot1[tokens.at(i)]+
					actualMorphemeCountOfRoot2[tokens.at(i)] ) * len;
		}
		else if(types.at(i) == "P")
		{	
			temp=tokens.at(i);
			reverse(temp);
			count+= countStringPrefix[temp];		
		}
		else if(types.at(i) == "S")
		{				
			count+= countStringSuffix[tokens.at(i)];		
		}
	}

	return count;
}

int getScoreOfSegmentation(string myParse, vector<string> typeStr)
{
	int max=0, count=0;
	for(int i=0;i<typeStr.size();++i)
	{
		count=getScoreOfSegmentation(myParse, typeStr.at(i));
		if(count>max)
		{
			max=count;
		}
	}
	return max;
}

int wordPartsInVocabulary(string myParse, string typeStr)
{
	vector<string> tokens;
	tokenize(myParse, tokens, "", "+");
	
	vector<string> types;
	tokenize(typeStr, types, "", "+");

	if(types.size() != tokens.size()) 
	{
		cout<<"Problem:";
		return 0;
	}

	//if no plus
	if(tokens.size()==1) return 1;	

	int len;
	string temp;
	int index;
	for(int i=0;i<tokens.size();++i)
	{		
		if(types.at(i) == "R")
		{
			if(i==0)
			{
				if(types.at(i+1) == "R") continue;
				if(types.at(i+1) == "P") continue;
				
				temp=tokens.at(i)+tokens.at(i+1);

				//check R+S is ok or not
				index=newMorphemesSuffix[tokens.at(i+1)];

				//if not found in the vocabulary
				//if(vocabulary.find(temp) == vocabulary.end())
				if(relationsSuffix[index].find(tokens.at(i)) == relationsSuffix[index].end())
					return 0;
			}
			else if (i == (tokens.size()-1) )
			{
				if(types.at(i-1) == "R") continue;
				if(types.at(i-1) == "S") continue;

				//check P+R is ok or not
				temp=tokens.at(i-1);
				reverse(temp);
				index=newMorphemesPrefix[temp];

				//if not found in the vocabulary
				//temp=tokens.at(i-1)+tokens.at(i);
				//if(vocabulary.find(temp) == vocabulary.end())

				temp=tokens.at(i);
				reverse(temp);
				if(relationsPrefix[index].find(temp)==relationsPrefix[index].end())
					return 0;
			}
			else 
			{
				if(types.at(i-1) == "R") continue;
				if(types.at(i-1) == "S") continue;

				//prev one is prefix
				//check for P+R

				//temp=tokens.at(i-1)+tokens.at(i);
				//if(vocabulary.find(temp) != vocabulary.end()) continue;

				temp=tokens.at(i-1);
				reverse(temp);
				index=newMorphemesPrefix[temp];
				temp=tokens.at(i);
				reverse(temp);
				if(relationsPrefix[index].find(temp)!=relationsPrefix[index].end())
					continue;

				
				if(types.at(i+1) == "R") continue;
				if(types.at(i+1) == "P") continue;

				//nex one is suffix
				//check for R+S

				//temp=tokens.at(i)+tokens.at(i+1);
				//if(vocabulary.find(temp) == vocabulary.end()) return 0;
				
				index=newMorphemesSuffix[tokens.at(i+1)];
				if(relationsSuffix[index].find(tokens.at(i)) == relationsSuffix[index].end())
					return 0;
			}
		}
		

		/*
		if(types.at(i) == "S")
		{
			if(types.at(i-1) == "R")
			{
				temp=tokens.at(i-1)+tokens.at(i);
				if(vocabulary.find(temp) == vocabulary.end())
					return 0;
			}
		}

		if(types.at(i) == "P")
		{
			if(types.at(i+1) == "R")
			{
				temp=tokens.at(i)+tokens.at(i+1);
				if(vocabulary.find(temp) == vocabulary.end())
					return 0;
			}
		}
		*/
	}

	return 1;
}

int wordPartsInVocabulary(string myParse, vector<string> typeStr)
{
	for(int i=0;i<typeStr.size();++i)
	{
		if(wordPartsInVocabulary(myParse, typeStr.at(i))) return 1;
	}
	return 0;
}

//v1 is myOutput
int compare(vector<string> v1, vector<string> v2, int& closest, int& H, int& I, int& D, int& A)
{
	//select the smallest one
	int min=10;
	int current;	
	for(int i=0;i<v1.size();++i)
	{
		current=noOfDiv(v1.at(i));
		if(zeroCount(v1.at(i)))
			current=0;
		if(current<min)
		{
			min=current;
		}
	}

	//copy the shortest one to v3
	vector<string> v3;
	for(int i=0;i<v1.size();++i)
	{
		current=noOfDiv(v1.at(i));
		if(zeroCount(v1.at(i)))
			current=0;
		if( current== min)
			v3.push_back(v1.at(i));
	}

	
	testMultipleOutputFile<<"My output:";
	for(int i=0;i<v3.size();++i)
		testMultipleOutputFile<<v3.at(i)<<", ";
	testMultipleOutputFile<<endl;

	testMultipleOutputFile<<"Actual output:";
	for(int i=0;i<v2.size();++i)
		testMultipleOutputFile<<v2.at(i)<<", ";
	testMultipleOutputFile<<endl;	

	vector<string> typeStr;
	string final="";
	int score,maxScore=0;
	
	for(int i=0;i<v3.size();++i)
	{		
		typeStr.clear();
		getType(v3.at(i),typeStr);
		
		//see whether R+S or P+R is possible in the vocabulary
		//if(!wordPartsInVocabulary(v3.at(i), typeStr)) continue;
		
		//see the morpheme Score*length score
		score = getScoreOfSegmentation(v3.at(i),typeStr);
		testMultipleOutputFile<<v3.at(i)<<" "<<score<<endl;
		if(score>maxScore)
		{
			maxScore=score;
			final=v3.at(i);
		}
	
	}
	
	if(maxScore==0)
	{
		for(int i=0;i<v3.size();++i)
		{
			typeStr.clear();
			getType(v3.at(i),typeStr);
		
			//see the morpheme Score*length score
			score = getScoreOfSegmentation(v3.at(i),typeStr);
			if(score>maxScore)
			{
				maxScore=score;
				final=v3.at(i);
			}
		}
	}

	if(maxScore==0) final=v3.at(0);

	testMultipleOutputFile<<"my Selected:"<<final<<" max score:"<<maxScore<<endl;
	vector<string> v4;
	v4.clear();
	v4.push_back(final);

	//v1 is myOutput
	int bestH=10, bestI=10, bestD=10, bestA=10;
	float ratio, bestRatio=-1;
	int besti, bestj;
	for(int i=0;i<v4.size();++i)
	{		
		for(int j=0;j<v2.size();++j)
		{
			//check whether the output has too many or too few characters
			//if(tooBig(v2.at(j), input)) continue;

			if( v4.at(i)==v2.at(j) )
			{
				H=noOfDiv(v4.at(i));
				I=0;
				D=0;
				A=0;
				testMultipleOutputFile<<"correct"<<endl;
				return 1;
			}
		}
	}
	
	string str1,str2;
	for(int i=0;i<v4.size();++i)
	{
		for(int j=0;j<v2.size();++j)
		{			
				str1=v2.at(j);
				str2=v4.at(i);
				ratio=getPrecisionRecall(v2.at(j), v4.at(i), H, I, D, A);
				if(ratio>=bestRatio)
				{
					if(H==0)
						//if( (I+D+A) > (bestI+bestD+bestA)) continue;
						if( (I+D) > (bestI+bestD)) continue;

					bestRatio=ratio;
					bestH=H;
					bestI=I;
					bestD=D;
					bestA=A;
					besti=i;
					bestj=j;
				}
				
		}
	}

	
	H=bestH;
	I=bestI;
	D=bestD;
	A=bestA;

	if(bestRatio==1) 
	{
		cout<<"Correct special"<<endl;
		return 1;
		
	}

	cout<<" Actual:"<<v2.at(bestj)<<" My:"<<v4.at(besti)<<" H:"<<bestH<<" I:"<<bestI<<" D:"<<bestD<<" A:"<<bestA<<endl;

	return 0;
}


//v1 is set of possible segmentations. We select the most optimal one according to some criterions.
string getOptimalSegmentation(vector<string> v1)
{
	//select the smallest one
	int min=10;
	int current;	
	for(int i=0;i<v1.size();++i)
	{
		current=noOfDiv(v1.at(i));
		if(zeroCount(v1.at(i)))
			current=0;
		if(current<min)
		{
			min=current;
		}
	}

	//copy the shortest one to v3
	vector<string> v3;
	for(int i=0;i<v1.size();++i)
	{
		current=noOfDiv(v1.at(i));
		if(zeroCount(v1.at(i)))
			current=0;
		if( current== min)
			v3.push_back(v1.at(i));
	}

	
	testMultipleOutputFile<<"My output:";
	for(int i=0;i<v3.size();++i)
		testMultipleOutputFile<<v3.at(i)<<", ";
	testMultipleOutputFile<<endl;

	testMultipleOutputFile<<"Actual output:";
	for(int i=0;i<v3.size();++i)
		testMultipleOutputFile<<v3.at(i)<<", ";
	testMultipleOutputFile<<endl;	

	vector<string> typeStr;
	string final="";
	int score,maxScore=0;
	
	for(int i=0;i<v3.size();++i)
	{		
		typeStr.clear();
		getType(v3.at(i),typeStr);
		
		//see whether R+S or P+R is possible in the vocabulary
		//if(!wordPartsInVocabulary(v3.at(i), typeStr)) continue;
		
		//see the morpheme Score*length score
		score = getScoreOfSegmentation(v3.at(i),typeStr);
		testMultipleOutputFile<<v3.at(i)<<" "<<score<<endl;
		if(score>maxScore)
		{
			maxScore=score;
			final=v3.at(i);
		}
	
	}
	
	if(maxScore==0)
	{
		for(int i=0;i<v3.size();++i)
		{
			typeStr.clear();
			getType(v3.at(i),typeStr);
		
			//see the morpheme Score*length score
			score = getScoreOfSegmentation(v3.at(i),typeStr);
			if(score>maxScore)
			{
				maxScore=score;
				final=v3.at(i);
			}
		}
	}

	if(maxScore==0) final=v3.at(0);

	testMultipleOutputFile<<"my Selected:"<<final<<" max score:"<<maxScore<<endl;
	return final;
}


void printVector(vector<string> v)
{
	for(int i=0;i<v.size();++i)
	{
		cout<<v.at(i)<<" ";
	}
	cout<<endl;
}

//ch1=u	ch2=V
int compare(char ch1, char ch2)
{
	if(ch2=='V')
		return ifVowel(ch1);
	if(ch2=='C')
		return !ifVowel(ch1);
	else return ch1==ch2;
}


void checkConditionalSimilarSuffixes(string& str)
{
	vector<string> tokens;
	tokenize(str, tokens,"","+");
	
	char ch1,ch2;
	int len;

	string newStr=tokens.at(0);
	for(int i=1;i<tokens.size();++i)
	{
	     //token(i-1)=take	token(i)=r
	     if( conditionalSimilarSuffixes.find(tokens.at(i)) != conditionalSimilarSuffixes.end())
		{
			len=tokens.at(i-1).length();
			ch1=tokens.at(i-1).at(len-1);	//get the last char
			//ch2=conditionalSimilarSuffixes[tokens.at(i)].at(0);	//get e from er
			ch2=conditionOfSimilarSuffix[tokens.at(i)];		//get e
			if(compare(ch1,ch2) )
			{
				newStr+="+"+conditionalSimilarSuffixes[tokens.at(i)];
			}
			else newStr+="+"+tokens.at(i);
		}
		else newStr+="+"+tokens.at(i);
	}

	str=newStr;
}

void testOnGivenCase()
{
	while(true)
	{
	cout<<"Give Test Case:";
	string input;
	cin>>input;
	//input="ring";

	vector<string> options;
	getDivisions(input,options);

	int flag=0;
	if(options.size()==0) cout<<input<<"\n";
	else 
	{
		for(int v=0;v<options.size();++v)
		{			
			string str=options.at(v);
			checkConditionalSimilarSuffixes(str);
			if(checkConstraints(str))
			{
				flag=1;
				cout<<endl;
				cout<<str;

			}
		}
		if(flag==0)
			 cout<<input<<"\n";

	}
	}
}

void getDivisionsOfCompositeRoot(string str, vector<string>& options)
{
		int len=str.length();
		
		string sub,rest,outputStr;		
		vector<string> newOptions;

		for(int i=0;i<len;++i)
		{
			newOptions.clear();
			sub=str.substr(0,i+1);
			
			if( vocabulary.find(sub)!=vocabulary.end() )
			{				
				if(i!=(len-1))
				{
					rest=str.substr(i+1,len-i-1);
					getDivisionsOfCompositeRoot(rest, newOptions);

					if(newOptions.size()>0)
					{						
						for(int v=0;v<newOptions.size();++v)
						{
							outputStr=sub+"+";
							outputStr+=newOptions.at(v);
							options.push_back(outputStr);
						}
						
					}
				}
				else 
				{					
					options.push_back(str);
				}
				
			}
		}
}


void seeMultipleRoots()
{
	Map::const_iterator pos;
	vector<string> options;
	cout<<"Printing Composite Roots"<<endl;
	for(pos=vocabulary.begin();pos!=vocabulary.end();++pos)
	{
		options.clear();
		getDivisionsOfCompositeRoot(pos->first,options);
		if(options.size() > 1)
			cout<<pos->first<<endl;
	}
}


void checkFreqDistribution()
{	
	ifstream checkFile("freqCheckFinal.txt");	
	char temp[1000];
	
	string str;
	vector<string> tokens;
	int i;
	int pos;
	string whole, sub;
	int count1=0, count2=0;
	int correct1=0, correct2=0;

	if (checkFile.is_open())
  	{
		//get the file data
		while (! checkFile.eof() )
  		{
	    		checkFile.getline(temp,10000);
			str=temp;
			if(str=="") continue;
			tokens.clear();
			tokenize(str,tokens,""," \r\n");

			char ch=tokens[1].at(0);
			if(ch=='1')
				i=2;
			else i=1;

			whole=tokens.at(0);			
			
			if(i==2)
			{
				pos=tokens.at(i).find_first_of('+');
				sub=tokens.at(i).substr(pos+1);
				
				if(vocabulary.find(sub) == vocabulary.end()) continue;
				
				
				if(vocabulary[whole] <= vocabulary[sub])
				{
					correct2++;
					cout<<whole<<":"<<sub<<" = "<<vocabulary[whole]<<" "<<vocabulary[sub]<<" ratio="<<(float)vocabulary[whole]/vocabulary[sub]<<endl;
				}
				else cout<<"Special: "<<whole<<":"<<sub<<" = "<<vocabulary[whole]<<" "<<vocabulary[sub]<<" ratio="<<(float)vocabulary[whole]/vocabulary[sub]<<endl;

				count2++;
			}
			else 
			{
				pos=tokens.at(i).find_first_of('+');
				sub=tokens.at(i).substr(0,pos);	

				if(vocabulary.find(sub) == vocabulary.end()) continue;

				count1++;
				
				if(vocabulary[whole] <= vocabulary[sub])
				{
					cout<<whole<<":"<<sub<<" = "<<vocabulary[whole]<<" "<<vocabulary[sub]<<" ratio="<<(float)vocabulary[whole]/vocabulary[sub]<<endl;
					correct1++;
				}
				else cout<<"Special: "<<whole<<":"<<sub<<" = "<<vocabulary[whole]<<" "<<vocabulary[sub]<<" ratio="<<(float)vocabulary[whole]/vocabulary[sub]<<endl;
			}
			
			
		}
	}
	
	cout<<"Total Freq Check:"<<(count1+count2)<<" Percentage:"<<(float)(correct1+correct2)/(count1+count2)<<endl;
	

}

int max(int a, int b)
{
	if (a>=b) return a;
	else return b;
}


int main(int argc,char *argv[])
{	
	Trie ob;	
	
	if(argc<=6)
	{
		cout<<"Arguments Missing: \nRight syntax: ./a.out inputFileName 1 1 0 1 0"<<endl;
		cout<<"where inputFileName contains the list of words (i.e., VOCABULARY) of a particular language, and"<<
			" the following five arguments define the algorithmic steps user wants to run (1/0 to On/Off).\n"<<endl;
		return 0;
	}
	
	int step1OnOff, step2OnOff, step3OnOff, step4OnOff, step5OnOff;
	string vocabularyFile = argv[1];
	
	step1OnOff=atoi(argv[2]);
	step2OnOff=atoi(argv[3]);
	step3OnOff=atoi(argv[4]);
	step4OnOff=atoi(argv[5]);
	step5OnOff=atoi(argv[6]);
	
	ob.setSteps( step2OnOff, step3OnOff, step4OnOff, step5OnOff );
	
	
		
	cout<<"\nSegmentation in progress (Look for Output/finalSegmentation.txt for final segmentation when the program ends) ...............\n";
	
	//Printing the STDOUT to a file.
	freopen ( "Output/consoleOut.txt", "w", stdout );
	
	
		
	/////////////////////////////////////////////////////////////////////
	//Get the input vocabulary from a file. This is the only input we require.
		
	//GET the vocabulary from a text file. We remove words of length 2 or less and also words of freq 1.	
	getVocabulary(vocabularyFile);
	//ob.printVocabulary();
	//ob.printAllIntoFile();

	int s=vocabulary.size()+smallRoots.size();	//+lowFreqWords.size();
	cout<<"Vocabulary size:"<<s<<endl;
		
	//seeMultipleRoots();
	//checkFreqDistribution();
	//Print the high frequency words.	
	getHighFreqWords();
	
	
	
	/////////////////////////////////////////////////////////////////////
	//Set the tunable parameters here.
		
	//Important thresholds that are used to prune the induced prefix/suffix list.
	//First one is for suffix, second one is for prefix.
	//These two thresholds are vocabulary-dependent
	//s -- denotes the vocabulary size
	//60000 is used to normalize the vocabulary.
	int thresholdSuffix=floor((float)(s*SUFFIX_CUTOFF_THRESHOLD)/60000);
	int thresholdPrefix=floor((float)(s*PREFIX_CUTOFF_THRESHOLD)/60000);
	ob.setThreshold(thresholdSuffix, thresholdPrefix);
	//cout << "\nCutoff threshold for suffix:" << thresholdSuffix << endl;
	//cout << "Cutoff threshold for prefix:" << thresholdPrefix << endl;
		
	
	//vocabulary dependent thresholds for allomorph (morphological change) induction
	int allomorph_replacement_threshold = max( (float)(s*ALLOMORPH_REPLACEMENT_THRESHOLD)/60000, ALLOMORPH_REPLACEMENT_THRESHOLD);
	int allomorph_deletion_threshold = max( (float)(s*ALLOMORPH_DELETION_THRESHOLD)/60000, ALLOMORPH_DELETION_THRESHOLD);
	int allomorph_addition_threshold = max( (float)(s*ALLOMORPH_ADDITION_THRESHOLD)/60000, ALLOMORPH_ADDITION_THRESHOLD);
	
	ob.setAllomorphThreshold( allomorph_replacement_threshold, allomorph_deletion_threshold,  allomorph_addition_threshold);
	//cout<<"\nThresholds for Allomorphs:"<<allomorph_replacement_threshold<<" "<<allomorph_deletion_threshold<<" "<<allomorph_addition_threshold<<endl;	


	
	
	/////////////////////////////////////////////////////////////////////
	//Run the segmentation algorithm step by step.
	
	//The main function to learn the segmentation. All the steps are defined inside this function.
	ob.divideAndConquer();
		
		
	
	/////////////////////////////////////////////////////////////////////
	//Apply the algorithm on a set of test cases.
		
	//Get the test cases
	//In this case, we segment each word in the initial vocabulary file
	getTestCases(vocabularyFile);
	
	int x,correct=0;
	cout<<"testCount:"<<testCount;
	int closest;
	int H,I,D,A, totalH=0,totalD=0,totalI=0, totalA=0;	

	testMultipleOutputFile.open("Output/testMultipleOutput.txt");
	typeFile.open("Output/typeFile4.txt");
	cout<<"\n\n\nSegmenting each word in the vocabulary"<<endl;
	
	
	vector<int> minArray[50];	// say there can be max 50 divisions
	int index, j, k;
	
	for(int i=0;i<testCount;++i)
	{		
		vector<string> options;
		vector<string> finalOptions;
		
		//cout<<"\nDividing:"<<inputs[i];
		getDivisions(inputs[i],options);

		int flag=0;
		
		if(options.size()==0) 
		{
			finalOptions.push_back(inputs[i]);
		}
		else 
		{		
			for(k=0;k<50;++k)
				minArray[k].clear();

			int current;
			for(j=0;j<options.size();++j)
			{
				current=noOfDiv(options.at(j));
				if(zeroCount(options.at(j)))
					current=0;
				minArray[current].push_back(j);
			}

			//starting from the minimum divisions check whether it satisfies normal Suffixal or Prefixal conditions
			for(k=0;k<50;++k)
			{
				if(minArray[k].size()==0) continue;

				for(j=0;j<minArray[k].size();++j)
				{
					index=minArray[k].at(j);
					string str=options.at(index);
					checkConditionalSimilarSuffixes(str);
				
					if(checkConstraints(str))
					{
						flag=1;					
						finalOptions.push_back(str);
						break;
					}
					
				}
				if(flag) break;
			}

			//see whether any other of the divisions (which have min seg points) are ok
			if(flag)
			for(int x=j+1;x<minArray[k].size();++x)
			{
					index=minArray[k].at(x);
					string str=options.at(index);
					checkConditionalSimilarSuffixes(str);
				
					if(checkConstraints(str))
					{
						finalOptions.push_back(str);
					}
			}
		
			if(flag==0)
				 finalOptions.push_back(inputs[i]);

		}

		cout<<inputs[i]<<":";
		testMultipleOutputFile<<inputs[i]<<":"<<endl;
		testMultipleOutputFile<<"================"<<endl;

		string final=getOptimalSegmentation(finalOptions);
		
		cout<<final<<endl;
		
		finalOutputFile<<inputs[i]<<":"<<final<<endl;
		
	}

	cout<<"\n\nProgram Ends Successfully"<<endl;
		
	return 0;
}

