insts=set()

fi=open('rcpsp-features.csv')

for i,l in enumerate(fi):
    if i:
        insts.add(l.split(',')[0])
fi.close()
print(insts)

params=set()
fp=open('incparams-rcpsp.csv')
for l in fp:
    params.add(l.strip())
fp.close()

print(params)

fr=open('/home/haroldo/inst/algsel/rcpsp/results.csv')
fro=open('rcpsp-results.csv', 'w')
fro.write('instance,algsetting,result\n')
for l in fr:
    lc=l.strip().split(',')
    iname=lc[0]
    if iname not in insts:
        continue
    pname=lc[1]
    if pname not in params:
        continue
    fro.write(l)
fro.close()
fr.close()
