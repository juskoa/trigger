arr = []
diff = []
for line in open("logkuboZdelay",'r'):
    if line.find("A chanel") > -1:
        info = line.split()
        var1 = info[0]
        arr.append(var1)

arr = map(int, arr)
for i in range(len(arr)-1):
  diff.append(arr[i+1]-arr[i])
  if(arr[i+1]-arr[i]) == 1:
    print i

diff = map(int,diff)

cout = 0
cout1 = 0
err = 0
for elem in diff:
  if elem == 32:
    cout = cout+1
  elif elem == 31:
    cout1 = cout1+1
  elif elem == 1:
    cout1 = cout1 + 1
  else:
    err = err +1

print "L0: ",cout
print "L1: ", cout1/2
print "Err: ", err
