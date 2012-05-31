import sys
def info(type, value, tb):
  if hasattr(sys, 'ps1') or not sys.stderr.isatty():
    sys.__excepthook__(type, value, tb)
  else:
    import traceback, pdb
    traceback.print_exception(type,value,tb)
    print
    pdb.pm()

sys.excepthook=info

