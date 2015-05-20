###############################################################################
# pil2swarm
###############################################################################

import run
#from run import file_debug, debug
import g
import sys
import re
import pil2c

#------------------------------------------------------------------------------
# SWARM specific process functions for use by mc
#------------------------------------------------------------------------------

def print_main_func():
	"""output the main function"""

	for line in g.pil_main_lines:
		print line,


def print_funcs():
	"""output the body functions"""

	if g.MEM != "distspmd":
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
	if g.MEM == "distforkjoin" or g.MEM == "distspmd":
		print "#include <swarm/nw_api.h>"
		print ""
		print "barrierID_t const GRAPH_BARRIER_0 = 0;"
	print "#include <pthread.h>"
#	print "#include <semaphore.h>"
	print ""
##	print "int _pil_barrier_counter = 0;"
#	print "sem_t _pil_turnstile;"
#	print "sem_t _pil_turnstile2;"
#	print "sem_t _pil_mutex;"
#	print ""


def set_num_threads():
	"""Set the number of threads in the runtime system"""

	if (g.FILE_TYPE in g.CHPL) and (g.DEBUG_LEVEL > 0):
		print ""
		print "\t//Setting the number of threads for the runtime system"
		run.file_debug(0, '\tprintf("Starting SWARM with ' + g.NUM_THREADS + ' threads.\\n");')
		print "\tswarm_Runtime_params_t _p;"
		print "\tswarm_Runtime_params_init(&_p);"
		print "\t_p.maxThreadCount = " + g.NUM_THREADS + ";"


def enter_swarm(tabs, label):
	"""Enter the SWARM runtime environment"""

	if g.MEM == "sharedspmd":
		print tabs + "swarm_Nat32_t *rank = (swarm_Nat32_t *) malloc(sizeof(swarm_Nat32_t));"
		print tabs + "swarm_Nat32_init(rank, _pil_index);"

	if g.PIL_MAIN:
		if g.MEM == "sharedspmd":
			print tabs + "do => pil_main_codelet(rank);"
		else:
			print tabs + "pil_main(cla->argc, cla->argv, cla->rank);"
			print tabs + "swarm_return;"
	else:
		runtime_params = ""
		if g.DEBUG_LEVEL > 0:
			runtime_params = ", &_p"
		if g.MEM == "sharedspmd":
			codelet = "done(rank)"
		else:
			codelet = "exit()"
		print ""
		print tabs + "// Start SWARM"
		print tabs + "pil_nodes.enter(node_" + label + "_input" + runtime_params + ") => " + codelet + ";"

def declare_pil_enter_arguments():


	print "#ifdef PIL2SWARM"

	#for node in g.nodes_entered:
	#	print "\tnode_" + node['label'] + "_Param input_" + node['label'] + ";"

	print "#endif /* PIL2SWARM */"


def process_pil_enter_arguments(node):
	"""Output the appropriate code to enter a node"""

	args = g.args[g.functions[node['label']]]
	for arg_name in args['list']:
		if arg_name == g.target_variables[node['label']]:
			continue
		if g.args[g.functions[node['label']]]['dict'][arg_name]['passed_by_reference']:
			print "\t\t\t" + arg_name + "_ptr = va_arg(argptr, " + g.variables[arg_name] + " *);"
			print "\t\t\tpil_params[_pil_rank].node_" + node['label'] + "_input." + arg_name + " = *" + arg_name + "_ptr;"
		else:
			print "\t\t\tpil_params[_pil_rank].node_" + node['label'] + "_input." + arg_name + " = va_arg(argptr, " + g.variables[arg_name] + ");"
		print "\t\t\tglobal." + arg_name + " = pil_params[_pil_rank].node_" + node['label'] + "_input." + arg_name + ";"
	print "\t\t\tbreak;"


def process_pil_enter(nodes, node):
	"""Output the appropriate code to enter a node"""

	param = "pil_params[_pil_rank].node_" + node['label'] + "_input"
	tabs = "\t\t\t"

	lm = re.match("\d+", g.intervals[node['label']]['lower'])
	if lm:
		interval_init = g.intervals[node['label']]['lower'] + ", "
	else:
		interval_init = param + "." + g.intervals[node['label']]['lower'] + ", "
	sm = re.match("\d+", g.intervals[node['label']]['step'])
	if sm:
		interval_init += g.intervals[node['label']]['step'] + ", "
	else:
		interval_init += param + "." + g.intervals[node['label']]['step'] + ", "
	um = re.match("\d+", g.intervals[node['label']]['upper'])
	if um:
		interval_init += g.intervals[node['label']]['upper']
	else:
		if g.intervals[node['label']]['upper'] in g.args[g.functions[node['label']]]['list']:
			interval_init += param + "." + g.intervals[node['label']]['upper']
		else:
			interval_init += "input._global->" + g.intervals[node['label']]['upper']
	if g.MEM == "sharedspmd":
		init = interval_init + ", " + param + ".index_array, " + param + ".data_array"
	else:
		init = interval_init + ", global.index_array, global.data_array"

	print "#ifdef PIL2SWARM"
	print tabs + "// [lower, step, upper] = [" + interval_init + "]"
	print tabs + "node_" + node['label'] + "_Param_init(&" + param + ", &global, " + init + ");"
	print tabs + "" + param + ".needs_freed = 0;"

	lower = 0
	step = 0
	upper = 0

	for n in nodes:
		if n['label'] == node['label']:
			try:
				int(n['lower'])
				lower = n['lower']
			except ValueError:
				lower = param + "." + n['lower']

			try:
				int(n['step'])
				step = n['step']
			except ValueError:
				step = param + "." + n['step']

			try:
				int(n['upper'])
				upper = n['upper']
			except ValueError:
				upper = param + "." + n['upper']

#	print tabs + "if (" + param + ".index_array.guid == NULL_GUID) {"
#	print tabs + "\tuint64_t _pil_node_" + node['label'] + "_num_iter = 0;"
#	print tabs + "\t_pil_node_" + node['label'] + "_num_iter = (" + upper + " - " + lower + ") / " + step + " + 1;"
#	print tabs + "\tpil_alloc(&" + param + ".index_array, (_pil_node_" + node['label'] + "_num_iter+1) * sizeof(int));"
#	print tabs + "\tint *_pil_ia = (int *) " + param + ".index_array.ptr;"
#	print tabs + "\tint _pil_index;"
#	print tabs + "\tfor (_pil_index = 0; _pil_index <= _pil_node_" + node['label'] + "_num_iter; _pil_index++) {"
#	print tabs + "\t\t_pil_ia[_pil_index] = 0;"
#	print tabs + "\t}"
#	print tabs + "}"


	print ""
	if g.MEM == "sharedspmd":
		pass
		#print tabs + "int _pil_rank = " + param + "." + g.nodes[node['label']]['rank'] + ";"
		#print tabs + "assert(_pil_rank >= 0 && _pil_rank < pil_get_nwCount());"
	else:
		print tabs + "int _pil_rank = 0;"

	if g.MEM == "sharedspmd":
		run.file_debug(1, tabs + 'printf("%d: pil_enter()\\n", _pil_rank);')
	else:
		run.file_debug(1, tabs + 'printf("pil_enter()\\n");')

	if g.PIL_MAIN:
		print tabs + "_pil_thread_lock[_pil_rank]._pil_in_pil_enter = TRUE;"
	print tabs + "swarm_Nat32_t label;"
	print tabs + "swarm_Nat32_init(&label, " + node['label'] + ");"
	#print tabs + "pil_nodes_Param pil_params;"
	#print tabs + "pil_params.context = NULL;"
	#print tabs + "pil_params.node_" + node['label'] + "_input = &" + param + ";"
	print tabs + "pil_nodes.enter(&label, &pil_params[_pil_rank]) => void;"
	print ""

	if g.PIL_MAIN:
		print tabs + "pthread_mutex_lock(&(_pil_thread_lock[_pil_rank]._pil_enter_mutex));"
		print tabs + "while(_pil_thread_lock[_pil_rank]._pil_in_pil_enter)"
		print tabs + "\t_pil_thread_lock[_pil_rank]._pil_cond_wait_ret = pthread_cond_wait(&(_pil_thread_lock[_pil_rank]._pil_enter_condition), &(_pil_thread_lock[_pil_rank]._pil_enter_mutex));"
		print tabs + "pthread_mutex_unlock(&(_pil_thread_lock[_pil_rank]._pil_enter_mutex));"
		print ""

	args = g.args[g.functions[node['label']]]
	for arg_name in args['list']:
		if arg_name == g.target_variables[node['label']]:
			continue
		if g.args[g.functions[node['label']]]['dict'][arg_name]['passed_by_reference']:
			if g.MEM == "sharedspmd":
				print tabs + "*" + arg_name + "_ptr = pil_params[_pil_rank].node_" + node['label'] + "_input." + arg_name + ";"
			else:
				print tabs + "*" + arg_name + "_ptr = global." + arg_name + ";"
	print tabs + "break;"
	print "#endif /* PIL2SWARM */"


def process_main():
	"""process_main - outputs the main function for the file."""

	print "swarm_type _pil_cmd_line_args"
	print "{"
	print "\tint argc;"
	print "\tchar **argv;"
	print "\tint rank;"
	print "};"
	print "#pragma swarm implement _pil_cmd_line_args"
	print ""
	print "swarm_type pil_swarm_main_Context;"
	print "swarm_procedure void pil_swarm_main(_pil_cmd_line_args *cla);"
	print "swarm_procedure void pil_swarm_main(_pil_cmd_line_args *cla)"
	print "{"

	if g.MEM == "sharedspmd":
		print "\tswarm_Dep_t _pil_done_dep;"
		print ""

	print "\tswarm_codelet entry()"
	print "\t{"
	run.file_debug(1, 'printf("swarm_codelet entry()\\n");')

	init = g.main_node_input[0] + ", " + g.main_node_input[1] + ", " + g.main_node_input[2] + ", global.index_array, global.data_array"

	tabs = "\t"
	if g.MEM == "sharedspmd":
		tabs += "\t"
		print tabs + "int _pil_index;"
		print tabs + "int _pil_NP = pil_get_nwCount();"
		print tabs + "_pil_done_dep.requires(_pil_NP+1) => exit();"
		if g.MEM == "sharedspmd" and g.PIL_MAIN:
			print tabs + "_pil_thread_lock = (struct _pil_thread_lock_t *) malloc(_pil_NP * sizeof(struct _pil_thread_lock_t));"
		print tabs + "for (_pil_index = 0; _pil_index < _pil_NP; _pil_index++) {"

	if not g.PIL_MAIN:
		print tabs + "\t// Initializing node_" + g.main_node_label + " input parameter"
		print tabs + "\tnode_" + g.main_node_label + "_Param *node_" + g.main_node_label + "_input = (node_" + g.main_node_label + "_Param *) malloc(sizeof(node_" + g.main_node_label + "_Param));"
		print tabs + "\tnode_" + g.main_node_label + "_Param_init(node_" + g.main_node_label + "_input, &global, " + init + "); // [lower, step, upper] = [" + init + "]"
		if "argc" in g.args[g.functions[g.main_node_label]]['list']:
			print tabs + "\tnode_" + g.main_node_label + "_input->argc = cla->argc;"
		if "argv" in g.args[g.functions[g.main_node_label]]['list']:
			print tabs + "\tnode_" + g.main_node_label + "_input->argv = cla->argv;"
		if g.MEM == "sharedspmd":
			print tabs + "\tnode_" + g.main_node_label + "_input->" + g.nodes[g.main_node_label]['rank'] + " = _pil_index;"

	enter_swarm(tabs + "\t", g.main_node_label)

	if g.MEM == "sharedspmd":
		print tabs + "}"
		print tabs + "swarm_Dep_satisfy(&_pil_done_dep, 1U);"

	print "\t}"
	print ""

	if g.MEM == "sharedspmd":
		if g.PIL_MAIN:
			print "\tswarm_codelet pil_main_codelet(swarm_Nat32_t *rank)"
			print "\t{"
			print "\t\tint _pil_rank = rank->value;"
			run.file_debug(1, 'printf("%d: swarm_codelet pil_main_codelet\\n", _pil_rank);')
			print "\t\t_pil_thread_lock_init(&_pil_thread_lock[_pil_rank]);"
			print "\t\tpil_main(cla->argc, cla->argv, _pil_rank);"
			print "\t\tdo => done(rank);"
			print "\t}"
			print ""
		print "\tswarm_codelet done(swarm_Nat32_t *rank)"
		print "\t{"
		print "\t\tint _pil_rank = rank->value;"
		run.file_debug(1, 'printf("%d: swarm_codelet done\\n", _pil_rank);')
		print "\t\tfree(rank);"
		print "\t\tswarm_Dep_satisfy(&_pil_done_dep, 1U);"
		print "\t}"
		print ""

	print "\tswarm_codelet exit()"
	print "\t{"
	run.file_debug(1, 'printf("swarm_codelet exit\\n");')
	if g.PIL_MAIN:
		print "\t\tfree(_pil_thread_lock);"
	print "\t\tswarm_return;"
	print "\t}"

	print "}"
	print ""

	if g.FILE_TYPE in g.CHPL:
		print "void chpl_main()"
	else:
		print "int main(int argc, char **argv)"
	print "{"
	tabs = "\t"
	print tabs + "// Place to store on the stack all of the global parameters"
	print tabs + "//global_Param global;"
	print tabs + "//global_Param_init();"
	run.set_stack_size()
	set_num_threads()

	print ""
	print tabs + "global.index_array.guid = NULL_GUID;"
	print tabs + "global.index_array.ptr = NULL;"
	print tabs + "global.data_array.guid = NULL_GUID;"
	print tabs + "global.data_array.ptr = NULL;"
	print ""

	print tabs + "pil_nw_init();"
#	print tabs + "sem_init(&_pil_mutex, 0, 1);"
#	print tabs + "sem_init(&_pil_turnstile, 0, 0);"
#	print tabs + "sem_init(&_pil_turnstile2, 0, 1);"
	print tabs + "int _pil_NP = pil_get_nwCount();"
#	print tabs + "_pil_send_buf = (struct _pil_communication_buffers **) malloc(_pil_NP*sizeof(struct _pil_communication_buffers *));"
	print tabs + "pil_params = (pil_nodes_Param *) malloc(_pil_NP * sizeof(pil_nodes_Param));"
	if g.MEM == "sharedspmd" and g.REUSE_ARRAYS:
		print tabs + "_pil_reuse_arrays = (struct _pil_reuse_arrays_t *) malloc(_pil_NP *sizeof(struct _pil_reuse_arrays_t));"
	print tabs + "for (int _pil_index = 0; _pil_index < _pil_NP; _pil_index++) {"
	print tabs + "\tpil_params[_pil_index].context = NULL;"
	if g.MEM == "sharedspmd" and g.REUSE_ARRAYS:
		for m in g.nodes:
			print tabs + "\t_pil_reuse_arrays[_pil_index]." + g.nodes[m]['func_name'] + "_param = NULL;"
			print tabs + "\t_pil_reuse_arrays[_pil_index].num_" + g.nodes[m]['func_name'] + "_params = 0;"
#	print tabs + "\t_pil_send_buf[_pil_index] = (struct _pil_communication_buffers *) malloc(_pil_NP*sizeof(struct _pil_communication_buffers));"
#	print tabs + "\tfor (int _pil_i = 0; _pil_i < _pil_NP; _pil_i++) {"
#	print tabs + "\t\t_pil_send_buf[_pil_index][_pil_i].ptr = NULL;"
#	print tabs + "\t\t_pil_send_buf[_pil_index][_pil_i].size = 0;"
#	print tabs + "\t\t_pil_send_buf[_pil_index][_pil_i].full = 0;"
#	print tabs + "\t}"
	print tabs + "}"
	print ""

	if g.MEM == "distforkjoin" or g.MEM == "distspmd":
		print tabs + "// Starts SWARM and the transport layer, and leaves them running."
		print tabs + "startup.swarm_enter();"
		print ""

	if g.PIL_MAIN and g.MEM != "sharedspmd":
		print tabs + "_pil_thread_lock = (struct _pil_thread_lock_t *) malloc(sizeof(struct _pil_thread_lock_t));"
		print tabs + "_pil_thread_lock_init(_pil_thread_lock);"
		print ""

	print tabs + "_pil_cmd_line_args cla;"
	print tabs + "cla.argc = argc;"
	print tabs + "cla.argv = argv;"
	print tabs + "cla.rank = 0;"
	print tabs + "pil_swarm_main.swarm_enter(&cla);"
	print ""

	#if g.MEM == "distforkjoin" or g.MEM == "distspmd" and g.PIL_MAIN == "True":
	if g.MEM == "distforkjoin" or g.MEM == "distspmd":
		print tabs + "// Transport layer must be shut down or process may hang."
		print tabs + "shutdown.swarm_enter();"
		print ""
	if g.FILE_TYPE in g.CHPL:
		print tabs + "return;"
	else:
		print tabs + "return EXIT_SUCCESS;"
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

	# output the swarm type for the function inside the node
	sys.stdout.write("// swarm_type for the function " + func_name + "\n")
	sys.stdout.write("swarm_type " + func_name + "_Param// : swarm_Any_t\n")
	sys.stdout.write("{\n")
	#sys.stdout.write("\tnode_" + label + "_Context *context;\n")
	sys.stdout.write("\tpil_nodes_Context *context;\n")
	sys.stdout.write("\n")
	if g.REUSE_ARRAYS:
		sys.stdout.write("\tstruct Node_" + label + "_Arrays node_" + label + "_arrays;\n")
		sys.stdout.write("\n")
	sys.stdout.write("\t" + "// in_args: " + in_args + ";\n")

	for arg_name in g.args[func_name]['list']:
		sys.stdout.write("\t" + g.variables[arg_name] + " " + arg_name + ";\n")
	sys.stdout.write("\tint _pil_iter;\n")

	sys.stdout.write("};\n")
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

	sys.stdout.write("\tswarm_Dep_t _pil_node_" + label + "_dep;\n")
	sys.stdout.write("\tint _pil_node_" + label + "_ctr;\n")
	sys.stdout.write("\t" + func_name + "_Param *" + func_name + "_param;\n")
	sys.stdout.write("\tuint8_t needs_freed;\n")
	sys.stdout.write("\n")

	if g.nodes[label]['rank'] not in g.args[func_name]['list']:
		sys.stdout.write("\tint " + g.nodes[label]['rank'] + ";\n")
	if index not in g.args[func_name]['list']:
		sys.stdout.write("\t" + g.variables[index] + " " + index + ";\n")
	for arg_name in g.args[func_name]['list']:
		sys.stdout.write("\t" + g.variables[arg_name] + " " + arg_name + ";\n")

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
		if g.MEM == "distforkjoin":
			print "registered_codelet_t const PROC_NODE_" + node['label'] + " = " + str(rc) + ";"
			rc += 1
			print "registered_codelet_t const SATISFY_" + node['label'] + " = " + str(rc) + ";"
			rc += 1
		if g.MEM == "distforkjoin" or g.MEM == "distspmd":
			print "registered_codelet_t const SWITCH_" + node['label'] + " = " + str(rc) + ";"
			rc += 1
		if g.MEM == "distspmd":
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

	sys.stdout.write("swarm_procedure void node_" + label + "(node_" + label + "_Param *node_" + label + "_input);\n")
	sys.stdout.write("swarm_procedure void " + func_name + "_swarm_p(" + func_name + "_Param *node_" + label + "_input);\n")
	sys.stdout.write("\n")


def swarm_process_global_param_init():
	"""generate the parameter init function for the global parameter"""

	args = ""
	for var in g.variables:
		if var in g.initial_values:
			args += ", " + g.variables[var] + " " + var
	print "static void global_Param_init(global_Param *inst" + args + ")"
	print "{"
	tabs = "\t"
	print tabs + "// TODO: what does this line mean?"
	print tabs + "inst->swarm_tptrs = global_Param_TYPE.instTptrs;"
	print ""
	for var in g.variables:
		if var in g.initial_values:
			print tabs + "inst->" + var + " = " + var + ";"
	print "}"
	print ""


def swarm_process_node_param_init(label, func_name):
	"""generate the parameter init function for the node"""

	print "static void node_" + label + "_Param_init(node_" + label + "_Param *inst, global_Param *g, swarm_int32_t l, swarm_int32_t s, swarm_int32_t u, gpp_t index_array, gpp_t data_array)"
	print "{"
	tabs = "\t"
	print tabs + "//TODO: what do these two lines mean?"
	#print tabs + "inst->swarm_tptrs = node_" + label + "_Param_getType()->instTptrs;"
	print tabs + "inst->swarm_tptrs = node_" + label + "_Param_TYPE.instTptrs;"
	#print tabs + "swarm_Any_initSuper(&inst->super_Any, node_" + label + "_Param_TYPE.instTptrs + _node_" + label + "_Param_TPTR_OFFS_swarm_Any);"
	print tabs + "inst->_pil_node_" + label + "_ctr = 0;"
	print tabs + "inst->" + func_name + "_param = NULL;"
	print ""
	print tabs + "inst->_global = g;"
	print tabs + "inst->" + g.LOWER + " = l;"
	print tabs + "inst->" + g.STEP + " = s;"
	print tabs + "inst->" + g.UPPER + " = u;"
	print tabs + "inst->index_array = index_array;"
	print tabs + "inst->data_array = data_array;"
	print tabs + "inst->needs_freed = 1;"
	print "}"
	print ""


def swarm_process_func_param_init(func_name, label):
	"""generate the parameter init function for the function in the node"""

	# find the input arguments
	in_args = ""
	for arg_name in g.args[func_name]['list']:
		in_args += ", " + g.variables[arg_name] + " " + arg_name

	print "static void " + func_name + "_Param_init(" + func_name + "_Param *inst, pil_nodes_Context *_pil_context" + in_args + ", int _pil_iter)"
	print "{"
	print "\t//TODO: what do these two lines mean?"
	run.file_debug(3, '\tfprintf(stdout, "initializing the unknown line 1\\n"); fflush(stdout);')
	#print "\tinst->swarm_tptrs = " + func_name + "_Param_getType()->instTptrs;"
	print "\tinst->swarm_tptrs = " + func_name + "_Param_TYPE.instTptrs;"
	#run.file_debug(3, '\tfprintf(stdout, "initializing the unknown line 2\\n"); fflush(stdout);')
	#print "\tswarm_Any_initSuper(&inst->super_Any, " + func_name + "_Param_TYPE.instTptrs+1);"
	print ""
	print "\t// set the context"
	run.file_debug(3, '\tfprintf(stdout, "initializing context _pil_context\\n"); fflush(stdout);')
	print "\tinst->context = _pil_context;"
	print ""

	print "\t// initialize each input parameter"
	for arg_name in g.args[func_name]['list']:
		run.file_debug(3, '\tfprintf(stdout, "initializing param ' + arg_name + '\\n"); fflush(stdout);')
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

#	if func_name in g.nw_calls.keys():
#		r = re.match(g.NW_RECV_RE, g.nw_calls[func_name])
#		if r:
#			return True
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
		print "\t\tnw_NetBuffer_t *_pil_buf = swarm_Transferable_to_nw_NetBuffer(swarm_input);"
		print "\t\tmemcpy(&num, _pil_buf->p, " + size + ");"


def swarm_process_swarm_nodes_procedure(label, func_name):
	"""output the code for the single swarm procedure that contains all the pil nodes codelets"""

	print "//--------------------------------------"
	print "// main swarm_procedure for pil_nodes"
	print "//--------------------------------------"
	print ""

	#--------------------------------------------------------------------------
	# prototype
	#--------------------------------------------------------------------------
	if g.PIL_MAIN:
		print "swarm_procedure void pil_nodes(swarm_Nat32_t *label, pil_nodes_Param *pil_params)"
	else:
		print "swarm_procedure void pil_nodes(node_" + label + "_Param *n_" + label + "_i)"
	print "{"

	#--------------------------------------------------------------------------
	# parameters
	#--------------------------------------------------------------------------
	node = None
	for n in g.nodes:
		#run.debug(4, n)
		#if g.nodes[n]['func_name'] == func_name:
		#	nodelet_label = n
		#	node = g.nodes[n]

		if g.MEM == "distforkjoin":
			print "\tswarm_Dep_t _pil_node_" + g.nodes[n]["label"] + "_dep;"
#		if g.MEM == "shared" or g.MEM == "sharedspmd":
#			if not g.REUSE_ARRAYS:
#				print "\t" + g.nodes[n]["func_name"] + "_Param *" + g.nodes[n]["func_name"] + "_param;"
		elif g.MEM == "distspmd":
			print "\t" + g.nodes[n]["func_name"] + "_Param " + g.nodes[n]["func_name"] + "_param;"

		#for l in g.targets[label]:
		#	if l != "0":
		#		print "\tnode_" + l + "_Param node_" + l + "_param;"
		#print ""

		if g.MEM == "distspmd" or g.MEM == "distforkjoin":
			print "\tnode_" + g.nodes[n]["label"] + "_Param node_" + g.nodes[n]["label"] + "_input;"
			print "\tint _pil_node_" + n + "_ctr;"
			print "\tswarm_nat32_t _pil_node_" + n + "_num_iter;"
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
	print ""

	print "\tswarm_Nat32_t rank;"


	#--------------------------------------------------------------------------
	# node entry
	#--------------------------------------------------------------------------
	print "\tswarm_codelet entry()"
	print "\t{"
	run.file_debug(1, '\t\tprintf("swarm_codelet pil_nodes.entry()\\n");')
	if g.PIL_MAIN:
		print "\t\tuint32_t l = label->value;"
		print "\t\tswitch(l)"
		print "\t\t{"
		for node in g.nodes_entered:
			print "\t\t\tcase " + node['label'] + ":"
			print "\t\t\t{"
			#print "\t\t\t\tnode_" + node['label'] + "_Param *node_" + node['label'] + "_input = pil_params->node_" + node['label'] + "_input;"
			print "\t\t\t\tdo => node_" + node['label'] + "_enter(&(pil_params->node_" + node['label'] + "_input));"
			print "\t\t\t\tbreak;"
			print "\t\t\t}"
		print "\t\t\tdefault:"
		print "\t\t\t{"
		print "\t\t\t\tfprintf(stdout, \"ERROR: this should never happen. case: '%d' in pil_nodes_entry()\\n\", l);"
		print "\t\t\t\tswarm_return;"
		print "\t\t\t\tbreak;"
		print "\t\t\t}"
		print "\t\t}"
	else:
		#print "\t\tnode_" + label + "_input = *n_" + label + "_i;"
		print "\t\tdo => node_" + label + "_enter(n_" + label + "_i);"
	print "\t}"
	print ""


def debug_codelet(tabs, label, name):
	""""""

	string = ""
	if g.MEM == "sharedspmd":
		string = tabs + 'printf("\t%d: node_' + label + '_' + name + '()\\n", _pil_rank); fflush(stdout);'
	else:
		string = tabs + 'printf("\tnode_' + label + '_' + name + '()\\n"); fflush(stdout);'

	return string


def output_body_function_invocation(tabs, func_name, label, doing_body_optimization):
	"""print the call to the body function"""
	#print "\t\t" + func_name + "(pil_p->in_args, pil_p->out_args);"
	first = True
	in_args = ""
	for arg_name in g.args[func_name]['list']:
		if first:
			first = False
		else:
			in_args += ", "

		if arg_name == "index_array" or arg_name == "data_array":
			if doing_body_optimization:
				in_args += "node_" + label + "_input->" + arg_name
			elif g.REUSE_ARRAYS:
				in_args += "pil_p->node_" + label + "_arrays." + arg_name
			else:
				in_args += arg_name

		# if the argment is a static array, we just pass the argument since it is a global
		elif arg_name in g.arrays:
			in_args += arg_name

		else:
			# argument type
			in_args += "(" + g.variables[arg_name]

			# argument modifier
			# check to see if we should pass the argument by reference or by value
			if g.args[func_name]['dict'][arg_name]['passed_by_reference']:
				in_args += " *)&"
			else:
				in_args += ")"

			# argument name
			if doing_body_optimization:
				in_args += "node_" + label + "_input->" + arg_name
			elif g.MEM == "distforkjoin" or g.MEM == "shared" or g.MEM == "sharedspmd":
				in_args += "pil_p->" + arg_name
			elif g.MEM == "distspmd":
				in_args += func_name + "_param." + arg_name

	print tabs + func_name + "(" + in_args + ");"


def copy_function_arguments_out(tabs, label, func_name, doing_body_optimization):
	"""copy the arguemtns used by the body fuction so we can use there values later"""

	print tabs + "// TODO: figure out a way to do this without _global"
	for arg_name in g.args[func_name]['list']:
		if g.args[func_name]['dict'][arg_name]['passed_by_reference']:
			if g.MEM == "distforkjoin":
				print tabs + "node_" + label + "_input->_global->" + arg_name + " = pil_p->" + arg_name + ";"
			elif g.MEM == "distspmd":
				print tabs + "node_" + label + "_input->_global->" + arg_name + " = " + func_name + "_param." + arg_name + ";"
			else:
				if doing_body_optimization:
					param = "node_" + label + "_input->"
				else:
					param = "pil_p->"
					print tabs + "node_" + label + "_input->" + arg_name + " = " + param + arg_name + ";"
				print tabs + "node_" + label + "_input->_global->" + arg_name + " = " + param + arg_name + ";"
	print ""


def set_data_array_pointers(tabs, data_array, num):
	"""make sure the pointers of the data_array gpps are set correctly"""

	print tabs + "int _pil_index;"
	print tabs + "for (_pil_index = 0; _pil_index < " + num + "; _pil_index++) {"
	print tabs + "\t" + data_array + "[_pil_index].ptr = pil_mem(" + data_array + "[_pil_index].guid);"
	print tabs + "}"


def swarm_process_node_procedure(label, index, func_name, key):
	"""process node procedure"""

	# TODO: make this multiple procedures?

	node = None

	#--------------------------------------------------------------------------
	# node enter
	#--------------------------------------------------------------------------
	print "\tswarm_codelet node_" + label + "_enter(node_" + label + "_Param *node_" + label + "_input)"
	print "\t{"

	if g.MEM == "sharedspmd":
		print "\t\tint _pil_rank = node_" + label + "_input->" + g.nodes[label]['rank'] + ";"
	run.file_debug(1, debug_codelet("\t\t", label, "enter"))

	if g.MEM == "distforkjoin":
		print "\t\tnw_registerAction(swarm_cargs(node_" + label + "_body), PROC_NODE_" + label + ", &nw_NetBuffer_TYPE, \"\");"
		print "\t\tnw_registerAction(swarm_cargs(node_" + label + "_satisfy), SATISFY_" + label + ", &nw_NetBuffer_TYPE, \"\");"
	if g.MEM == "distforkjoin" or g.MEM == "distspmd":
		print "\t\tnw_registerAction(swarm_cargs(node_" + label + "_switch), SWITCH_" + label + ", &nw_NetBuffer_TYPE, \"\");"
		print "\t\tnw_barrierAll(GRAPH_BARRIER_" + label + ", swarm_cargs(main));"
		print "\t}"
		print ""
		#----------------------------------------------------------------------
		# node main
		#----------------------------------------------------------------------
		print "\tswarm_codelet main()"
		print "\t{"
		run.file_debug(1, debug_codelet("\t\t", label, "main"))

	if g.graph[label].has_merge():
		fork_site = g.graph[label].get_matching_fork()
		print "\t\tint tmp = __sync_add_and_fetch(&_pil_node_" + fork_site + "_counter, 1);"
		print "\t\tif (tmp != _pil_node_" + fork_site + "_total) {"
		print "\t\t\treturn;"
		print "\t\t}"

	# TODO: rename these so they can't clash with variable names in the user code
	print "\t\tswarm_Locale_t *root = swarm_getRootLocale(NULL);"
	print "\t\tsize_t _pil_np = swarm_Locale_getLeaves(root, NULL, 0);"
	print "\t\tNUM_THREADS = (uint32_t) _pil_np;"
	print "\t\tnode_" + label + "_input->_pil_node_" + label + "_ctr = 0;"
	print "\t\tint _pil_node_" + label + "_num_iter = 0;"

	if node:
		print "\t\t// init function parameters"
		for arg_name in g.args[func_name]['list']:
			line = "\t\t" + arg_name + " = "
			if g.args[func_name]['dict'][arg_name]['passed_by_reference']:
				line += "&"
			line += func_name + "_param." + arg_name + ";"
			print line

	if g.MEM == "distforkjoin":
		print "\t\tif (nw_getNodeID() == 0)"
		print "\t\t{"

	print "\t\tif (node_" + label + "_input->" + g.STEP + " < 0)"
	print "\t\t{"
	print "\t\t\t_pil_node_" + label + "_num_iter = (node_" + label + "_input->" + g.UPPER + " - node_" + label + "_input->" + g.LOWER + ")/(0 - node_" + label + "_input->" + g.STEP + ") + 1;"
	print "\t\t}"
	print "\t\telse"
	print "\t\t{"
	print "\t\t\t_pil_node_" + label + "_num_iter = (node_" + label + "_input->" + g.UPPER + " - node_" + label + "_input->" + g.LOWER + ")/node_" + label + "_input->" + g.STEP + " + 1;"
	print "\t\t}"

	tabs = "\t\t"
	print tabs + "if (_pil_node_" + label + "_num_iter == 1) {"
	tabs += "\t"
	print tabs + "// single loop iteration optimization so we don't have to create and schedule a body and switch codelet"
	print tabs + "if (node_" + label + "_input->index_array.guid != NULL_GUID) {"
	set_data_array_pointers(tabs + "\t", "((gpp_t *)node_" + label + "_input->data_array.ptr)", "((int *)node_" + label + "_input->index_array.ptr)[1]")
	print tabs + "}"
	output_body_function_invocation(tabs, func_name, label, True)
	copy_function_arguments_out(tabs, label, func_name, True)
	output_switch(tabs, label, func_name, key, True)
	tabs = tabs[:-1]
	print tabs + "}"
	print tabs + "else // not single iteration optimization"
	print tabs + "{"
	tabs += "\t"

	if g.MEM == "distforkjoin":
		print tabs + "_pil_node_" + label + "_dep.requires(_pil_node_" + label + "_num_iter+1) => node_" + label + "_bcast;"
	elif g.MEM == "shared" or g.MEM == "sharedspmd":
		print tabs + "node_" + label +"_input->_pil_node_" + label + "_dep.requires(_pil_node_" + label + "_num_iter+1) => node_" + label + "_switch(node_" + label + "_input);"
	print ""

	if g.MEM == "shared" or g.MEM == "sharedspmd":
		if g.FILE_TYPE in g.CHPL:
			print tabs + "" + func_name + "_param = chpl_malloc(_pil_node_" + label + "_num_iter, sizeof(" + func_name + "_Param), 0, 0, NULL);"
		else:
			if g.REUSE_ARRAYS:
				param = func_name + "_param"
				num_params = "num_" + func_name + "_params"
				if g.MEM == "sharedspmd":
					param = "_pil_reuse_arrays[_pil_rank]." + param
					num_params = "_pil_reuse_arrays[_pil_rank]." + num_params
				print tabs + "if (" + param + " == NULL || " + num_params + " < _pil_node_" + label + "_num_iter) {"
				print tabs + "\tint _pil_index;"
				print tabs + "\tfor (_pil_index = 0; _pil_index < " + num_params + "; ++_pil_index) {"
				print tabs + "\t\tfree(" + param + "[_pil_index].node_" + label + "_arrays.index_array.ptr);"
				print tabs + "\t\tfree(" + param + "[_pil_index].node_" + label + "_arrays.data_array.ptr);"
				print tabs + "\t}"
				print tabs + "\tfree(" + param + ");"
				print tabs + "\t" + param + " = (" + func_name + "_Param *) malloc(_pil_node_" + label + "_num_iter * sizeof(" + func_name + "_Param));"
				print tabs + "\tfor (_pil_index = 0; _pil_index < _pil_node_" + label + "_num_iter; ++_pil_index) {"
				print tabs + "\t\t" + param + "[_pil_index].node_" + label + "_arrays.index_array.ptr = NULL;"
				print tabs + "\t\t" + param + "[_pil_index].node_" + label + "_arrays.data_array.ptr = NULL;"
				print tabs + "\t\t" + param + "[_pil_index].node_" + label + "_arrays.index_array_size = 0;"
				print tabs + "\t\t" + param + "[_pil_index].node_" + label + "_arrays.data_array_size = 0;"
				print tabs + "\t}"
				print tabs + "\t" + num_params + " = _pil_node_" + label + "_num_iter;"
				print tabs + "}"
				print tabs + "node_" + label + "_input->" + func_name + "_param = " + param + ";"
				print ""
			else:
				print tabs + "node_" + label + "_input->" + func_name + "_param = (" + func_name + "_Param *) malloc(_pil_node_" + label + "_num_iter * sizeof(" + func_name + "_Param));"
	run.file_debug(2, tabs + "fprintf(stdout, \"lower: %d  upper: %d  step: %d\\n\", node_" + label + "_input->" + g.LOWER + ", node_" + label + "_input->" + g.UPPER + ", node_" + label + "_input->" + g.STEP + "); fflush(stdout);")

	in_args = ""
	for arg_name in g.args[func_name]['list']:
		in_args += ", node_" + label + "_input->" + arg_name
	if g.MEM == "distspmd":
			print tabs + "if (pil_get_nwID() < _pil_node_" + label + "_num_iter)"
			print tabs + "{"
			run.file_debug(2, '\t\t\tfprintf(stdout, "' + func_name + '_Param_init call\\n");')
			print tabs + "\tint _pil_iter = pil_get_nwID();"
			print tabs + "\t" + func_name + "_Param_init(&" + func_name + "_param, swarm_context" + in_args + ", _pil_iter);"
			print tabs + "\tdo => node_" + label + "_body();"
			print tabs + "}"
			print tabs + "else"
			print tabs + "{"
			print tabs + "\tdo => node_" + label + "_bar;"
			print tabs + "}"
	else:
		LEAVES_LOOP_OPT = False
		if LEAVES_LOOP_OPT:
			print tabs + "if (_pil_node_" + label + "_num_iter > NUM_THREADS) {"
			#----------------------------------------
			# step < 0
			#----------------------------------------
			print tabs + "\tif (node_" + label + "_input->" + g.STEP + " < 0)"
			print tabs + "\t{"
			print tabs + "\t\tfor (node_" + label + "_input->" + index + " = node_" + label + "_input->" + g.UPPER + "; node_" + label + "_input->" + index + " >= node_" + label + "_input->" + g.LOWER + "; node_" + label + "_input->" + index + " += node_" + label + "_input->" + g.STEP + ")"
			print tabs + "\t\t{"
			if g.MEM == "distforkjoin":
				print tabs + "\t\t\tnw_NetBuffer_t *nb;"
				print tabs + "\t\t\tnb = nw_NetBuffer_new(NULL, sizeof(" + func_name + "_Param));"
				print tabs + "\t\t\t" + func_name + "_Param *param = (" + func_name + "_Param *)nb->p;"
				print tabs + "\t\t\t" + func_name + "_Param_init(param, swarm_context" + in_args + ", _pil_iter);"
				run.file_debug(2, '\t\t\t\tfprintf(stdout, "scheduling on index: %d\\n", node_" + label + "_input->' + index + '); fflush(stdout);')
				print tabs + "\t\t\t//nw_call(node_" + label + "_input->" + index + ", PROC_NODE_" + label + ", nw_NetBuffer_to_swarm_Transferable(nb), NULL, NULL);"
				print tabs + "\t\t\tnw_call(_pil_iter, PROC_NODE_" + label + ", nw_NetBuffer_to_swarm_Transferable(nb), NULL, NULL);"
				print tabs + "\t\t\t_pil_iter += 1;"
			else:
				run.file_debug(2, '\t\t\t\tfprintf(stdout, "' + func_name + '_Param_init call\\n");')
				print tabs + "\t\t\tint _pil_iter = (node_" + label + "_input->" + index + " - node_" + label + "_input->" + g.LOWER + ")/(0 - node_" + label + "_input->" + g.STEP + ");"
				print tabs + "\t\t\t" + func_name + "_Param_init(&" + func_name + "_param[_pil_iter], swarm_context" + in_args + ", _pil_iter);"
				#print tabs + "\t\t\tdo => node_" + label + "_body(&" + func_name + "_param[_pil_iter]);"
				print tabs + "\t\t\tdo => node_" + label + "_body();"
			print tabs + "\t\t}"
			print tabs + "\t}"
			#----------------------------------------
			# step > 0
			#----------------------------------------
			print tabs + "\telse"
			print tabs + "\t{"
			print tabs + "\t\tfor (node_" + label + "_input->" + index + " = node_" + label + "_input->" + g.LOWER + "; node_" + label + "_input->" + index + " <= node_" + label + "_input->" + g.UPPER + "; node_" + label + "_input->" + index + " += node_" + label + "_input->" + g.STEP + ")"
			print tabs + "\t\t{"
			if g.MEM == "distforkjoin":
				print tabs + "\t\t\tnw_NetBuffer_t *nb;"
				print tabs + "\t\t\tnb = nw_NetBuffer_new(NULL, sizeof(" + func_name + "_Param));"
				print tabs + "\t\t\t" + func_name + "_Param *param = (" + func_name + "_Param *)nb->p;"
				print tabs + "\t\t\t" + func_name + "_Param_init(param, swarm_context" + in_args + ", _pil_iter);"
				run.file_debug(2, '\t\t\t\tfprintf(stdout, "scheduling on index: %d\\n", node_" + label + "_input->' + index + '); fflush(stdout);')
				print tabs + "\t\t\t//nw_call(node_" + label + "_input->" + index + ", PROC_NODE_" + label + ", nw_NetBuffer_to_swarm_Transferable(nb), NULL, NULL);"
				print tabs + "\t\t\tnw_call(_pil_iter, PROC_NODE_" + label + ", nw_NetBuffer_to_swarm_Transferable(nb), NULL, NULL);"
				print tabs + "\t\t\t_pil_iter += 1;"
			else:
				run.file_debug(2, '\t\t\t\tfprintf(stdout, "' + func_name + '_Param_init call\\n");')
				print tabs + "\t\t\tint _pil_iter = (node_" + label + "_input->" + index + " - node_" + label + "_input->" + g.LOWER + ")/node_" + label + "_input->" + g.STEP + ";"
				print tabs + "\t\t\t" + func_name + "_Param_init(&" + func_name + "_param[_pil_iter], swarm_context" + in_args + ", _pil_iter);"
				#print tabs + "\t\t\tdo => node_" + label + "_body(&" + func_name + "_param[_pil_iter]);"
				print tabs + "\t\t\tdo => node_" + label + "_body();"
			print tabs + "\t\t}"
			print tabs + "\t}"
			print tabs + "}"
			print tabs + "if (_pil_node_" + label + "_num_iter <= NUM_THREADS) {"
			print tabs + "\tswarm_Locale_scheduleToLeaves(swarm_getRootLocale(NULL), _pil_node_" + label + "_num_iter, swarm_cargs(node_" + label + "_body), NULL, NULL, NULL, swarm_Scheduler_ORDER_FIFO);"
			print tabs + "}"
		else:
			print tabs + "int chunks = _pil_node_" + label + "_num_iter / NUM_THREADS;"
			print tabs + "int remainder = _pil_node_" + label + "_num_iter % NUM_THREADS;"
			print tabs + "for(int c = 0; c < chunks; c++) {"
			print tabs + "\tswarm_Locale_scheduleToLeaves(swarm_getRootLocale(NULL), NUM_THREADS, swarm_cargs(node_" + label + "_body), node_" + label + "_input, NULL, NULL, swarm_Scheduler_ORDER_FIFO);"
			print tabs + "}"
			print tabs + "if(remainder > 0)"
			print tabs + "\tswarm_Locale_scheduleToLeaves(swarm_getRootLocale(NULL), remainder, swarm_cargs(node_" + label + "_body), node_" + label + "_input, NULL, NULL, swarm_Scheduler_ORDER_FIFO);"

	print tabs + "swarm_Dep_satisfy(&node_" + label + "_input->_pil_node_" + label + "_dep, 1U);"

	if g.MEM == "distforkjoin":
		print tabs + "}"

	if g.MEM == "distforkjoin" or g.MEM == "distspmd":
		print tabs + "swarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"

	run.file_debug(2, '\t\tfprintf(stdout, "leaving node_' + label + '.entry()\\n"); fflush(stdout);')
	tabs = tabs[:-1]
	print tabs + "}"
	tabs = tabs[:-1]
	print tabs + "}"
	print ""

	#--------------------------------------------------------------------------
	# node body
	#--------------------------------------------------------------------------
	print "\tswarm_codelet node_" + label + "_body()"
	print "\t{"

	print "\t\tnode_" + label + "_Param *node_" + label + "_input = (node_" + label + "_Param *) swarm_input;"
	print "\t\tint _pil_rank = node_" + label + "_input->" + g.nodes[label]['rank'] + ";"

	print "\t\tint _pil_iter = __sync_fetch_and_add(&node_" + label + "_input->_pil_node_" + label + "_ctr, 1);"
	in_args = ""
	for arg_name in g.args[func_name]['list']:
		if arg_name == index:
			in_args += ", (node_" + label + "_input->" + g.LOWER + " + _pil_iter * node_" + label + "_input->" + g.STEP + ")"
		else:
			in_args += ", node_" + label + "_input->" + arg_name
	if g.REUSE_ARRAYS:
		print "\t\t" + func_name + "_Param_init(&node_" + label + "_input->" + func_name + "_param[_pil_iter], swarm_context" + in_args + ", _pil_iter);"
	else:
		print "\t\t\t" + func_name + "_Param_init(&node_" + label + "_input->" + func_name + "_param[_pil_iter], swarm_context" + in_args + ", _pil_iter);"
	if g.MEM == "shared" or g.MEM == "sharedspmd":
		print "\t\t" + func_name + "_Param *pil_p = &(node_" + label + "_input->" + func_name + "_param[_pil_iter]);"
	else:
		print "\t\t" + func_name + "_Param *pil_p = &" + func_name + "_param;"

	run.file_debug(1, debug_codelet("\t\t", label, "body"))

	if g.MEM == "distforkjoin":
		print "\t\tnw_NetBuffer_t *_pil_buf = swarm_Transferable_to_nw_NetBuffer(swarm_input);"
		print "\t\t" + func_name + "_Param *pil_p = _pil_buf->p;"
		print "\t\t//" + func_name + "_param = pil_p;"

	if not g.REUSE_ARRAYS:
		print "\t\tgpp_t index_array;"
		print "\t\tgpp_t data_array;"
	print "\t\tif (pil_p->index_array.guid != NULL_GUID) {"
	print "\t\t\tint *_pil_gia = (int *)pil_p->index_array.ptr;"
	print "\t\t\tgpp_t *_pil_gda = (gpp_t *)pil_p->data_array.ptr;"
	if g.REUSE_ARRAYS:
		print "\t\t\tif (pil_p->node_" + label + "_arrays.index_array_size < 2*sizeof(int)) {"
		print "\t\t\t\tif (pil_p->node_" + label + "_arrays.index_array.ptr)"
		print "\t\t\t\t\tpil_free(pil_p->node_" + label + "_arrays.index_array);"
		print "\t\t\t\tpil_alloc(&(pil_p->node_" + label + "_arrays.index_array), 2*sizeof(int));"
		print "\t\t\t\tpil_p->node_" + label + "_arrays.index_array_size = 2*sizeof(int);"
		print "\t\t\t} // else we can save the overhead of an alloc and reuse the previously allocated size."
		print "\t\t\tint *_pil_lia = (int*) pil_p->node_" + label + "_arrays.index_array.ptr;"
	else:
		print "\t\t\tpil_alloc(&index_array, 2*sizeof(int));"
		print "\t\t\tint *_pil_lia = (int*) index_array.ptr;"
	print "\t\t\tint _pil_num = _pil_gia[pil_p->_pil_iter+1] - _pil_gia[pil_p->_pil_iter];"
	print "\t\t\tif (_pil_num > 0) {"
	print "\t\t\t\t_pil_lia[0] = 0;"
	print "\t\t\t\t_pil_lia[1] = _pil_num;"
	print ""

	print "\t\t\t\tsize_t _pil_data_size = _pil_num * sizeof(gpp_t);"
	if g.REUSE_ARRAYS:
		print "\t\t\t\tif (pil_p->node_" + label + "_arrays.data_array_size < _pil_data_size) {"
		print "\t\t\t\t\tif (pil_p->node_" + label + "_arrays.data_array.ptr)"
		print "\t\t\t\t\t\tpil_free(pil_p->node_" + label + "_arrays.data_array);"
		print "\t\t\t\t\tpil_alloc(&(pil_p->node_" + label + "_arrays.data_array), _pil_data_size);"
		print "\t\t\t\t\tpil_p->node_" + label + "_arrays.data_array_size = _pil_data_size;"
		print "\t\t\t\t} // else we can save the overhead of an alloc and reuse the previously allocated size."
		print "\t\t\t\tgpp_t *_pil_lda = (gpp_t *) pil_p->node_" + label + "_arrays.data_array.ptr;"
	else:
		print "\t\t\t\tpil_alloc(&data_array, _pil_data_size);"
		print "\t\t\t\tgpp_t *_pil_lda = (gpp_t *) data_array.ptr;"
	print "\t\t\t\tmemcpy(_pil_lda, &_pil_gda[_pil_gia[pil_p->_pil_iter]], _pil_data_size);"
	print ""

	set_data_array_pointers("\t\t\t\t", "_pil_lda", "_pil_num")

	print "\t\t\t}"
	print "\t\t}"
	print ""

	tabs = "\t\t"
	if g.MEM != "distspmd":
		output_body_function_invocation(tabs, func_name, label, False)
	else:
		if node:
			lines = g.func_lines[g.nodes[nodelet_label]['func_name']][2:-2] # strip off prototype and { as well as closing }
			print tabs + "// node contents"
			dest = ""
			for line in lines:
				if swarm_is_nw_send(line):
					dest = swarm_print_nw_send(line, label)
				else:
					print tabs + line,
			print tabs + "// end node contents"
	print ""

	copy_function_arguments_out(tabs, label, func_name, False)

	if func_name not in g.nodes:
		if g.MEM == "distforkjoin":
			print "\t\tnw_call(0, SATISFY_" + label + ", NULL, NULL, NULL);"
		elif g.MEM == "distspmd":
			print "\t\tdo => node_" + label + "_bar();"
		else:
			#print "\t\tdo => node_" + label + "_satisfy;"
			print "\t\tswarm_Dep_satisfy(&node_" + label + "_input->_pil_node_" + label + "_dep, 1U);"
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
	if g.MEM == "distforkjoin" or g.MEM == "distspmd":
		print "\t\tswarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"
		print "\t\tdo => node_" + label + "_bar;"
	print "\t}"
	print ""

	#--------------------------------------------------------------------------
	# body nodelets
	#--------------------------------------------------------------------------

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
		print "\t\tnode_" + label + "_input->_global->target_id = f_param.target_id;"
		print "\t\tdo => node_" + label + "_bar();"
		if g.MEM == "distforkjoin" or g.MEM == "distspmd":
			print "\t\tswarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"
		print "\t}"
		print ""

	#--------------------------------------------------------------------------
	# node satisfy
	#--------------------------------------------------------------------------
	#if g.MEM == "shared" or g.MEM == "distforkjoin":
	if g.MEM == "distforkjoin":
		print "\tswarm_codelet node_" + label + "_satisfy()"
		print "\t{"
		run.file_debug(1, debug_codelet("\t\t", label, "satisfy"))
		print "\t\tswarm_Dep_satisfy(&_pil_node_" + label + "_dep, 1U);"
		if g.MEM == "distforkjoin" or g.MEM == "distspmd":
			print "\t\tswarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"
		print "\t}"
		print ""

	#--------------------------------------------------------------------------
	# node bar
	#--------------------------------------------------------------------------
	if g.MEM == "distspmd":
		print "\tswarm_codelet node_" + label + "_bar()"
		print "\t{"
		run.file_debug(1, debug_codelet("\t\t", label, "bar"))
		print "\t\tswarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"
		#print "\t\tnw_barrierAll(GRAPH_BARRIER_" + label + ", swarm_cargs(node_" + label + "_switch));"
		print "\t\tnw_barrierAll(GRAPH_BARRIER_" + label + ", swarm_cargs(node_" + label + "_bcast));"
		run.file_debug(2, '\t\tfprintf(stdout, "%d: leaving bar codelet\\n", pil_get_nwID()); fflush(stdout);')
		print "\t}"
		print ""

	#--------------------------------------------------------------------------
	# node bcast
	#--------------------------------------------------------------------------
	if g.MEM == "distforkjoin" or g.MEM == "distspmd":
		print "\tswarm_codelet node_" + label + "_bcast()"
		print "\t{"
		run.file_debug(1, debug_codelet("\t\t", label, "bcast"))
		print "\t\tif (pil_get_nwID() == 0)"
		print "\t\t{"
		print "\t\t\tint i;"
		print "\t\t\t//printf(\"num_nodes: %d\\n\", nw_getNodeCount());"
		print "\t\t\t// schedule a node_" + label + "_switch codelet on each node."
		print "\t\t\tnw_NetBuffer_t *nb;"
		print "\t\t\tnb = nw_NetBuffer_new(NULL, sizeof(int));"
		print "\t\t\tint *next = (int *)nb->p;"
		print "\t\t\t*next = node_" + label + "_input->_global->" + key + ";"
		print "\t\t\tnw_callAll(SWITCH_" + label + ", nw_NetBuffer_to_swarm_Transferable(nb), NULL, NULL);"
		print "\t\t}"
		print "\t\tswarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"
		print "\t}"
		print ""

	#--------------------------------------------------------------------------
	# node switch
	#--------------------------------------------------------------------------
	tabs = "\t"
	print tabs + "swarm_codelet node_" + label + "_switch(node_" + label + "_Param *node_" + label + "_input)"
	print tabs + "{"
	tabs += "\t"

	print "\n" + tabs + "// TODO: merge index_array and data_array\n"

	if g.MEM == "sharedspmd":
		print tabs + "int _pil_rank = node_" + label + "_input->" + g.nodes[label]['rank'] + ";"

	run.file_debug(1, debug_codelet(tabs, label, "switch"))

	if g.MEM == "distforkjoin" or g.MEM == "distspmd":
		print tabs + "nw_NetBuffer_t *_pil_buf = swarm_Transferable_to_nw_NetBuffer(swarm_input);"
		print tabs + "int *next = (int*)_pil_buf->p;"

	output_switch(tabs, label, func_name, key, False)

	#--------------------------------------------------------------------------
	# node exit
	#--------------------------------------------------------------------------

	if g.MEM == "distforkjoin" or g.MEM == "distspmd":
		print tabs + "swarm_dispatch(swarm_next, swarm_next_this, NULL, NULL, NULL);"
	run.file_debug(2, tabs + 'fprintf(stdout, "leaving node_' + label + '.node_' + label + '_switch()\\n"); fflush(stdout);')
	tabs = tabs[:-1]
	print tabs + "}"

	print  ""


def output_switch_case_0(tabs, label, func_name):
	"""output the code for the case '0'"""

	print tabs + "// case 0;"
	if g.MEM == "sharedspmd":
		print tabs + "free(node_" + label + "_input->" + func_name + "_param);"
	print tabs + "swarm_Nat32_init(&rank, node_" + label + "_input->" + g.nodes[label]['rank'] + ");"
	if g.FILE_TYPE in g.CHPL:
		print tabs + "chpl_free(" + func_name + "_param, 0, NULL);"
	else:
		if not g.REUSE_ARRAYS and g.MEM != "sharedspmd":
			print tabs + "free(node_" + label + "_input->" + func_name + "_param);"
	print tabs + "do => program_exit();"
	entered = False
	for ne in g.nodes_entered:
		if label == ne['label']:
			entered = True
	print tabs + "if (node_" + label + "_input->needs_freed) {"
	print tabs + "\tfree(node_" + label + "_input);"
	print tabs + "}"


def output_switch_case(tabs, label, func_name, target, doing_body_optimization):
	"""output the code for a nonzero switch case"""

	node_input = "node_" + label + "_input"
	node_param = node_input + "->"
	if not doing_body_optimization:
		node_param += func_name + "_param->"
	nwcall = None
	for n in g.nw_calls:
		if n['label'] == target:
			nwcall = n
	if nwcall:
		target = pil2c.pil_nw_call(tabs, nwcall, node_input, target)

	if target == "0": # this is necessary because of the pil_nw_call function
		output_switch_case_0(tabs, label, func_name)
	else:
		if target == label:
			print tabs + "do => node_" + target + "_enter(node_" + target + "_input);"
		else:
			print tabs + "node_" + target + "_Param *node_" + target + "_param = (node_" + target + "_Param *) malloc(sizeof(node_" + target + "_Param));"

			lm = re.match("\d+", g.intervals[target]['lower'])
			if lm:
				interval_init = g.intervals[target]['lower'] + ", "
			elif g.MEM == "distforkjoin" or g.MEM == "distspmd":
				interval_init = node_input + "->_global->" + g.intervals[target]['lower'] + ", "
			else:
				interval_init = node_param + g.intervals[target]['lower'] + ", "
			sm = re.match("\d+", g.intervals[target]['step'])
			if sm:
				interval_init += g.intervals[target]['step'] + ", "
			elif g.MEM == "distforkjoin" or g.MEM == "distspmd":
				interval_init += node_input + "->_global->" + g.intervals[target]['step'] + ", "
			else:
				interval_init += node_param + g.intervals[target]['step'] + ", "
			um = re.match("\d+", g.intervals[target]['upper'])
			if um:
				interval_init += g.intervals[target]['upper']
			elif g.MEM == "distforkjoin" or g.MEM == "distspmd":
				interval_init += node_input + "->_global->" + g.intervals[target]['upper']
			else:
				if g.intervals[target]['upper'] in g.args[g.functions[label]]['list']:
					interval_init += node_param + g.intervals[target]['upper']
				else:
					interval_init += node_input + "->_global->" + g.intervals[target]['upper']
			init = interval_init + ", node_" + label + "_input->_global->index_array, node_" + label + "_input->_global->data_array"
			print tabs + "// [lower, step, upper] = [" + interval_init + "]"
			print tabs + "node_"+ target + "_Param_init(node_" + target + "_param, node_" + label + "_input->_global, " + init + ");"
			for arg_name in g.args[g.functions[target]]['list']:
				if arg_name in g.args[g.functions[label]]['list'] and (g.MEM == "shared" or g.MEM == "sharedspmd"):
					if arg_name == g.nodes[target]['rank']:
						if g.graph[label].has_fork():
							print tabs + "node_" + target + "_param->" + arg_name + " = _pil_index;"
						else:
							print tabs + "node_" + target + "_param->" + arg_name + " = node_" + label + "_input->" + arg_name + ";"
					else:
						print tabs + "node_" + target + "_param->" + arg_name + " = node_" + label + "_input->" + arg_name + ";"
				else:
					print tabs + "node_" + target + "_param->" + arg_name + " = node_" + label + "_input->_global->" + arg_name + ";"
			for arg_name in g.args[g.functions[label]]['list']:
				if arg_name not in g.args[g.functions[target]]['list']:
					print tabs + "node_" + target + "_param->_global->" + arg_name + " = node_" + label + "_input->_global->" + arg_name + ";"
			rank = g.nodes[target]['rank']
			if g.MEM == "sharedspmd" and rank not in g.args[g.functions[target]]:
					print tabs + "node_" + target + "_param->" + rank + " = node_" + label + "_input->" + rank + ";"
			run.file_debug(2, tabs + tabs + 'fprintf(stdout, "calling node_' + target + '.node_' + target + '_enter()\\n"); fflush(stdout);')
			if g.FILE_TYPE in g.CHPL:
				print tabs + "chpl_free(" + func_name + "_param, 0, NULL);"
			else:
				if not g.REUSE_ARRAYS and g.MEM != "sharedspmd":
					print tabs + "free(node_" + label + "_input->" + func_name + "_param);"
			print tabs + "do => node_" + target + "_enter(node_" + target + "_param);"
			entered = False
			for ne in g.nodes_entered:
				if label == ne['label']:
					entered = True
			if target != label:
				print tabs + "if (node_" + label + "_input->needs_freed) {"
				print tabs + "\tfree(node_" + label + "_input);"
				print tabs + "}"


def output_switch_body(tabs, label, func_name, cond, doing_body_optimization):
	"""output the switch statements and cases"""

	#----------------------------------------
	# handle switch case
	#----------------------------------------
	print tabs + "switch(" + cond + ")"
	print tabs + "{"

	# handle the switch statement
	for target in g.targets[label]:
		print tabs + "\tcase " + target + ":"
		print tabs + "\t{"
		if target == "0":
			output_switch_case_0(tabs+"\t\t", label, func_name)
			print tabs + "\t\tbreak;"
		else:
			output_switch_case(tabs+"\t\t", label, func_name, target, doing_body_optimization)
			print tabs + "\t\tbreak;"
		print tabs + "\t}"

	print tabs + "\tdefault:"
	print tabs + "\t{"
	print tabs + "\t\tfprintf(stdout, \"ERROR: this should never happen. case: '%d' in node_" + label + "_switch\\n\", " + cond + ");"
	print tabs + "\t\tswarm_return;"
	print tabs + "\t\tbreak;"
	print tabs + "\t}"
	print tabs + "}"


def output_switch(tabs, label, func_name, key, doing_body_optimization):
	"""output code for a switch statement"""

	if g.MEM == "distforkjoin" or g.MEM == "distspmd":
		output_switch_body(tabs, label, func_name, "*next", doing_body_optimization)
	elif g.graph[label].has_fork():
		print tabs + "// this is a fork"
		print tabs + "_pil_node_" + label + "_counter = 0;"
		if doing_body_optimization:
			print tabs + "_pil_node_" + label + "_total = node_" + label + "_input->_pil_num_targets;"
		else:
			print tabs + "_pil_node_" + label + "_total = node_" + label + "_input->" + func_name + "_param[0]._pil_num_targets;"
		run.file_debug(2, tabs + 'fprintf(stdout, "firing %d nodes\\n", _pil_node_' + label + '_total); fflush(stdout);')
		print tabs + "int _pil_index;"
		print tabs + "for(_pil_index = 0; _pil_index < _pil_node_" + label + "_total; _pil_index++) {"
		cond = key + "[_pil_index]"
		output_switch_body(tabs + "\t", label, func_name, cond, doing_body_optimization)
		print tabs + "}"
	else:
		if doing_body_optimization:
			cond = "node_" + label + "_input->" + key
		else:
			cond = "node_" + label + "_input->" + func_name + "_param[0]." + key
		output_switch_body(tabs, label, func_name, cond, doing_body_optimization)


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
			print "\t\tparam->context->node_" + label + "_input->_global->" + arg_name + " = param->" + arg_name + ";"
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

	for node in g.node_labels:
		print "int _pil_node_" + node + "_counter = 0;"
		print "int _pil_node_" + node + "_total = 0;"
	print ""


def handle_nodes(nodes):
	"""Output the SWARM code for each node in nodes"""

	if g.REUSE_ARRAYS:
		for node in nodes:
			label = node['label']
			print "struct Node_" + label + "_Arrays {"
			print "\tgpp_t index_array;"
			print "\tgpp_t data_array;"
			print "\tsize_t index_array_size;"
			print "\tsize_t data_array_size;"
			print "};"
		#print "struct Node_" + label + "_Arrays *node_" + label + "_arrays = NULL;"
	print ""

	if g.PIL_MAIN:
		print "struct _pil_thread_lock_t {"
		print "\tpthread_cond_t _pil_enter_condition;"
		print "\tpthread_mutex_t _pil_enter_mutex;"
		print "\tint _pil_cond_wait_ret;"
		print "\tint _pil_in_pil_enter;"
		print "};"
		print "struct _pil_thread_lock_t *_pil_thread_lock;"
		print ""
		print "void _pil_thread_lock_init(struct _pil_thread_lock_t *ptl) {"
		print "\tpthread_cond_init(&(ptl->_pil_enter_condition), NULL);"
		print "\tpthread_mutex_init(&(ptl->_pil_enter_mutex), NULL);"
		print "\tptl->_pil_in_pil_enter = FALSE;"
		print "}"
		print ""

	sys.stdout.write("// context for each node function\n")
	for m in nodes:
		sys.stdout.write("//swarm_type node_" + m['label'] + "_Context;\n")
	sys.stdout.write("swarm_type pil_nodes_Context;\n")
	sys.stdout.write("\n")

	for m in nodes:
		swarm_process_type(m['label'], m['index'], m['lower'], m['step'], m['upper'], m['func_name'], m['in_args'])
	if g.REUSE_ARRAYS:
		if g.MEM == "sharedspmd":
			print "struct _pil_reuse_arrays_t {"
			for m in nodes:
				print "\t" + m['func_name'] + "_Param *" + m['func_name'] + "_param;"
				print "\tint num_" + m['func_name'] + "_params;"
			print "};"
			print "struct _pil_reuse_arrays_t *_pil_reuse_arrays;"
			print ""
		else:
			for m in nodes:
				print m['func_name'] + "_Param *" + m['func_name'] + "_param = NULL;"
				print "int num_" + m['func_name'] + "_params = 0;"
			print ""

	print "swarm_type pil_nodes_Param"
	print "{"
	print "\tpil_nodes_Context *context;"
	for node in g.nodes_entered:
		print "\tnode_" + node['label'] + "_Param node_" + node['label'] + "_input;"
	print "};"
	print ""

	print "#pragma swarm implement global_Param"
	print "#pragma swarm implement pil_nodes_Param"
	for m in nodes:
		swarm_process_type_pragmas(m['label'], m['func_name'])
	print ""
	print "// Place to store on the stack all of the global parameters"
	print "global_Param global;"
	print "pil_nodes_Param *pil_params;"
	print "uint32_t NUM_THREADS;"
	print ""

	if g.MEM == "distforkjoin" or g.MEM == "distspmd":
		swarm_process_network_registered_codelets(nodes)
		swarm_process_network_function_prototypes()
	#for m in nodes:
	#	swarm_process_function_prototype(m['label'], m['func_name'])
	if g.PIL_MAIN:
		print "swarm_procedure void pil_nodes(swarm_Nat32_t *label, pil_nodes_Param *pil_params);"
	else:
		print "swarm_procedure void pil_nodes(node_" + g.main_node_label + "_Param *n_" + g.main_node_label + "_i);"

	#swarm_process_global_param_init()
	for m in nodes:
		swarm_process_param_init(m['label'], m['lower'], m['step'], m['upper'], m['func_name'], m['in_args'])

	if g.MEM == "distforkjoin" or g.MEM == "distspmd":
		swarm_process_network_procedures()
	swarm_process_swarm_nodes_procedure(g.main_node_label, g.main_node_func_name)
	for m in nodes:
		swarm_process_node_procedure(m['label'], m['index'], m['func_name'], m['cond'])
	print "\tswarm_codelet program_exit()"
	print "\t{"

	if g.PIL_MAIN:
		if g.MEM == "sharedspmd":
			print "\t\tint _pil_rank = rank.value;"
		else:
			print "\t\tint _pil_rank = 0;"
		print "\t\tpthread_mutex_lock(&(_pil_thread_lock[_pil_rank]._pil_enter_mutex));"
		print "\t\t_pil_thread_lock[_pil_rank]._pil_in_pil_enter = FALSE;"
		print "\t\tpthread_cond_signal(&(_pil_thread_lock[_pil_rank]._pil_enter_condition));"
		print "\t\tpthread_mutex_unlock(&(_pil_thread_lock[_pil_rank]._pil_enter_mutex));"

	print "\t\tswarm_return;"
	print "\t}"
	print "}"
	print ""

	for m in nodes:
		swarm_process_swarm_procedure(m['func_name'])
