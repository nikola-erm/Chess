import os

projectName = 'chess'
cacheDir = '.build'

srcDep = dict()
hdrDep = dict()

def getName(d, f):
	return (d + '/' + f)[2:].replace('\\', '/')

print('Scaning files')

for dirName, subdirList, fileList in os.walk('.'):
    for f in fileList:
    	fname, ext = os.path.splitext(f)
    	dep = None
    	if ext == '.cpp':
    		dep = srcDep
    	elif ext == '.h':
    		dep = hdrDep
    	if dep is not None:
    		me = getName(dirName, f)
    		dep[me] = set()
    		with open(dirName + '/' + f) as lines:
    			for line in lines:
    				if line.startswith('#include'):
    					he = line[8:].strip()
    					if he[0] == '"':
    						dep[me].add(getName(dirName, he[1:-1]))
    					else:
    						dep[me].add(he[1:-1])
    		
    	             
print('Generating Makefile')

with open('Makefile', 'w') as out:
	out.write('# This file is auto generated\n\n')
	out.write('FLAGS=-c -O2 -Wall -std=c++17 -I.\n\n')

	out.write('all:')
	for k in srcDep:
		out.write(' ' + cacheDir + '/' + k[:-4].replace('/', '__') + '.o')
	out.write('\n\tg++ -o ' + projectName + '.exe')
	for k in srcDep:
		out.write(' ' + cacheDir + '/' + k[:-4].replace('/', '__') + '.o')
	out.write('\n\n')

	for k, v in hdrDep.items():
		out.write('.INTERMEDIATE: ' + cacheDir + '/' + k + '\n\n')
		out.write(cacheDir + '/' + k + ': ' + k)
		for x in v:
			if x in hdrDep:
				out.write(' ' + x)
		out.write('\n\n')

	for k, v in srcDep.items():
		out.write(cacheDir + '/' + k[:-4].replace('/', '__') + '.o: ' + k)
		for x in v:
			if x in hdrDep:
				out.write(' ' + cacheDir + '/' + x)
		out.write('\n\tg++ $(FLAGS) ' + k + ' -o ' + cacheDir + '/' + k[:-4].replace('/', '__') + '.o\n\n')

	out.close()

print('Done')