from pyblt import *
import random
import gc
import math
import csv
from multiprocessing import Process, Lock
import os


PROCESSES = 4

def trial(hedge,k,entries):
  trials=0
  successes=0
  while (True):

    p=PYBLT(entries=entries,value_size=9,hedge=hedge,num_hashes=k)
    for _ in range(entries):
      p.insert(random.getrandbits(32), 0)
    if (p.list_entries()[0]==True):
      successes+=1
    trials+=1
    ssize=p.get_serialized_size()

    prob = successes/trials
    if successes<trials:
      ci = 1.96*math.sqrt(prob*(1-prob)/trials)
    else:
      ci = -(math.exp(math.log(.05)/trials)-1)
    if  ( ci<=(1/240)/3 ):  
      return successes,trials,prob,ci,ssize


def validate_rows(lock,m):
  count=0
  with open(sys.argv[1],'r') as fd:
    fd.readline()
    for row in csv.reader(fd, delimiter=','):
      n,hedge,keys,size, goal = row
      n=int(n)
      if (n!=m+PROCESSES*count):
        continue
      count+=1
      hedge=float(hedge)
      keys=int(keys)
      goal=float(goal)
      size=int(size)
      
      test_passed=True
      
      # first independent trial
      successes1,trials1,prob1,ci1,ssize1 = trial(hedge,keys,n)
      
      # check size of the IBLT
      if not (ssize == size):
        # this should not probabilistic and should always pass
        lock.acquire()
        print (n,"FAIL: size test")
        lock.release()
        return()
      
      # second independent trial
      successes2,trials2,prob2,ci2,ssize2 = trial(hedge,keys,n)

      # check decode rate of the IBLT
      if (not (prob1-ci1 >= goal or (goal>prob1-ci1 and goal<prob1+ci1))) or 
         (not (prob2-ci2 >= goal or (goal>prob2-ci2 and goal<prob2+ci2))):
              lock.acquire()
              print (n,"Double check decode rates for %d items:\t%f\t%f" %(n,prob1,prob2)
              lock.release()

      lock.acquire()
      print("%d, %f, %d, %d, %d, %f, %f, %d, %f, %f, %d" % (n, hedge, keys, 
             successes1, trials1, prob1,
             successes2, trials2, prob2))
      lock.release()
    return()
    
lock = Lock()

# count from 1; can't have an IBLT with 0 entries
for num in range(1,PROCESSES+1):
    Process(target=validate_rows, args=(lock, num)).start()
        
