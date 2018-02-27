#!/usr/bin/python3

from __future__ import print_function
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
	foundvert = glob(fullpath+"/**/*.vs", recursive = True) + glob(fullpath+"/**/*.vert", recursive = True)
	foundfrag = glob(fullpath+"/**/*.fs", recursive = True) + glob(fullpath+"/**/*.frag", recursive = True)

	for vs in foundvert:
		vslist.append(VertexShader(vs, fullpath = fullpath))

	for fs in foundfrag:
		fslist.append(FragmentShader(fs, fullpath = fullpath))

	return((vslist,fslist))

def export_json(vslist, fslist, opath):
	pairs = {}
	for vs in vslist:
		for fs in fslist:
			if(vs.basename == fs.basename or vs.pair == fs.longname or fs.pair == vs.longname):
				pairs[vs.extendedbasename if fs.pair == None else fs.extendedbasename] = (vs.__dict__,fs.__dict__)

	floating = {}

	for vs in vslist:
		floating[vs.longname] = vs.__dict__
	for fs in fslist:
		floating[fs.longname] = fs.__dict__

	ofile = open(opath,'w')
	ofile.write(json.dumps({"pairs": pairs, "all": floating}, indent=2))
	ofile.close() 

def parse_args():
	parser = argparse.ArgumentParser()
	parser.add_argument("srcpaths", nargs='+', help="Path to directory full of vertex (.vs, .vert) and fragment (.fs, .frag) shader files")
	parser.add_argument("-o", "--output", default="assets/shaders.js", help="Path for output javascript file")
	return(parser.parse_args())

class VertexShader():
	def __init__(self, path, fullpath = "./"):
		f = open(path, 'r')

		self.type = 'vertex'
		self.pair = None
		
		self.lines = []
		self.uniforms = []
		self.attributes = []
		for line in f:
			self.lines.append("      {0}".format(line))
			splitline = line.split()
			if(len(self.lines) == 1):
				self.pair = parse_pair(line)
			if(len(splitline) != 0):
				attmatch = re.search(R"layout\(location = [0-9]+\)", line)
				if(splitline[0] == "uniform"):
					self.uniforms.append([splitline[1],splitline[2].replace(';', '')])
				elif(attmatch != None):
					self.attributes.append([splitline[-2], splitline[-1].replace(';', '')])

		self.src = ''.join(self.lines)
		self.lines = len(self.lines)
		self.filename = pathutils.split(path)[-1]
		self.name, self.ext = pathutils.splitext(self.filename)
		self.basename = pathutils.basename(self.name)
		self.name = "vert_{0}".format(pathutils.basename(self.name))
		longprefix = [seg.strip() for seg in pathutils.relpath(path, start=fullpath).split(pathutils.sep)[0:-1] ]
		self.longname =  '_'.join(longprefix + [self.name])
		self.extendedbasename = '_'.join(longprefix + [self.basename])

class FragmentShader():
	def __init__(self, path, fullpath = "./"):
		f = open(path, 'r')

		self.type = 'fragment'
		self.pair = None

		self.lines = []
		self.uniforms = []
		for line in f:
			self.lines.append("      {0}".format(line))
			splitline = line.split()
			if(len(self.lines) == 1):
				self.pair = parse_pair(line)
			if(len(splitline) != 0):
				if(splitline[0] == "uniform"):
					self.uniforms.append([splitline[1], splitline[2].replace(';', '')])

		self.src = ''.join(self.lines)
		self.lines = len(self.lines)
		self.filename = pathutils.split(path)[-1]
		self.name, self.ext = pathutils.splitext(self.filename)
		self.basename = pathutils.basename(self.name)
		self.name = "frag_{0}".format(pathutils.basename(self.name))
		longprefix = [seg.strip() for seg in pathutils.relpath(path, start=fullpath).split(pathutils.sep)[0:-1] ]
		self.longname =  '_'.join(longprefix + [self.name])
		self.extendedbasename = '_'.join(longprefix + [self.basename])

def parse_pair(line):
	match = re.search(R"^// PAIR:\s+(\S+)\s*$", line);
	if(match != None):
		ref = match.groups()[-1].strip()
		return(ref);
	return(None);

if __name__ == "__main__":
	main()