#!/usr/bin/python

###############################################################################
# run.py
###############################################################################

# TODO: I know I'm going to have problems when we have complex types, but
# haven't fixed it yet. For example if someone wants to make a 'char **l' type
# they could write it as 'char * * l" or 'char* *l' and both would break my
# current assumptions that you either write 'char** l' or 'char **l'. The
# spaces are going to cause problems.


#------------------------------------------------------------------------------
# The doc string that is output if the user does not call the program
# correctly.
#------------------------------------------------------------------------------

"""
pilc v0.2
written by asmith

PILC, the pil compiler, will parse an input program written in the parallel
intermediate language (PIL) and generate the corresponding backend code file
specified. The generatred code is output to stdout.

PILC takes 1 argument:

    input_file - The name of the input file to transform into backend code.

PILC accepts the following options:

    -i file_type, --input=file_type
        Specify the type of front end that generated the *.pil file. If none
        specified, default will be assumed. Valid file_types are {default,
        chpl}.

    -v, --debug=level
        Set the amount of help output information to give. -v specifies
        verbose, which is a debug level of 1. Possible levels are 0-9. 0 Gives
        no debug information, 9 gives all debug information. Default is 0.

    -h, --help
        Output this help message.

    -o backend, --output=backend
        Set the type of backend code to generate. If none is specified,
        sequential C will be assumed. Current backends are {C, omp, swarm, ocr,
		afl}.

Usage:

    > pilc [-i file_type] [-h] [--debug=level] [-o backend] input_file
"""


#------------------------------------------------------------------------------
# Importing libraries
#------------------------------------------------------------------------------

import re
import sys
from string import strip
import getopt
import pil2ocr, pil2afl, pil2swarm, pil2c, g


#------------------------------------------------------------------------------
# Function declarations
#------------------------------------------------------------------------------

#---------------------------------------
# helper functions
#---------------------------------------

def usage():
	"""print usage help message and exit"""

	sys.stderr.write(__doc__)
	sys.exit()

def debug(level, line):
	"""Allow debugging information to be printed if the DEBUG_LEVEL is set high
	enough. A DEBUG_LEVEL of 0 will output almost no information. A
	DEBUG_LEVEL of 9 will output all debugging statements."""

	if level <= g.DEBUG_LEVEL:
		sys.stderr.write(line + "\n")


def file_debug(level, line):
	"""The line will be written into the file if its level is set high enough.
	A FILE_DEBUG_LEVEL of 0 will output nothing wile a FILE_DUBUG_LEVEL of 9 will
	ouput everything."""

	if level <= g.FILE_DEBUG_LEVEL:
		print(line)


def error(line):
	"""print error statement and die"""

	sys.stderr.write("ERROR: " + line + "\n")
	sys.exit(1)


def warning(line):
	"""print warning statement"""

	sys.stderr.write("WARNING: " + line + "\n")


def process_command_line_arguments():
	"""Read the command line arguments and set the appropriate variables"""

	try:
		opts, inargs = getopt.getopt(sys.argv[1:], "hvi:o:m:", ["help", "input=", "debug=", "output=", "mem=", "rstream"])
	except getopt.GetoptError, err:
		# print help information and exit:
		print str(err) # will print something like "option -a not recognized"
		usage()
	for o, a in opts:
		if o in ("-v"):
			g.DEBUG_LEVEL = 1
		elif o in ("--debug"):
			g.DEBUG_LEVEL = a
		elif o in ("-h", "--help"):
			usage()
		elif o in ("-i", "--input"):
			g.FILE_TYPE = a
		elif o in ("-m", "--memory"):
			g.MEM = a
		elif o in ("-o", "--output"):
			g.OUTPUT = a
			debug(2, "OUTPUT: " + g.OUTPUT)
		elif o in ("--rstream"):
			g.RSTREAM = True
		else:
			assert False, "unhandled option"

	if len(inargs) != 1:
		usage()
	input_filename = inargs[0]

	# set the base name of the file for later use.
	longname = re.match(".*/(\w+).pil$", input_filename)
	name = re.match("(\w+).pil$", input_filename)
	# name.group(1) - the base name of the file
	if longname:
		g.FILE = longname.group(1)
	elif name:
		g.FILE = name.group(1)
	else:
		error("Invalid file '" + input_filename + "'. Can only process *.pil files")

	return input_filename


def handle_main_node(label, lower, step, upper, func_name):
	"""set the first node to call from the main function."""

	# TODO: this assumes that the first node in the file is main
	if g.first_node:
		g.first_node = False
		g.main_node_label = label
		debug(4, "main_node_input: [" + lower + ", " + step + ", " + upper + "]")
		g.main_node_input = [lower, step, upper]
		g.main_node_func_name = func_name


def set_stack_size():
	"""Set the maximum size of the stack so we reduce the likelihood of having
	a stack overflow error."""

	print ""
	print "\t// Setting the stack size"
	print "\tstruct rlimit stackrl;"
	print "\tstackrl.rlim_cur = " + g.stacksize + ";"
	print "\tsetrlimit(RLIMIT_STACK, &stackrl);"


def split_args(in_args):
	"""Split a list of input arguments into the individual arguments"""

	# this will split the arguments on commas. However, if the argument is
	# passed by reference, it will look like '&arg' or '& arg'
	split_args = re.split("\s*,\s*", in_args)

	return split_args


def process_func_args(func_name, in_args):
	"""process the input arguments for a function. store them in a global data
	   structure so we can refer to their type later when we need to."""

	debug(2, "reading arguments for function " + func_name)

	# initialize the structure for the function.
	g.args[func_name] = {}
	g.args[func_name]['dict'] = {}
	g.args[func_name]['list'] = []

	if not in_args:
		return

	# split in_args into each argument and its type
	# split on commas
	ags = split_args(in_args)

	split_in_args = []
	for a in ags:
		#arg = re.match("(\w+)\s*([&*]{0,1})*\s*(\w+)", a)
		arg = re.match("([&]{0,1})\s*[\(]{0,1}\s*(\w+)\s*[\)]{0,1}", a)
		if arg:
			#arg_type = arg.group(1)
			arg_modifier = arg.group(1)
			arg_name = arg.group(2)
			#split_in_args.append({'name': arg_name, 'modifier': arg_modifier, 'type': arg_type})
			split_in_args.append({'name': arg_name, 'modifier': arg_modifier})
		else:
			error("couldn't match the arg in process_func_args: '" + a + "' from in_args: '" + in_args + "', ags: " + str(ags))

	# set the type for each arg from the spit information
	for sia in split_in_args:
		# output debug info
		debug_string = ""
		if sia['modifier']:
			debug_string += " " + sia['modifier']
		debug_string += " " + sia['name'] + "\n"
		debug(3, debug_string)

		# save info
		g.args[func_name]['list'].append(sia['name'])
		g.args[func_name]['dict'][sia['name']] = {}
		if sia['modifier']:
			g.args[func_name]['dict'][sia['name']]['modifier'] = sia['modifier']
		else:
			g.args[func_name]['dict'][sia['name']]['modifier'] = None


def match_function(string):
	"""check to see if the string is a function declaration"""

	f = re.match('(void)\s+(\w+)\s*\(\s*(.*)\s*\)(.*)', string)
	# f.group(1) - return value
	# f.group(2) - function name
	# f.group(3) - input parameters
	# f.group(4) - anything after the function. probably white space or the opening {

	return f


def process_function(lines, lineno):
	"""reads a function from lines starting at lineno and generates the
	   appropriate output."""

	# the number of currently open braces
	braces = 0
	started = False
	processing_pil_main = False


	if True:
		pn = re.match('_pil_node\s+(\w+)\s+(.*)', lines[lineno])
		# pn.group(1) - label
		# pn.group(2) - function

		pnl = re.match('_pil_nodelet\s+(\w+)\s+(.*)', lines[lineno])
		# pnl.group(1) - label
		# pnl.group(2) - function

		label = ""

		if pn:
			label = pn.group(1)
			f = match_function(pn.group(2))
			ret = f.group(1)
			func_name = f.group(2)
			in_args = f.group(3)
			rest = f.group(4)
			#g.nodes.append({'label': label, 'func_name': func_name})
			#g.nodes[label] = func_name
			g.nodes[label] = {'label': label, 'func_name': func_name}
			g.context_variables[label] = []
		elif pnl:
			label = pnl.group(1)
			f = match_function(pnl.group(2))
			ret = f.group(1)
			func_name = f.group(2)
			in_args = f.group(3)
			rest = f.group(4)
			#g.nodelets.append({'label': label, 'func_name': func_name})
			#g.nodelets[label] = func_name
			g.nodelets[label] = {'label': label, 'func_name': func_name}
		else:
			f = match_function(lines[lineno])

		if f:
			ret = f.group(1)
			func_name = f.group(2)
			in_args = f.group(3)
			rest = f.group(4)
			
			if func_name == "pil_main":
				g.PIL_MAIN = True
				processing_pil_main = True

			line = ret + " " + func_name + "(" + in_args + ")" + rest + "\n"
			if processing_pil_main:
				g.pil_main_lines.append(line)
			else:
				g.func_lines[func_name] = []
				if g.RSTREAM:
					g.func_lines[func_name].append("#pragma rstream map\n")
				g.func_lines[func_name].append(line)
		lineno += 1

	while True:
		for c in lines[lineno]:
			bo = re.search('{', c) # checks the character for {
			if bo:
				started = True
				braces += 1
			bc = re.search('}', c) # checkes the character for }
			if bc:
				braces -= 1

		# output the line

		e = re.match('\s*pil_enter\s*\(\s*(\w+)\s*,\s*(\w+)\s*.*', lines[lineno])
		# e.group(1) - number or name of node to enter
		# e.group(2) - number of arguments going to be passed

		# pil_barrier
		b = re.match("(\s*)pil_barrier_all\s*\((\w+)\)\s*;.*", lines[lineno])
		# b.group(1) - whitespace for matching indent
		# b.group(2) - continuation nodelet

		# pil_recv
		#r = re.match("(\s*)pil_recv\s*\(\s*(\w+)\s*,\s*([&]{0,1})\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*\)\s*;.*", lines[lineno])
		r = re.match(g.NW_RECV_RE, lines[lineno])
		#r = re.match("(\s*)pil_recv\s*\((\w+)\s*,\s*([&]{0,1})\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+).*", lines[lineno])
		# r.group(1) - whitespace for matching indent
		# r.group(2) - source
		# r.group(3) - modifier
		# r.group(4) - pointer
		# r.group(5) - size
		# r.group(6) - continuation

		pc = re.match("(\s*)_pil_context\s+(\w+)\s+(\w+)\s*;", lines[lineno])
		# pc.group(1) - whitespace for matching indent
		# pc.group(2) - type
		# pc.group(3) - name

		if e:
			g.nodes_entered.append({'label': e.group(1), 'num_args': e.group(2)})
			if processing_pil_main:
				g.pil_main_lines.append(lines[lineno])
			else:
				g.func_lines[func_name].append(lines[lineno])
		elif b:
			space = b.group(1)
			continuation = b.group(2)
			debug(5, "pil_barrier_all")
			#line = space + "nw_barrierAll(GRAPH_BARRIER_0, swarm_cargs(_pil_" + continuation + "));\n"
			line = "nw_barrierAll(GRAPH_BARRIER_0, swarm_cargs(_pil_" + continuation + "));"
			g.nw_calls[label] = line
			#if processing_pil_main:
			#	g.pil_main_lines.append(line)
			#else:
			#	g.func_lines[func_name].append(line)
		elif r:
			space = r.group(1)
			source = r.group(2)
			modifier = r.group(3)
			pointer = r.group(4)
			size = r.group(5)
			continuation = r.group(6)
			debug(4, "pil_recv")
			#line = "pil_recv(" + source + ");"
			g.nw_calls[label] = lines[lineno]
		elif pc:
			space = pc.group(1)
			arg_type = pc.group(2)
			arg_name = pc.group(3)
			g.context_variables[label].append({"name": arg_name, "type": arg_type})
		else:
			if processing_pil_main:
				g.pil_main_lines.append(lines[lineno])
			else:
				#print lines[lineno],
				g.func_lines[func_name].append(lines[lineno])

		# read until the matching }
		lineno += 1
		if (braces == 0) and started:
			if processing_pil_main:
				g.pil_main_lines.append("\n")
				processing_pil_main = False
			else:
				g.func_lines[func_name].append("\n")
			break

#	if g.PIL_MAIN and func_name == "pil_main":
#		sys.stderr.write("nodes_entered:\n")
#		for node in g.nodes_entered:
#			sys.stderr.write(node['label'] + "\n")
#		sys.stderr.write("\n")

	return lineno-1


def process_node(lines, lineno):
	"""finds the node from the lines and returns it"""

	line = lines[lineno]

	print ""
	print "//-----------------------------------------------------------------------------"
	print "// " + line,
	print "//-----------------------------------------------------------------------------"
	print ""

	m = re.match(g.MAP_RE, line)
	if not m:
		error("invalid node line at line number " + str(lineno+1))

	return line


def process_input_arguments(func_name, in_args):
	"""Look through each of the arguments in the function call of the node to
	   see which are passed by are passed by reference. Set the appropriate
	   field in args so we can refer to it later"""

	# break each of the arguments from the string to so we can refer to it.
	split_in_args = split_args(in_args)

	arguments = {}
	for arg in split_in_args:
		ARG_RE = "\s*([&]{0,1})\s*[\(]{0,1}\s*(\w+)\s*[\(]{0,1}\s*"
		pbr = re.match(ARG_RE, arg)
		if pbr:
			mod = pbr.group(1)
			arg_name = pbr.group(2)
			if mod:
				modifier = True
			else:
				modifier = False
			arguments[arg_name] = {'modifier': modifier, 'name': arg_name}
		elif arg == '':
			pass
		else:
			error("regular expression didn't match in process_input_arguments. arg: '" + arg + "'")

	for arg_name in arguments:
		if arguments[arg_name]['modifier']:
			g.args[func_name]['dict'][arg_name]['passed_by_reference'] = True
		else:
			g.args[func_name]['dict'][arg_name]['passed_by_reference'] = False


def make_targets(targets):
	"""take the string of targets from a node and turn it into a list of the targets"""

	split_targets = re.split("\s*,\s*", targets)

	seen = []
	target_list = []
	for target in split_targets:
		debug(1, "target: " + target)

		if target not in seen:
			target_list.append(target)
			seen.append(target)
		else:
			warning("Caught a duplicate destination target '" + target + "'. Removing it.")
	
	return target_list


def handle_nodes(nodes):
	"""process the list of nodes and output he corresponding code"""
	# Assumptions: the node() line is all one one line

	split_nodes = []
	curnode = -1
	for m in nodes:
		split_nodes.append({})
		curnode += 1

		# TODO: make this a function call or something so i can change the node language more easily
		# no need to error check this since we already did in process_node
		ma = re.match(g.MAP_RE, m)

		split_nodes[curnode]["label"] = ma.group(1)
		split_nodes[curnode]["index"] = ma.group(2)
		split_nodes[curnode]["lower"] = ma.group(3)
		split_nodes[curnode]["step"] = ma.group(4)
		split_nodes[curnode]["upper"] = ma.group(5)
		split_nodes[curnode]["cond"] = ma.group(6)
		split_nodes[curnode]["targets"] = ma.group(7)
		split_nodes[curnode]["func"] = ma.group(8)
		split_nodes[curnode]["func_name"] = ma.group(9)
		split_nodes[curnode]["in_args"] = ma.group(10)

	# go through the nodes and one at a time output the code. The multiple
	# loops are necessary so that the code is output together for each
	# function.
	# TODO: some of these loops could be combined together for performance
	for m in split_nodes:
		g.functions[m['label']] = m['func_name']
		g.intervals[m['label']] = {'lower': m['lower'], 'step': m['step'], 'upper': m['upper']}
		g.targets[m['label']] = make_targets(m['targets'])
		g.target_variables[m['label']] = m['cond'] 

	for m in split_nodes:
		# store the input args so we can refer to their type later
		process_func_args(m['func_name'], m['in_args'])
		process_input_arguments(m['func_name'], m['in_args'])
		if m['index'] == 'NULL':
			warning("Caught a NULL loop index variable that will be replaced with '" + g.INDEX + "'")
			m['index'] = g.INDEX
		g.indices[m['label']] = m['index']

	for m in split_nodes:
		handle_main_node(m['label'], m['lower'], m['step'], m['upper'], m['func_name'])

	if g.OUTPUT == "C" or g.OUTPUT == "omp":
		pil2c.handle_nodes(split_nodes)
	elif g.OUTPUT == "swarm":
		pil2swarm.handle_nodes(split_nodes)
	elif g.OUTPUT == "afl":
		pil2afl.handle_nodes(split_nodes)
	elif g.OUTPUT == "ocr":
		pil2ocr.handle_nodes(split_nodes)
	else:
		error("Unknown OUTPUT backend: " + g.OUTPUT)

	return split_nodes


def read_file(input_filename):
	"""read the contents of the input file into the data structure"""

	global NUM_LINES

	# read the file
	debug(1, "Reading file " + input_filename)
	infile = open(input_filename, 'r')
	lines = infile.readlines()
	infile.close
	NUM_LINES = len(lines)

	return lines


def process_header():
	"""Output the common header file for files. E.g., includes libraries. This
	is backend independent."""

	print ""
	print "// Common library includes"
	print "#include <stdio.h>"
	print "#include <stdlib.h>"
	print "#include <stdint.h>"
	print "#include <assert.h>"
	print "#include <string.h>"
	print ""
	print "// Includes to manage the maximum size of the stack"
	print "#include <sys/resource.h>"
	print "#include <unistd.h>"
	print ""
#print '#include "' + g.header_file_name + '" // The headerfile generated by pilc'
	print '#include "pil_mem.h" // Include for PIL to manage memory'
	print '#include "pil_init.h" // Include for PIL to init the evironment before it starts'
	print '#include "pil_enter.h" // Include for PIL to be able to call pil_enter()'
	print '#include "pil_nw.h" // Include for distributed PIL'
	print ""
	if g.FILE_TYPE in g.CHPL:
		print "// Chapel includes"
		print '#include "_main.c"'
		print ""
	else:
		pass


def process_pil_enter(nodes):
	"""Output the pil_enter function"""

	if not g.nodes_entered:
		return

	print \
"""
void pil_enter(int func, int num_args, ...)
{
	va_list argptr;
	va_start(argptr, num_args);
"""

	#g.nodes_entered.sort()
	if g.OUTPUT == "C" or g.OUTPUT == "omp":
		#pil2c.declare_pil_enter_arguments()
		pass
	elif g.OUTPUT == "swarm":
		pil2swarm.declare_pil_enter_arguments()
	elif g.OUTPUT == "afl":
		#pil2afl.declare_pil_enter_arguments()
		pass
	elif g.OUTPUT == "ocr":
		pil2ocr.declare_pil_enter_arguments()
	else:
		error("Unknown OUTPUT backend: " + g.OUTPUT)

	passed_args = {}

	for node in g.nodes_entered:
		args = g.args[g.functions[node['label']]]
		for arg_name in args['list']:
			if arg_name not in passed_args.keys():
				passed_args[arg_name] = g.args[g.functions[node['label']]]['dict'][arg_name]['passed_by_reference']

	for arg_name in passed_args.keys():
		if arg_name == g.target_variables[node['label']]:
			continue
		#elif arg_name == 'index_array' or arg_name == 'data_array':
		#	continue
		if passed_args[arg_name]: # passed by reference
			print "\t" + g.variables[arg_name] + " *" + arg_name + "_ptr = NULL;"
		#elif g.OUTPUT == "ocr":
		#	print "\t" + g.variables[arg_name] + " " + arg_name + ";"

	print \
"""
	// process the arguments for each library function
	switch (func)
	{
		case 0:
			//error("can't enter node 0\\n");
			printf("can't enter node 0\\n");
			exit(EXIT_FAILURE);
			break;
"""
	for node in g.nodes_entered:
		print "\t\tcase " + node['label'] + ":"
		if g.OUTPUT == "C" or g.OUTPUT == "omp":
			pil2c.process_pil_enter_arguments(node)
		elif g.OUTPUT == "swarm":
			pil2swarm.process_pil_enter_arguments(node)
		elif g.OUTPUT == "afl":
			pil2afl.process_pil_enter_arguments(node)
		elif g.OUTPUT == "ocr":
			pil2ocr.process_pil_enter_arguments(node)
		else:
			error("Unknown OUTPUT backend: " + g.OUTPUT)

	print \
"""
		default:
			printf("ERROR: invalid type\\n");
			break;
	}

	va_end(argptr);

	// call the appropriate library function
	switch (func)
	{
"""

	for node in g.nodes_entered:
		print "\t\tcase " + node['label'] + ":"
		print "\t\t{"

		if g.OUTPUT == "C" or g.OUTPUT == "omp":
			pil2c.process_pil_enter(nodes, node)
		elif g.OUTPUT == "swarm":
			pil2swarm.process_pil_enter(nodes, node)
		elif g.OUTPUT == "afl":
			pil2afl.process_pil_enter(nodes, node)
		elif g.OUTPUT == "ocr":
			pil2ocr.process_pil_enter(nodes, node)
		else:
			error("Unknown OUTPUT backend: " + g.OUTPUT)

		print "\t\t}"

	print \
"""
		default:
		{
			printf("ERROR: unknown function id (%d) in pil_enter() or unemplemented for specified backend\\n", func);
			break;
		}
	}
}
"""


def process_file(input_filename, lines):
	"""Take the contets of the *.pil file and output the corresponding backend
	code. This is where all of the backend specific calls are made."""

	# TODO: this function should be made into two functions. One tha processes
	# the file and generates all of the data structures and one that calls all
	# of the backend specific functions that outputs the code.

	# open the output files
#	if g.OUTPUT == "C" or g.OUTPUT == "omp" or g.OUTPUT == "afl" or g.OUTPUT == "ocr":
#		g.header_file_name = "pil.h"
#	elif g.OUTPUT == "swarm":
#		g.header_file_name = "pil.swh"
#	else:
#		error("Unknown OUTPUT backend: " + g.OUTPUT)
	g.header_file_name = "pil.h"

	g.h_file = open(g.header_file_name, "w")
	g.h_file.write("#ifndef PIL_H\n")
	g.h_file.write("#define PIL_H\n\n")
	g.h_file.write("#ifdef PIL2OCR\n")
	g.h_file.write("#include \"ocr.h\"\n")
	g.h_file.write("typedef ocrGuid_t guid_t;\n")
	g.h_file.write("#else\n")
	g.h_file.write("#define NULL_GUID NULL\n")
	g.h_file.write("typedef void* guid_t;\n")
	g.h_file.write("#endif // PIL2OCR\n")
	g.h_file.write("\n")
	g.h_file.write("typedef struct {\n")
	g.h_file.write("\tguid_t guid;\n")
	g.h_file.write("\tvoid *ptr;\n")
	g.h_file.write("} gpp_t;\n\n")

	# data structure to store nodes we encounter in so that we can process them
	# all together later
	nodes = []

	# 1) print the header
	if g.OUTPUT == "C" or g.OUTPUT == "omp":
		pil2c.process_header()
	elif g.OUTPUT == "swarm":
		pil2swarm.process_header(input_filename)
	elif g.OUTPUT == "afl":
		pil2afl.process_header()
	elif g.OUTPUT == "ocr":
		pil2ocr.process_header()
	else:
		error("Unknown OUTPUT backend: " + g.OUTPUT)

	# 2) process the file
	lineno = -1
	while (lineno < len(lines)-1):

		lineno += 1
		l = lines[lineno]

		#line = re.split('\s+', l)
		l = strip(l)

		# the line is empty
		#e = re.match('\B', l)
		#if e:
		if l == '':
			print l
			continue

		# the line is a comment
		c = re.match('#(.*)', l)
		# c.group(1) - the comment text
		if c:
			d = re.match('#ifdef(.*)', l)
			if d:
				print l
				continue
			d = re.match('#ifndef(.*)', l)
			if d:
				print l
				continue
			d = re.match('#endif(.*)', l)
			if d:
				print l
				continue
			d = re.match('#else(.*)', l)
			if d:
				print l
				continue
			d = re.match('#include(.*)', l)
			if d:
				print l
				continue
			d = re.match('#undef(.*)', l)
			if d:
				print l
				continue
			d = re.match('#define(.*)', l)
			if d:
				print l
			else:
				print "//" + c.group(1)
			continue

		c = re.match('//(.*)', l)
		# c.group(1) - the comment text
		if c:
			print "//" + c.group(1)
			continue

		# the line is a C style block comment on a single line
		# TODO: still don't account for multi-line block comments
		c = re.match('/\*(.*)\*/', l)
		# c.group(1) - the comment text
		if c:
			print "/*" + c.group(1) + "*/"
			continue

		# TODO: make a function to handle this
		# the line is a variable
		v = re.match('(\s*(\w+)\s*([*&]*)\s*(\w+)\s*(=\s*(NULL)\s*){0,1});', l) # NULL initialization
		#v = re.match('(\s*(\w+)\s*([*&]*)\s*(\w+)\s*);', l)
		# v.group(1) - the whole statement
		# v.group(2) - the variable type
		# v.group(3) - the variable modifier
		# v.group(4) - the variable name
		# v.group(5) - the assignment
		# v.group(6) - the variable initialization
		va = re.match('(\s*(\w+)\s*([*&]*)\s*(\(\*(\w+)\))\[(\w+)\]s*);', l)
		# va.group(1) - the whole statement
		# va.group(2) - the variable type
		# va.group(3) - the variable modifier
		# va.group(4) - the variable name as (*name)
		# va.group(5) - the variable name
		# va.group(6) - the variable size

		vas = re.match('(\s*(\w+)\s*([*&]*)\s*(\w+)\[(\w+)\]\[(\w+)\]s*);', l)
		# va.group(1) - the whole statement
		# va.group(2) - the variable type
		# va.group(3) - the variable modifier
		# va.group(4) - the variable name
		# va.group(5) - the variable size
		# va.group(6) - the variable size
		if v:
			var_type = v.group(2)
			var_modifier = v.group(3)
			var_name = v.group(4)
			var_init = v.group(6)
			g.variables[var_name] = var_type
			if var_modifier:
				g.variables[var_name] += " " + var_modifier
			if var_init:
				#warning("Caught a NULL initialized pointer that won't be NULL initialized: '" + var_name + "'")
				g.initial_values[var_name] = var_init
			continue
		if va:
			var_type = va.group(2)
			var_modifier = va.group(3)
			var_sname = va.group(4)
			var_name = va.group(5)
			var_size = va.group(6)
			g.variables[var_name] = var_type
			g.arrays[var_name] = var_size
			if var_modifier:
				g.variables[var_name] += " " + var_modifier
			continue
		if vas:
			var_type = vas.group(2)
			var_modifier = vas.group(3)
			var_name = vas.group(4)
			var_sizex = vas.group(5)
			var_sizey = vas.group(6)
			debug(4, "VAS match: " + var_name + "\n")
			g.variables[var_name] = var_type
			g.arrays[var_name] = [var_sizex, var_sizey]
			if var_modifier:
				g.variables[var_name] += " " + var_modifier
			continue

		# the line is a function declaration
		f = re.match('void\s+\w+\s*\(', l)
		if f:
			#debug(3, v.group(0))
			lineno = process_function(lines, lineno)
			continue

		pn = re.match('_pil_node\s+(\w+).*', l)
		if pn:
			label = pn.group(1);
			lineno = process_function(lines, lineno)
			continue

		pnl = re.match('_pil_nodelet\s+(\w+).*', l)
		if pnl:
			label = pnl.group(1);
			lineno = process_function(lines, lineno)
			continue

		# the line is a node
		m = re.match(g.MAP_RE, l)
		if m:
			# add the node to the nodes list for later processing
			nodes.append(process_node(lines, lineno))
			continue

		# if we have made it this far, the line is invalid
		warning("invalid line: " + l)

	# 3) create the global data structure
	if g.OUTPUT == "C" or g.OUTPUT == "omp":
		pil2c.process_variables()
	elif g.OUTPUT == "swarm":
		pil2swarm.process_variables()
	elif g.OUTPUT == "afl":
		pil2afl.process_variables()
	elif g.OUTPUT == "ocr":
		pil2ocr.process_variables()
	else:
		error("Unknown OUTPUT backend: " + g.OUTPUT)

	# 4) now that the globals are available, we can output pil_main and the body functions
	if g.OUTPUT == "C" or g.OUTPUT == "omp":
		pil2c.print_main_func()
		pil2c.print_funcs()
	elif g.OUTPUT == "swarm":
		pil2swarm.print_main_func()
		pil2swarm.print_funcs()
	elif g.OUTPUT == "afl":
		pil2afl.print_main_func()
		pil2afl.print_funcs()
	elif g.OUTPUT == "ocr":
		pil2ocr.print_main_func()
		pil2ocr.print_funcs()
	else:
		error("Unknown OUTPUT backend: " + g.OUTPUT)

	# 5) process all of the nodes
	split_nodes = handle_nodes(nodes)

	# 6) output pil_enter()
	process_pil_enter(split_nodes)

	# 7) print the main function
	if g.OUTPUT == "C" or g.OUTPUT == "omp":
		pil2c.process_main()
	elif g.OUTPUT == "swarm":
		pil2swarm.process_main()
	elif g.OUTPUT == "afl":
		pil2afl.process_main()
	elif g.OUTPUT == "ocr":
		pil2ocr.process_main(split_nodes)
	else:
		error("Unknown OUTPUT backend: " + g.OUTPUT)

	g.h_file.write("#endif // PIL_H\n")
	g.h_file.close()


#------------------------------------------------------------------------------
# main()
#------------------------------------------------------------------------------

if __name__ == "__main__":
	"""The main function."""

	# read the command line arguments
	input_filename = process_command_line_arguments()

	warning("Manually increasing stack size to compensate for never returning from a function")
	warning("Unoptimized handling of stride of less than 0.")
	warning("Error will occure if you have 0 loop iterations. E.g. forall i in [1..0 by 1]")
	warning("SWARM functions are currently prepended with 'swarm_' which may clash with built-in SWARM functions")

	# read the file into lines
	lines = read_file(input_filename)

	# process the file
	process_file(input_filename, lines)

	# success message
	if g.FILE_TYPE not in g.DEFAULT:
		l = 0
	else:
		l = 1
	debug(l, "Compilation successful")

