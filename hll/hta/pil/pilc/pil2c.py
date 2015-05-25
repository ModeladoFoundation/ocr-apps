###############################################################################
# pil2c
###############################################################################

import run
#from run import file_debug, debug
import g
import sys
import re


#------------------------------------------------------------------------------
# C specific process functions for use by mc
#------------------------------------------------------------------------------

def print_main_func():
	"""output the main function"""

	for line in g.pil_main_lines:
		print line,


def print_funcs():
	"""output the body functions"""

	process_last = []
	for func in g.func_lines.keys():
		for label in g.functions:
			if func == g.functions[label]:
				process_last.append(func)
		if func not in process_last:
			for line in g.func_lines[func]:
				print line,

	for func in process_last:
		for line in g.func_lines[func]:
			print line,

def pass_arguments_declaration(func_name):
	"""generate the string of arguments for the call to node"""

#	arguments = ""
#	first = True
#	for arg in g.args[func_name]['list']:
#		if first:
#			first = False
#		else:
#			arguments += ", "
#		arguments += g.variables[arg] + " " + arg

	arguments = "struct _pil_global_s *_pil_global"
	return arguments


def pass_arguments_call(func_name):
	"""generate the string of argumens for the node"""

#	arguments = ""
#	first = True
#	for arg in g.args[func_name]['list']:
#		if first:
#			first = False
#		else:
#			arguments += ", "
#		arguments += arg

	arguments = "_pil_global"
	return arguments


def c_process_function_prototype(node):
	"""Output a function prototype for each node function"""

	label = node['label']

	prototype = "void node_" + label + "("
	prototype += pass_arguments_declaration(node['func_name'])
	prototype += ");\n"

#	g.h_file.write(prototype)
	sys.stdout.write(prototype)


def print_loop_less_than_zero(tabs, m, print_loop):

	step_is_int = True
	try:
		step = int(m['step'])
	except ValueError:
		step_is_int = False # step is a variable

	upper_is_int = True
	try:
		upper = int(m['upper'])
	except ValueError:
		upper_is_int = False # upper is a variable

	lower_is_int = True
	try:
		lower = int(m['lower'])
	except ValueError:
		lower_is_int = False # lower is a variable

	if step_is_int:
		step = m['step']
	else:
		step = "_pil_global->" + m['step']
	if upper_is_int:
		upper = m['upper']
	else:
		upper = "_pil_global->" + m['upper']
	if lower_is_int:
		lower = m['lower']
	else:
		lower = "_pil_global->" + m['lower']

	index = m['index']
	if print_loop:
		print tabs + "for(" + index + " = " + upper + "; " + index + " >= " + lower + "; " + index + " += " + step + ")"
	else:
		print tabs + index + " = " + lower + ";"


def print_loop_greater_than_zero(tabs, m, print_loop):

	step_is_int = True
	try:
		step = int(m['step'])
	except ValueError:
		step_is_int = False # step is a variable

	upper_is_int = True
	try:
		upper = int(m['upper'])
	except ValueError:
		upper_is_int = False # upper is a variable

	lower_is_int = True
	try:
		lower = int(m['lower'])
	except ValueError:
		lower_is_int = False # lower is a variable

	if step_is_int:
		step = m['step']
	else:
		step = "_pil_global->" + m['step']
	if upper_is_int:
		upper = m['upper']
	else:
		upper = "_pil_global->" + m['upper']
	if lower_is_int:
		lower = m['lower']
	else:
		lower = "_pil_global->" + m['lower']

	index = m['index']
	if print_loop:
		print tabs + "for(" + index + " = " + lower + "; " + index + " <= " + upper + "; " + index + " += " + step + ")"
	else:
		print tabs + index + " = " + lower + ";"


def print_loop_body(tabs, node):
	"""output the loop body of the body node"""

	label = node['label']

	print tabs + "{"
	tabs += "\t"

	split_arrays(tabs, node)

	tabs += "\t"
	call_body_function(tabs, node)

	tabs = tabs[:-1]
	print tabs + "}"
	tabs = tabs[:-1]
	print tabs + "}"


def split_arrays(tabs, node):
	"""print code to split the index and data arrays, if necessary"""

	label = node['label']

	print tabs + "// we create a new local data_array and set its pointer to be an offset of the global data_array as a way to split the data_array."
	print tabs + "// this works because we are on shared memory. the result will be 'merged' for us automatically."
	print tabs + "gpp_t _pil_index_array;"
	print tabs + "_pil_index_array.guid = _pil_global->index_array.guid; // global index_array"
	print tabs + "_pil_index_array.ptr = pil_mem(_pil_global->index_array.guid); // global index_array"
	print tabs + "int *_pil_gia = (int *) _pil_global->index_array.ptr; // global ia pointer"
	print tabs + "gpp_t *_pil_gda = (gpp_t *) _pil_global->data_array.ptr; // global da pointer"
	print tabs + "{"
	tabs += "\t"
	if not g.REUSE_ARRAYS:
		print tabs + "gpp_t index_array; // local index_array"
		print tabs + "gpp_t data_array; // local data_array"

	step_is_int = True
	try:
		step = int(node['step'])
	except ValueError:
		step_is_int = False # step is a variable

	lower_is_int = True
	try:
		lower = int(node['lower'])
	except ValueError:
		lower_is_int = False # lower is a variable

	if step_is_int:
		step = node['step']
	else:
		step = "_pil_global->" + node['step']
	if lower_is_int:
		lower = node['lower']
	else:
		lower = "_pil_global->" + node['lower']
	if pred_has_fork(label):
		print tabs + "int _pil_iter = (" + node['index'] + ") / " + step + ";"
	else:
		print tabs + "int _pil_iter = (" + node['index'] + " - " + lower + ") / " + step + ";"
	print tabs + "if (_pil_index_array.ptr) { // only split if index_array.ptr is not NULL"
	tabs += "\t"
	if g.REUSE_ARRAYS:
		arrays = arrays_string(label)
		num_params = "num_node_" + label + "_params"
		if g.MEM == "spmd":
			num_params = "_pil_reuse_arrays[_pil_rank]." + num_params
		print tabs + "if (" + arrays + "[_pil_iter].index_array_size < 2*sizeof(int)) {"
		tabs += "\t"
		print tabs + "if (" + arrays + "[_pil_iter].index_array.ptr)"
		print tabs + "\tpil_free(" + arrays + "[_pil_iter].index_array);"
		print tabs + "pil_alloc(&(" + arrays + "[_pil_iter].index_array), 2*sizeof(int));"
		print tabs + arrays + "[_pil_iter].index_array_size = 2*sizeof(int);"
		tabs = tabs[:-1]
		print tabs + "} // else we can save the overhead of an alloc and reuse the previously allocated size."
		print tabs + "int *_pil_lia = (int *) " + arrays + "[_pil_iter].index_array.ptr; // local ia pointer"
	else:
		print tabs + "pil_alloc(&index_array, 2*sizeof(int));"
		print tabs + "int *_pil_lia = (int *) index_array.ptr; // local ia pointer"
	print tabs + "int _pil_num = _pil_gia[_pil_iter+1] - _pil_gia[_pil_iter];"
	print tabs + "if (_pil_num > 0) {"
	tabs += "\t"
	print tabs + "_pil_lia[0] = 0;"
	print tabs + "_pil_lia[1] = _pil_num;"
	print ""
	print tabs + "size_t _pil_data_size = _pil_num*sizeof(gpp_t);"
	if g.REUSE_ARRAYS:
		print tabs + "if (" + arrays + "[_pil_iter].data_array_size < _pil_data_size) {"
		tabs += "\t"
		print tabs + "if (" + arrays + "[_pil_iter].data_array.ptr)"
		print tabs + "\tpil_free(" + arrays + "[_pil_iter].data_array);"
		print tabs + "pil_alloc(&(" + arrays + "[_pil_iter].data_array), _pil_data_size);"
		print tabs + "" + arrays + "[_pil_iter].data_array_size = _pil_data_size;"
		tabs = tabs[:-1]
		print tabs + "} // else we can save the overhead of an alloc and reuse the previously allocated size."
		print tabs + "gpp_t *_pil_lda = (gpp_t *) " + arrays + "[_pil_iter].data_array.ptr; // local da pointer"
	else:
		print tabs + "pil_alloc(&data_array, _pil_data_size);"
		print tabs + "//data_array.ptr = &_pil_gda[_pil_gia[_pil_iter]];"
		print tabs + "gpp_t *_pil_lda = (gpp_t *) data_array.ptr; // local da pointer"
	print tabs + "memcpy(_pil_lda, &_pil_gda[_pil_gia[_pil_iter]], _pil_data_size);"
	print ""
	print tabs + "for (int _pil_index = 0; _pil_index < _pil_num; _pil_index++) {"
	print tabs + "\t_pil_lda[_pil_index].ptr = pil_mem(_pil_lda[_pil_index].guid);"
	print tabs + "}"
	tabs = tabs[:-1]
	print tabs + "}"
	tabs = tabs[:-1]
	print tabs + "}"


def call_body_function(tabs, node):
	"""output code to call the body function"""

	in_args = body_function_arguments(node)

	print tabs + node['func_name'] + "(" + in_args + "); // function call with the new local data_array set appropriately"

	# reassign the arguments to the global parameter
#	for arg in g.args[node['func_name']]['list']:
#		if arg == "index_array" or arg == "data_array" or arg in g.arrays:
#			continue
#		print tabs + "_pil_global->" + arg + " = " + arg + ";"


def arrays_string(label):
	"""   """
	arrays = "node_" + label + "_arrays"
	if g.MEM == "spmd":
		arrays = "_pil_reuse_arrays[_pil_rank]." + arrays
	return arrays

def body_function_arguments(node):
	"""make a list of the arguments to the body function"""

	label = node['label']
	func_name = node['func_name']

#	for arg in g.args[node['func_name']]['list']:
#		if arg == node['index'] or arg == "index_array" or arg == "data_array" or arg in g.arrays:
#			continue
#		else:
#			print tabs + "\t\t" + g.variables[arg] + " " + arg + " = _pil_global->" + arg + ";"
#	in_args = node['in_args']
#	if g.REUSE_ARRAYS:
#		in_args = re.sub("index_array", arrays + "[_pil_iter].index_array", in_args)
#		in_args = re.sub("data_array", arrays + "[_pil_iter].data_array", in_args)

	in_args = ""
	first = True
	for arg in g.args[func_name]['list']:
		if first:
			first = False
		else:
			in_args += ", "

		if g.REUSE_ARRAYS and (arg == "index_array" or arg == "data_array"):
			if g.args[func_name]['dict'][arg]['passed_by_reference']:
				in_args += "&"
			in_args += arrays_string(label) + "[_pil_iter]." + arg
		else:
			if g.args[func_name]['dict'][arg]['passed_by_reference']:
				in_args += "&"
			if arg == g.nodes[label]['index']:
				in_args += arg
			#elif arg in g.arrays:
			#	in_args += arg
			else:
				in_args += "_pil_global->" + arg

	return in_args


def print_omp_pragma(tabs, node):

	print "#ifdef OMP"
	print tabs + "#pragma omp parallel for private(" + node['index'] + ") schedule(runtime)"
	print "#endif"


def pred_has_fork(label):
	"""return True if the predecessor of the label has a fork"""

	for p in g.graph[label].get_pred():
		if g.graph[p].has_fork():
			return True
	return False


def print_num_iter(tabs, upper, lower, step):
	"""output code to calculate the number of iterations of this pil node"""

	print tabs + "int _pil_num_iter = 0;"
	print tabs + "if (" + step + " < 0)"
	print tabs + "{"
	print tabs + "\t_pil_num_iter = (" + upper + " - " + lower + ")/(0 - " + step + ") + 1;"
	print tabs + "}"
	print tabs + "else"
	print tabs + "{"
	print tabs + "\t_pil_num_iter = (" + upper + " - " + lower + ")/" + step + " + 1;"
	print tabs + "}"


# TODO: change this function so it doesn't take 'm', but the pieces of 'm' that it needs
def c_process_node_procedure(m):
	"""Output the body of a node function"""

	tabs = "\t"

	print "void node_" + m['label'] + "(" + pass_arguments_declaration(m['func_name']) + ")"
	print "{"
	run.file_debug(1, tabs + "fprintf(stdout, \"node_" + m['label'] + "\\n\");")
	run.file_debug(2, tabs + "fprintf(stdout, \"lower: %d  upper: %d  step: %d\\n\", " + m['lower'] + ", " + m['upper'] + ", " + m['step'] + ");")

	if g.graph[m['label']].has_merge():
		merge = g.graph[m['label']].get_matching_fork()
		if (merge == "0"):
			print "ERROR: node " + m['label'] + " has a merge point, but it doesn't have a matching fork"
			print "known fork sites are:"
			for n in g.graph:
				if g.graph[n].has_fork():
					print n
			print "exit(1);"
		#print '\tprintf("in node_' + m['label'] + '\\n");'
		print tabs + "_pil_node_" + merge + "_counter += 1;"
		print tabs + "if (_pil_node_" + merge + "_counter != _pil_node_" + merge + "_total) {"
		print tabs + "\treturn;"
		print tabs + "}"

#	if "P" in g.variables:
#		print "#ifdef OMP"
#		print tabs + "omp_set_num_threads(P);"
#		print "#endif"
#		print ""

	#print tabs + "for(" + m['index'] + " = " + m['lower'] + "; " + m['index'] + " != (" + m['upper'] + "+" + m['step'] + "); " + m['index'] + " += " + m['step'] + ")"
	step_is_int = True
	try:
		step = int(m['step'])
	except ValueError:
		step_is_int = False # step is a variable

	upper_is_int = True
	try:
		upper = int(m['upper'])
	except ValueError:
		upper_is_int = False # upper is a variable

	lower_is_int = True
	try:
		lower = int(m['lower'])
	except ValueError:
		lower_is_int = False # lower is a variable

#	print tabs + "// locally declare the index variable so the complier doesn't complain about it being global"
#	print tabs + g.variables[m['index']] + " " + m['index'] + ";"

#	for arg in g.args[m['func_name']]['list']:
#		print tabs + g.variables[arg] + " " + arg + " = _pil_global->" + arg + ";"
#	if m['index'] not in g.args[m['func_name']]['list']:
	print tabs + g.variables[m['index']] + " " + m['index'] + " = _pil_global->" + m['index'] + ";"
	if g.MEM == "spmd" and g.REUSE_ARRAYS:
		print tabs + "int _pil_rank = _pil_global->" + m['rank'] + ";"

	if g.REUSE_ARRAYS:
		if step_is_int:
			step = m['step']
		else:
			step = "_pil_global->" + m['step']
		if upper_is_int:
			upper = m['upper']
		else:
			upper = "_pil_global->" + m['upper']
		if lower_is_int:
			lower = m['lower']
		else:
			lower = "_pil_global->" + m['lower']

		label = m['label']
		print ""

		print_num_iter(tabs, upper, lower, step)

		arrays = "node_" + label + "_arrays"
		num_params = "num_node_" + label + "_params"
		if g.MEM == "spmd":
			arrays = "_pil_reuse_arrays[_pil_rank]." + arrays
			num_params = "_pil_reuse_arrays[_pil_rank]." + num_params

		print tabs + "if (" + arrays + " == NULL || " + num_params + " < _pil_num_iter) {"
		print tabs + "\tfor (int _pil_index = 0; _pil_index < " + num_params + "; ++_pil_index) {"
		print tabs + "\t\tfree(" + arrays + "[_pil_index].index_array.ptr);"
		print tabs + "\t\tfree(" + arrays + "[_pil_index].data_array.ptr);"
		print tabs + "\t}"
		print tabs + "\tfree(" + arrays + ");"
		print tabs + "\t" + arrays + " = malloc(_pil_num_iter * sizeof(struct Node_" + label + "_Arrays));"
		print tabs + "\tfor (int _pil_index = 0; _pil_index < _pil_num_iter; ++_pil_index) {"
		print tabs + "\t\t" + arrays + "[_pil_index].index_array.ptr = NULL;"
		print tabs + "\t\t" + arrays + "[_pil_index].data_array.ptr = NULL;"
		print tabs + "\t\t" + arrays + "[_pil_index].index_array_size = 0;"
		print tabs + "\t\t" + arrays + "[_pil_index].data_array_size = 0;"
		print tabs + "\t}"
		print tabs + "\t" + num_params + " = _pil_num_iter;"
		print tabs + "}"
		print ""


	if pred_has_fork(m['label']):
		print_loop_body(tabs, m)
	else:
		if step_is_int:
			print_pragma = True
			if step < 0:
				if upper_is_int and lower_is_int:
					num_iter = (int(upper) - int(lower))/(0 - int(step)) + 1
					if num_iter <= 1:
						print_pragma = False
				if print_pragma:
					print_omp_pragma(tabs, m)
				print_loop_less_than_zero(tabs, m, print_pragma)
			else:
				if upper_is_int and lower_is_int:
					num_iter = (int(upper) - int(lower))/int(step) + 1
					if num_iter <= 1:
						print_pragma = False
				if print_pragma:
					print_omp_pragma(tabs, m)
				print_loop_greater_than_zero(tabs, m, print_pragma)
			print_loop_body(tabs, m)
		else:
			print tabs + "if(_pil_global->" + m['step'] + " < 0)"
			print tabs + "{"
			tabs += "\t"
			print_omp_pragma(tabs, m)
			print_loop_less_than_zero(tabs, m, True)
			print_loop_body(tabs, m)
			tabs = tabs[:-1]
			print tabs + "}"
			print tabs + "else"
			print tabs + "{"
			tabs += "\t"
			print_omp_pragma(tabs, m)
			print_loop_greater_than_zero(tabs, m, True)
			print_loop_body(tabs, m)
			tabs = tabs[:-1]
			print tabs + "}"

	print ""
	print "\n" + tabs + "// TODO: need to merge index_array and data_array"
	print ""

	if g.graph[m['label']].has_fork():
		print tabs + "_pil_node_" + m['label'] + "_counter = 0;"
		print tabs + "_pil_node_" + m['label'] + "_total = _pil_global->_pil_num_targets;"
		print tabs + "int _pil_tasks_created = 0;"
		print tabs + "for(int _pil_index = 0; _pil_index < _pil_global->_pil_num_targets; _pil_index++) {"
		output_switch(tabs + "\t", m['label'], "_pil_global->" + m['cond'] + "[_pil_index]")
		print tabs + "}"
		print tabs + "#pragma omp taskwait"
	else:
		output_switch(tabs, m['label'], "_pil_global->" + m['cond'])

	print "}"
	print ""


def pil_nw_call(tabs, nwc, param, target):
	"""output code for a pil network call"""

	line = nwc['func'] + "("
	if nwc['func'] == "pil_send":
		line += param + "->" + nwc['rank'] + ", " + param + "->" + nwc['dest'] + ", " + param + "->" + nwc['buf']
	elif nwc['func'] == "pil_recv":
		line += param + "->" + nwc['rank'] + ", " + param + "->" + nwc['src']  + ", &" + param + "->" + nwc['buf']
	else:
		print "ERROR: wrong network function"
	line += ", " + param + "->" + nwc['size'] + ", " + param + "->" + nwc['offset'] + ", " + nwc['succ'] + ");"
	print tabs + line
	next_targets = g.graph[target].get_succ()
	next_target = next_targets[0]

	nextnwc = None
	for n in g.nw_calls:
		if next_target == n['label']:
			nextnwc = n
			break
	if nextnwc:
		return pil_nw_call(tabs, nextnwc, param, next_target)
	return next_target


def output_switch(tabs, label, switch_case):
	"""output the switch statements for ending a node"""

	print tabs + "switch (" + switch_case + ")"
	print tabs + "{"
	for target in g.targets[label]:
		print tabs + "\tcase " + target + ":"
		print tabs + "\t{"
		# TODO: This assumes that a target of 0 is exit

		if target == "0":
			print tabs + "\t\t//pil_cleanup();"
			print tabs + "\t\treturn;"
		else:
			nwc = None
			for n in g.nw_calls:
				if target == n['label']:
					nwc = n
					break
			if nwc:
				next_target = pil_nw_call(tabs, nwc, "_pil_global", target)
				if next_target == "0":
					pass
				else:
					print tabs + "\t\tnode_" + next_target + "(_pil_global);"
			elif g.graph[label].has_fork():
				print tabs + "\t\t_pil_global->" + g.nodes[target]['rank'] + " = _pil_global->_pil_task_names[_pil_tasks_created++];"
				print tabs + "\t\tstruct _pil_global_s _pil_new_global = *_pil_global;"
				print tabs + "\t\t#pragma omp task firstprivate(_pil_new_global)"
				print tabs + "\t\t{"
				#print tabs + "\t\t\tstruct _pil_global_s _new_pil_global;"
				#copy_pil_global(tabs + "\t\t\t", target, "_new_pil_global", "_pil_global")
				print tabs + "\t\t\tnode_" + target + "(&_pil_new_global);"
				print tabs + "\t\t}"
			else:
				print tabs + "\t\tnode_" + target + "(" + pass_arguments_call(g.functions[target]) + ");"

		print tabs + "\t\tbreak;"
		print tabs + "\t}"
	print tabs + "\tdefault:"
	print tabs + "\t{"
	print tabs + "\t\tfprintf(stdout, \"ERROR: Unknown case '%d' in node_" + label + "\\n\", " + switch_case + ");"
	if g.FILE_TYPE in g.CHPL:
		print tabs + "\t\tchpl_exit_all(EXIT_FAILURE);"
	else:
		print tabs + "\t\texit(EXIT_FAILURE);"
	print tabs + "\t\tbreak;"
	print tabs + "\t}"
	print tabs + "}"


def copy_pil_global(tabs, target, dest, src):
	"""copy the values of src to dest"""

	for var_name in g.variables:
		line = tabs
		if var_name in g.arrays:
			pil2ocr.copy_array(tabs, target, var_name, dest, src)
		else:
			line += dest + "." + var_name + " = " + src + "->" + var_name
		line +=  ";"
		if var_name in g.arrays:
			continue
		print line


def process_header():
	"""Outputs the header for the file. E.g., includes libraries."""

	run.process_header()

	print "#ifdef OMP"
	print "#include <omp.h>"
	print "#endif"
	print ""


def process_variables():
	"""Create the global variables"""

	run.debug(1,"process_variables")


	print "// Global variables"
	print "struct _pil_global_s {"
	tabs = "\t"
	for var_name in g.variables:
		run.debug(4,var_name + "\n")
		declaration = ""
		if var_name in g.arrays:
			#declaration += g.variables[var_name] + " (*" + var_name + ")[" + g.arrays[var_name] + "]"
			declaration += g.variables[var_name] + " " + var_name
			for size in g.arrays[var_name]:
				declaration += "[" + size + "]"
		else:
			declaration += g.variables[var_name] + " " + var_name
			if var_name in g.initial_values:
				declaration += " = " + g.initial_values[var_name]
		declaration +=  ";"
		if var_name in g.arrays:
			continue
		print tabs + declaration
	for array in g.arrays:
		declaration = ""
		declaration += g.variables[array] + " " + array
		for size in g.arrays[array]:
			declaration += "[" + size + "]"
		declaration += ";"
		print tabs + declaration
	print "};"
	for node in g.node_labels:
		print "int _pil_node_" + node + "_counter = 0;"
		print "int _pil_node_" + node + "_total = 0;"
	print ""

#	print "// array globals are here until we can handle them better"
#	for var in g.arrays:
#		string = g.variables[var] + " " + var
#		for size in g.arrays[var]:
#			string += "[" + size + "]"
#		string += ";"
#		print string

#	print ""
#	print "int _pil_barrier_counter = 0;"
#	print "sem_t _pil_turnstile;"
#	print "sem_t _pil_turnstile2;"
#	print "sem_t _pil_mutex;"
#	print ""


def handle_nodes(nodes):
	"""Output the C code for each node in nodes."""

	if g.REUSE_ARRAYS:
		for node in nodes:
			print "struct Node_" + node['label'] + "_Arrays {"
			print "\tgpp_t index_array;"
			print "\tgpp_t data_array;"
			print "\tsize_t index_array_size;"
			print "\tsize_t data_array_size;"
			print "};"

		tabs = ""
		if g.MEM == "spmd":
			print "struct _pil_reuse_arrays_t {"
			tabs += "\t"
			for node in nodes:
				print tabs + "struct Node_" + node['label'] + "_Arrays *node_" + node['label'] + "_arrays;"
				print tabs + "int num_node_" + node['label'] + "_params;"
			print "};"
			print "struct _pil_reuse_arrays_t *_pil_reuse_arrays;"
		else:
			for node in nodes:
				print tabs + "struct Node_" + node['label'] + "_Arrays *node_" + node['label'] + "_arrays;"
				print tabs + "int num_node_" + node['label'] + "_params;"
		print ""

	g.h_file.write("// Node function prototypes\n")
	print "//----------------------------------------"
	print "// Node function prototypes"
	print "//----------------------------------------"
	print ""
	for n in nodes:
		c_process_function_prototype(n)
	print "\n"

	print "//----------------------------------------"
	print "// Node function bodies"
	print "//----------------------------------------"
	print ""
	for n in nodes:
		c_process_node_procedure(n)
	print "\n"


def declare_pil_enter_arguments():
	"""declare arguments for pil_enter"""

	print "\t struct _pil_global_s _pil_global;"


def process_pil_enter_arguments(node):
	"""Output the appropriate code to enter a node"""

	#print "#ifdef PIL2C"
	args = g.args[g.functions[node['label']]]
	for arg_name in args['list']:
		if arg_name == g.target_variables[node['label']]:
			continue
		#elif arg_name == 'index_array' or arg_name == 'data_array':
		#	continue
		if g.args[g.functions[node['label']]]['dict'][arg_name]['passed_by_reference']:
			print "\t\t\t" + arg_name + "_ptr = va_arg(argptr, " + g.variables[arg_name] + " *);"
			print "\t\t\t_pil_global." + arg_name + " = *" + arg_name + "_ptr;"
		else:
			print "\t\t\t_pil_global." + arg_name + " = va_arg(argptr, " + g.variables[arg_name] + ");"



	#print "#endif"
	print "\t\t\tbreak;"


def process_pil_enter(nodes, node):
	"""Output the appropriate code to enter a node"""

	lower = 0
	step = 0
	upper = 0

	for n in nodes:
		if n['label'] == node['label']:
			step_is_int = True
			try:
				step = int(n['step'])
			except ValueError:
				step_is_int = False # step is a variable

			upper_is_int = True
			try:
				upper = int(n['upper'])
			except ValueError:
				upper_is_int = False # upper is a variable

			lower_is_int = True
			try:
				lower = int(n['lower'])
			except ValueError:
				lower_is_int = False # lower is a variable

			if step_is_int:
				step = n['step']
			else:
				step = "_pil_global." + n['step']
			if upper_is_int:
				upper = n['upper']
			else:
				upper = "_pil_global." + n['upper']
			if lower_is_int:
				lower = n['lower']
			else:
				lower = "_pil_global." + n['lower']
			func_name = n['func_name']

	print "#ifdef PIL2C"
	print "/*"
	print "\t\t\tif (_pil_global.index_array.guid == NULL_GUID) {"
	print_num_iter("\t\t\t\t", upper, lower, step)
	print "\t\t\t\tpil_alloc(&_pil_global.index_array, (_pil_num_iter+1) * sizeof(int));"
	print "\t\t\t\tint *_pil_ia = (int *) _pil_global.index_array.ptr;"
	print "\t\t\t\tfor (int _pil_index = 0; _pil_index <= _pil_num_iter; _pil_index++) {"
	print "\t\t\t\t\t_pil_ia[_pil_index] = 0;"
	print "\t\t\t\t}"
	print "\t\t\t}"
	print "*/"
	print "\t\t\tnode_" + node['label'] + "(&_pil_global);"
	args = g.args[g.functions[node['label']]]
	for arg_name in args['list']:
		if arg_name == g.target_variables[node['label']]:
			continue
		#elif arg_name == 'index_array' or arg_name == 'data_array':
		#	continue
		if g.args[g.functions[node['label']]]['dict'][arg_name]['passed_by_reference']:
			print "\t\t\t*" + arg_name + "_ptr = _pil_global." + arg_name + ";"
	print "\t\t\tbreak;"
	print "#endif"


def process_main():
	"""Output the main function for the file."""

	if g.FILE_TYPE in g.CHPL:
		print "void chpl_main()"
	else:
		print "int main(int ac, char **av)"
	print "{"
	tabs = "\t"
	run.file_debug(1, tabs + 'fprintf(stdout, "Using C backend of PIL\\n");')
#	run.set_stack_size()

	print tabs + "// initialize PIL"
#	print tabs + "int _pil_NP = pil_get_nwCount();"

	fork_in_graph = False
	for node_label in g.graph:
		if g.graph[node_label].has_fork():
			fork_in_graph = True
	if fork_in_graph:
		print "#ifdef OMP"
		print tabs + "omp_set_nested(1);"
		print "#endif"

	print tabs + "pil_nw_init();"
#	print tabs + "sem_init(&_pil_mutex, 0, 1);"
#	print tabs + "sem_init(&_pil_turnstile, 0, 0);"
#	print tabs + "sem_init(&_pil_turnstile2, 0, 1);"
#	print tabs + "_pil_send_buf = (struct _pil_communication_buffers **) malloc(_pil_NP*sizeof(struct _pil_communication_buffers *));"
	if g.MEM == "spmd" and g.REUSE_ARRAYS:
		print tabs + "_pil_reuse_arrays = (struct _pil_reuse_arrays_t *) malloc(pil_get_nwCount() * sizeof(struct _pil_reuse_arrays_t));"
		print tabs + "for (int _pil_index = 0; _pil_index < pil_get_nwCount(); _pil_index++) {"
		for node in g.nodes:
			print tabs + "\t_pil_reuse_arrays[_pil_index].node_" + g.nodes[node]['label'] + "_arrays = NULL;"
			print tabs + "\t_pil_reuse_arrays[_pil_index].num_node_" + g.nodes[node]['label'] + "_params = 0;"
#	print tabs + "\t_pil_send_buf[_pil_index] = (struct _pil_communication_buffers *) malloc(_pil_NP*sizeof(struct _pil_communication_buffers));"
#	print tabs + "\tfor (int _pil_i = 0; _pil_i < _pil_NP; _pil_i++) {"
#	print tabs + "\t\t_pil_send_buf[_pil_index][_pil_i].ptr = NULL;"
#	print tabs + "\t\t_pil_send_buf[_pil_index][_pil_i].size = 0;"
#	print tabs + "\t\t_pil_send_buf[_pil_index][_pil_i].full = 0;"
#	print tabs + "\t}"
		print tabs + "}"
	print ""

	if fork_in_graph or g.MEM == "spmd":
		print tabs + "// set up parallelism"
		print "#ifdef OMP"
		print tabs + "#pragma omp parallel"
		print "#endif"
		print tabs + "{"
		tabs += "\t"

	if fork_in_graph:
		print "#ifdef OMP"
		print tabs + "#pragma omp single"
		print "#endif"
		print tabs + "{"
		tabs += "\t"

	if g.MEM == "spmd":
		print tabs + "#pragma omp for"
		print tabs + "for (int _pil_rank = 0; _pil_rank < pil_get_nwCount(); _pil_rank++) {"
		tabs += "\t"
	if g.PIL_MAIN:
		if g.MEM == "spmd":
			#print tabs + "#pragma omp task"
			print tabs + "{"
			print tabs + "\tpil_main(ac, av, _pil_rank);"
			print tabs + "} // omp for"
		else:
			print tabs + "int _pil_rank = 0;"
			print tabs + "pil_main(ac, av, _pil_rank);"
	else:
		print tabs + "struct _pil_global_s _pil_global;"
		if g.MEM == "spmd":
			#print tabs + "#pragma omp task private(_pil_global)"
			print tabs + "{"
			tabs += "\t"
			print tabs + "_pil_global." + g.nodes[g.main_node_label]["rank"] + " = _pil_rank;"
		for var_name in g.variables:
			if var_name == "argc":
				print tabs + "_pil_global.argc = ac;"
			elif var_name == "argv":
				print tabs + "_pil_global.argv = av;"
		print tabs + "pil_init(&(_pil_global.index_array), &(_pil_global.data_array));"
		print tabs + "node_" + g.main_node_label + "(&_pil_global);"
		if g.MEM == "spmd":
			tabs = tabs[:-1]
			print tabs + "} // omp for"
	if g.MEM == "spmd":
		tabs = tabs[:-1]
		print tabs + "} // for rank"
		#print tabs + "#pragma omp taskwait"

	if fork_in_graph:
		tabs = tabs[:-1]
		print tabs + "} // omp single"
	if fork_in_graph or g.MEM == "spmd":
		tabs = tabs[:-1]
		print tabs + "} // omp parallel"

	if g.FILE_TYPE in g.CHPL:
		print tabs + "return;"
	else:
		print tabs + "return EXIT_SUCCESS;"
	print "}"
