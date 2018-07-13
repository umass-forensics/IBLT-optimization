import random
from pyblt import PYBLT
import math 

def basic_test():
  PYBLT.set_parameter_filename('param.export.0.995833333333333.2018-07-12.csv')

  f=PYBLT(10,9)
  f.insert(8,"txn_10008")
  f.insert(2, "txn_1002")
  # print(f.dump_table())
  print("F Entries: ")
  success, entries = f.list_entries()
  print(success)
  for x in entries:
    print(x,entries[x])


  g=PYBLT(10,9)  
  g.insert(8,"txn_10008")
  g.insert(7,"txn_10010")
  print("G Entries: ")
  success, entries = g.list_entries()
  print(success)
  for x in entries:
    print(x,entries[x])



  h=g.subtract(f)
  print("\nh=g-f;  should have 7, and then 2:")

  success, entries = h.list_entries()
  print(success)
  for x in entries:
    print(x,entries[x])
  del(h)

  h=f.subtract(g)

  print("\nh=f-g;  should have 2, and then 7:")
  success, entries = h.list_entries()
  print(success)
  for x in entries:
    print(x,entries[x])
  del(h)

basic_test()

