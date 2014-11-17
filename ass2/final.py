# -*- coding: utf-8 -*-
from __future__ import unicode_literals
import unicodedata
import re
import codecs
import  operator
import locale
import sys
L=132
def product(nums):
    return reduce(operator.mul, nums, 1)
def memo(f):
    table = {}
    def fmemo(*args):
        if args not in table:
            table[args] = f(*args)
        return table[args]
    fmemo.memo = table
    return fmemo
def splits(text,L=132):
     return [(text[:i+1], text[i+1:]) 
            for i in range(min(len(text), L))]
@memo
def segment(text):
    if not text: return []
    candidates = ([first]+segment(rem) for first,rem in splits(text))
    return max(candidates, key=Pwords)
def Pwords(words): 
    return product(Pw(w) for w in words)
class Pdist(dict):
    def __init__(self, data=[], N=None, missingfn=None):
        for count,key in data:
	    key=key.strip()
            self[key] = self.get(key, 0) + int(count)
        self.N = float(N or sum(self.itervalues()))
        self.missingfn = missingfn or (lambda k, N: 1./N)
    def __call__(self, key): 
        if key in self: return self[key]/self.N  
        else: return self.missingfn(key, self.N)

def datafile(name, sep=' '):
    wordlist= codecs.open(name, "r", "utf-8")
    vocab=wordlist.readlines()
    for line in vocab:
        yield line.split(sep)
    

def avoid_long_words(key, N):
    return 10./(N * 10**len(key))
N = 5726784
Pw  = Pdist(datafile('filwords.txt'), N, avoid_long_words)
s="इसबीचआईआईटीपरिषदकीबैठकतीनसितंबरकोहोनीहै"
sys.stdout = codecs.getwriter(locale.getpreferredencoding())(sys.stdout)
lines = tuple(codecs.open("hi-wordseg.txt", 'r',"utf-8"))
for g in lines:
	#s= s.encode('utf-8')
	g=g.strip()
	result=segment(g)
	d=''
	for a in result :
	   d=d+a+' '
	print d
