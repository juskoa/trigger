chanelA = []
chanelB = []
chanelBm= []
chA = []
chB = []

for line in open("logIP",'r'):
    if line.find("A chanel") > -1:
        chanelA.append(line.split()[0])
    if line.find("B chanel") > -1:
        chanelB.append(line.split()[0])
        chanelBm.append(line.split()[4][5:])

chanelA = map(int, chanelA)
chanelB = map(int, chanelB)

for i in range(len(chanelA)-1):
    diff = chanelA[i+1] - chanelA[i]
    row = []
    if (diff == 1):
        row.append(chanelA[i])
        row.append("L1")
        chA.append(row)
    elif(diff == 280):
        row.append(chanelA[i])
        row.append("L0")
        chA.append(row)
    elif(diff == 119 or diff == 120):
        pass
    else:
        print "unknown step", diff, chanelA[i]

for i in range(len(chanelBm)):
    row = []
    if(chanelBm[i][:4]=="0001"):
        row.append(chanelB[i])
        row.append("Lh")
        row.append(chanelBm[i][4:])
        chB.append(row)
    if(chanelBm[i][:4]=="0010"):
        row.append(chanelB[i])
        row.append("Lm")
        row.append(chanelBm[i][4:])
        chB.append(row)
counter = 0
for item in chB:
    if (item[1]=="Lh"):
        if(counter == 6):
            pass
        else:
            print "incomplete message at ", item[0]
        counter = 0
    elif(item[1]=="Lm"):
        counter = counter + 1
chBp = []

for i in range(len(chB)-6):
    mes = []
    if(chB[i][1]=="Lh"):
        mes.append(chB[i][0])
        mes.append("L1m")
        mes.append(chB[i][2][:8]+chB[i+1][2]+chB[i+2][2])
        mes.append(chB[i+3][2])
        mes.append(chB[i+4][2][:8]+chB[i+5][2]+chB[i+6][2])
        chBp.append(mes)

out = open("out","w")
for item in chA:
    out.write(str(item[0])+" " + item[1]+"\n")

for item in chB:
    out.write(str(item[0])+" " + item[1]+" "+item[2]+"\n")
out.close()

comb = chA + chBp
comb.sort()
outB = open("outB","w")
for item in chBp:
    outB.write(str(item[0])+" " + str(int(item[2],2))+" "+ str(int(item[3],2))+" "+ str(hex(int(item[4],2)))+"\n")
outB.close()

outc = open("outc","w")
for item in comb:
    if(len(item)==2):
        outc.write(str(item[0])+" " + item[1]+"\n")
    if(len(item)>2):
        outc.write(str(item[0])+" " +item[1]+" "+ str(int(item[2],2))+" "+ str(int(item[3],2))+" "+ str(hex(int(item[4],2)))+"\n")

outc.close()

