import types
def isnumber(number,message):
  try:
    num=int(number) 
  except ValueError:
    fulmes=message+' '+number+'is not number'
    print fulmes
    #MywError(fulmes)
    return None
  else:
    return num
if __name__=='__main__':
  a=isnumber('dd','bb')
  print a
  if a!=None: print 'som true'
  else: print 'som false'
