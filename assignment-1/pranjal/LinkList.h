#include <iostream>
#include <string>


using namespace std;

class Node
{
	public:
	long double prob;
	string word;
	Node *next;	
	Node *prev;
	Node(){}
	Node(string word,long double prob)
	{
		this->word=word;
		this->prob=prob;
	}	
};

class LinkList
{
	int max;
	int currentPos;
	Node *currentPointer;
	Node *head;
	Node *tail;

	public:
	LinkList();
	LinkList(int max);		
	void setMax(int max);
	void add(string word, long double prob);
	void deleteLast();
	void clear();
	void printList();
	void setIterator();
	long double getNext(string & next);
};

LinkList::LinkList()
{	
	Node *sentinel=new Node("",10000);
	sentinel->next=NULL;
	sentinel->prev=NULL;
	
	head=sentinel;
	tail=sentinel;
	
	
	currentPos=0;
	currentPointer=NULL;
}

void LinkList::setMax(int max)
{
	this->max=max;
}

LinkList::LinkList(int max)
{
	this->max=max;

	Node *sentinel=new Node("",10000);
	sentinel->next=NULL;
	sentinel->prev=NULL;
	
	head=sentinel;
	tail=sentinel;
	
	
	currentPos=0;
	currentPointer=NULL;
}

void LinkList::add(string word, long double prob)
{
	//Node *prev=head;
	
	Node *p=head;

	while(p->next!=NULL)
	{
		if(p->next->prob < prob) break;
		else p=p->next;
	}
	
	
	//insert new element after p
	Node *newN=new Node(word, prob);
	newN->next=p->next;
	newN->prev=p;

	if(p->next==NULL) tail=p;
	else if (p->word==tail->word) tail=newN;

	if(p->next) p->next->prev=newN;
	p->next=newN;
	

	currentPos++;
	if(currentPos>max)
	{
		//cout<<"X";
		deleteLast();
	}
}

void LinkList::deleteLast()
{
	tail->next=NULL;
	tail=tail->prev;
	
	currentPos--;
}

void LinkList::printList()
{
	Node *p=head;
	while(p->next!=NULL)
	{
		cout<<"Word:"<<p->next->word<<" Prob:"<<p->next->prob<<"\n";
		p=p->next;
	}
}

void LinkList::setIterator()
{
	currentPointer=head;
}

long double LinkList::getNext(string& next)
{
	long double prob;
	if(currentPointer->next)
	{ 
		next=currentPointer->next->word;
		prob=currentPointer->next->prob;
		currentPointer=currentPointer->next;
		return prob;
	}	
	else 
	{
		return 0;
	}
}

void LinkList::clear()
{
	head->next=NULL;
	head->prev=NULL;
	tail=head;
	currentPos=0;
}
