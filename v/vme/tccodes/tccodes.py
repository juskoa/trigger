#!/usr/bin/python
import xlrd,types,sys
class TCCodes:
  def __init__(self, fn="TCCodes.xls"):
    self.book = xlrd.open_workbook(fn)
    print "The number of worksheets:", self.book.nsheets, \
      " Worksheet name(s):", self.book.sheet_names()
    self.sh = self.book.sheet_by_index(0)
    print "Sheet:", self.sh.name, "nrows ncols:",self.sh.nrows, self.sh.ncols
    print "Cell 1B:", self.sh.cell_value(rowx=0, colx=1), \
      "1D:", self.sh.cell_value(rowx=0, colx=3)
    self.l0inputs={}; self.l1inputs={}
    self.l0functions={}
    self.descriptors={}; self.classcodes={}
    rx=62
    rx= self.findrow(rx,'Level 0 Input Codes') 
    if rx != -1: rx= self.get_l0l1des(rx+3, self.l0inputs, 3, 2)
    rx= self.findrow(rx,'Level 1 Input Codes') 
    if rx != -1: rx= self.get_l0l1des(rx+2, self.l1inputs, 3, 2)
    print "# of L0/L1 inputs:", len(self.l0inputs), len(self.l1inputs)
    rx= self.findrow(rx,'L0 Functions') 
    if rx != -1: rx= self.get_l0l1des(rx+3, self.l0functions, 1, 2)
    print "# of l0functions:", len(self.l0functions)
    rx= self.findrow(rx,'Descriptor Codes') 
    if rx != -1: rx= self.get_l0l1des(rx+3, self.descriptors, 1, 3)
    print "# of descriptors:", len(self.descriptors)
    rx= self.findrow(rx,'Active Trigger Class Codes') 
    if rx != -1: rx= self.get_l0l1des(rx+3, self.classcodes, 1, 2)
    print "# of class codes:", len(self.classcodes)
    #if type(ch) is types.StringType:
    #for i in range(3):
    #  print i,':',type(self.sh.cell_value(rx, i))
  def getv(self, rowx, colx):
    return(self.sh.cell_value(rowx, colx))
  def findrow(self, rx, text):
    """ search from row rx till the end 
    """
    while 1:
      if self.getv(rx,1)==text: return rx
      rx= rx+1
      if rx>=self.sh.nrows: return -1
  def get_l0l1des(self,fromrx, dict, ixkey, ixvalue):
    """Operation:
    - take rows from fromrx (ixkey and ixvalue columns)
    - dict[ixkey]= row[ixvalue]
    RC:
    index pointing to row where row[ixkey]=='' or after last row
    """
    rx= fromrx-1; #ii=-1
    while(1):
      rx=rx+1; #ii=ii+1
      if rx>= self.sh.nrows: return rx
      #if rx> 402: print "rx:",rx
      r= self.sh.cell_value(rx,ixkey)
      #if self.sh.cell(rx,0) is xlrd.empty_cell:
      if r=='':
        #print 'empty cell'
        return rx
      # following valid for L0/1 inputs:
      #if type(r) != types.FloatType:
      #  self.errExit("r[1] is not float",rx)
      #  continue
      #if int(r) != ii: self.errExit("%d expected"%ii,rx)
      #print r,":", self.sh.cell_value(rx,ixvalue)
      dict[r]= self.sh.cell_value(rx,ixvalue)
  def errExit(self,text, rx):
    print "Error:",text, self.sh.row(rx)
    sys.exit(8)
# Refer to docs for more details.
# Feedback on API is welcomed.
def main():
  tcc= TCCodes()
if __name__== "__main__": main()

