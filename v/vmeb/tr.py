#!/usr/bin/python
def a(*args, **kwargs):
  print args
  print args[:]
  print kwargs

if __name__ == '__main__':
  a(1,2,abc=1,defg='e')

