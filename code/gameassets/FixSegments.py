from glob import glob
import sys
from os import path as pathutils

def main():
	objfiles = glob("*.obj")

	for obj in objfiles:
		newcontent = filterObj(obj)
		f = open(obj.replace(".obj", ".nobj"), 'w')
		f.write(newcontent);


def filterObj(fpath):
	content = []
	with open(fpath, 'r') as objfile:
		for line in objfile:
			if(line.find("usemtl") == -1):
				content.append(line)
	return(''.join(content))

if __name__ == "__main__":
	main()