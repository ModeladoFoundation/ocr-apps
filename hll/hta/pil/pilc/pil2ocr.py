###############################################################################
# pil2ocr
###############################################################################

import run
#from run import file_debug, debug
import g
import sys
import re
import pil2c


#------------------------------------------------------------------------------
# OCR specific process functions for use by pilc
#------------------------------------------------------------------------------

def print_main_func():
	"""output the main function"""

	for line in g.pil_main_lines:
		print line,


def print_funcs():
	"""output the body functions"""

	if g.RSTREAM:
		outfile = open(g.FILE + ".src.c", 'w')

	for func in g.func_lines.keys():
		for line in g.func_lines[func]:
			if g.RSTREAM:
				outfile.write(line)
			else:
				print line,

	if g.RSTREAM:
		outfile.close()


def ocr_process_function_prototype(label, func_name):
	"""Output a function prototype for each node function"""

	print "ocrGuid_t node_" + label + "_enter(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]);"
	print "ocrGuid_t node_" + label + "_body(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]);"
	print "ocrGuid_t node_" + label + "_exit(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]);"


def ocr_process_type(label, index, func_name, lower, step, upper, in_args):
	"""processes a type"""

	print "//--------------------------------------"
	print "// types for node_" + label
	print "//--------------------------------------"
	print ""

	# output the type for the node
	print "// type for the node_" + label + " function"
	print "struct node_" + label + "_param"
	print "{"
	# TODO: these names could clash with user defined variable names. what do we do about that?
	#print "\tstruct global_param *_global; // TODO: make this be global_db_guid instead" # TODO: make this be global_db_guid instead
	print "\tocrGuid_t global_db_guid;"
	print "\tint32_t " + g.LOWER + "; // initial: " + lower
	print "\tint32_t " + g.STEP + "; // initial: " + step
	print "\tint32_t " + g.UPPER + "; // initial: " + upper
	print "\tuint64_t _pil_iter;"
	print "\tocrGuid_t node_" + label + "_exit_param_event_guid;"
	print "\tocrGuid_t node_" + label + "_exit_index_array_event_guid;"
	print "\tocrGuid_t node_" + label + "_exit_data_array_event_guid;"
	print ""

	if index not in g.args[func_name]['list']:
		print "\t" + g.variables[index] + " " + index + ";"
	rank = g.nodes[label]['rank']
	if rank not in g.args[func_name]['list']:
		print "\tint " + rank + ";"
	for arg_name in g.args[func_name]['list']:
		if arg_name == "index_array" or arg_name == "data_array":
			pass
		else:
			string = "\t" + g.variables[arg_name] + " " + arg_name
			if arg_name in g.arrays:
				for size in g.arrays[arg_name]:
					string += "[" + size + "]"
			string += ";"
			print string

	print "};"
	print ""

	print "int _pil_node_" + label + "_counter = 0;"
	print "int _pil_node_" + label + "_total = 0;"
	print ""


def copy_array(tabs, target, arg_name, dest, src):
	"""copy a statically declared array from one parameter to another"""

	index = ""
	counter = 0
	for size in g.arrays[arg_name]:
		variable = "_pil_index" + str(counter)
		index += "[" + variable + "]"
		print tabs + "for (int " + variable + " = 0; " + variable + " < " + size + "; " + variable + "++) {"
		tabs = increment_tabs(tabs)
		counter += 1
	print tabs + "\t" + dest + "->" + arg_name + index + " = " + src + "->" + arg_name + index + ";"
	for size in g.arrays[arg_name]:
		tabs = decrement_tabs(tabs)
		print tabs + "}"


def enter_body(tabs, target, func_name, index):
	""""""

	print tabs + "// create one event per body instance to notify when to run exit"
	print tabs + "ocrGuid_t exit_param_event_guid;"
	print tabs + "ocrEventCreate(&exit_param_event_guid, PIL_EVENT_T, true);"
	run.file_debug(3, tabs + "fprintf(stdout, \"created exit_event (%ld) for iter %ld\\n\", exit_param_event_guid, iter);")
	print tabs + "ocrAddDependence(exit_param_event_guid, exit_guid, /*slot=*/(3*iter)+0+3, /*mode*/DB_DEFAULT_MODE);"
	run.file_debug(3, tabs + "fprintf(stdout, \"added dependence between exit edt (%ld) and exit_param_event (%ld)for node_" + target + "_exit\\n\", exit_guid, exit_param_event_guid);")
	print tabs + "ocrGuid_t exit_index_array_event_guid;"
	print tabs + "ocrEventCreate(&exit_index_array_event_guid, PIL_EVENT_T, true);"
	run.file_debug(3, tabs + "fprintf(stdout, \"created exit_event (%ld) for iter %ld\\n\", exit_index_array_event_guid, iter);")
	print tabs + "ocrAddDependence(exit_index_array_event_guid, exit_guid, /*slot=*/(3*iter)+1+3, /*mode*/DB_DEFAULT_MODE);"
	run.file_debug(3, tabs + "fprintf(stdout, \"added dependence between exit edt (%ld) and exit_index_array_event (%ld)for node_" + target + "_exit\\n\", exit_guid, exit_index_array_event_guid);")
	print tabs + "ocrGuid_t exit_data_array_event_guid;"
	print tabs + "ocrEventCreate(&exit_data_array_event_guid, PIL_EVENT_T, true);"
	run.file_debug(3, tabs + "fprintf(stdout, \"created exit_event (%ld) for iter %ld\\n\", exit_data_array_event_guid, iter);")
	print tabs + "ocrAddDependence(exit_data_array_event_guid, exit_guid, /*slot=*/(3*iter)+2+3, /*mode*/DB_DEFAULT_MODE);"
	run.file_debug(3, tabs + "fprintf(stdout, \"added dependence between exit edt (%ld) and exit_data_array_event (%ld)for node_" + target + "_exit\\n\", exit_guid, exit_data_array_event_guid);")
	print ""

	print tabs + "// allocate a data block for the parameter to pass in"
	print tabs + "ocrGuid_t param_db_guid;"
	print tabs + "if (num_iterations > 1) {"
	allocate_param_db(tabs + "\t", "", target, "n")
	print tabs + "}"
	print tabs + "else {"
	print tabs + "\tparam_db_guid = depv[0].guid;"
	print tabs + "\tn" + target + "p = input;"
	print tabs + "}"
	print ""

	print tabs + "n" + target + "p->node_" + target + "_exit_param_event_guid = exit_param_event_guid;"
	print tabs + "n" + target + "p->node_" + target + "_exit_index_array_event_guid = exit_index_array_event_guid;"
	print tabs + "n" + target + "p->node_" + target + "_exit_data_array_event_guid = exit_data_array_event_guid;"

	print tabs + "if (num_iterations > 1) {"
	tabs += "\t"
	print tabs + "// populate the parameter"
	ocr_param_init(tabs, target, target, "n")
	print tabs + "n" + target + "p->_pil_iter = iter;"

	# pass on each argument
	for arg_name in g.args[func_name]['list']:
		if arg_name == "argc":
			print tabs + "//n" + target + "p->argc = cmd_line_ptr->argc;"
			print tabs + "n" + target + "p->argc = input->argc;"
		elif arg_name == "argv":
			print tabs + "//n" + target + "p->argv = argv;"
			print tabs + "n" + target + "p->argv = input->argv;"
		elif arg_name == "index_array" or arg_name == "data_array":
			continue
		else:
			if arg_name in g.arrays:
				copy_array(tabs, target, arg_name, "n"+target+"p", "input")
			else:
				print tabs + "n" + target + "p->" + arg_name + " = input->" + arg_name + ";"
	arg_name = g.nodes[target]['rank']
	if arg_name not in g.args[func_name]['list']:
		print tabs + "n" + target + "p->" + arg_name + " = input->" + arg_name + ";"

	tabs = tabs[:-1]
	print tabs + "}"
	print ""

	index_helper(tabs, False)

	print tabs + "// split data_array"
	print tabs + "gpp_t _pil_split_index_array;"
	print tabs + "gpp_t _pil_split_data_array;"
	print tabs + "_pil_split_index_array.guid = index_array.guid;"
	print tabs + "_pil_split_index_array.ptr = index_array.ptr;"
	print tabs + "_pil_split_data_array.guid = data_array.guid;"
	print tabs + "_pil_split_data_array.ptr = data_array.ptr;"
	print tabs + "gpp_t *_pil_sda = NULL;"
	print tabs + "if (num_iterations > 1 && index_array.guid != NULL_GUID) {"
	tabs += "\t"
	print tabs + "size_t _pil_index_size = 2 * sizeof(int);"
	print tabs + "size_t _pil_data_size = _pil_num * sizeof(gpp_t);"
	allocate_db(tabs, "_pil_split_index_array.guid", "_pil_split_index_array.ptr", "_pil_index_size");
	allocate_db(tabs, "_pil_split_data_array.guid", "_pil_split_data_array.ptr", "_pil_data_size");
	print tabs + "// initialize the split index array\n"
	print tabs + "int *_pil_sia = (int *) _pil_split_index_array.ptr;"
	print tabs + "_pil_sia[0] = 0;"
	print tabs + "_pil_sia[1] = _pil_num;"
	print tabs + "_pil_sda = (gpp_t *) _pil_split_data_array.ptr;"
	print tabs + "memcpy(_pil_split_data_array.ptr, &_pil_da[_pil_ia[iter]], _pil_data_size);"
	tabs = tabs[:-1]
	print tabs + "}"
	print tabs + "else"
	print tabs + "{"
	print tabs + "\t_pil_sda = (gpp_t *) data_array.ptr;"
	print tabs + "}"
	print ""

	enter_node(tabs, True, False, target)
#	print tabs + "// create an event for a dependence on the parameter data block"
#	create_event(tabs, target, "param")
#	create_event(tabs, target, "global")
#	create_event(tabs, target, "index_array")
#	create_event(tabs, target, "data_array")
#	print ""
#
#	print tabs + "// create the body edt"
#	create_edt(tabs, target, "NULL", False, True)
#
#	print tabs + "// add dependences"
#	create_dependence(tabs, target, True, "param", 0)
#	create_dependence(tabs, target, True, "global", 1)
#	create_dependence(tabs, target, True, "index_array", 2)
#	create_dependence(tabs, target, True, "data_array", 3)
#	print ""
#	print tabs + "// add the contents of the data_array as dependences to the body node"
#	print tabs + "int _pil_iter;"
#	print tabs + "for (_pil_iter = 0; _pil_iter < _pil_num; _pil_iter++) {"
#	create_event(tabs + "\t", target, "data_array_item")
#	print tabs + "\tocrGuid_t data_array_item_db_guid = _pil_sda[_pil_iter].guid;"
#	create_dependence(tabs + "\t", target, True, "data_array_item", "_pil_iter+4")
#	satisfy_dependence(tabs + "\t", target, "data_array_item")
#	print tabs + "}"
#
#	satisfy_dependence(tabs, target, "param")
#	satisfy_dependence(tabs, target, "global")
#	print tabs + "index_array_db_guid = _pil_split_index_array.guid;"
#	satisfy_dependence(tabs, target, "index_array")
#	print tabs + "data_array_db_guid = _pil_split_data_array.guid;"
#	satisfy_dependence(tabs, target, "data_array")

	print ""
	print tabs + "iter++;"


def print_num_iterations(tabs, param, old):
	""""""

	o = ""
	if old:
		o = "old_"
	print tabs + "uint64_t " + o + "num_iterations = 0;"
	print tabs + "if (" + param + "->" + g.STEP + " < 0) {"
	print tabs + "\t" + o + "num_iterations = (" + param + "->" + g.UPPER + " - " + param + "->" + g.LOWER + ")/(0 - " + param + "->" + g.STEP + ") + 1;"
	print tabs + "} else {"
	print tabs + "\t" + o + "num_iterations = (" + param + "->" + g.UPPER + " - " + param + "->" + g.LOWER + ")/" + param + "->" + g.STEP + " + 1;"
	print tabs + "}"



def output_node_entry(m):
	"""This is the entry to a node. It schedules the parallel body of the
	work."""

	print "ocrGuid_t node_" + m['label'] + "_enter(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])"
	print "{"
	#print "\tocrGuid_t node_" + m['label'] + "_body_type, node_" + m['label'] + "_body_guid, exit_guid;"
	print "\tocrGuid_t exit_guid;"
	print "\tstruct node_" + m['label'] + "_param *input = (struct node_" + m['label'] + "_param *) depv[0].ptr;"
	print "\tstruct node_" + m['label'] + "_param *n" + m['label'] + "p;"
	if needs_global(m['label']):
		print "\tstruct global_param *global = (struct global_param *) depv[1].ptr;"
	print "\tocrGuid_t global_db_guid = depv[1].guid;"
	print "\tgpp_t index_array;"
	print "\tindex_array.guid = depv[2].guid;"
	print "\tindex_array.ptr = depv[2].ptr;"
	print "\tocrGuid_t index_array_db_guid = index_array.guid;"
	print "\tgpp_t data_array;"
	print "\tdata_array.guid = depv[3].guid;"
	print "\tdata_array.ptr = depv[3].ptr;"
	print "\tocrGuid_t data_array_db_guid = data_array.guid;"
	print "\t" + g.variables[m['index']] + " " + m['index'] + ";"
	print ""
	print "\tuint64_t iter = 0;"
	run.file_debug(2, "\n\tfprintf(stdout, \"\\nnode_" + m['label'] + "_enter edt\\n\");")
	run.file_debug(3, "\tfprintf(stdout, \"lower: %d  upper: %d  step: %d\\n\", input->" + g.LOWER + ", input->" + g.UPPER + ", input->" + g.STEP + ");")

	tabs = "\t"
	if g.graph[m['label']].has_merge():
		merge = g.graph[m['label']].get_matching_fork()
		if (merge == "0"):
			print "ERROR: node " + m['label'] + " has a merge point, but it doesn't have a matching fork"
			print "known fork sites are:"
			for n in g.graph:
				if g.graph[n].has_fork():
					print n
			print "exit(1);"
		#print tabs + 'printf("in node_' + m['label'] + '\\n");'
		print tabs + "_pil_node_" + merge + "_counter += 1;"
		print tabs + "if (_pil_node_" + merge + "_counter != _pil_node_" + merge + "_total) {"
		tabs = increment_tabs(tabs)
		run.file_debug(1, tabs + "fprintf(stdout, \"not ready to execute node yet\\ndestroying data-block (%ld) for param in node_" + m['label'] + "_enter\\n\", depv[0].guid);")
		print tabs + "ocrDbDestroy(depv[0].guid);"
		print tabs + "return NULL_GUID;"
		tabs = decrement_tabs(tabs)
		print tabs + "}"

	print ""
	print_num_iterations("\t", "input", False)
	run.file_debug(2, "\tfprintf(stdout, \"scheduling %ld body edts\\n\", num_iterations);")
	print ""
	# see if we need the command line parameters. TODO: This assumes the only varialbles named argc or argv are command line variables
	if "argc" in g.args[m['func_name']]['list'] or "argv" in g.args[m['func_name']]['list']:
		#print "\t//struct global_param *global = (struct global_param *) RMD_DB_ACQUIRE(input->global_db_guid, 0);"
		#print "\tstruct global_parm *global;"
		#print "\tocrDbAcquire(input->global_db_guid, (void **) &global, /*flags=*/FLAGS);"
		#print "\t//rmd_cmd_line_t *cmd_line_ptr = RMD_DB_ACQUIRE(global->cmd_line_db_guid, 0);"
		pass
	if "argv" in g.args[m['func_name']]['list']:
		#print "\tocrGuid_t argv_db_guid;"
		#print "\t//char **argv = (char **) RMD_DB_ALLOC(&argv_db_guid, cmd_line_ptr->argc*sizeof(char*), 0, &loc);" # TODO: this should probably be core local
		#print "\tchar **argv;"
		#print "\tocrDbCreate(&argv_db_guid, (void ***) &input->argv, input->argc*sizeof(char*), /*flags=*/FLAGS, /*location=*/NULL, NO_ALLOC);"
		#print "\tint i;"
		#print "\tfor (i = 0; i < cmd_line_ptr->argc; i++)"
		#print "\tfor (i = 0; i < input->argc; i++)"
		#print '\t\targv[i] = (char *) cmd_line_ptr + cmd_line_ptr->argv[i];'
		#print ""
		pass

	print tabs + "int _pil_num_da = 0;"
	print tabs + "if (index_array.guid != NULL_GUID) {"
	print tabs + "\t_pil_num_da = ((int*)index_array.ptr)[1];"
	print tabs + "}"


	#----------------------------------
	# single iteration optimization
	#----------------------------------
	print tabs + "if (num_iterations == 1 && _pil_num_da == 0) {"
	tabs += "\t"
	print tabs + "struct global_param *global = depv[1].ptr;"
	call_body_function(tabs, m)
	print tabs + "n" + m['label'] + "p = input;"
	targets = g.targets[m['label']]
	if len(targets) == 1 and targets[0] == "0":
		pass
	else:
		print_num_iterations(tabs, "n" + m['label'] + "p", True)
	output_switch(tabs, m)
	exit_node_cleanup(tabs, m, False)
	tabs = tabs[:-1]
	print tabs + "} // single iteration optimization"
	print tabs + "else {"
	tabs += "\t"

	print tabs + "// create the exit edt for this node"
	print tabs + "ocrGuid_t exit_template_guid;"
	print tabs + "int _pil_num_dep = 3*num_iterations+3;"
	print tabs + "ocrEdtTemplateCreate(&exit_template_guid, node_" + m['label'] + "_exit, 0, _pil_num_dep);"
	print tabs + "ocrEdtCreate(&exit_guid, exit_template_guid, /*paramc*/0, /*paramv=*/NULL, /*depc=*/_pil_num_dep, /*depv=*/NULL, /*properties=*/0, /*affinity*/NULL_GUID, /*outputEvent*/NULL);"
	print tabs + "ocrEdtTemplateDestroy(exit_template_guid);"
	run.file_debug(3, tabs + "fprintf(stdout, \"created edt (%ld) for node_" + m['label'] + " exit_guid\\n\", exit_guid);")
	print tabs + "" + "ocrGuid_t global_event_guid, index_array_event_guid, data_array_event_guid;"
	print tabs + "" + "ocrEventCreate(&global_event_guid, PIL_EVENT_T, true);"
	print tabs + "" + "ocrEventCreate(&index_array_event_guid, PIL_EVENT_T, true);"
	print tabs + "" + "ocrEventCreate(&data_array_event_guid, PIL_EVENT_T, true);"
	print tabs + "" + "ocrAddDependence(global_event_guid, exit_guid, /*slot=*/0, /*mode*/DB_DEFAULT_MODE);"
	print tabs + "" + "ocrAddDependence(index_array_event_guid, exit_guid, /*slot=*/1, /*mode*/DB_DEFAULT_MODE);"
	print tabs + "" + "ocrAddDependence(data_array_event_guid, exit_guid, /*slot=*/2, /*mode*/DB_DEFAULT_MODE);"
	print tabs + "" + "ocrEventSatisfy(global_event_guid, global_db_guid);"
	print tabs + "" + "ocrEventSatisfy(index_array_event_guid, index_array_db_guid);"
	print tabs + "" + "ocrEventSatisfy(data_array_event_guid, data_array_db_guid);"
	print ""
	print tabs + "ocrGuid_t body_guid[num_iterations];"
	print ""

	print tabs + "if (input->" + g.STEP + " < 0)"
	print tabs + "{"
	print tabs + "\tfor (" + m['index'] + " = input->" + g.UPPER + "; " + m['index'] + " >= input->" + g.LOWER + "; " + m['index'] + " += input->" + g.STEP + ")"
	print tabs + "\t{"
	print tabs + '\t\tprintf("step less than one not fully tested\\n");'
	print tabs + "\t} // for"
	print tabs + "}"
	print tabs + "else"
	print tabs + "{"
	print tabs + "\tfor (input->" + m['index'] + " = input->" + g.LOWER + "; input->" + m['index'] + " <= input->" + g.UPPER + "; input->" + m['index'] + " += input->" + g.STEP + ")"
	print tabs + "\t{"
	enter_body(tabs + "\t\t", m['label'], m['func_name'], m['index'])
	print tabs + "\t} // for"
	print tabs + "} // step < 0"
	print tabs + "if ((num_iterations == 1 && _pil_num_da == 0) /* optimization */ || (_pil_num_da == 0) /* nothing in data_array to acquire */) {"
	run.file_debug(1, tabs + "\tfprintf(stdout, \"destroying data-block (%ld) for param in node_" + m['label'] + "_enter\\n\", depv[0].guid);")
	print tabs + "\tocrDbDestroy(depv[0].guid);"
	print tabs + "}"
	print ""
	tabs = tabs[:-1]
	print tabs + "} // single iteration optimization"
	print ""

	print tabs + "return NULL_GUID;"
	print "}"
	print ""


def index_helper(tabs, body):
	""""""

	print tabs + "gpp_t *_pil_da = (gpp_t *) data_array.ptr;"
	print tabs + "int *_pil_ia = (int *) index_array.ptr;"
	print tabs + "int _pil_num = 0;"
	print tabs + "if (index_array.guid != NULL_GUID) {"
	if body:
		print tabs + "\tint _pil_beg = _pil_ia[0];"
		print tabs + "\tint _pil_end = _pil_ia[1];"
		print tabs + "\t_pil_num = _pil_end - _pil_beg;"
		print tabs + "\tint _pil_iter;"
		print tabs + "\tfor (_pil_iter = 0; _pil_iter < _pil_num; _pil_iter++) {"
		print tabs + "\t\t_pil_da[_pil_iter].ptr = depv[_pil_iter + 4].ptr;"
		print tabs + "\t}"
	else:
		print tabs + "\tint _pil_beg = _pil_ia[iter];"
		print tabs + "\tint _pil_end = _pil_ia[iter+1];"
		print tabs + "\t_pil_num = _pil_end - _pil_beg;"
	print tabs + "}"
	print ""


def call_body_function(tabs, m):
	"""output code to call the body function and set returned arguments"""

	arg_num = 0;
	for arg_name in g.args[m['func_name']]['list']:
		arg_num += 1

	first = True
	in_args = ""
	for arg_name in g.args[m['func_name']]['list']:
		if first:
			first = False
		else:
			in_args += ", "

		if arg_name == "index_array" or arg_name == "data_array":
			if g.args[m['func_name']]['dict'][arg_name]['passed_by_reference']:
				in_args += "&"
			in_args += arg_name
		else:
			# check to see if we should pass the argument by reference or by value
			in_args += "(" + g.variables[arg_name]
			if g.args[m['func_name']]['dict'][arg_name]['passed_by_reference']:
				in_args += " *)&"
			elif arg_name in g.arrays:
				size = g.arrays[arg_name][0]
				in_args += " (*)[" + size + "])"
			else:
				in_args += ")"
			in_args += "input->" + arg_name
	run.file_debug(3, tabs + "fprintf(stdout, \"calling body function\\n\");")
	print tabs + m['func_name'] + "(" + in_args + ");"
	run.file_debug(3, tabs + "fprintf(stdout, \"returned from body function\\n\");")
	print ""
	for arg_name in g.args[m['func_name']]['list']:
		if arg_name == "index_array" or arg_name == "data_array":
			continue
		if g.args[m['func_name']]['dict'][arg_name]['passed_by_reference']:
			print tabs + "global->" + arg_name + " = input->" + arg_name + ";"
	print ""


def output_node_body(m):
	"""This is the work that gets done by each iteration of the node."""
	print "ocrGuid_t node_" +m['label'] + "_body(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])"
	print "{"
	run.file_debug(2, '\tfprintf(stdout, "\\nnode_' + m['label'] + '_body() edt\\n"); fflush(stdout);\n')
	print "\tstruct node_" + m['label'] + "_param *input = (struct node_" + m['label'] + "_param *) depv[0].ptr;"
	run.file_debug(3, "\tfprintf(stdout, \"receiving data-block (%ld) in iteration %d\\n\", depv[0].guid, input->" + m['index'] + ");")
	print "\tstruct global_param *global = (struct global_param *) depv[1].ptr;"
	print "\tgpp_t index_array;"
	print "\tindex_array.guid = depv[2].guid;"
	print "\tindex_array.ptr = depv[2].ptr;"
	print "\tgpp_t data_array;"
	print "\tdata_array.guid = depv[3].guid;"
	print "\tdata_array.ptr = depv[3].ptr;"
	print ""

	index_helper("\t", True)

	call_body_function("\t", m)

	print "\tocrEventSatisfy(input->node_" + m['label'] + "_exit_param_event_guid, depv[0].guid);"
	run.file_debug(3, "\tfprintf(stdout, \"satisfied event (%ld) with db (%ld) for exit_param_event_guid\\n\", input->node_" + m['label'] + "_exit_param_event_guid, depv[0].guid);")
	print "\tocrEventSatisfy(input->node_" + m['label'] + "_exit_index_array_event_guid, index_array.guid);"
	run.file_debug(3, "\tfprintf(stdout, \"satisfied event (%ld) with db (%ld) for exit_index_array_event_guid\\n\", input->node_" + m['label'] + "_exit_index_array_event_guid, index_array.guid);")
	print "\tocrEventSatisfy(input->node_" + m['label'] + "_exit_data_array_event_guid, data_array.guid);"
	run.file_debug(3, "\tfprintf(stdout, \"satisfied event (%ld) with db (%ld) for exit_data_array_event_guid\\n\", input->node_" + m['label'] + "_exit_data_array_event_guid, data_array.guid);")
	print ""
	print "\treturn NULL_GUID;"
	print "}"
	print ""


def level_to_tabs(level):

	tabs = ""
	for i in range(level):
		tabs += "\t"
	return tabs


def increment_tabs(tabs):

	return tabs + "\t"


def decrement_tabs(tabs):

	return tabs[1:]


def create_edt(tabs, target, depv, finish, body):
	"""the line to create an EDT"""

	num_dependences = "4"

	print tabs + "ocrGuid_t node_" + target + "_template_guid;"

	if body:
		num_dependences += "+_pil_num"
		print tabs + "ocrEdtTemplateCreate(&node_" + target + "_template_guid, node_" + target + "_body, 0, " + num_dependences + ");"
		print tabs + "ocrEdtCreate(&body_guid[iter], node_" + target + "_template_guid, /*paramc=*/0, /*paramv=*/(u64 *)NULL, /*depc=*/" + num_dependences + ", /*depv=*/" + depv + ",",
	else:
		print tabs + "ocrEdtTemplateCreate(&node_" + target + "_template_guid, node_" + target + "_enter, 0, " + num_dependences + ");"
		print tabs + "ocrEdtCreate(&node_" + target + "_guid, node_" + target + "_template_guid, /*paramc=*/0, /*paramv=*/(u64 *)NULL, /*depc=*/" + num_dependences + ", /*depv=*/" + depv + ",",
	if finish:
		print "/*properties=*/EDT_PROP_FINISH",
	else:
		print "/*properties=*/0",
	print ", /*affinity*/NULL_GUID, ",
	if finish:
		print "/*outputEvent*/&finish_guid",
	else:
		print "/*outputEvent*/NULL",
	print ");"
	print tabs + "ocrEdtTemplateDestroy(node_" + target + "_template_guid);"

	if body:
		arg = "body_guid[iter]"
	else:
		arg = "node_" + target + "_guid"
	run.file_debug(3, tabs + "fprintf(stdout, \"created edt (%ld) for node_" + target + "_body\\n\", " + arg + ");")
	print ""

def create_event(tabs, target, name):
	""""""

	print tabs + "ocrGuid_t enter_" + name + "_event_guid;"
	print tabs + "ocrEventCreate(&enter_" + name + "_event_guid, PIL_EVENT_T, true);"
	run.file_debug(3, tabs + "fprintf(stdout, \"created enter_" + name + "_event (%ld) for node_ " + target + "\\n\", enter_" + name + "_event_guid);")


def create_dependence(tabs, target, body, name, slot):
	""""""

	if body:
		print tabs + "ocrAddDependence(enter_" + name + "_event_guid, body_guid[iter], /*slot=*/" + str(slot) + ", /*mode*/DB_DEFAULT_MODE);"
	else:
		print tabs + "ocrAddDependence(enter_" + name + "_event_guid, node_" + target + "_guid, /*slot=*/" + str(slot) + ", /*mode*/DB_DEFAULT_MODE);"
	run.file_debug(3, tabs + "fprintf(stdout, \"added dependence between enter edt (%ld) and param " + name + " event (%ld)for node_" + target + "_enter\\n\", node_" + target + "_guid, enter_" + name + "_event_guid);")


def satisfy_dependence(tabs, target, name):
	""""""

	#if name == "data_array" or name == "index_array":
	if name == "data_array_item":
		print tabs + "if (" + name + "_db_guid != NULL_GUID) {\n", "\t",
	print tabs + "//ocrDbRelease(" + name + "_db_guid);"
	if name == "data_array_item":
		print tabs + "}"
	print tabs + "ocrEventSatisfy(enter_" + name + "_event_guid, " + name + "_db_guid);"
	run.file_debug(3, tabs + "fprintf(stdout, \"satisfied event (%ld) for node_" + target + " enter\\n\", enter_" + name + "_event_guid);")


def enter_node(tabs, body, finish, target):
	"""Output code to enter a node"""

	print ""
	print tabs + "// begin enter_node"
	#print tabs + "START_PROFILE(pil_enter_node)"

	print "#if 0"
	# create the events
	create_event(tabs, target, "param")
	create_event(tabs, target, "global")
	create_event(tabs, target, "index_array")
	create_event(tabs, target, "data_array")
	print ""

	# create the EDT
	create_edt(tabs, target, "NULL", finish, body)

	# add the dependences
	create_dependence(tabs, target, body, "param", 0)
	create_dependence(tabs, target, body, "global", 1)
	create_dependence(tabs, target, body, "index_array", 2)
	create_dependence(tabs, target, body, "data_array", 3)
	print ""

	# satisfy and release the dependences
	satisfy_dependence(tabs, target, "param")
	satisfy_dependence(tabs, target, "global")
	satisfy_dependence(tabs, target, "index_array")
	satisfy_dependence(tabs, target, "data_array")

	if body:
		print tabs + "int _pil_iter;"
		print tabs + "for (_pil_iter = 0; _pil_iter < _pil_num; _pil_iter++) {"
		create_event(tabs + "\t", target, "data_array_item")
		print tabs + "\tocrGuid_t data_array_item_db_guid = _pil_sda[_pil_iter].guid;"
		create_dependence(tabs + "\t", target, True, "data_array_item", "_pil_iter+4")
		satisfy_dependence(tabs + "\t", target, "data_array_item")
		print tabs + "}"

	print "#else"

	num = "4"
	if body:
		num += "+_pil_num"
	print tabs + "guid_t _pil_depv[" + num + "];"
	print tabs + "_pil_depv[0] = param_db_guid;"
	print tabs + "_pil_depv[1] = global_db_guid;"
	if body:
		print tabs + "_pil_depv[2] = _pil_split_index_array.guid;"
		print tabs + "_pil_depv[3] = _pil_split_data_array.guid;"
	else:
		print tabs + "_pil_depv[2] = index_array_db_guid;"
		print tabs + "_pil_depv[3] = data_array_db_guid;"

	if body:
		print tabs + "int _pil_iter;"
		print tabs + "for (_pil_iter = 0; _pil_iter < _pil_num; _pil_iter++) {"
		print tabs + "\t_pil_depv[_pil_iter+4] = _pil_sda[_pil_iter].guid;"
		print tabs + "}"

	# create the EDT
	create_edt(tabs, target, "_pil_depv", finish, body)
	print "#endif"

	#print tabs + "EXIT_PROFILE"
	print tabs + "// end enter_node"
	print ""


def needs_global(target):
	""""""

	needs_global = False

	if target == "0":
		return False

	for n in g.nw_calls:
		if target == n['label']:
			return False

	lm = re.match("\d+", g.intervals[target]['lower'])
	if not lm:
		if g.intervals[target]['lower'] not in g.args[g.functions[target]]:
			needs_global = True
	sm = re.match("\d+", g.intervals[target]['step'])
	if not sm:
		if g.intervals[target]['step'] not in g.args[g.functions[target]]:
			needs_global = True
	um = re.match("\d+", g.intervals[target]['upper'])
	if not um:
		if g.intervals[target]['upper'] not in g.args[g.functions[target]]:
			needs_global = True

	return needs_global

def ocr_param_init(tabs, label, target, string):
	"""output a param_init call"""

	for n in g.nw_calls:
		if target == n['label']:
			return

	init = ""
	lm = re.match("\d+", g.intervals[target]['lower'])
	if lm:
		init += g.intervals[target]['lower'] + ", "
	else:
		if g.intervals[target]['lower'] in g.args[g.functions[target]]:
			#init += "input_" + g.functions[target]->"
			init += "input->"
		else:
			init  += "global->"
			needs_global = True
		init += g.intervals[target]['lower'] + ", "
	sm = re.match("\d+", g.intervals[target]['step'])
	if sm:
		init += g.intervals[target]['step'] + ", "
	else:
		if g.intervals[target]['step'] in g.args[g.functions[target]]:
			#init += "input_" + g.functions[target]->"
			init += "input->"
		else:
			init  += "global->"
			needs_global = True
		init += g.intervals[target]['step'] + ", "
	um = re.match("\d+", g.intervals[target]['upper'])
	if um:
		init += g.intervals[target]['upper']
	else:
		if g.intervals[target]['upper'] in g.args[g.functions[target]]:
			#init += "input_" + g.functions[target]->"
			init += "input->"
		else:
			init  += "global->"
			needs_global = True
		init += g.intervals[target]['upper']

	print tabs + "// [lower, step, upper] = [" + init + "]"
	#print tabs + "node_"+ target + "_param_init(n" + target + "p, input->global_db_guid, " + init + ");"
	print tabs + "node_"+ target + "_param_init(" + string + target + "p, n" + label + "p->global_db_guid, " + init + ");"


def initialize_data_array(tabs):
	""""""

	print tabs + "if (data_array.ptr != NULL) {"
	print tabs + "\tpil_free(data_array);"
	print tabs + "}"
	print tabs + "pil_alloc(&data_array, _pil_ia[num_iterations]*sizeof(gpp_t));"
	print tabs + "data_array_db_guid = data_array.guid;"

def initialize_index_array(tabs):
	""""""

	print tabs + "if (index_array.ptr != NULL) {"
	print tabs + "\tpil_free(index_array);"
	print tabs + "}"
	print tabs + "pil_alloc(&index_array, (num_iterations+1)*sizeof(int));"
	print tabs + "index_array_db_guid = index_array.guid;"
	print tabs + "_pil_ia = (int *) index_array.ptr;"
	print tabs + "for (_pil_iter = 0; _pil_iter <= num_iterations; _pil_iter++) {"
	print tabs + "\t_pil_ia[_pil_iter] = 0;"
	print tabs + "}"


def output_switch_case_0(tabs, m, target):
	"""output the code for the exit case"""

	print tabs + "// Exit target"

	print tabs + "exit_case = 1;"

	if not g.OCR_WAIT and g.PIL_MAIN:
		if g.MEM == "sharedspmd":
			print tabs + "int _pil_rank = n" + m['label'] + "p->" + g.nodes[m['label']]['rank'] + ";"
		else:
			print tabs + "int _pil_rank = 0;"
		print tabs + "pthread_mutex_lock(&(_pil_thread_lock[_pil_rank]._pil_enter_mutex));"
		print tabs + "_pil_thread_lock[_pil_rank]._pil_in_pil_enter = FALSE;"
		print tabs + "pthread_cond_signal(&(_pil_thread_lock[_pil_rank]._pil_enter_condition));"
		print tabs + "pthread_mutex_unlock(&(_pil_thread_lock[_pil_rank]._pil_enter_mutex));"

	if g.PIL_MAIN:
		destroy = True
		for node in g.nodes_entered:
			if m['label'] == node['label']:
				destroy = False
		if destroy:
			run.file_debug(1, tabs + "fprintf(stdout, \"destroying data-block (%ld) in node_" + m['label'] + "_exit for depv[0]\\n\", depv[0].guid);")
			print tabs + "ocrDbDestroy(depv[0].guid);"
	else:
		print tabs + "int _pil_index;"
		if g.MEM == "sharedspmd":
			start = "0"
		else:
			start = "0"
		print tabs + "for (_pil_index = " + start + "; _pil_index < 4; _pil_index++)"
		print tabs + "{"
		print tabs + "\tif (depv[_pil_index].guid) {"
		run.file_debug(1, tabs + "\t\tfprintf(stdout, \"destroying data-block (%ld) in node_" + m['label'] + "_exit for depv[%d]\\n\", depv[_pil_index].guid, _pil_index);")
		print tabs + "\t\tocrDbDestroy(depv[_pil_index].guid);"
		print tabs + "\t}"
		print tabs + "}"
		print ""

	if g.MEM == "sharedspmd" and not g.PIL_MAIN:
		print tabs + "pil_shutdown();"
	elif not g.PIL_MAIN:
		print tabs + "pil_cleanup();"


def allocate_param_in_case(tabs, target):
	"""allocate the param db in the switch case"""

	allocate_param_db(tabs + "", "", target, "t")

def output_switch_case(tabs, m, target):
	"""output code for a nonzero switch case"""

	#print tabs + 'printf("firing node ' + target + '\\n");'
	print tabs + "ocrGuid_t node_" + target + "_guid, param_db_guid;"
	print tabs + "struct node_" + target + "_param *t" + target + "p;"
	print tabs + "ocrGuid_t index_array_db_guid = index_array.guid;"
	print tabs + "ocrGuid_t data_array_db_guid = data_array.guid;"

	if g.PIL_MAIN:
		in_entered = False
		for node in g.nodes_entered:
			if target == node['label']:
				in_entered = True
		if in_entered:
			if g.MEM != "sharedspmd":
				print tabs + "int _pil_rank = 0;"
			else:
				print tabs + "int _pil_rank = n" + m['label'] + "p->" + g.nodes[m['label']]['rank'] + ";"
			print tabs + "param_db_guid = pil_enter_db_table[_pil_rank].node_" + target + "_param_gpp.guid;"
			print tabs + "t" + target + "p = pil_enter_db_table[_pil_rank].node_" + target + "_param_gpp.ptr;"
		else:
			allocate_param_in_case(tabs, target)
	else:
		allocate_param_in_case(tabs, target)

	ocr_param_init(tabs, m['label'], target, "t")

	print ""
	network_call = False
	for n in g.nw_calls:
		if target == n['label']:
			network_call = True
	if not network_call:
		for arg_name in g.args[g.functions[target]]['list']:
			if arg_name == "index_array" or arg_name == "data_array":
				continue
			elif arg_name in g.arrays:
				copy_array(tabs, target, arg_name, "t" + target + "p", "n" + m['label'] + "p")
			elif arg_name == g.nodes[target]['rank'] and g.graph[m['label']].has_fork():
				print tabs + "t" + target + "p->" + arg_name + " = n" + m['label'] + "p->_pil_task_names[_pil_thread_index];"
			elif arg_name in g.args[g.functions[m['label']]]['list']:
				print tabs + "t" + target + "p->" + arg_name + " = n" + m['label'] + "p->" + arg_name + ";"
			else:
				print tabs + "t" + target + "p->" + arg_name + " = global->" + arg_name + ";"
		if g.MEM == "sharedspmd":
			arg_name = g.nodes[target]['rank']
			if arg_name not in g.args[g.functions[target]]['list']:
				if arg_name in g.args[g.functions[m['label']]]['list']:
					print tabs + "t" + target + "p->" + arg_name + " = n" + m['label'] + "p->" + arg_name + ";"
				else:
					print tabs + "t" + target + "p->" + arg_name + " = global->" + arg_name + ";"
		print ""

	print_num_iterations(tabs, "t" + target + "p", False)
	print ""

	print tabs + "if (index_array.guid != NULL_GUID) {"
	tabs += "\t"
	print tabs + "int *_pil_ia = NULL;"
	print tabs + "int _pil_iter;"
	print tabs + "if (num_iterations != old_num_iterations) {"
	initialize_index_array(tabs + "\t")
	print tabs + "}"
	print tabs + "else {"
	print tabs + "\t_pil_ia = (int *) index_array.ptr;"
	print tabs + "\t_pil_ia[0] = 0;"
	print tabs + "\tfor (_pil_iter = 0; _pil_iter < num_iterations; _pil_iter++) {"
	print tabs + "\t\tint *_pil_split_index_array = (int *) depv[(3*_pil_iter)+4].ptr;"
	print tabs + "\t\t_pil_ia[_pil_iter+1] = _pil_ia[_pil_iter] + _pil_split_index_array[1];"
	print tabs + "\t}"
	print tabs + "}"
	print ""
	print tabs + "if (_pil_ia[num_iterations] > 0) {"
	tabs += "\t"
	initialize_data_array(tabs)
	print tabs + "gpp_t *_pil_da = (gpp_t *) data_array.ptr;"
	print tabs + "for (_pil_iter = 0; _pil_iter < num_iterations; _pil_iter++) {"
	print tabs + "\tint _pil_beg = _pil_ia[_pil_iter];"
	print tabs + "\tint _pil_end = _pil_ia[_pil_iter+1];"
	print tabs + "\tint _pil_num = _pil_end - _pil_beg;"
	print tabs + "\tgpp_t _pil_split_data_array;"
	print tabs + "\t_pil_split_data_array.guid = depv[(3*_pil_iter)+5].guid;"
	print tabs + "\t_pil_split_data_array.ptr = depv[(3*_pil_iter)+5].ptr;"
	print tabs + "\tsize_t _pil_data_size = _pil_num * sizeof(gpp_t);"
	print tabs + "\tmemcpy(&_pil_da[_pil_ia[_pil_iter]], _pil_split_data_array.ptr, _pil_data_size);"
	print tabs + "}"
	tabs = tabs[:-1]
	print tabs + "}"
	tabs = tabs[:-1]
	print tabs + "}"
	print ""

	enter_node(tabs, False, False, target)


def output_switch(tabs, m):
	"""output code to decide which EDT to fire next"""

	print tabs + "uint8_t exit_case = 0;"
	if g.graph[m['label']].has_fork():
		fork = g.graph[m['label']].get_matching_merge()
		if (fork == "0"):
			print "ERROR: node " + m['label'] + " has a fork point, but it doesn't have a matching merge"
			print "known merge sites are:"
			for n in g.graph:
				if g.graph[n].has_merge():
					print n
			print "exit(1);"
		print tabs + "_pil_node_" + m['label'] + "_counter = 0;"
		print tabs + "_pil_node_" + m['label'] + "_total = global->_pil_num_targets;"

	string = "switch (n" + m['label'] + "p->" + m['cond']
	if m['cond'] in g.arrays:
		print tabs + "int _pil_thread_index;"
		print tabs + "for (_pil_thread_index = 0; _pil_thread_index < n" + m['label'] + "p->" + "_pil_num_targets" + "; _pil_thread_index++) {"
		string += "[_pil_thread_index]"
		tabs = increment_tabs(tabs)
	string += ")"
	print tabs + string
	print tabs + "{"
	tabs = increment_tabs(tabs)

	targets = g.targets[m['label']]
	for target in targets:
		print tabs + "case " + target + ":"
		print tabs + "{"
		tabs = increment_tabs(tabs)

		nwc = None
		for n in g.nw_calls:
			if target == n['label']:
				nwc = n
				break
		if nwc:
			target = pil2c.pil_nw_call(tabs, nwc, "n" + m['label'] + "p", target)

		# TODO: This assumes that a target of 0 is exit
		if target == "0":
			output_switch_case_0(tabs, m, target)
		else:
			output_switch_case(tabs, m, target)

		print tabs + "break;"
		tabs = decrement_tabs(tabs)
		print tabs + "}"
	print tabs + "default:"
	print tabs + "{"
	tabs = increment_tabs(tabs)
	if m['cond'] in g.arrays:
		print tabs + "fprintf(stdout, \"ERROR: Unknown case '%d' in node_" + m['label'] + "_exit\\n\", n" + m['label'] + "p->" + m['cond'] + "[_pil_thread_index]);"
	else:
		print tabs + "fprintf(stdout, \"ERROR: Unknown case '%d' in node_" + m['label'] + "_exit\\n\", n" + m['label'] + "p->" + m['cond'] + ");"
	if g.FILE_TYPE in g.CHPL:
		print tabs + "chpl_exit_all(EXIT_FAILURE);"
	else:
		print tabs + "exit(EXIT_FAILURE);"
	print tabs + "break;"
	tabs = decrement_tabs(tabs)
	print tabs + "}"
	tabs = decrement_tabs(tabs)
	print tabs + "}"
	if m['cond'] in g.arrays:
		tabs = decrement_tabs(tabs)
		print tabs + "}"
	print ""


def output_node_exit(m):
	"""This is the confluence edt of a node. All of the nodes of work meet
	here when they are done."""

	tabs = ""

	print tabs + "ocrGuid_t node_" + m['label'] + "_exit(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])"
	print tabs + "{"
	tabs = increment_tabs(tabs)
	print tabs + "struct node_" + m['label'] + "_param *n" + m['label'] + "p = (struct node_" + m['label'] + "_param *) depv[3].ptr;"
	targets = g.targets[m['label']]
	ng = False
	for target in targets:
		if needs_global(target):
			ng = True
	global_is_used = False
	for target in g.targets[m['label']]:
		if target == "0":
			continue
		else:
			network_call = False
			for n in g.nw_calls:
				if target == n['label']:
					network_call = True
			if not network_call:
				for arg_name in g.args[g.functions[target]]['list']:
					if arg_name not in g.args[g.functions[m['label']]]['list']:
						global_is_used = True
	if ng or global_is_used:
		print tabs + "struct global_param *global = (struct global_param *) depv[0].ptr;"
	if len(targets) == 1 and targets[0] == "0":
		pass
	else:
		print tabs + "ocrGuid_t global_db_guid = depv[0].guid;"
		print tabs + "gpp_t index_array;"
		print tabs + "index_array.guid = depv[1].guid;"
		print tabs + "index_array.ptr = depv[1].ptr;"
		print tabs + "gpp_t data_array;"
		print tabs + "data_array.guid = depv[2].guid;"
		print tabs + "data_array.ptr = depv[2].ptr;"
		print ""
		print_num_iterations(tabs, "n" + m['label'] + "p", True)
	run.file_debug(2, '\n' + tabs + 'fprintf(stdout, "\\nnode_' + m['label'] + '_exit() edt\\n");\n')
	print ""

	output_switch(tabs, m)
	exit_node_cleanup(tabs, m, True)

	print tabs + "return NULL_GUID;"
	tabs = decrement_tabs(tabs)
	print "}"
	print ""


def exit_node_cleanup(tabs, m, inexit):
	"""cleanup any data in a node exit"""

	d = ""
	if inexit:
		d = "3"
	else:
		d = "0"
	destroy = True
	if g.PIL_MAIN:
#		destroy = False
#	else:
		for node in g.nodes_entered:
			if m['label'] == node['label']:
				destroy = False
	if destroy:
		print tabs + "if (!exit_case) {"
		tabs = increment_tabs(tabs)
		run.file_debug(1, tabs + "fprintf(stdout, \"destroying data-block (%ld) for param in node_" + m['label'] + "_exit\\n\", depv[" + d + "].guid);")
		print tabs + "ocrDbDestroy(depv[" + d + "].guid);"
		tabs = decrement_tabs(tabs)
		print tabs + "}"
		print ""

	if inexit:
		print tabs + "int _pil_index;"
		print tabs + "for (_pil_index = 4; _pil_index < depc; _pil_index++)"
		print tabs + "{"
		print tabs + "\tif (depv[_pil_index].guid && depv[_pil_index].guid != depv[1].guid && depv[_pil_index].guid != depv[2].guid) {"
		run.file_debug(1, tabs + "\t\tfprintf(stdout, \"destroying data-block (%ld) in node_" + m['label'] + "_exit for depv[%d]\\n\", depv[_pil_index].guid, _pil_index);")
		print tabs + "\t\tocrDbDestroy(depv[_pil_index].guid);"
		print tabs + "\t}"
		print tabs + "}"
		print ""


# TODO: change this function so it doesn't take 'm', but the pieces of 'm' that it needs
# TODO: change this so it doesn't generate a single edt, but 2 or 3. An entry, a body and a confluence edt
def ocr_process_node_procedure(m):
	"""Output the body of a node function"""

	output_node_entry(m)
	output_node_body(m)
	output_node_exit(m)


def process_header():
	"""Outputs the header for the file. E.g., includes libraries."""

	run.process_header()

	print "#include <string.h>"
	print ""
	print "// Includes for the OCR library"
	print '#include "ocr.h"'

	if g.OCR_WAIT:
		print '#include "extensions/ocr-legacy.h" // needed for ocrWait()'
	else:
		print "#include <pthread.h> // needed to replace ocrWait()"

	print '//#include "ocr-runtime.h"'
	print '#include <errno.h>'
	print ""
	#print "#define FLAGS 0xdead"
	print "#define FLAGS 0"
	#print "#define PIL_EVENT_T OCR_EVENT_STICKY_T"
	print "#define PIL_EVENT_T OCR_EVENT_ONCE_T"
	print ""
	check_allocated_db()


def process_variables():
	"""Create a global store for all variables declared for use on shared
	   memory only."""

	print "//--------------------------------------"
	print "// global parameter"
	print "//--------------------------------------"
	print ""
	print "struct global_param"
	print "{"
	print "\tocrGuid_t cmd_line_db_guid;"
	for var in g.variables:
		if var in g.arrays:
			continue
		print "\t" + g.variables[var] + " " + var + ";"
	print "};"
	print ""
	print "struct ocr_cmd_line_t"
	print "{"
	print "\tint argc;"
	print "\tchar **argv;"
	print "};"
	print ""

	if g.PIL_MAIN:
		print "struct pil_enter_db_table_t {"
		print "\tgpp_t global;"
		for node in g.nodes_entered:
			print "\tgpp_t node_" + node['label'] + "_param_gpp;"
		print "};"
		print "struct pil_enter_db_table_t *pil_enter_db_table;"
		print ""

	if not g.OCR_WAIT and g.PIL_MAIN:
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


def ocr_process_global_param_init():
	"""generate the parameter init function for the global parameter"""

	args = ""
	for var in g.variables:
		if var in g.initial_values:
			args += ", " + g.variables[var] + " " + var
	print "static void global_param_init(struct global_param *inst" + args + ")"
	print "{"
	for var in g.variables:
		if var in g.initial_values:
			print "\tinst->" + var + " = " + var + ";"
	print "}"
	print ""


def ocr_process_node_param_init(func_name, label):
	"""generate the parameter init function for the function in the node"""

	# find the input arguments
	in_args = ""
	for arg_name in g.args[func_name]['list']:
		in_args += ", " + g.variables[arg_name] + " " + arg_name

	print "static void node_" + label + "_param_init(struct node_" + label + "_param *inst, ocrGuid_t g, int64_t l, int64_t s, int64_t u)" #+ in_args + ")"
	print "{"
	print "\tinst->global_db_guid = g;"
	print "\tinst->" + g.LOWER + " = l;"
	print "\tinst->" + g.STEP + " = s;"
	print "\tinst->" + g.UPPER + " = u;"

	#print ""
	#print "\t// initialize each input parameter"
	#for arg_name in g.args[func_name]['list']:
	#	run.file_debug(3, '\tfprintf(stdout, "initializing param ' + arg_name + '\\n"); fflush(stdout);')
	#	print "\tinst->" + arg_name + " = " + arg_name + ";"

	print "}"
	print ""


def ocr_process_param_init(label, lower, step, upper, func_name, in_args):
	"""process param init"""

	print "//--------------------------------------"
	print "// init functions for node_" + label
	print "//--------------------------------------"
	print ""

	ocr_process_node_param_init(func_name, label)
	#ocr_process_func_param_init(func_name, label)


def handle_nodes(nodes):
	"""Output the C code for each node in nodes."""

	#--------------------------------------------------------------------------
	# pil_cleanup()
	#--------------------------------------------------------------------------
	print "void pil_cleanup() {"
	tabs = "\t"
	print tabs + "pil_nw_cleanup();"
#	print tabs + "for (int dest = 0; dest < pil_get_nwCount(); dest++) {"
#	tabs = increment_tabs(tabs)
#	print tabs + "free(_pil_send_buf[_pil_rank][dest].ptr);"
#	tabs = decrement_tabs(tabs)
#	print tabs + "}"
#	print tabs + "free(_pil_send_buf[_pil_rank]);"
	if g.PIL_MAIN:
		print tabs + "for (int _pil_rank = 0; _pil_rank < pil_get_nwCount(); _pil_rank++) {"
		tabs = increment_tabs(tabs)
		run.file_debug(1, tabs + "fprintf(stdout, \"destroying data-block (%lld) in pil_cleanup for global_db_guid\\n\", (long long)pil_enter_db_table[_pil_rank].global.guid);")
		print tabs + "if (pil_enter_db_table[_pil_rank].global.guid) ocrDbDestroy(pil_enter_db_table[_pil_rank].global.guid);"
		for node in g.nodes_entered:
			run.file_debug(1, tabs + "fprintf(stdout, \"destroying data-block (%lld) in pil_cleanup for node_" + node['label'] + "_param\\n\", (long long)pil_enter_db_table[_pil_rank].node_" + node['label'] + "_param_gpp.guid);")
			print tabs + "if (pil_enter_db_table[_pil_rank].node_" + node['label'] + "_param_gpp.guid) ocrDbDestroy(pil_enter_db_table[_pil_rank].node_" + node['label'] + "_param_gpp.guid);"
		tabs = decrement_tabs(tabs)
		print tabs + "}"
#	print tabs + "free(_pil_send_buf);"
	if not g.OCR_WAIT and g.PIL_MAIN:
		print tabs + "free(_pil_thread_lock);"
	if g.PIL_MAIN:
		print tabs + "free(pil_enter_db_table);"
	run.file_debug(2, tabs + "fprintf(stdout, \"calling ocrShutdown()\\n\");")
	print tabs + "ocrShutdown();"
	print "}"

	#--------------------------------------------------------------------------
	# pil_shutdown()
	#--------------------------------------------------------------------------
	if g.MEM == "sharedspmd":
		print "int _pil_num_threads_exited = 0;"
		print "void pil_shutdown() {"
		print tabs + "if (pil_get_nwCount() == __sync_add_and_fetch(&_pil_num_threads_exited, 1)) {"
		print tabs + "\tpil_cleanup();"
		print tabs + "}"
		print "}"
		print ""

	print "// Map function prototypes"
	for m in nodes:
		ocr_process_function_prototype(m['label'], m['func_name'])
	print ""

#	print "int _pil_barrier_counter = 0;"
#	print "sem_t _pil_turnstile;"
#	print "sem_t _pil_turnstile2;"
#	print "sem_t _pil_mutex;"
#	print ""

	for m in nodes:
		ocr_process_type(m['label'], m['index'], m['func_name'], m['lower'], m['step'], m['upper'], m['in_args'])

	#ocr_process_global_param_init()
	for m in nodes:
		ocr_process_param_init(m['label'], m['lower'], m['step'], m['upper'], m['func_name'], m['in_args'])

	for m in nodes:
		ocr_process_node_procedure(m)


def declare_pil_enter_arguments():

	tabs = "\t"
	print tabs + "struct global_param *global = pil_enter_db_table[_pil_rank].global.ptr;"
	print tabs + "ocrGuid_t global_db_guid = pil_enter_db_table[_pil_rank].global.guid;"
	print tabs + "gpp_t index_array;"
	print tabs + "gpp_t data_array;"

	for node in g.nodes_entered:
		print tabs + "struct node_" + node['label'] + "_param *n" + node['label'] + "p = pil_enter_db_table[_pil_rank].node_" + node['label'] + "_param_gpp.ptr;"
		print tabs + "ocrGuid_t node_" + node['label'] + "_param_db_guid = pil_enter_db_table[_pil_rank].node_" + node['label'] + "_param_gpp.guid;"
	print ""
	#allocate_global_db(tabs)
	print ""


def process_pil_enter_arguments(node):
	"""Output the appropriate code to enter a node"""

	tabs = "\t\t\t"

	args = g.args[g.functions[node['label']]]
	for arg_name in args['list']:
		if arg_name == g.target_variables[node['label']]:
			continue
		elif arg_name == "index_array" or arg_name == "data_array":
			if g.args[g.functions[node['label']]]['dict'][arg_name]['passed_by_reference']:
				print tabs + arg_name + "_ptr = va_arg(argptr, gpp_t *);"
				print tabs + arg_name + " = *" + arg_name + "_ptr;"
			else:
				print tabs + arg_name + " = va_arg(argptr, gpp_t);"
		elif g.args[g.functions[node['label']]]['dict'][arg_name]['passed_by_reference']:
			print tabs + arg_name + "_ptr = va_arg(argptr, " + g.variables[arg_name] + " *);"
			print tabs + "n" + node['label'] + "p->" + arg_name + " = *" + arg_name + "_ptr;"
			print tabs + "global->" + arg_name + " = *" + arg_name + "_ptr;"
		else:
			print tabs + "n" + node['label'] + "p->" + arg_name + " = va_arg(argptr, " + g.variables[arg_name] + ");"
			print tabs + "global->" + arg_name + " = n" + node['label'] + "p->" + arg_name + ";"
	print tabs + "break;"


def process_pil_enter(nodes, node):
	"""Output the appropriate code to enter a node"""

	label = node['label']
	tabs = "\t\t\t"

	#print tabs + "struct node_" + label + "_param *output;"
	#print tabs + "node_" + label + "_param_init(input_" + label + ", global_db_guid, " + g.intervals[label]['lower'] + ", " + g.intervals[label]['step'] + ", " + g.intervals[label]['upper'] + ");"
	run.file_debug(2, tabs + "fprintf(stdout, \"pil_enter: entering node %d\\n\", " + label + ");")
	print tabs + "ocrGuid_t node_" + label + "_guid;"
	print tabs + "ocrGuid_t finish_guid;"
	print tabs + "ocrGuid_t index_array_db_guid = index_array.guid;"
	print tabs + "ocrGuid_t data_array_db_guid = data_array.guid;"
	print tabs + "ocrGuid_t param_db_guid = node_" + label + "_param_db_guid;"
	ocr_param_init(tabs, label, label, "n")

#	print tabs + "if (index_array.guid == NULL_GUID) {"
#	print_num_iterations(tabs + "\t", "n" + node['label'] + "p", False)
#	print tabs + "\tint *_pil_ia;"
#	print tabs + "\tint _pil_iter;"
#	initialize_index_array(tabs + "\t")
#	#print tabs + "\tindex_array_db_guid = index_array.guid;"
#	#initialize_data_array(tabs + "\t")
#	#print  tabs + "\tdata_array_db_guid = data_array.guid;"
#	#print tabs + "\tdata_array_db_guid = NULL_GUID;"
#	print tabs + "}"

	if not g.OCR_WAIT:
		print tabs + "_pil_thread_lock[_pil_rank]._pil_in_pil_enter = TRUE;"

	enter_node(tabs, False, True, label)


	if g.OCR_WAIT:
		create_event(tabs, label, "wait")
		print tabs + "ocrAddDependence(finish_guid, enter_wait_event_guid, 0, DB_DEFAULT_MODE);"
		#print tabs + "ocrGuid_t returned_guid = ocrWait(finish_guid);"
		print tabs + "ocrWait(enter_wait_event_guid);"
		print tabs + "ocrEventDestroy(enter_wait_event_guid);"
	else:
		print tabs + "pthread_mutex_lock(&(_pil_thread_lock[_pil_rank]._pil_enter_mutex));"
		print tabs + "while(_pil_thread_lock[_pil_rank]._pil_in_pil_enter)"
		print tabs + "\t_pil_thread_lock[_pil_rank]._pil_cond_wait_ret = pthread_cond_wait(&(_pil_thread_lock[_pil_rank]._pil_enter_condition), &(_pil_thread_lock[_pil_rank]._pil_enter_mutex));"
		print tabs + "pthread_mutex_unlock(&(_pil_thread_lock[_pil_rank]._pil_enter_mutex));"
	#print tabs + "//ocrCleanup();"
	#print tabs + "// reinit the runtime"
	#ocr_init(3, nodes)

	args = g.args[g.functions[label]]
	for arg_name in args['list']:
		if arg_name == g.target_variables[label]:
			continue
		if g.args[g.functions[node['label']]]['dict'][arg_name]['passed_by_reference']:
			print tabs + "*" + arg_name + "_ptr = global->" + arg_name + ";"
	print tabs + "break;"

def ocr_init(level, nodes):
	"""Initialize the OCR runtime"""

	return

	tabs = level_to_tabs(level)

	print tabs + "ocrEdt_t fctPtrArray[" + str(3*len(nodes)) + "];"
	for i in range(len(nodes)):
		print tabs + "fctPtrArray[" + str((3*i)+0) + "] = &node_" + nodes[i]['label'] + "_enter;"
		print tabs + "fctPtrArray[" + str((3*i)+1) + "] = &node_" + nodes[i]['label'] + "_body;"
		print tabs + "fctPtrArray[" + str((3*i)+2) + "] = &node_" + nodes[i]['label'] + "_exit;"
	print tabs + "ocrInit(&argc, argv, 1, fctPtrArray);"


def check_allocated_db():
	""""""

	tabs = "\t"

	print "void check_allocated_db(u8 err) {"
	print tabs + '//printf("pil allocated a DB\\n");'
	print tabs + "if (err) {"
	print tabs + "\tif (err == ENOMEM) {"
	print tabs + "\t\tassert(0 == \"ENOMEM\");"
	print tabs + "\t}"
	print tabs + "\telse if (err == ENXIO) {"
	print tabs + "\t\tassert(0 == \"ENXIO\");"
	print tabs + "\t}"
	print tabs + "\telse if (err == EINVAL) {"
	print tabs + "\t\tassert(0 == \"EINVAL\");"
	print tabs + "\t}"
	print tabs + "\telse if (err == EBUSY) {"
	print tabs + "\t\tassert(0 == \"EBUSY\");"
	print tabs + "\t}"
	print tabs + "\telse if (err == EPERM) {"
	print tabs + "\t\tassert(0 == \"EPERM\");"
	print tabs + "\t}"
	print tabs + "}"
	print "}"


def allocate_db(tabs, name, ptr, size):
	""""""

	print tabs + "check_allocated_db(ocrDbCreate(&" + name + ", (void **) &" + ptr + ", " + size + ", /*flags=*/FLAGS, /*affinity=*/NULL_GUID, NO_ALLOC));"
	run.file_debug(1, tabs + "fprintf(stdout, \"allocated data-block (%ld) for " + name + "\\n\", " + name + ");")


def allocate_array_db(tabs, name):
	"""allocate a datablock"""

	size = ""
	if name == "data_array":
		size = "sizeof(gpp_t)"
	elif name == "index_array":
		size = "2*sizeof(int)"
	else:
		error("unrecognized name")

	print tabs + "check_allocated_db(ocrDbCreate(&" + name + "_db_guid, (void **) &ptr, " + size + ", /*flags=*/FLAGS, /*affinity=*/NULL_GUID, NO_ALLOC));"
	run.file_debug(1, tabs + "fprintf(stdout, \"allocated data-block (%ld) for " + name + "_db_guid\\n\", " + name + "_db_guid);")


def allocate_param_db(tabs, name, label, string):
	"""allocate a datablock"""

	print tabs + "check_allocated_db(ocrDbCreate(&" + name + "param_db_guid, (void **) &" + string + label + "p, sizeof(struct node_" + label + "_param), /*flags=*/FLAGS, /*affinity=*/NULL_GUID, NO_ALLOC));"
	run.file_debug(1, tabs + "fprintf(stdout, \"allocated data-block (%ld) for node " + label + " param_db_guid\\n\", param_db_guid);")


def allocate_global_db(tabs):
	"""allocate the global datablock"""

	print tabs + "check_allocated_db(ocrDbCreate(&global_db_guid, (void **) &global, sizeof(struct global_param), /*flags=*/FLAGS, /*affinity=*/NULL_GUID, NO_ALLOC));"
	run.file_debug(1, tabs + "fprintf(stdout, \"allocated data-block (%ld) for global_db_guid\\n\", global_db_guid);")
	#print "\t//global_param_init();"


def process_main(nodes):
	"""Output the main function for the file."""

	label = g.main_node_label
	init = g.main_node_input[0] + ", " + g.main_node_input[1] + ", " + g.main_node_input[2]

	if g.FILE_TYPE in g.CHPL:
		print "// dummy chpl_main function to appease the Chapel and start the OCR runtime"
		print "void chpl_main()"
		print "{"
		print "\treturn;"
		print "}"
		print ""
		print ""
#	elif g.OUTPUT != "ocr":
#		print "int main(int argc, char **argv)"
#		print "{"
#		print "\tpil_init(argc, argv);"
#		print "\treturn EXIT_SUCCESS;"
#		print "}"
#		print ""
#		print ""

	tabs = "\t"

	print "ocrGuid_t pil_main_edt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])"
	print "{"

	if g.PIL_MAIN or "argc" in g.args[g.main_node_func_name]['list'] or "argv" in g.args[g.main_node_func_name]['list']:
		print tabs + "int argc = getArgc(depv[0].ptr);"
		print tabs + "char **argv = (char **) malloc(argc * sizeof(char *));"
		print tabs + "for (int _pil_index = 0; _pil_index < argc; _pil_index++)"
		print tabs + "{"
		print tabs + "\tchar *arg = getArgv(depv[0].ptr, _pil_index);"
		print tabs + "\targv[_pil_index] = malloc(strlen(arg)+1);"
		print tabs + "\tstrcpy(argv[_pil_index], arg);"
		print tabs + "\t//printf('argv[%d]: %s\\n', _pil_index, argv[_pil_index]);"
		print tabs + "}"

	if g.PIL_MAIN or g.MEM == "sharedspmd":
		print tabs + "int *_pil_rank_ptr = depv[2].ptr;"
		print tabs + "int _pil_rank = *_pil_rank_ptr;"

	if g.PIL_MAIN:
		print tabs + "pil_enter_db_table[_pil_rank].global.guid = depv[1].guid;"
		print tabs + "pil_enter_db_table[_pil_rank].global.ptr = depv[1].ptr;"
		for node in g.nodes_entered:
			name = "pil_enter_db_table[_pil_rank].node_" + node['label'] + "_param_gpp"
			print tabs + "check_allocated_db(ocrDbCreate(&" + name + ".guid, (void **) &" + name + ".ptr, sizeof(struct node_" + node['label'] + "_param), /*flags=*/FLAGS, /*affinity=*/NULL_GUID, NO_ALLOC));"
			run.file_debug(1, tabs + "fprintf(stdout, \"allocated data-block (%ld) for " + name + "\\n\", " + name + ".guid);")

	if g.PIL_MAIN:
		if g.MEM == "sharedspmd":
			print tabs + "pil_main(argc, argv, _pil_rank);"
		else:
			print tabs + "pil_main(argc, argv, _pil_rank);"
	else:
		print tabs + "ocrGuid_t node_1_guid, param_db_guid;"
		print tabs + "ocrGuid_t global_db_guid = depv[1].guid;"
		print tabs + "ocrGuid_t index_array_db_guid = NULL_GUID;"
		print tabs + "ocrGuid_t data_array_db_guid = NULL_GUID;"
		print tabs + "struct node_" + label + "_param *n" + label + "p;"
		print ""
		allocate_param_db(tabs, "", label, "n")
		print ""
		print tabs + "node_" + label + "_param_init(n" + label + "p, global_db_guid, " + init + ");"#in_args + ");"
		# see if we need the command line parameters. TODO: This assumes the only varialbles named argc or argv are command line variables
		if "argc" in g.args[g.main_node_func_name]['list'] or "argv" in g.args[g.main_node_func_name]['list']:
			print tabs + "n" + label + "p->argc = argc;"
		if "argv" in g.args[g.main_node_func_name]['list']:
			print tabs + "n" + label + "p->argv = argv;"
		if g.MEM == "sharedspmd":
			print tabs + "n" + label + "p->" + g.nodes[label]['rank'] + " = _pil_rank;"
		print ""
		enter_node(tabs, False, False, label)

	print tabs + "// destroy rank db guid"
	run.file_debug(1, tabs + "fprintf(stdout, \"destroying data-block (%lld) in _pil_main_edt for rank_db_guid\\n\", (long long)depv[2].guid);")
	print tabs + "ocrDbDestroy(depv[2].guid); // rank_db_guid"
	print ""

	if g.PIL_MAIN:
		print tabs + "for (int _pil_index = 0;_pil_index < argc; _pil_index++) {"
		print tabs + "\tfree(argv[_pil_index]);"
		print tabs + "}"
		print tabs + "free(argv);"
	if g.PIL_MAIN and g.MEM == "sharedspmd":
		print tabs + "pil_shutdown();"
	elif g.PIL_MAIN:
		print tabs + "pil_cleanup();"
	print tabs + "return NULL_GUID;"
	print "}"
	print ""

	print "//u8 main_edt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])"
	print "ocrGuid_t mainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])"
	print "//int ocr_main(int argc, char **argv)"
	print "{"
	run.file_debug(1, tabs + 'fprintf(stdout, "main_edt()\\n");')
	run.file_debug(1, tabs + 'fprintf(stdout, "Using OCR backend of PIL\\n");')

	#print tabs + 'const char *_PIL_NUM_THREADS = "PIL_NUM_THREADS";'
	#print tabs + "char *_pil_NP_str = getenv(_PIL_NUM_THREADS);"
	#print tabs + "int _pil_NP = 1;"
	#print tabs + "if (_pil_NP_str != NULL) {"
	#print tabs + "\t_pil_NP = atoi(_pil_NP_str);"
	#print tabs + "}"
#	print tabs + "int _pil_NP = pil_get_nwCount();"
#	print tabs + "sem_init(&_pil_mutex, 0, 1);"
#	print tabs + "sem_init(&_pil_turnstile, 0, 0);"
#	print tabs + "sem_init(&_pil_turnstile2, 0, 1);"
#	print tabs + "_pil_send_buf = (struct _pil_communication_buffers **) malloc(_pil_NP*sizeof(struct _pil_communication_buffers *));"
#	print tabs + "for (int _pil_index = 0; _pil_index < _pil_NP; _pil_index++) {"
#	print tabs + "\t_pil_send_buf[_pil_index] = (struct _pil_communication_buffers *) malloc(_pil_NP*sizeof(struct _pil_communication_buffers));"
#	print tabs + "\tint _pil_i;"
#	print tabs + "\tfor (_pil_i = 0; _pil_i < _pil_NP; _pil_i++) {"
#	print tabs + "\t\t_pil_send_buf[_pil_index][_pil_i].ptr = NULL;"
#	print tabs + "\t\t_pil_send_buf[_pil_index][_pil_i].size = 0;"
#	print tabs + "\t\t_pil_send_buf[_pil_index][_pil_i].full = 0;"
#	print tabs + "\t}"
#	print tabs + "}"
#	print ""
	print tabs + "pil_nw_init();"
	if g.PIL_MAIN:
		print tabs + "pil_enter_db_table = (struct pil_enter_db_table_t *) malloc(pil_get_nwCount() * sizeof(struct pil_enter_db_table_t));"
	if not g.OCR_WAIT and g.PIL_MAIN:
		print tabs + "_pil_thread_lock = (struct _pil_thread_lock_t *) malloc(pil_get_nwCount() * sizeof(struct _pil_thread_lock_t));"
	print ""

	if g.MEM == "sharedspmd":
		print tabs + "for (int _pil_rank = 0; _pil_rank <pil_get_nwCount(); _pil_rank++) {"
		tabs += "\t"
	else:
		print tabs + "int _pil_rank = 0;"

	print tabs + "ocrGuid_t node_main_guid, global_db_guid, rank_db_guid;"
	print tabs + "struct global_param *global;"
	print tabs + "int *_pil_rank_ptr;"
	print ""
	allocate_global_db(tabs)
	print tabs + "check_allocated_db(ocrDbCreate(&rank_db_guid, (void **) &_pil_rank_ptr, sizeof(int), /*flags=*/FLAGS, /*affinity=*/NULL_GUID, NO_ALLOC));"
	run.file_debug(1, tabs + "fprintf(stdout, \"allocated data-block (%ld) for rank_db_guid\\n\", rank_db_guid);")
	print tabs + "*_pil_rank_ptr = _pil_rank;"
	print ""
	if not g.OCR_WAIT and g.PIL_MAIN:
		print tabs + "_pil_thread_lock_init(&_pil_thread_lock[_pil_rank]);"
		print ""

	print tabs + "ocrGuid_t enter_param_event_guid;"
	print tabs + "ocrEventCreate(&enter_param_event_guid, PIL_EVENT_T, true);"
	print tabs + "ocrGuid_t enter_global_event_guid;"
	print tabs + "ocrEventCreate(&enter_global_event_guid, PIL_EVENT_T, true);"
	print tabs + "ocrGuid_t enter_rank_event_guid;"
	print tabs + "ocrEventCreate(&enter_rank_event_guid, PIL_EVENT_T, true);"
	print ""
	print tabs + "ocrGuid_t node_main_template_guid;"
	print tabs + "ocrEdtTemplateCreate(&node_main_template_guid, pil_main_edt, 0, 3);"
	print tabs + "ocrEdtCreate(&node_main_guid, node_main_template_guid, /*paramc=*/0, /*paramv=*/(u64 *)NULL, /*depc=*/3, /*depv=*/NULL, /*properties=*/0 , /*affinity*/NULL_GUID,  /*outputEvent*/NULL );"
	print tabs + "ocrEdtTemplateDestroy(node_main_template_guid);"
	print ""
	print tabs + "ocrAddDependence(enter_param_event_guid, node_main_guid, /*slot=*/0, /*mode*/DB_DEFAULT_MODE);"
	print tabs + "ocrAddDependence(enter_global_event_guid, node_main_guid, /*slot=*/1, /*mode*/DB_DEFAULT_MODE);"
	print tabs + "ocrAddDependence(enter_rank_event_guid, node_main_guid, /*slot=*/2, /*mode*/DB_DEFAULT_MODE);"
	print ""
	print tabs + "ocrEventSatisfy(enter_param_event_guid, depv[0].guid);"
	print tabs + "ocrDbRelease(global_db_guid);"
	print tabs + "ocrEventSatisfy(enter_global_event_guid, global_db_guid);"
	print tabs + "ocrDbRelease(rank_db_guid);"
	print tabs + "ocrEventSatisfy(enter_rank_event_guid, rank_db_guid);"

	if g.MEM == "sharedspmd":
		tabs = tabs[:-1]
		print tabs + "}"

	print ""
	print tabs + "return NULL_GUID;"
	print "}"

