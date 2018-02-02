#!/usr/bin/python3

import os
pathutils = os.path
import sys
import re
from glob import glob
import argparse
import json
import re

def main():
	args = parse_args()
	for path in args.srcpaths:
		if(not valid_path(path)):
			print("Path '{0}' is inaccessible! Skipping...".format(path), file=sys.stderr)
			break
		else:
			vertshaders, fragshaders = gather_shaders(path)
	export_json(vertshaders, fragshaders, args.output);

def valid_path(path):
	try:
		os.listdir(path)
	except:
		return(False)
	return(True)

def gather_shaders(path):
	vslist = []
	fslist = []
	fullpath = pathutils.abspath(path)
	foundvert = glob(fullpath+"/*.vs") + glob(fullpath+"/*.vert")
	foundfrag = glob(fullpath+"/*.fs") + glob(fullpath+"/*.frag")

	for vs in foundvert:
		vslist.append(VertexShader(vs))

	for fs in foundfrag:
		fslist.append(FragmentShader(fs))

	return((vslist,fslist))

def export_json(vslist, fslist, opath):
	contents = []

	for vs in vslist:
		contents.append(vs.__dict__);
	for fs in fslist:
		contents.append(fs.__dict__);

	ofile = open(opath,'w')
	ofile.write(json.dumps(contents, indent=2))
	ofile.close() 

def parse_args():
	parser = argparse.ArgumentParser()
	parser.add_argument("srcpaths", nargs='+', help="Path to directory full of vertex (.vs, .vert) and fragment (.fs, .frag) shader files")
	parser.add_argument("-o", "--output", default="assets/shaders.js", help="Path for output javascript file")
	return(parser.parse_args())

class VertexShader():
	def __init__(self, path):
		f = open(path, 'r')

		self.type = 'vertex'
		
		self.lines = []
		self.uniforms = []
		self.attributes = []
		for line in f:
			self.lines.append("      {0}".format(line))
			splitline = line.split()
			if(len(splitline) != 0):
				attmatch = re.search(R"layout\(location = [0-9]+\)", line)
				if(splitline[0] == "uniform"):
					self.uniforms.append([splitline[1],splitline[2].replace(';', '')])
				elif(attmatch != None):
					self.attributes.append([splitline[-2], splitline[-1].replace(';', '')])

		self.src = ''.join(self.lines)
		self.lines = len(self.lines)
		self.name, self.ext = pathutils.splitext(path)
		self.name = "vert_{0}".format(pathutils.basename(self.name))

class FragmentShader():
	def __init__(self, path):
		f = open(path, 'r')

		self.type = 'fragment'

		self.lines = []
		self.uniforms = []
		for line in f:
			self.lines.append("      {0}".format(line))
			splitline = line.split()
			if(len(splitline) != 0):
				if(splitline[0] == "uniform"):
					self.uniforms.append([splitline[1], splitline[2].replace(';', '')])

		self.src = ''.join(self.lines)
		self.lines = len(self.lines)
		self.name, self.ext = pathutils.splitext(path)
		self.name = "frag_{0}".format(pathutils.basename(self.name))

if __name__ == "__main__":
	main()