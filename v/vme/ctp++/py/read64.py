import math
channelA = []
channelB = []
channelBm = []
ChAL0 =[]
ChAL1 = []

ChBL1 = []
ChBDa = []

missing = []
for line in open("logData64_2",'r'):
    if line.find("A chanel") > -1:
        info = line.split()
        var1 = info[0]
        channelA.append(var1)
    if line.find("B chanel") > -1:
        info = line.split()
        var1 = info[0]
        channelB.append(var1)
        channelBm.append(info[4][3:])
        print line

channelA = map(int, channelA)
channelB = map(int, channelB)

for i in range(len(channelA)-1):
    diff = channelA[i+1]-channelA[i]
    if(diff == 1):
        ChAL1.append(channelA[i])
    elif(diff == 128):
        ChAL0.append(channelA[i])
    elif(diff == 127):
        pass
    else:
        print "Unknown step!", diff

for i in range(len(channelBm)):
    if(1):
        ChBL1.append(channelB[i])
    elif(channelBm[i] == "010101000011111111"):
        ChBDa.append(channelB[i])
    else:
        print "Unknown CMD!", channelB[i]

print "first L0 BC: ", ChAL0[0]
err = 0
for i in range(len(ChAL0)-1):
    if(ChAL0[i+1]-ChAL0[i] == 256):
        pass
    else:
        err = err + 1

if (err == 0):
    print "L0 test passed", len(ChAL0), " tests"
else:
    print "L0 test failed", len(ChAL0), " success", err , " failed"

print "first L1 BC: ", ChAL1[0]
err = 0
for i in range(len(ChAL1)-1):
    if(ChAL1[i+1]-ChAL1[i] == 256):
        pass
    else:
        err = err + 1

if (err == 0):
    print "L1 test passed", len(ChAL1), " tests"
else:
    print "L1 test failed", len(ChAL1), " success", err , " failed"

print "first L1m BC: ", ChBL1[0]
err = 0
for i in range(len(ChBL1)-1):
    if(ChBL1[i+1]-ChBL1[i] == 64):
        pass
    else:
        err = err + 1
        missing.append(ChBL1[i])
#print "Error L1m", ChBL1[i+1]-ChBL1[i], " at ", ChBL1[i]

if (err == 0):
    print "L1m test passed", len(ChBL1), " tests"
else:
    print "L1m test failed:", len(ChBL1), " test", err , " failed"

missing.sort()
missingDiff = []
for i in range(len(missing)-1):
    missingDiff.append(missing[i+1] - missing[i])
    print missingDiff[i],"at",missing[i]




#diff = map(int,diff)

#cout = 0
#err = 0
#for elem in diff:
#  if elem == 32:
#    cout = cout+1
#  else:
#    err = err+1
    

#print "L0: ",cout
#print "L1: ", err/2

