###############################################################################
# pil2swarm
###############################################################################

import run
#from run import run.file_debug, debug
import g
import sys
import re

#------------------------------------------------------------------------------
# SWARM specific process functions for use by mc
#------------------------------------------------------------------------------

def print_main_func():
	"""output the main function"""

	for line in g.pil_main_lines:
		print line,


def print_funcs():
	"""output the body functions"""

	if g.MEM != "spmd":
		for func in g.func_lines.keys():
			for line in g.func_lines[func]:
				print line,


def process_header(filename):
	"""process_header - outputs the header for the file. E.g. includes libraries"""

	print "// Swarm MLL code generated from " + filename

	run.process_header()

	print "// SWARM specific includes"
	print "#include <swarm/Nat32.h>"
	print "#include <swarm/Procedure.h>"
	print "#include <swarm/Runtime.h>"
	if g.MEM == "forkjoin" or g.MEM == "spmd":
		print "#include <swarm/nw_api.h>"
		print ""
		print "barrierID_t const GRAPH_BARRIER_0 = 0;"
	print ""


def set_num_threads():
	"""Set the number of threads in the runtime system"""

	if (g.FILE_TYPE in g.CHPL) and (g.DEBUG_LEVEL > 0):
		print ""
		print "\t//Setting the number of threads for the runtime system"
		run.file_debug(0, '\tprintf("Starting SWARM with ' + g.NUM_THREADS + ' threads.\\n");')
		print "\tswarm_Runtime_params_t _p;"
		print "\tswarm_Runtime_params_init(&_p);"
		print "\t_p.maxThreadCount = " + g.NUM_THREADS + ";"


def enter_swarm():
	"""Enter the SWARM runtime environment"""

	runtime_params = ""
	if g.DEBUG_LEVEL > 0:
		runtime_params = ", &_p"
	print ""
	print "\t// Start SWARM"
	print "\tnode_" + g.main_node_label + ".swarm_enter(&input" + runtime_params + ");"

def declare_pil_enter_arguments():


	print "#ifdef PIL2SWARM"

	for node in g.nodes_entered:
		print "\tnode_" + node['label'] + "_Param input_" + node['label'] + ";"

	print "#endif /* PIL2SWARM */"


def process_pil_enter_arguments(node):
	"""Output the appropriate code to enter a node"""

	#print "#ifdef PIL2C"
	args = g.args[g.functions[node['label']]]
	for arg_name in args['list']:
		if arg_name == g.target_variables[node['label']]:
			continue
		if g.args[g.functions[node['label']]]['dict'][arg_name]['passed_by_reference']:
			print "\t\t\t" + arg_name + "_ptr = va_arg(argptr, " + g.variables[arg_name] + " *);"
			print "\t\t\tinput_" + node['label'] + "." + arg_name + " = *" + arg_name + "_ptr;"
		else:
			print "\t\t\tinput_" + node['label'] + "." + arg_name + " = va_arg(argptr, " + g.variables[arg_name] + ");"
		print "\t\t\tglobal." + arg_name + " = input_" + node['label'] + "." + arg_name + ";"
	#print "#endif"
	print "\t\t\tbreak;"


def process_pil_enter(nodes, node):
	"""Output the appropriate code to enter a node"""

	lm = re.match("\d+", g.intervals[node['label']]['lower'])
	if lm:
		interval_init = g.intervals[node['label']]['lower'] + ", "
	else:
		interval_init = "input_" + node['label'] + "." + g.intervals[node['label']]['lower'] + ", "
	sm = re.match("\d+", g.intervals[node['label']]['step'])
	if sm:
		interval_init += g.intervals[node['label']]['step'] + ", "
	else:
		interval_init += "input_" + node['label'] + "." + g.intervals[node['label']]['step'] + ", "
	um = re.match("\d+", g.intervals[node['label']]['upper'])
	if um:
		interval_init += g.intervals[node['label']]['upper']
	else:
		if g.intervals[node['label']]['upper'] in g.args[g.functions[node['label']]]['list']:
			interval_init += "input_" + node['label'] + "." + g.intervals[node['label']]['upper']
		else:
			interval_init += "input->_global->" + g.intervals[node['label']]['upper']
	init = interval_init + ", global.index_array, global.data_array"

	print "#ifdef PIL2SWARM"
	print "\t\t\t// [lower, step, upper] = [" + interval_init + "]"
	print "\t\t\tnode_" + node['label'] + "_Param_init(&input_"+ node['label'] +", &global, " + init + ");"

	lower = 0
	step = 0
	upper = 0

	for n in nodes:
		if n['label'] == node['label']:
			try:
				int(n['lower'])
				lower = n['lower']
			except ValueError:
				lower = "input_" + node['label'] + "." + n['lower']

			try:
				int(n['step'])
				step = n['step']
			except ValueError:
				step = "input_" + node['label'] + "." + n['step']

			try:
				int(n['upper'])
				upper = n['upper']
			except ValueError:
				upper = "input_" + node['label'] + "." + n['upper']

	print "\t\t\tif (input_" + node['label'] + ".index_array.guid == NULL_GUID) {"
	print "\t\t\t\tuint64_t _pil_num_iter = 0;"
	print "\t\t\t\t_pil_num_iter = (" + upper + " - " + lower + ") / " + step + " + 1;"
	print "\t\t\t\tpil_alloc(&input_" + node['label'] + ".index_array, (_pil_num_iter+1) * sizeof(int));"
	print "\t\t\t\tint *_pil_ia = (int *) input_" + node['label'] + ".index_array.ptr;"
	print "\t\t\t\tint _pil_index;"
	print "\t\t\t\tfor (_pil_index = 0; _pil_index <= _pil_num_iter; _pil_index++) {"
	print "\t\t\t\t\t_pil_ia[_pil_index] = 0;"
	print "\t\t\t\t}"
	print "\t\t\t}"


	print "\t\t\tnode_" + node['label'] + ".swarm_enter(&input_"+ node['label'] +");"
	print ""

	args = g.args[g.functions[node['label']]]
	for arg_name in args['list']:
		if arg_name == g.target_variables[node['label']]:
			continue
		if g.args[g.functions[node['label']]]['dict'][arg_name]['passed_by_reference']:
			print "\t\t\t*" + arg_name + "_ptr = global." + arg_name + ";"
	print "\t\t\tbreak;"
	print "#endif /* PIL2SWARM */"


def process_main():
	"""process_main - outputs the main function for the file."""

	init = g.main_node_input[0] + ", " + g.main_node_input[1] + ", " + g.main_node_input[2] + ", global.index_array, global.data_array"

	if g.FILE_TYPE in g.CHPL:
		print "void chpl_main()"
	else:
		print "int main(int argc, char **argv)"
	print "{"
	print "\t// Place to store on the stack all of the global parameters"
	print "\t//global_Param global;"
	print "\t//global_Param_init();"
	run.set_stack_size()
	set_num_threads()
	print "\tglobal.index_array.guid = NULL_GUID;"
	print "\tglobal.index_array.ptr = NULL;"
	print "\tglobal.data_array.guid = NULL_GUID;"
	print "\tglobal.data_array.ptr = NULL;"
	print ""

	if g.MEM == "forkjoin" or g.MEM == "spmd":
		print "\t// Starts SWARM and the transport layer, and leaves them running."
		print "\tstartup.swarm_enter();"
	print ""
	if g.PIL_MAIN:
		print "\tpil_main(argc, argv);"
	else:
		print "\t// Initializing node_" + g.main_node_label + " input parameter"
		print "\tnode_" + g.main_node_label + "_Param input;"
		print "\tnode_" + g.main_node_label + "_Param_init(&input, &global, " + init + "); // [lower, step, upper] = [" + init + "]"
		if "argc" in g.args[g.functions[g.main_node_label]]['list']:
			print "\tinput.argc = argc;"
		if "argv" in g.args[g.functions[g.main_node_label]]['list']:
			print "\tinput.argv = argv;"
		enter_swarm()
	print ""
	#if g.MEM == "forkjoin" or g.MEM == "spmd" and g.PIL_MAIN == "True":
	if g.MEM == "forkjoin" or g.MEM == "spmd":
		print "\t// Transport layer must be shut down or process may hang."
		print "\tshutdown.swarm_enter();"
	print ""
	if g.FILE_TYPE in g.CHPL:
		print "\treturn;"
	else:
		print "\treturn EXIT_SUCCESS;"
	print "}"
	print ""
	print "//#pragma swarm implement swarm2c_ProcedureReturn"
	print ""


# TODO: should this be broken into two functions?
# TODO: do i actually need to pass the values through?
def swarm_process_type(label, index, lower, step, upper, func_name, in_args):
	"""processes a type"""

	sys.stdout.write("//--------------------------------------\n")
	sys.stdout.write("// types for node_" + label + "\n")
	sys.stdout.write("//--------------------------------------\n")
	sys.stdout.write("\n")

	# output the swarm type for the node
	sys.stdout.write("// swarm_type for the node_" + label + " function\n")
	sys.stdout.write("swarm_type node_" + label + "_Param// : swarm_Any_t\n")
	sys.stdout.write("{\n")
	# TODO: these names could clash with user defined variable names. what do we do about that?
	sys.stdout.write("\tglobal_Param *_global;\n")
	sys.stdout.write("\tswarm_int32_t " + g.LOWER + "; // initial: " + lower + "\n")
	sys.stdout.write("\tswarm_int32_t " + g.STEP + "; // initial: " + step + "\n")
	sys.stdout.write("\tswarm_int32_t " + g.UPPER + "; // initial: " + upper + "\n")
	sys.stdout.write("\n")

	if index not in g.args[func_name]['list']:
		sys.stdout.write("\t" + g.variables[index] + " " + index + ";\n")
	for arg_name in g.args[func_name]['list']:
		sys.stdout.write("\t" + g.variables[arg_name] + " " + arg_name + ";\n")

	sys.stdout.write("};\n")
	sys.stdout.write("\n")

	# output the swarm type for the function inside the node
	sys.stdout.write("// swarm_type for the function " + func_name + "\n")
	sys.stdout.write("swarm_type " + func_name + "_Param// : swarm_Any_t\n")
	sys.stdout.write("{\n")
	sys.stdout.write("\tnode_" + label + "_Context *context;\n")
	sys.stdout.write("\n")
	sys.stdout.write("\t" + "// in_args: " + in_args + ";\n")

	for arg_name in g.args[func_name]['list']:
		sys.stdout.write("\t" + g.variables[arg_name] + " " + arg_name + ";\n")
	sys.stdout.write("\tint _pil_iter;\n")

	sys.stdout.write("};\n")
	sys.stdout.write("\n")


def swarm_process_type_pragmas(label, func_name):
	"""output the swarm implement pragmas for the swarm types"""

	# output the swarm implement pragmas for each type
	print "#pragma swarm implement node_" + label + "_Param"
	print "#pragma swarm implement " + func_name + "_Param"


def swarm_process_network_registered_codelets(nodes):
	"""output the registered codelets for the program"""

	# must start at 1 since in pil_nw.h we have to devine GRAPH_BARRIER_0 = 0
	rc = 1 # see comment above
	print "//registered codelets and barriers"
	for node in nodes:
		print "barrierID_t const GRAPH_BARRIER_" + node['label'] + " = " + str(rc) + ";"
		rc += 1
		if g.MEM == "forkjoin":
			print "registered_codelet_t const PROC_NODE_" + node['label'] + " = " + str(rc) + ";"
			rc += 1
			print "registered_codelet_t const SATISFY_" + node['label'] + " = " + str(rc) + ";"
			rc += 1
		if g.MEM == "forkjoin" or g.MEM == "spmd":
			print "registered_codelet_t const SWITCH_" + node['label'] + " = " + str(rc) + ";"
			rc += 1
		if g.MEM == "spmd":
			n = None
			for l in g.nodes:
				run.debug(4, l)
				if g.nodes[l]['func_name'] == node['func_name']:
					nodelet_label = l
					n = g.nodes[l]
			if n and swarm_has_nw_recv(node['func_name']):
				print "registered_codelet_t const NODELET_" + node['label'] + " = " + str(rc) + ";"
				rc += 1
	print ""


def swarm_process_network_function_prototypes():
	"""output the prototypes for starting up and shutting down the network"""

	print "// network function prototypes"
	print "swarm_procedure void startup();"
	print "swarm_procedure void shutdown();"
	print ""


def swarm_process_function_prototype(label, func_name):
	"""process func prototype"""

	sys.stdout.write("//--------------------------------------\n")
	sys.stdout.write("// function prototypes for node_" + label + "\n")
	sys.stdout.write("//--------------------------------------\n")
	sys.stdout.write("\n")

	sys.stdout.write("swarm_procedure void node_" + label + "(node_" + label + "_Param *input);\n")
	sys.stdout.write("swarm_procedure void " + func_name + "_swarm_p(" + func_name + "_Param *input);\n")
	sys.stdout.write("\n")


def swarm_process_global_param_init():
	"""generate the parameter init function for the global parameter"""

	args = ""
	for var in g.variables:
		if var in g.initial_values:
			args += ", " + g.variables[var] + " " + var
	print "static void global_Param_init(global_Param *inst" + args + ")"
	print "{"
	print "\t// TODO: what does this line mean?"
	#print "\tinst->swarm_tptrs = global_Param_getType()->instTptrs;"
	print "\tinst->swarm_tptrs = global_Param_TYPE.instTptrs;"
	print ""
	for var in g.variables:
		if var in g.initial_values:
			print "\tinst->" + var + " = " + var + ";"
	print "}"
	print ""


def swarm_process_node_param_init(label, func_name):
	"""generate the parameter init function for the node"""

	print "static void node_" + label + "_Param_init(node_" + label + "_Param *inst, global_Param *g, swarm_int32_t l, swarm_int32_t s, swarm_int32_t u, gpp_t index_array, gpp_t data_array)"
	print "{"
	print "\t//TODO: what do these two lines mean?"
	#print "\tinst->swarm_tptrs = node_" + label + "_Param_getType()->instTptrs;"
	print "\tinst->swarm_tptrs = node_" + label + "_Param_TYPE.instTptrs;"
	#print "\tswarm_Any_initSuper(&inst->super_Any, node_" + label + "_Param_TYPE.instTptrs + _node_" + label + "_Param_TPTR_OFFS_swarm_Any);"
	print ""
	print "\tinst->_global = g;"
	print "\tinst->" + g.LOWER + " = l;"
	print "\tinst->" + g.STEP + " = s;"
	print "\tinst->" + g.UPPER + " = u;"
	print "\tinst->index_array = index_array;"
	print "\tinst->data_array = data_array;"
	print "}"
	print ""


def swarm_process_func_param_init(func_name, label):
	"""generate the parameter init function for the function in the node"""

	# find the input arguments
	in_args = ""
	for arg_name in g.args[func_name]['list']:
		in_args += ", " + g.variables[arg_name] + " " + arg_name

	print "static void " + func_name + "_Param_init(" + func_name + "_Param *inst, node_" + label + "_Context *_pil_context" + in_args + ", int _pil_iter)"
	print "{"
	print "\t//TODO: what do these two lines mean?"
	run.file_debug(2, '\tfprintf(stdout, "initializing the unknown line 1\\n"); fflush(stdout);')
	#print "\tinst->swarm_tptrs = " + func_name + "_Param_getType()->instTptrs;"
	print "\tinst->swarm_tptrs = " + func_name + "_Param_TYPE.instTptrs;"
	#run.file_debug(2, '\tfprintf(stdout, "initializing the unknown line 2\\n"); fflush(stdout);')
	#print "\tswarm_Any_initSuper(&inst->super_Any, " + func_name + "_Param_TYPE.instTptrs+1);"
	print ""
	print "\t// set the context"
	run.file_debug(2, '\tfprintf(stdout, "initializing context _pil_context\\n"); fflush(stdout);')
	print "\tinst->context = _pil_context;"
	print ""

	print "\t// initialize each input parameter"
	for arg_name in g.args[func_name]['list']:
		run.file_debug(2, '\tfprintf(stdout, "initializing param ' + arg_name + '\\n"); fflush(stdout);')
		print "\tinst->" + arg_name + " = " + arg_name + ";"
	print "\tinst->_pil_iter = _pil_iter;"

	print "}"
	print ""


def swarm_process_param_init(label, lower, step, upper, func_name, in_args):
	"""process param init"""

	print "//--------------------------------------"
	print "// init functions for node_" + label
	print "//--------------------------------------"
	print ""

	swarm_process_node_param_init(label, func_name)
	swarm_process_func_param_init(func_name, label)


def swarm_process_network_procedures():
	"""output the network swarm procedures"""

	print "//--------------------------------------"
	print "// network procedures"
	print "//--------------------------------------"
	print ""
	print "swarm_procedure void startup()"
	print "{"
	print "\tswarm_codelet entry()"
	print "\t{"
	print "\t\tswarm_transport_startup(NULL, swarm_cargs(done));"
	print "\t}"
	print ""
	print "\tswarm_codelet done()"
	print "\t{"
	print "\t\tswarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"
	print "\t\tswarm_return;"
	print "\t}"
	print "}"
	print ""
	print "swarm_procedure void shutdown()"
	print "{"
	print "\tswarm_codelet entry()"
	print "\t{"
	print "\t\tswarm_transport_shutdown(swarm_cargs(done));"
	print "\t}"
	print ""
	print "\tswarm_codelet done()"
	print "\t{"
	print "\t\tswarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"
	print "\t\tswarm_return;"
	print "\t}"
	print "}"
	print ""


nw_send_string = "(\s*)pil_send\s*\(\s*(\w+)\s*,\s*([&]{0,1})\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*\)\s*;.*"
	# s.group(1) - whitespace for matching indent
	# s.group(2) - destination
	# s.group(3) - modifier
	# s.group(4) - pointer
	# s.group(5) - size
	# s.group(6) - continuation
def swarm_print_nw_send(line, label):
	"""output network send code"""

	s = re.match(nw_send_string, line)

	if s:
		space = s.group(1)
		dest = s.group(2)
		modifier = s.group(3)
		pointer = s.group(4)
		size = s.group(5)
		continuation = s.group(6)

		print "//\t" + line,
		print "\t" + space + "nw_NetBuffer_t *nb;"
		print "\t" + space + "nb = nw_NetBuffer_new(NULL, " + size + ");"
		print "\t" + space + "memcpy(nb->p, " + modifier + pointer + ", " + size + ");"
		print "\t" + space + "nw_call(" + dest + ", NODELET_" + label + ", nw_NetBuffer_to_swarm_Transferable(nb), NULL, NULL);"

		return dest
	return False


def swarm_is_nw_send(line):
	"""check to see if line is a pil_send call."""

	s = re.match(nw_send_string, line)
	if s:
		return True
	return False


def swarm_has_nw_recv(func_name):
	"""check to see if the node has a pil_recv call"""

	if func_name in g.nw_calls.keys():
		r = re.match(g.NW_RECV_RE, g.nw_calls[func_name])
		if r:
			return True
	return False


def swarm_get_recv_src(func_name):
	"""return the source of a network recv call"""

	r = re.match(g.NW_RECV_RE, g.nw_calls[func_name])
	if r:
		space = r.group(1)
		source = r.group(2)
		modifier = r.group(3)
		pointer = r.group(4)
		size = r.group(5)
		continuation = r.group(6)
		return source


def swarm_print_nw_call(func_name):
	"""output the network call for the node"""

	r = re.match(g.NW_RECV_RE, g.nw_calls[func_name])
	if r:
		space = r.group(1)
		source = r.group(2)
		modifier = r.group(3)
		pointer = r.group(4)
		size = r.group(5)
		continuation = r.group(6)
		run.debug(4, "pil_recv")
		print "\t\tnw_NetBuffer_t *buf = swarm_Transferable_to_nw_NetBuffer(swarm_input);"
		print "\t\tmemcpy(&num, buf->p, " + size + ");"


def swarm_process_node_procedure(label, index, func_name, key):
	"""process node procedure"""

	# TODO: make this multiple procedures?

	print "//--------------------------------------"
	print "// main swarm_procedure for " + label
	print "//--------------------------------------"
	print ""

	#--------------------------------------------------------------------------
	# prototype
	#--------------------------------------------------------------------------
	print "swarm_procedure void node_" + label + "(node_" + label + "_Param *input)"
	print "{"

	#--------------------------------------------------------------------------
	# parameters
	#--------------------------------------------------------------------------
	node = None
	for l in g.nodes:
		run.debug(4, l)
		if g.nodes[l]['func_name'] == func_name:
			nodelet_label = l
			node = g.nodes[l]

	if g.MEM == "shared" or g.MEM == "forkjoin":
		print "\tswarm_Dep_t dep;"
	if g.MEM == "shared":
		print "\t" + func_name + "_Param *" + func_name + "_param;"
	elif g.MEM == "spmd":
		print "\t" + func_name + "_Param " + func_name + "_param;"

	for l in g.targets[label]:
		if l != "0":
			print "\tnode_" + l + "_Param node_" + l + "_param;"
	print ""

	if node:
		print "\t// _pil_context variables"
		for var in g.context_variables[nodelet_label]:
			print "\t" + var['type'] + " " + var['name'] + ";"
		print "\t// function parameters"
		#if index not in g.args[func_name]['list']:
		#	sys.stdout.write("\t" + g.variables[index] + " " + index + ";\n")
		for arg_name in g.args[func_name]['list']:
			line = "\t" + g.variables[arg_name] + " "
			if g.args[func_name]['dict'][arg_name]['passed_by_reference']:
				line += "*"
			line += arg_name + ";\n"
			print line
		print ""


	#--------------------------------------------------------------------------
	# node entry
	#--------------------------------------------------------------------------
	print "\tswarm_codelet entry()"
	print "\t{"
	#run.file_debug(1, '\t\tfprintf(stdout, "node_' + label + '.entry(num: %d)\\n", num); fflush(stdout);')
	run.file_debug(1, '\t\tfprintf(stdout, "node_' + label + '.entry()\\n"); fflush(stdout);')

	if g.MEM == "forkjoin":
		print "\t\tnw_registerAction(swarm_cargs(node_" + label + "_body), PROC_NODE_" + label + ", &nw_NetBuffer_TYPE, \"\");"
		print "\t\tnw_registerAction(swarm_cargs(node_" + label + "_satisfy), SATISFY_" + label + ", &nw_NetBuffer_TYPE, \"\");"
	if g.MEM == "forkjoin" or g.MEM == "spmd":
		print "\t\tnw_registerAction(swarm_cargs(node_" + label + "_switch), SWITCH_" + label + ", &nw_NetBuffer_TYPE, \"\");"
		if node and swarm_has_nw_recv(func_name):
			print "\t\tnw_registerAction(swarm_cargs(_pil_recv), NODELET_" + label + ", &nw_NetBuffer_TYPE, \"\");"
		print "\t\tnw_barrierAll(GRAPH_BARRIER_" + label + ", swarm_cargs(main));"
		print "\t}"
		print ""
		#----------------------------------------------------------------------
		# node main
		#----------------------------------------------------------------------
		print "\tswarm_codelet main()"
		print "\t{"
		run.file_debug(1, '\t\tfprintf(stdout, "node_' + label + '.main\\n"); fflush(stdout);')

	# TODO: rename these so they can't clash with variable names in the user code
	print "\t\tswarm_nat32_t num;"

	if node:
		print "\t\t// init function parameters"
		for arg_name in g.args[func_name]['list']:
			line = "\t\t" + arg_name + " = "
			if g.args[func_name]['dict'][arg_name]['passed_by_reference']:
				line += "&"
			line += func_name + "_param." + arg_name + ";"
			print line

	if g.MEM == "forkjoin":
		print "\t\tif (nw_getNodeID() == 0)"
		print "\t\t{"

	print "\t\tif (input->" + g.STEP + " < 0)"
	print "\t\t{"
	print "\t\t\tnum = (input->" + g.UPPER + " - input->" + g.LOWER + ")/(0 - input->" + g.STEP + ") + 1;"
	print "\t\t}"
	print "\t\telse"
	print "\t\t{"
	print "\t\t\tnum = (input->" + g.UPPER + " - input->" + g.LOWER + ")/input->" + g.STEP + " + 1;"
	print "\t\t}"

	if g.MEM == "forkjoin":
		print "\t\tdep.requires(num+1) => node_" + label + "_bcast;"
	elif g.MEM == "shared":
		print "\t\tdep.requires(num+1) => node_" + label + "_switch;"
	print ""

	if g.MEM == "shared":
		if g.FILE_TYPE in g.CHPL:
			print "\t\t" + func_name + "_param = chpl_malloc(num, sizeof(" + func_name + "_Param), 0, 0, NULL);"
		else:
			print "\t\t" + func_name + "_param = malloc(num * sizeof(" + func_name + "_Param));"
	run.file_debug(1, "\t\tfprintf(stdout, \"lower: %d  upper: %d  step: %d\\n\", input->" + g.LOWER + ", input->" + g.UPPER + ", input->" + g.STEP + "); fflush(stdout);")

	in_args = ""
	for arg_name in g.args[func_name]['list']:
		in_args += ", input->" + arg_name
	if g.MEM == "spmd":
			print "\t\tif (pil_get_nwID() < num)"
			print "\t\t{"
			run.file_debug(1, '\t\t\tfprintf(stdout, "' + func_name + '_Param_init call\\n");')
			print "\t\t\t" + func_name + "_Param_init(&" + func_name + "_param, swarm_context" + in_args + ", _pil_iter);"
			print "\t\t\tdo => node_" + label + "_body();"
			print "\t\t}"
			print "\t\telse"
			print "\t\t{"
			print "\t\t\tdo => node_" + label + "_bar;"
			print "\t\t}"
	else:
		print "\t\tint _pil_iter = 0;"
		#----------------------------------------
		# step < 0
		#----------------------------------------
		print "\t\tif (input->" + g.STEP + " < 0)"
		print "\t\t{"
		print "\t\t\tfor (input->" + index + " = input->" + g.UPPER + "; input->" + index + " >= input->" + g.LOWER + "; input->" + index + " += input->" + g.STEP + ")"
		print "\t\t\t{"
		if g.MEM == "forkjoin":
			print "\t\t\t\tnw_NetBuffer_t *nb;"
			print "\t\t\t\tnb = nw_NetBuffer_new(NULL, sizeof(" + func_name + "_Param));"
			print "\t\t\t\t" + func_name + "_Param *param = (" + func_name + "_Param *)nb->p;"
			print "\t\t\t\t" + func_name + "_Param_init(param, swarm_context" + in_args + ", _pil_iter);"
			run.file_debug(1, '\t\t\t\tfprintf(stdout, "scheduling on index: %d\\n", input->' + index + '); fflush(stdout);')
			print "\t\t\t\t//nw_call(input->" + index + ", PROC_NODE_" + label + ", nw_NetBuffer_to_swarm_Transferable(nb), NULL, NULL);"
			print "\t\t\t\tnw_call(_pil_iter, PROC_NODE_" + label + ", nw_NetBuffer_to_swarm_Transferable(nb), NULL, NULL);"
			print "\t\t\t\t_pil_iter += 1;"
		else:
			run.file_debug(1, '\t\t\t\tfprintf(stdout, "' + func_name + '_Param_init call\\n");')
			print "\t\t\t\tint _pil_iter = (input->" + index + " - input->" + g.LOWER + ")/(0 - input->" + g.STEP + ");"
			print "\t\t\t\t" + func_name + "_Param_init(&" + func_name + "_param[_pil_iter], swarm_context" + in_args + ", _pil_iter);"
			print "\t\t\t\tdo => node_" + label + "_body(&" + func_name + "_param[_pil_iter]);"
		print "\t\t\t}"
		print "\t\t}"
		#----------------------------------------
		# step > 0
		#----------------------------------------
		print "\t\telse"
		print "\t\t{"
		print "\t\t\tfor (input->" + index + " = input->" + g.LOWER + "; input->" + index + " <= input->" + g.UPPER + "; input->" + index + " += input->" + g.STEP + ")"
		print "\t\t\t{"
		if g.MEM == "forkjoin":
			print "\t\t\t\tnw_NetBuffer_t *nb;"
			print "\t\t\t\tnb = nw_NetBuffer_new(NULL, sizeof(" + func_name + "_Param));"
			print "\t\t\t\t" + func_name + "_Param *param = (" + func_name + "_Param *)nb->p;"
			print "\t\t\t\t" + func_name + "_Param_init(param, swarm_context" + in_args + ", _pil_iter);"
			run.file_debug(1, '\t\t\t\tfprintf(stdout, "scheduling on index: %d\\n", input->' + index + '); fflush(stdout);')
			print "\t\t\t\t//nw_call(input->" + index + ", PROC_NODE_" + label + ", nw_NetBuffer_to_swarm_Transferable(nb), NULL, NULL);"
			print "\t\t\t\tnw_call(_pil_iter, PROC_NODE_" + label + ", nw_NetBuffer_to_swarm_Transferable(nb), NULL, NULL);"
			print "\t\t\t\t_pil_iter += 1;"
		else:
			run.file_debug(1, '\t\t\t\tfprintf(stdout, "' + func_name + '_Param_init call\\n");')
			print "\t\t\t\tint _pil_iter = (input->" + index + " - input->" + g.LOWER + ")/input->" + g.STEP + ";"
			print "\t\t\t\t" + func_name + "_Param_init(&" + func_name + "_param[_pil_iter], swarm_context" + in_args + ", _pil_iter);"
			print "\t\t\t\tdo => node_" + label + "_body(&" + func_name + "_param[_pil_iter]);"
		print "\t\t\t}"
		print "\t\t}"

	if g.MEM == "shared" or g.MEM == "forkjoin":
		print "\t\tswarm_Dep_satisfy(&dep, 1U);"

	if g.MEM == "forkjoin":
		print "\t\t}"

	if g.MEM == "forkjoin" or g.MEM == "spmd":
		print "\t\tswarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"

	print "\t}"
	print ""

	#--------------------------------------------------------------------------
	# node body
	#--------------------------------------------------------------------------
	if g.MEM == "forkjoin" or g.MEM == "spmd":
		print "\tswarm_codelet node_" + label + "_body()"
	else:
		print "\tswarm_codelet node_" + label + "_body(" + func_name +"_Param *pil_p)"
	#print "\tswarm_codelet node_" + label + "_body()"
	print "\t{"

	run.file_debug(1, '\t\tfprintf(stdout, "node_' + label + '.node_' + label + '()\\n"); fflush(stdout);')

	if g.MEM == "forkjoin":
		print "\t\tnw_NetBuffer_t *buf = swarm_Transferable_to_nw_NetBuffer(swarm_input);"
		print "\t\t" + func_name + "_Param *pil_p = buf->p;"
		print "\t\t//" + func_name + "_param = pil_p;"

	print "\t\tgpp_t index_array;"
	print "\t\tgpp_t data_array;"
	print "\t\tif (pil_p->index_array.guid != NULL_GUID) {"
	print "\t\t\tint *_pil_gia = (int *)pil_p->index_array.ptr;"
	print "\t\t\tgpp_t *_pil_gda = (gpp_t *)pil_p->data_array.ptr;"
	print "\t\t\tpil_alloc(&index_array, 2*sizeof(int));"
	print "\t\t\tint *_pil_lia = (int*) index_array.ptr;"
	print "\t\t\tint _pil_num = _pil_gia[pil_p->_pil_iter+1] - _pil_gia[pil_p->_pil_iter];"
	print "\t\t\t_pil_lia[0] = 0;"
	print "\t\t\t_pil_lia[1] = _pil_num;"
	print ""

	print "\t\t\tsize_t _pil_data_size = _pil_num * sizeof(gpp_t);"
	print "\t\t\tpil_alloc(&data_array, _pil_data_size);"
	print "\t\t\tgpp_t *_pil_lda = (gpp_t *) data_array.ptr;"
	print "\t\t\tmemcpy(_pil_lda, &_pil_gda[_pil_gia[pil_p->_pil_iter]], _pil_data_size);"
	print ""

	print "\t\t\tint _pil_index;"
	print "\t\t\tfor (_pil_index = 0; _pil_index < _pil_num; _pil_index++) {"
	print "\t\t\t\t_pil_lda[_pil_index].ptr = pil_mem(_pil_lda[_pil_index].guid);"
	print "\t\t\t}"
	print "\t\t}"
	print ""

	if g.MEM != "spmd":
# taken from below
		#print "\t\t" + func_name + "(pil_p->in_args, pil_p->out_args);"
		first = True
		in_args = ""
		for arg_name in g.args[func_name]['list']:
			if first:
				first = False
			else:
				in_args += ", "

			if arg_name == "index_array" or arg_name == "data_array":
				in_args += arg_name
				continue
	
			# check to see if we should pass the argument by reference or by value
			in_args += "(" + g.variables[arg_name]
			if g.args[func_name]['dict'][arg_name]['passed_by_reference']:
				in_args += " *)&"
			else:
				in_args += ")"
			if g.MEM == "forkjoin" or g.MEM == "shared":
				in_args += "pil_p->" + arg_name
			elif g.MEM == "spmd":
				in_args += func_name + "_param." + arg_name
	
		print "\t\t" + func_name + "(" + in_args + ");"
	else:
		if node:
			lines = g.func_lines[g.nodes[nodelet_label]['func_name']][2:-2] # strip off prototype and { as well as closing }
			print "\t\t// node contents"
			dest = ""
			for line in lines:
				if swarm_is_nw_send(line):
					dest = swarm_print_nw_send(line, label)
				else:
					print "\t",
					print line,
			print "\t\t// end node contents"

	run.file_debug(1, '\t\tfprintf(stdout, "leaving swarm_procedure ' + func_name + '_swarm_p\\n"); fflush(stdout);')
	print "\t\t// TODO: figure out a way to do this without _global"
	for arg_name in g.args[func_name]['list']:
		if g.args[func_name]['dict'][arg_name]['passed_by_reference']:
			#print "\t\tswarm_context->input->_global->" + arg_name + " = input->" + arg_name + ";"
			if g.MEM == "forkjoin":
				print "\t\tinput->_global->" + arg_name + " = pil_p->" + arg_name + ";"
			else:
				if g.MEM == "spmd":
					print "\t\tinput->_global->" + arg_name + " = " + func_name + "_param." + arg_name + ";"
				else:
					print "\t\tinput->_global->" + arg_name + " = pil_p->" + arg_name + ";"
# end taken from below

	if func_name not in g.nodes:
		if g.MEM == "forkjoin":
			print "\t\tnw_call(0, SATISFY_" + label + ", NULL, NULL, NULL);"
		elif g.MEM == "spmd":
			print "\t\tdo => node_" + label + "_bar();"
		else:
			#print "\t\tdo => node_" + label + "_satisfy;"
			print "\t\tswarm_Dep_satisfy(&dep, 1U);"
	else:
		if swarm_has_nw_recv(func_name):
			src = swarm_get_recv_src(func_name)
			#if dest:
			if src:
				print "\t\tif (pil_get_nwID() != " + src + ")"
				print "\t\t{"
				print "\t\t\tdo => node_" + label + "_bar();"
				print "\t\t}"
			#if dest and not swarm_has_nw_recv(func_name):
			#	print "\t\tdo => node_" + label + "_bar();"
			else:
				if func_name in g.nw_calls.keys():
					print "\t\t" + g.nw_calls[func_name]
				else:
					print "\t\tdo => node_" + label + "_bar();"
	if g.MEM == "forkjoin" or g.MEM == "spmd":
		print "\t\tswarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"
	print "\t}"
	print ""

	#--------------------------------------------------------------------------
	# body nodelets
	#--------------------------------------------------------------------------

#	for line in g.pil_main_lines:
#		print line,
#	for func in g.func_lines.keys():
#		for line in g.func_lines[func]:
#			print line,

	if node and nodelet_label in g.nodelets.keys():
		print "\t// nodelet"
		print "\tswarm_codelet _pil_" + g.nodelets[node['label']]['func_name'] + "()"
		print "\t{"
		run.file_debug(1, '\t\tprintf("%d: _pil_' + g.nodelets[node['label']]['func_name'] + '()\\n", pil_get_nwID()); fflush(stdout);')
		swarm_print_nw_call(func_name)
		#print "\t\t" + g.nodelets[f] + "(" + in_args + ");"
		#print g.func_lines[node['func_name']][2:-1]
		lines = g.func_lines[g.nodelets[nodelet_label]['func_name']][2:-2] # strip off prototype and { as well as closing }
		print "\t\t// nodelet contents"
		for line in lines:
			print "\t",
			print line,
		print "\t\t// end nodelet contents"
		print "\t\tinput->_global->target_id = f_param.target_id;"
		print "\t\tdo => node_" + label + "_bar();"
		if g.MEM == "forkjoin" or g.MEM == "spmd":
			print "\t\tswarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"
		print "\t}"
		print ""

	#--------------------------------------------------------------------------
	# node satisfy
	#--------------------------------------------------------------------------
	#if g.MEM == "shared" or g.MEM == "forkjoin":
	if g.MEM == "forkjoin":
		print "\tswarm_codelet node_" + label + "_satisfy()"
		print "\t{"
		run.file_debug(1, '\t\tfprintf(stdout, "node_' + label + ' satisfy\\n"); fflush(stdout);')
		print "\t\tswarm_Dep_satisfy(&dep, 1U);"
		if g.MEM == "forkjoin" or g.MEM == "spmd":
			print "\t\tswarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"
		print "\t}"
		print ""

	#--------------------------------------------------------------------------
	# node bar
	#--------------------------------------------------------------------------
	if g.MEM == "spmd":
		print "\tswarm_codelet node_" + label + "_bar()"
		print "\t{"
		run.file_debug(1, '\t\tfprintf(stdout, "%d: node_' + label + '_bar()\\n", pil_get_nwID()); fflush(stdout);')
		print "\t\tswarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"
		#print "\t\tnw_barrierAll(GRAPH_BARRIER_" + label + ", swarm_cargs(node_" + label + "_switch));"
		print "\t\tnw_barrierAll(GRAPH_BARRIER_" + label + ", swarm_cargs(node_" + label + "_bcast));"
		run.file_debug(1, '\t\tfprintf(stdout, "%d: leaving bar codelet\\n", pil_get_nwID()); fflush(stdout);')
		print "\t}"
		print ""

	#--------------------------------------------------------------------------
	# node bcast
	#--------------------------------------------------------------------------
	if g.MEM == "forkjoin" or g.MEM == "spmd":
		print "\tswarm_codelet node_" + label + "_bcast()"
		print "\t{"
		run.file_debug(1, '\t\tfprintf(stdout, "node_' + label + '_bcast()\\n"); fflush(stdout);')
		print "\t\tif (pil_get_nwID() == 0)"
		print "\t\t{"
		print "\t\t\tint i;"
		print "\t\t\t//printf(\"num_nodes: %d\\n\", nw_getNodeCount());"
		print "\t\t\t// schedule a node_" + label + "_switch codelet on each node."
		print "\t\t\tnw_NetBuffer_t *nb;"
		print "\t\t\tnb = nw_NetBuffer_new(NULL, sizeof(int));"
		print "\t\t\tint *next = (int *)nb->p;"
		print "\t\t\t*next = input->_global->" + key + ";"
		print "\t\t\tnw_callAll(SWITCH_" + label + ", nw_NetBuffer_to_swarm_Transferable(nb), NULL, NULL);"
		print "\t\t}"
		print "\t\tswarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"
		print "\t}"
		print ""

	#--------------------------------------------------------------------------
	# node switch
	#--------------------------------------------------------------------------
	print "\tswarm_codelet node_" + label + "_switch()"
	print "\t{"

	print "\n\t\t// TODO: merge index_array and data_array\n"

	if g.MEM == "forkjoin" or g.MEM == "spmd":
		print "\t\tnw_NetBuffer_t *buf = swarm_Transferable_to_nw_NetBuffer(swarm_input);"
		print "\t\tint *next = (int*)buf->p;"

	run.file_debug(1, '\t\tfprintf(stdout, "node_' + label + '.node_' + label + '_switch()\\n"); fflush(stdout);')
	#print "\t\tswitch (" + func_name + "_param->" + key + ")"
	#if g.MEM == "forkjoin" or g.MEM == "spmd":
	#	print "\t\tswitch (input->" + key + ")"
	#else:
	#	print "\t\tswitch (input->_global->" + key + ")"
	if g.MEM == "forkjoin" or g.MEM == "spmd":
		print "\t\tswitch (*next)"
	else:
		print "\t\tswitch (input->_global->" + key + ")"
	print "\t\t{"

	#----------------------------------------
	# handle switch case
	#----------------------------------------

	# TODO: make the following a function based off of the target label
	# TODO: this assumes that node_0 means exit. should we change it?
	# handle the switch statement
	for l in g.targets[label]:
		print "\t\t\tcase " + l + ":"
		print "\t\t\t{"
		if l == "0":
			print "\t\t\t\tdo => node_" + label + "_exit;"
			print "\t\t\t\tbreak;"
		else:
			lm = re.match("\d+", g.intervals[l]['lower'])
			if lm:
				interval_init = g.intervals[l]['lower'] + ", "
			elif g.MEM == "forkjoin" or g.MEM == "spmd":
				interval_init = "input->_global->" + g.intervals[l]['lower'] + ", "
			else:
				interval_init = func_name + "_param->" + g.intervals[l]['lower'] + ", "
			sm = re.match("\d+", g.intervals[l]['step'])
			if sm:
				interval_init += g.intervals[l]['step'] + ", "
			elif g.MEM == "forkjoin" or g.MEM == "spmd":
				interval_init += "input->_global->" + g.intervals[l]['step'] + ", "
			else:
				interval_init += func_name + "_param->" + g.intervals[l]['step'] + ", "
			um = re.match("\d+", g.intervals[l]['upper'])
			if um:
				interval_init += g.intervals[l]['upper']
			elif g.MEM == "forkjoin" or g.MEM == "spmd":
				interval_init += "input->_global->" + g.intervals[l]['upper']
			else:
				if g.intervals[l]['upper'] in g.args[g.functions[label]]['list']:
					interval_init += func_name + "_param->" + g.intervals[l]['upper']
				else:
					interval_init += "input->_global->" + g.intervals[l]['upper']
			init = interval_init + ", input->_global->index_array, input->_global->data_array"
			print "\t\t\t\t// [lower, step, upper] = [" + interval_init + "]"
			print "\t\t\t\tnode_"+ l + "_Param_init(&node_" + l + "_param, input->_global, " + init + ");"
			for arg_name in g.args[g.functions[l]]['list']:
				if arg_name in g.args[g.functions[label]]['list'] and g.MEM == "shared":
					print "\t\t\t\tnode_" + l + "_param." + arg_name + " = input->_global->" + arg_name + ";"
				else:
					print "\t\t\t\tnode_" + l + "_param." + arg_name + " = input->_global->" + arg_name + ";"
			print "\t\t\t\tnode_"+ l + ".enter(&node_" + l + "_param) => node_" + label + "_exit;"
			print "\t\t\t\tbreak;"
		print "\t\t\t}"

	print "\t\t\tdefault:"
	print "\t\t\t{"
	if g.MEM == "forkjoin":
		print "\t\t\t\tfprintf(stdout, \"ERROR: this should never happen. case: '%d' in node_" + label + "_switch\\n\", *next);"
	else:
		#print "\t\t\t\tfprintf(stdout, \"ERROR: this should never happen. case: '%d' in node_" + label + "_switch\\n\", " + func_name + "_param->" + key + ");"
		print "\t\t\t\tfprintf(stdout, \"ERROR: this should never happen. case: '%d' in node_" + label + "_switch\\n\", input->_global->" + key + ");"
	#print "\t\t\t\tswarm_endProcedure(swarm_context);"
	print "\t\t\t\tswarm_return;"
	print "\t\t\t\tbreak;"
	print "\t\t\t}"
	print "\t\t}"
	if g.MEM == "forkjoin" or g.MEM == "spmd":
		print "\t\tswarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"
	print "\t}"

	#--------------------------------------------------------------------------
	# node exit
	#--------------------------------------------------------------------------
	# print exit codelets for each label
	#for l in g.targets[label]:
	print ""
	print "\tswarm_codelet node_" + label + "_exit()"
	print "\t{"

	run.file_debug(1, '\t\tfprintf(stdout, "node_' + label + '.node_' + label + '_exit()\\n"); fflush(stdout);')

	if g.MEM == "forkjoin" or g.MEM == "spmd":
		print "\t\tswarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"
	else:
		if g.FILE_TYPE in g.CHPL:
			print "\t\tchpl_free(" + func_name + "_param, 0, NULL);"
		else:
			print "\t\tfree(" + func_name + "_param);"
		#print "\t\tswarm_endProcedure(swarm_context);"
	print "\t\tswarm_return;"
	print "\t}"

	print "}"
	print  ""


def swarm_process_swarm_procedure(func_name):
	""" process swarm procedure"""

	return

	print "swarm_procedure void " + func_name + "_swarm_p(" + func_name + "_Param *param)"
	print "{"
	print "\tswarm_codelet entry()"
	print "\t{"
	run.file_debug(1, '\t\tfprintf(stdout, "in swarm_procedure ' + func_name + '_swarm_p\\n");')
	#print "\t\t" + func_name + "(param->in_args, param->out_args);"
	first = True
	in_args = ""
	for arg_name in g.args[func_name]['list']:
		if first:
			first = False
		else:
			in_args += ", "

		# check to see if we should pass the argument by reference or by value
		in_args += "(" + g.variables[arg_name]
		if g.args[func_name]['dict'][arg_name]['passed_by_reference']:
			in_args += " *)&"
		else:
			in_args += ")"
		in_args += "param->" + arg_name

	print "\t\t" + func_name + "(" + in_args + ");"
	run.file_debug(1, '\t\tfprintf(stdout, "leaving swarm_procedure ' + func_name + '_swarm_p\\n");')
	for arg_name in g.args[func_name]['list']:
		if g.args[func_name]['dict'][arg_name]['passed_by_reference']:
			print "\t\tparam->context->input->_global->" + arg_name + " = param->" + arg_name + ";"
	#print "\t\tswarm_endProcedure(swarm_context);"
	print "\t\tswarm_return;"
	print "\t}"
	print "}"
	print ""


def process_variables():
	"""Create a global store for all variables declared for use on shared
	   memory only."""

	sys.stdout.write("//--------------------------------------\n")
	sys.stdout.write("// global parameter\n")
	sys.stdout.write("//--------------------------------------\n")
	sys.stdout.write("\n")
	# output the swarm type for the node
	sys.stdout.write("swarm_type global_Param// : swarm_Any_t\n")
	sys.stdout.write("{\n")
	for var in g.variables:
		sys.stdout.write("\t" + g.variables[var] + " " + var + ";\n")
	sys.stdout.write("};\n")
	sys.stdout.write("\n")


def handle_nodes(nodes):
	"""Output the SWARM code for each node in nodes"""

	sys.stdout.write("// context for each node function\n")
	for m in nodes:
		sys.stdout.write("swarm_type node_" + m['label'] + "_Context;\n")
	sys.stdout.write("\n")

	for m in nodes:
		swarm_process_type(m['label'], m['index'], m['lower'], m['step'], m['upper'], m['func_name'], m['in_args'])

	print "#pragma swarm implement global_Param"
	for m in nodes:
		swarm_process_type_pragmas(m['label'], m['func_name'])
	print ""
	print "// Place to store on the stack all of the global parameters"
	print "global_Param global;\n"

	if g.MEM == "forkjoin" or g.MEM == "spmd":
		swarm_process_network_registered_codelets(nodes)
		swarm_process_network_function_prototypes()
	for m in nodes:
		swarm_process_function_prototype(m['label'], m['func_name'])

	swarm_process_global_param_init()
	for m in nodes:
		swarm_process_param_init(m['label'], m['lower'], m['step'], m['upper'], m['func_name'], m['in_args'])

	if g.MEM == "forkjoin" or g.MEM == "spmd":
		swarm_process_network_procedures()
	for m in nodes:
		swarm_process_node_procedure(m['label'], m['index'], m['func_name'], m['cond'])

	for m in nodes:
		swarm_process_swarm_procedure(m['func_name'])
