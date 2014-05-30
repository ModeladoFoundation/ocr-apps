###############################################################################
# pil2ocr
###############################################################################

import run
#from run import file_debug, debug
import g
import sys
import re


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
	for arg_name in g.args[func_name]['list']:
		if arg_name == "index_array" or arg_name == "data_array":
			continue
		print "\t" + g.variables[arg_name] + " " + arg_name + ";"

	print "};"
	print ""


def enter_body(tabs, target, func_name, index):
	""""""

	print tabs + "// create one event per body instance to notify when to run exit"
	print tabs + "ocrGuid_t exit_param_event_guid;"
	print tabs + "ocrEventCreate(&exit_param_event_guid, OCR_EVENT_STICKY_T, true);"
	run.file_debug(2, tabs + "fprintf(stdout, \"created exit_event (%ld) for iter %ld\\n\", exit_param_event_guid, iter);")
	print tabs + "ocrAddDependence(exit_param_event_guid, exit_guid, /*slot=*/(3*iter)+0+3, /*mode*/DB_DEFAULT_MODE);"
	run.file_debug(2, tabs + "fprintf(stdout, \"added dependence between exit edt (%ld) and body event (%ld)for node_" + target + "_exit\\n\", exit_guid, exit_param_event_guid);")
	print tabs + "ocrGuid_t exit_index_array_event_guid;"
	print tabs + "ocrEventCreate(&exit_index_array_event_guid, OCR_EVENT_STICKY_T, true);"
	run.file_debug(2, tabs + "fprintf(stdout, \"created exit_event (%ld) for iter %ld\\n\", exit_index_array_event_guid, iter);")
	print tabs + "ocrAddDependence(exit_index_array_event_guid, exit_guid, /*slot=*/(3*iter)+1+3, /*mode*/DB_DEFAULT_MODE);"
	run.file_debug(2, tabs + "fprintf(stdout, \"added dependence between exit edt (%ld) and body event (%ld)for node_" + target + "_exit\\n\", exit_guid, exit_index_array_event_guid);")
	print tabs + "ocrGuid_t exit_data_array_event_guid;"
	print tabs + "ocrEventCreate(&exit_data_array_event_guid, OCR_EVENT_STICKY_T, true);"
	run.file_debug(2, tabs + "fprintf(stdout, \"created exit_event (%ld) for iter %ld\\n\", exit_data_array_event_guid, iter);")
	print tabs + "ocrAddDependence(exit_data_array_event_guid, exit_guid, /*slot=*/(3*iter)+2+3, /*mode*/DB_DEFAULT_MODE);"
	run.file_debug(2, tabs + "fprintf(stdout, \"added dependence between exit edt (%ld) and body event (%ld)for node_" + target + "_exit\\n\", exit_guid, exit_data_array_event_guid);")
	print ""

	print tabs + "// create an event for a dependence on the parameter data block"
	create_event(tabs, target, "param")
	create_event(tabs, target, "global")
	create_event(tabs, target, "index_array")
	create_event(tabs, target, "data_array")
	print ""

	print tabs + "// allocate a data block for the parameter to pass in"
	print tabs + "ocrGuid_t param_db_guid;"
	allocate_param_db(tabs, "", target)
	run.file_debug(2, tabs + "fprintf(stdout, \"allocated data-block (%ld) for iter %ld\\n\", param_db_guid, iter);")
	print ""

	print tabs + "// populate the parameter"
	ocr_param_init(3, target, target)
	print tabs + "n" + target + "p->_pil_iter = iter;"
	print tabs + "n" + target + "p->node_" + target + "_exit_param_event_guid = exit_param_event_guid;"
	print tabs + "n" + target + "p->node_" + target + "_exit_index_array_event_guid = exit_index_array_event_guid;"
	print tabs + "n" + target + "p->node_" + target + "_exit_data_array_event_guid = exit_data_array_event_guid;"

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
			print tabs + "n" + target + "p->" + arg_name + " = input->" + arg_name + ";"

	print ""

	index_helper(tabs, False)
	print ""

	print tabs + "// create the body edt"
	create_edt(tabs, target, False, True)

	print tabs + "// add dependences"
	create_dependence(tabs, target, True, "param", 0)
	create_dependence(tabs, target, True, "global", 1)
	create_dependence(tabs, target, True, "index_array", 2)
	create_dependence(tabs, target, True, "data_array", 3)
	print ""

	print tabs + "// split data_array"
	print tabs + "gpp_t _pil_split_index_array;"
	print tabs + "gpp_t _pil_split_data_array;"
	print tabs + "size_t _pil_index_size = 2 * sizeof(int);"
	print tabs + "size_t _pil_data_size = _pil_num * sizeof(gpp_t);"
	allocate_db(tabs, "_pil_split_index_array.guid", "_pil_split_index_array.ptr", "_pil_index_size");
	allocate_db(tabs, "_pil_split_data_array.guid", "_pil_split_data_array.ptr", "_pil_data_size");
	print tabs + "int *_pil_sia = (int *) _pil_split_index_array.ptr;"
	print tabs + "_pil_sia[0] = 0;"
	print tabs + "_pil_sia[1] = _pil_num;"
	print tabs + "gpp_t *_pil_sda = (gpp_t *) _pil_split_data_array.ptr;"
	print tabs + "memcpy(_pil_split_data_array.ptr, &_pil_da[_pil_ia[iter]], _pil_data_size);"
	print ""
	print tabs + "// add the contents of the data_array as dependences to the body node"
	print tabs + "int _pil_iter;"
	print tabs + "for (_pil_iter = 0; _pil_iter < _pil_num; _pil_iter++) {"
	create_event(tabs + "\t", target, "data_array_item")
	print tabs + "\tocrGuid_t data_array_item_db_guid = _pil_sda[_pil_iter].guid;"
	create_dependence(tabs + "\t", target, True, "data_array_item", "_pil_iter+4")
	satisfy_dependence(tabs + "\t", target, "data_array_item")
	print tabs + "}"

	satisfy_dependence(tabs, target, "param")
	satisfy_dependence(tabs, target, "global")
	print tabs + "index_array_db_guid = _pil_split_index_array.guid;"
	satisfy_dependence(tabs, target, "index_array")
	print tabs + "data_array_db_guid = _pil_split_data_array.guid;"
	satisfy_dependence(tabs, target, "data_array")
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
	run.file_debug(1, "\n\tfprintf(stdout, \"\\nnode_" + m['label'] + "_enter edt\\n\");")
	run.file_debug(1, "\tfprintf(stdout, \"lower: %d  upper: %d  step: %d\\n\", input->" + g.LOWER + ", input->" + g.UPPER + ", input->" + g.STEP + ");")

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

	print "\t// create the exit edt for this node"
	print "\tocrGuid_t exit_template_guid;"
	print "\tint _pil_num_dep = 3*num_iterations+3;"
	print "\tocrEdtTemplateCreate(&exit_template_guid, node_" + m['label'] + "_exit, 0, _pil_num_dep);"
	print "\tocrEdtCreate(&exit_guid, exit_template_guid, /*paramc*/0, /*paramv=*/NULL, /*depc=*/_pil_num_dep, /*depv=*/NULL, /*properties=*/0, /*affinity*/NULL_GUID, /*outputEvent*/NULL);"
	run.file_debug(2, "\tfprintf(stdout, \"created edt (%ld) for node_" + m['label'] + " exit_guid\\n\", exit_guid);")
	print "\t" + "ocrGuid_t global_event_guid, index_array_event_guid, data_array_event_guid;"
	print "\t" + "ocrEventCreate(&global_event_guid, OCR_EVENT_STICKY_T, true);"
	print "\t" + "ocrEventCreate(&index_array_event_guid, OCR_EVENT_STICKY_T, true);"
	print "\t" + "ocrEventCreate(&data_array_event_guid, OCR_EVENT_STICKY_T, true);"
	print "\t" + "ocrAddDependence(global_event_guid, exit_guid, /*slot=*/0, /*mode*/DB_DEFAULT_MODE);"
	print "\t" + "ocrAddDependence(index_array_event_guid, exit_guid, /*slot=*/1, /*mode*/DB_DEFAULT_MODE);"
	print "\t" + "ocrAddDependence(data_array_event_guid, exit_guid, /*slot=*/2, /*mode*/DB_DEFAULT_MODE);"
	print "\t" + "ocrEventSatisfy(global_event_guid, global_db_guid);"
	print "\t" + "ocrEventSatisfy(index_array_event_guid, index_array_db_guid);"
	print "\t" + "ocrEventSatisfy(data_array_event_guid, data_array_db_guid);"
	print ""
	print "\tocrGuid_t body_guid[num_iterations];"
	print ""

	print "\tif (input->" + g.STEP + " < 0)"
	print "\t{"
	print "\t\tfor (" + m['index'] + " = input->" + g.UPPER + "; " + m['index'] + " >= input->" + g.LOWER + "; " + m['index'] + " += input->" + g.STEP + ")"
	print "\t\t{"
	print '\t\t\tprintf("step less than one not fully tested\\n");'
	print "\t\t}"
	print "\t}"
	print "\telse"
	print "\t{"
	print "\t\tfor (input->" + m['index'] + " = input->" + g.LOWER + "; input->" + m['index'] + " <= input->" + g.UPPER + "; input->" + m['index'] + " += input->" + g.STEP + ")"
	print "\t\t{"
	enter_body("\t\t\t", m['label'], m['func_name'], m['index'])
	print "\t\t}"
	print "\t}"
	print ""

	print "\t// schedule the exit guid now that all of the dependencies have been added"
	run.file_debug(2, "\tfprintf(stdout, \"scheduling edt (%ld) for node_" + m['label'] + "_exit\\n\", exit_guid);")
	print "\t//ocrEdtSchedule(exit_guid);"
	print ""

	# see if we need the command line parameters. TODO: This assumes the only varialbles named argc or argv are command line variables
	if "argc" in g.args[m['func_name']]['list'] or "argv" in g.args[m['func_name']]['list']:
		#print "\tocrDbRelease(global->cmd_line_db_guid);"
		#print "\tocrDbRelease(input->global_db_guid);"
		pass
	if "argv" in g.args[m['func_name']]['list']:
		#print "\tocrDbFree(argv_db_guid);"
		pass

	run.file_debug(2, "\tfprintf(stdout, \"freeing data-block (%ld) for depv[0] in node_" + m['label'] + "_enter\\n\", depv[0]);")
	print "\t//ocrDbFree(depv[0].guid, depv[0].ptr);"
	print "\t//ocrDbDestroy(depv[0].guid);"
	print ""
	print "\treturn NULL_GUID;"
	print "}"
	print ""


def index_helper(tabs, body):
	""""""

	print tabs + "gpp_t *_pil_da = (gpp_t *) data_array.ptr;"
	print tabs + "int *_pil_ia = (int *) index_array.ptr;"
	if body:
		print tabs + "int _pil_beg = _pil_ia[0];"
		print tabs + "int _pil_end = _pil_ia[1];"
		print tabs + "int _pil_num = _pil_end - _pil_beg;"
	else:
		print tabs + "int _pil_beg = 0;"
		print tabs + "int _pil_end = 0;"
		print tabs + "int _pil_num = 0;"
		print tabs + "if (_pil_ia[0] != -1) {"
		print tabs + "\t_pil_beg = _pil_ia[iter];"
		print tabs + "\t_pil_end = _pil_ia[iter+1];"
		print tabs + "\t_pil_num = _pil_end - _pil_beg;"
		print tabs + "}"


def output_node_body(m):
	"""This is the work that gets done by each iteration of the node."""
	print "ocrGuid_t node_" +m['label'] + "_body(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])"
	print "{"
	run.file_debug(1, '\tfprintf(stdout, "\\nnode_' + m['label'] + '_body() edt\\n"); fflush(stdout);\n')
	run.file_debug(3, "\tfprintf(stdout, \"receiving data-block (%ld) in iteration UNKNOWN\\n\", depv[0].guid);")
	print "\tstruct node_" + m['label'] + "_param *input = (struct node_" + m['label'] + "_param *) depv[0].ptr;"
	print "\tstruct global_param *global = (struct global_param *) depv[1].ptr;"
	print "\tgpp_t index_array;"
	print "\tindex_array.guid = depv[2].guid;"
	print "\tindex_array.ptr = depv[2].ptr;"
	print "\tgpp_t data_array;"
	print "\tdata_array.guid = depv[3].guid;"
	print "\tdata_array.ptr = depv[3].ptr;"
	print ""

	index_helper("\t", True)
	print "\tint _pil_iter;"
	print "\tfor (_pil_iter = 0; _pil_iter < _pil_num; _pil_iter++) {"
	print "\t\t_pil_da[_pil_iter].ptr = depv[_pil_iter + 4].ptr;"
	print "\t}"

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
			else:
				in_args += ")"
			in_args += "input->" + arg_name
	print ""
	run.file_debug(3, "\tfprintf(stdout, \"calling body function\\n\");")
	print "\t" + m['func_name'] + "(" + in_args + ");"
	run.file_debug(3, "\tfprintf(stdout, \"returned from body function\\n\");")
	print ""
	for arg_name in g.args[m['func_name']]['list']:
		if arg_name == "index_array" or arg_name == "data_array":
			continue
		if g.args[m['func_name']]['dict'][arg_name]['passed_by_reference']:
			print "\tglobal->" + arg_name + " = input->" + arg_name + ";"
	print ""
	print "\tocrEventSatisfy(input->node_" + m['label'] + "_exit_param_event_guid, depv[0].guid);"
	run.file_debug(2, "\tfprintf(stdout, \"satisfied event (%ld) with db (%ld)\\n\", input->node_" + m['label'] + "_exit_param_event_guid, depv[0].guid);")
	print "\tocrEventSatisfy(input->node_" + m['label'] + "_exit_index_array_event_guid, index_array.guid);"
	run.file_debug(2, "\tfprintf(stdout, \"satisfied event (%ld) with db (%ld)\\n\", input->node_" + m['label'] + "_exit_index_array_event_guid, index_array.guid);")
	print "\tocrEventSatisfy(input->node_" + m['label'] + "_exit_data_array_event_guid, data_array.guid);"
	run.file_debug(2, "\tfprintf(stdout, \"satisfied event (%ld) with db (%ld)\\n\", input->node_" + m['label'] + "_exit_data_array_event_guid, data_array.guid);")
	print ""
	print "\treturn NULL_GUID;"
	print "}"
	print ""


def level_to_tabs(level):

	tabs = ""
	for i in range(level):
		tabs += "\t"
	return tabs


def create_edt(tabs, target, finish, body):
	"""the line to create an EDT"""

	num_dependences = "4"

	print tabs + "ocrGuid_t node_" + target + "_template_guid;"

	if body:
		num_dependences += "+_pil_num"
		print tabs + "ocrEdtTemplateCreate(&node_" + target + "_template_guid, node_" + target + "_body, 0, " + num_dependences + ");"
		print tabs + "ocrEdtCreate(&body_guid[iter], node_" + target + "_template_guid, /*paramc=*/0, /*paramv=*/(u64 *)NULL, /*depc=*/" + num_dependences + ", /*depv=*/NULL,",
	else:
		print tabs + "ocrEdtTemplateCreate(&node_" + target + "_template_guid, node_" + target + "_enter, 0, " + num_dependences + ");"
		print tabs + "ocrEdtCreate(&node_" + target + "_guid, node_" + target + "_template_guid, /*paramc=*/0, /*paramv=*/(u64 *)NULL, /*depc=*/" + num_dependences + ", /*depv=*/NULL,",
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

	run.file_debug(2, tabs + "fprintf(stdout, \"created edt (%ld) for node_" + target + "_enter\\n\", node_" + target + "_guid);")
	print ""

def create_event(tabs, target, name):
	""""""

	print tabs + "ocrGuid_t enter_" + name + "_event_guid;"
	print tabs + "ocrEventCreate(&enter_" + name + "_event_guid, OCR_EVENT_STICKY_T, true);"
	run.file_debug(2, tabs + "fprintf(stdout, \"created enter_" + name + "_event (%ld) for node_ " + target + "\\n\", enter_" + name + "_event_guid);")


def create_dependence(tabs, target, body, name, slot):
	""""""

	if body:
		print tabs + "ocrAddDependence(enter_" + name + "_event_guid, body_guid[iter], /*slot=*/" + str(slot) + ", /*mode*/DB_DEFAULT_MODE);"
	else:
		print tabs + "ocrAddDependence(enter_" + name + "_event_guid, node_" + target + "_guid, /*slot=*/" + str(slot) + ", /*mode*/DB_DEFAULT_MODE);"
	run.file_debug(2, tabs + "fprintf(stdout, \"added dependence between enter edt (%ld) and param " + name + " event (%ld)for node_" + target + "_enter\\n\", node_" + target + "_guid, enter_" + name + "_event_guid);")


def satisfy_dependence(tabs, target, name):
	""""""

	#if name == "data_array" or name == "index_array":
	if name == "data_array_item":
		print tabs + "if (" + name + "_db_guid != NULL_GUID)\n", "\t",
	print tabs + "ocrDbRelease(" + name + "_db_guid);"
	print tabs + "ocrEventSatisfy(enter_" + name + "_event_guid, " + name + "_db_guid);"
	run.file_debug(2, tabs + "fprintf(stdout, \"satisfied event (%ld) for node_" + target + " enter\\n\", enter_" + name + "_event_guid);")


def enter_node(level, finish, label, target):
	"""Output code to enter a node"""

	tabs = level_to_tabs(level)

	print ""
	print tabs + "// begin enter_node"

	# create the events
	create_event(tabs, target, "param")
	create_event(tabs, target, "global")
	create_event(tabs, target, "index_array")
	create_event(tabs, target, "data_array")
	create_event(tabs, target, "wait")
	print ""

	# create the EDT
	create_edt(tabs, target, finish, False)

	# add the dependences
	create_dependence(tabs, target, False, "param", 0)
	create_dependence(tabs, target, False, "global", 1)
	create_dependence(tabs, target, False, "index_array", 2)
	create_dependence(tabs, target, False, "data_array", 3)
	print ""

	# satisfy and release the dependences
	satisfy_dependence(tabs, target, "param")
	satisfy_dependence(tabs, target, "global")
	satisfy_dependence(tabs, target, "index_array")
	satisfy_dependence(tabs, target, "data_array")

	print tabs + "// end enter_node"
	print ""


def needs_global(target):
	""""""

	needs_global = False

	if target == "0":
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

def ocr_param_init(level, label, target):
	"""output a param_init call"""

	tabs = level_to_tabs(level)

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
	print tabs + "node_"+ target + "_param_init(n" + target + "p, n" + label + "p->global_db_guid, " + init + ");"
	

def initialize_data_array(tabs):
	""""""

	print tabs + "pil_free(data_array);"
	print tabs + "pil_alloc(&data_array, _pil_ia[num_iterations]*sizeof(gpp_t));"
	print tabs + "data_array_db_guid = data_array.guid;"

def initialize_index_array(tabs):
	""""""

	print tabs + "pil_free(index_array);"
	print tabs + "pil_alloc(&index_array, (num_iterations+1)*sizeof(int));"
	print tabs + "index_array_db_guid = index_array.guid;"
	print tabs + "_pil_ia = (int *) index_array.ptr;"
	print tabs + "for (_pil_iter = 0; _pil_iter <= num_iterations; _pil_iter++) {"
	print tabs + "\t_pil_ia[_pil_iter] = 0;"
	print tabs + "}"


def output_node_exit(m):
	"""This is the confluence edt of a node. All of the nodes of work meet
	here when they are done."""

	print "ocrGuid_t node_" + m['label'] + "_exit(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])"
	print "{"
	print "\tint i;"
	print "\tstruct node_" + m['label'] + "_param *n" + m['label'] + "p = (struct node_" + m['label'] + "_param *) depv[3].ptr;"
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
			for arg_name in g.args[g.functions[target]]['list']:
				if arg_name not in g.args[g.functions[m['label']]]['list']:
					global_is_used = True
	if ng or global_is_used:
		print "\tstruct global_param *global = (struct global_param *) depv[0].ptr;"
	if len(targets) == 1 and target == "0":
		pass
	else:
		print "\tocrGuid_t global_db_guid = depv[0].guid;"
		print "\tgpp_t index_array;"
		print "\tindex_array.guid = depv[1].guid;"
		print "\tindex_array.ptr = depv[1].ptr;"
		print "\tgpp_t data_array;"
		print "\tdata_array.guid = depv[2].guid;"
		print "\tdata_array.ptr = depv[2].ptr;"
		print ""
		print_num_iterations("\t", "n" + m['label'] + "p", True)
	run.file_debug(1, '\n\tfprintf(stdout, "\\nnode_' + m['label'] + '_exit() edt\\n");\n')
	print ""

	print "\tswitch (n" + m['label'] + "p->" + m['cond'] + ")" # TODO: pass this into the edt
	print "\t{"

	for target in targets:
		print "\t\tcase " + target + ":"
		print "\t\t{"
		# TODO: This assumes that a target of 0 is exit
		if target == "0":
			print "\t\t\t// Exit target"
			print "#if 0"
			print "\t\t\tfor (i = 0; i < depc; i++)"
			print "\t\t\t{"
			#run.file_debug(2, "\t\t\t\tfprintf(stdout, \"freeing data-block (%ld) for db[%d] in node_" + m['label'] + "_exit\\n\", depv[i].guid, i);")
			#print "\t\t\t\tocrDbFree(depv[i].guid, depv[i].ptr);"
			run.file_debug(2, "\t\t\t\tfprintf(stdout, \"destroying data-block (%ld) in node_" + m['label'] + "_exit\\n\", depv[i+3].guid);")
			print "\t\t\t\t//ocrDbDestroy(depv[i+3].guid);"
			print "\t\t\t}"
			#run.file_debug(2, "\t\t\tfprintf(stdout, \"freeing data-block (%ld) for global_db_guid in node_" + m['label'] + "_exit\\n\", input_" + m['label'] + "->global_db_guid);")
			#print "\t\t\tstruct global_param *global;"
			#print "\t\t\tocrDbAcquire(input_" + m['label'] + "->global_db_guid, (void **) &global, /*flags=*/FLAGS);"
			#print "\t\t\tocrDbFree(input_" + m['label'] + "->global_db_guid, global);"
			run.file_debug(2, "\t\t\tfprintf(stdout, \"destroying data-block (%ld) for global_db in node_" + m['label'] + "_exit\\n\", global_db_guid);")
			print "\t\t\t//ocrDbDestroy(global_db_guid);"
			print "#endif"
			if not g.PIL_MAIN:
				run.file_debug(2, "\t\t\tfprintf(stdout, \"calling ocrShutdown()\\n\");")
				print "\t\t\tocrShutdown();"
			print "\t\t\treturn NULL_GUID;"
		else:
			#print "\t\t\tocrGuid_t node_" + target + "_type, node_" + target + "_guid, node_" + target + "_db_guid;"
			print "\t\t\tocrGuid_t node_" + target + "_guid, param_db_guid;"
			print "\t\t\tstruct node_" + target + "_param *n" + target + "p;"
			print "\t\t\tocrGuid_t index_array_db_guid = index_array.guid;"
			print "\t\t\tocrGuid_t data_array_db_guid = data_array.guid;"
			allocate_param_db("\t\t\t", "", target)
			run.file_debug(2, "\t\t\tfprintf(stdout, \"allocated data-block (%ld) for node_" + target + "_db_guid\\n\", node_" + target + "_db_guid);")
			print ""

			ocr_param_init(3, m['label'], target)

			print ""
			for arg_name in g.args[g.functions[target]]['list']:
				if arg_name == "index_array" or arg_name == "data_array":
					continue
				elif arg_name in g.args[g.functions[m['label']]]['list']:
					print "\t\t\tn" + target + "p->" + arg_name + " = n" + m['label'] + "p->" + arg_name + ";"
				else:
					print "\t\t\tn" + target + "p->" + arg_name + " = global->" + arg_name + ";"
			print ""

			print_num_iterations("\t\t\t", "n" + target + "p", False)
			print ""

			print "\tint *_pil_ia = NULL;"
			print "\tint _pil_iter;"
			print "\tif (num_iterations != old_num_iterations) {"
			initialize_index_array("\t\t")
			print "\t}"
			print "\telse {"
			print "\t\t_pil_ia = (int *) index_array.ptr;"
			print "\t\t_pil_ia[0] = 0;"
			print "\t\tfor (_pil_iter = 0; _pil_iter < num_iterations; _pil_iter++) {"
			print "\t\t\tint *_pil_split_index_array = (int *) depv[(3*_pil_iter)+4].ptr;"
			print "\t\t\t_pil_ia[_pil_iter+1] = _pil_ia[_pil_iter] + _pil_split_index_array[1];"
			print "\t\t}"
			print "\t}"
			print ""
			initialize_data_array("\t\t")
			print "\tgpp_t *_pil_da = (gpp_t *) data_array.ptr;"
			print "\tfor (_pil_iter = 0; _pil_iter < num_iterations; _pil_iter++) {"
			print "\t\tint _pil_beg = _pil_ia[_pil_iter];"
			print "\t\tint _pil_end = _pil_ia[_pil_iter+1];"
			print "\t\tint _pil_num = _pil_end - _pil_beg;"
			print "\t\tgpp_t _pil_split_data_array;"
			print "\t\t_pil_split_data_array.guid = depv[(3*_pil_iter)+5].guid;"
			print "\t\t_pil_split_data_array.ptr = depv[(3*_pil_iter)+5].ptr;"
			print "\t\tsize_t _pil_data_size = _pil_num * sizeof(gpp_t);"
			print "\t\tmemcpy(&_pil_da[_pil_ia[_pil_iter]], _pil_split_data_array.ptr, _pil_data_size);"
			print "\t}"
			print ""

			enter_node(3, False, m['label'], target)
			print "\t\t\treturn NULL_GUID;"
		print "\t\t\tbreak;"
		print "\t\t}"
	print "\t\tdefault:"
	print "\t\t{"
	print "\t\t\tfprintf(stdout, \"ERROR: Unknown case '%d' in node_" + m['label'] + "_exit\\n\", n" + m['label'] + "p->" + m['cond'] + ");"
	if g.FILE_TYPE in g.CHPL:
		print "\t\t\tchpl_exit_all(EXIT_FAILURE);"
	else:
		print "\t\t\texit(EXIT_FAILURE);"
	print "\t\t\tbreak;"
	print "\t\t}"
	print "\t}"
	print ""
	run.file_debug(2, "\tfprintf(stdout, \"freeing data-block (%ld)\\n\", depv[0].guid);")
	print "\tfor (i = 0; i < depc; i++)"
	print "\t{"
	#print "\t\tocrDbFree(depv[i].guid, depv[i].ptr);"
	run.file_debug(2, "\t\tfprintf(stdout, \"destroying data-block (%ld) for depv[%d] in node_" + m['label'] + "_exit\\n\", depv[i].guid, i);")
	print "\t\t//ocrDbDestroy(depv[i].guid);"
	print "\t}"
	print ""
	print "\treturn NULL_GUID;"
	print "}"
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
	print '#include "extensions/ocr-lib.h" // needed for ocrWait()'
	print '//#include "ocr-runtime.h"'
	print '#include <errno.h>'
	print ""
	print "#define FLAGS 0xdead"
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
		print "\t" + g.variables[var] + " " + var + ";"
	print "};"
	print ""
	print "struct ocr_cmd_line_t"
	print "{"
	print "\tint argc;"
	print "\tchar **argv;"
	print "};"
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
	#	run.file_debug(2, '\tfprintf(stdout, "initializing param ' + arg_name + '\\n"); fflush(stdout);')
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

	print "// Map function prototypes"
	for m in nodes:
		ocr_process_function_prototype(m['label'], m['func_name'])
	print ""

	for m in nodes:
		ocr_process_type(m['label'], m['index'], m['func_name'], m['lower'], m['step'], m['upper'], m['in_args'])

	#ocr_process_global_param_init()
	for m in nodes:
		ocr_process_param_init(m['label'], m['lower'], m['step'], m['upper'], m['func_name'], m['in_args'])

	for m in nodes:
		ocr_process_node_procedure(m)


def declare_pil_enter_arguments():

	print "\tstruct global_param *global;"
	print "\tocrGuid_t global_db_guid;"
	print "\tgpp_t index_array;"
	print "\tgpp_t data_array;"

	for node in g.nodes_entered:
		print "\tstruct node_" + node['label'] + "_param *n" + node['label'] + "p;"
		print "\tocrGuid_t node_" + node['label'] + "_param_db_guid;"
	print ""
	allocate_global_db("\t")
	for node in g.nodes_entered:
		allocate_param_db("\t", "node_" + node['label'] + "_", node['label'])
	print ""


def process_pil_enter_arguments(node):
	"""Output the appropriate code to enter a node"""

	#print "#ifdef PIL2C"
	args = g.args[g.functions[node['label']]]
	for arg_name in args['list']:
		if arg_name == g.target_variables[node['label']]:
			continue
		elif arg_name == "index_array" or arg_name == "data_array":
			if g.args[g.functions[node['label']]]['dict'][arg_name]['passed_by_reference']:
				print "\t\t\t" + arg_name + "_ptr = va_arg(argptr, gpp_t *);"
				print "\t\t\t" + arg_name + " = *" + arg_name + "_ptr;"
			else:
				print "\t\t\t" + arg_name + " = va_arg(argptr, gpp_t);"
		elif g.args[g.functions[node['label']]]['dict'][arg_name]['passed_by_reference']:
			print "\t\t\t" + arg_name + "_ptr = va_arg(argptr, " + g.variables[arg_name] + " *);"
			print "\t\t\tn" + node['label'] + "p->" + arg_name + " = *" + arg_name + "_ptr;"
			print "\t\t\tglobal->" + arg_name + " = *" + arg_name + "_ptr;"
		else:
			print "\t\t\tn" + node['label'] + "p->" + arg_name + " = va_arg(argptr, " + g.variables[arg_name] + ");"
			print "\t\t\tglobal->" + arg_name + " = n" + node['label'] + "p->" + arg_name + ";"
	#print "#endif"
	print "\t\t\tbreak;"


def process_pil_enter(nodes, node):
	"""Output the appropriate code to enter a node"""

	label = node['label']

	#print "\t\t\tstruct node_" + label + "_param *output;"
	#print "\t\t\tnode_" + label + "_param_init(input_" + label + ", global_db_guid, " + g.intervals[label]['lower'] + ", " + g.intervals[label]['step'] + ", " + g.intervals[label]['upper'] + ");"
	run.file_debug(1, "\t\t\tfprintf(stdout, \"pil_enter: entering node %d\\n\", " + label + ");")
	print "\t\t\tocrGuid_t node_" + label + "_guid;"
	print "\t\t\tocrGuid_t finish_guid;"
	print "\t\t\tocrGuid_t index_array_db_guid = index_array.guid;"
	print "\t\t\tocrGuid_t data_array_db_guid = data_array.guid;"
	print "\t\t\tocrGuid_t param_db_guid = node_" + label + "_param_db_guid;"
	ocr_param_init(3, label, label)

	print "\t\t\tif (index_array.guid == NULL_GUID) {"
	print_num_iterations("\t\t\t\t", "n" + node['label'] + "p", False)
	print "\t\t\t\tint *_pil_ia;"
	print "\t\t\t\tint _pil_iter;"
	initialize_index_array("\t\t\t\t")
	print "\t\t\t\tindex_array_db_guid = index_array.guid;"
	initialize_data_array("\t\t\t\t")
	print  "\t\t\t\tdata_array_db_guid = data_array.guid;"
	print "\t\t\t}"

	enter_node(3, True, label, label)
	print "ocrAddDependence(finish_guid, enter_wait_event_guid, 0, DB_DEFAULT_MODE);"
	#print "\t\t\tocrGuid_t returned_guid = ocrWait(finish_guid);"
	print "\t\t\tocrWait(enter_wait_event_guid);"
	print "\t\t\tocrEventDestroy(enter_wait_event_guid);"
	#print "\t\t\t//ocrCleanup();"
	#print "\t\t\t// reinit the runtime"
	#ocr_init(3, nodes)

	args = g.args[g.functions[label]]
	for arg_name in args['list']:
		if arg_name == g.target_variables[label]:
			continue
		if g.args[g.functions[node['label']]]['dict'][arg_name]['passed_by_reference']:
			print "\t\t\t*" + arg_name + "_ptr = global->" + arg_name + ";"
	print "\t\t\tbreak;"

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
	print tabs + "if (err) {"
	print tabs + "\tif (err == ENOMEM) {"
	print tabs + "\t\tassert(0 == ENOMEM);"
	print tabs + "\t}"
	print tabs + "\telse if (err == ENXIO) {"
	print tabs + "\t\tassert(0 == ENXIO);"
	print tabs + "\t}"
	print tabs + "\telse if (err == EINVAL) {"
	print tabs + "\t\tassert(0 == EINVAL);"
	print tabs + "\t}"
	print tabs + "\telse if (err == EBUSY) {"
	print tabs + "\t\tassert(0 == EBUSY);"
	print tabs + "\t}"
	print tabs + "\telse if (err == EPERM) {"
	print tabs + "\t\tassert(0 == EPERM);"
	print tabs + "\t}"
	print tabs + "}"
	print "}"


def allocate_db(tabs, name, ptr, size):
	""""""

	print tabs + "check_allocated_db(ocrDbCreate(&" + name + ", (void **) &" + ptr + ", " + size + ", /*flags=*/FLAGS, /*affinity=*/NULL_GUID, NO_ALLOC));"
	run.file_debug(2, tabs + "fprintf(stdout, \"allocated data-block (%ld) for " + name + "\\n\", " + name + ");")


def allocate_array_db(tabs, name):
	"""allocate a datablock"""

	size = ""
	if name == "data_array":
		size = "gpp_t"
	elif name == "index_array":
		size = "int"
	else:
		error("unrecognized name")

	print tabs + "check_allocated_db(ocrDbCreate(&" + name + "_db_guid, (void **) &ptr, sizeof(" + size + "), /*flags=*/FLAGS, /*affinity=*/NULL_GUID, NO_ALLOC));"
	run.file_debug(2, tabs + "fprintf(stdout, \"allocated data-block (%ld) for " + name + "_db_guid\\n\", " + name + "_db_guid);")


def allocate_param_db(tabs, name, label):
	"""allocate a datablock"""

	print tabs + "check_allocated_db(ocrDbCreate(&" + name + "param_db_guid, (void **) &n" + label + "p, sizeof(struct node_" + label + "_param), /*flags=*/FLAGS, /*affinity=*/NULL_GUID, NO_ALLOC));"
	run.file_debug(2, tabs + "fprintf(stdout, \"allocated data-block (%ld) for param_db_guid\\n\", param_db_guid);")


def allocate_global_db(tabs):
	"""allocate the global datablock"""

	print tabs + "check_allocated_db(ocrDbCreate(&global_db_guid, (void **) &global, sizeof(struct global_param), /*flags=*/FLAGS, /*affinity=*/NULL_GUID, NO_ALLOC));"
	run.file_debug(2, tabs + "fprintf(stdout, \"allocated data-block (%ld) for global_db_guid\\n\", global_db_guid);")
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

	print "//u8 main_edt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])"
	print "ocrGuid_t mainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])"
	print "//int ocr_main(int argc, char **argv)"
	print "{"
	run.file_debug(1, '\tfprintf(stdout, "main_edt()\\n");')
	run.file_debug(1, '\tfprintf(stdout, "Using OCR backend of PIL\\n");')

	print "\tint i;"
	print "\tint argc = getArgc(depv[0].ptr);"
	print "\tchar **argv = (char **) malloc(argc * sizeof(char *));"
	print "\tfor (i = 0; i < argc; i++)"
	print "\t{"
	print "\t\tchar *arg = getArgv(depv[0].ptr, i);"
	print "\t\targv[i] = malloc(strlen(arg));"
	print "\t\tstrcpy(argv[i], arg);"
	print "\t\t//printf('argv[%d]: %s\\n', i, argv[i]);"
	print "\t}"

	if g.PIL_MAIN:
		#ocr_init(1, nodes)
		print "\tpil_main(argc, argv);"
	else:
		print "\tocrGuid_t node_" + label + "_guid, param_db_guid, global_db_guid;"
		print "\tocrGuid_t index_array_db_guid = NULL_GUID;"
		print "\tocrGuid_t data_array_db_guid = NULL_GUID;"
		print "\tstruct node_" + label + "_param *n" + label + "p;"
		print "\tstruct global_param *global;"
		print ""
		#ocr_init(1, nodes)
		allocate_param_db("\t", "", label)
		allocate_global_db("\t")
		print "\tvoid *ptr;"
		allocate_array_db("\t", "index_array")
		print "\tint *_pil_ia = (int *) ptr;"
		print "\t*_pil_ia = -1;"
		allocate_array_db("\t", "data_array")
		print "\tgpp_t *_pil_da = (gpp_t *) ptr;"
		print "\t_pil_da->guid = NULL_GUID;"
		print "\t_pil_da->ptr = NULL;"

#		if "argc" in g.args[g.main_node_func_name]['list'] or "argv" in g.args[g.main_node_func_name]['list']:
#			print "\tglobal->cmd_line_db_guid = depv[0].guid;"
		print ""
		print "\tnode_" + label + "_param_init(n" + label + "p, global_db_guid, " + init + ");"#in_args + ");"
		# see if we need the command line parameters. TODO: This assumes the only varialbles named argc or argv are command line variables
		if "argc" in g.args[g.main_node_func_name]['list'] or "argv" in g.args[g.main_node_func_name]['list']:
			#print "\tstruct global_parm *global;"
			#print "\tocrDbAcquire(input->global_db_guid, (void **) &global, /*flags=*/FLAGS);"
			print "\tn" + label + "p->argc = argc;"
		if "argv" in g.args[g.main_node_func_name]['list']:
			print "\tn" + label + "p->argv = argv;"
		print ""
		enter_node(1, False, label, label)
		print "\t//ocrCleanup();"
	print ""
	if g.PIL_MAIN:
		run.file_debug(2, "\tfprintf(stdout, \"calling ocrShutdown()\\n\");")
		print "\tocrShutdown();"
	print "\treturn NULL_GUID;"
	print "}"

