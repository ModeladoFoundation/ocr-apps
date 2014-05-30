###############################################################################
# pil2afl
###############################################################################

import run
#from run import file_debug, debug
import g
import sys
import re


#------------------------------------------------------------------------------
# AFL specific process functions for use by mc
#------------------------------------------------------------------------------

def print_main_func():
	"""output the main function"""

	for line in g.pil_main_lines:
		print line,


def print_funcs():
	"""output the body functions"""

	for func in g.func_lines.keys():
		for line in g.func_lines[func]:
			print line,


def afl_process_function_prototype(label, func_name):
	"""Output a function prototype for each node function"""

	print "rmd_guid_t node_" + label + "(uint64_t, int, void **, rmd_guid_t *);"
	print "rmd_guid_t node_" + label + "_body(uint64_t, int, void **, rmd_guid_t *);"
	print "rmd_guid_t node_" + label + "_exit(uint64_t, int, void **, rmd_guid_t *);"


def afl_process_type(label, index, func_name, lower, step, upper, in_args):
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
	print "\trmd_guid_t global_db_guid;"
	print "\tint32_t " + g.LOWER + "; // initial: " + lower
	print "\tint32_t " + g.STEP + "; // initial: " + step
	print "\tint32_t " + g.UPPER + "; // initial: " + upper
	print "\trmd_guid_t node_" + label + "_exit_guid;"
	print ""

	if index not in g.args[func_name]['list']:
		print "\t" + g.variables[index] + " " + index + ";"
	for arg_name in g.args[func_name]['list']:
		print "\t" + g.variables[arg_name] + " " + arg_name + ";"

	print "};"
	print ""

#	# output the type for the function inside the node
#	print "// type for the function " + func_name
#	print "struct " + func_name + "_param"
#	print "{"
#	print "\t" + "// in_args: " + in_args + ";"
#
#	for arg_name in g.args[func_name]['list']:
#		print "\t" + g.variables[arg_name] + " " + arg_name + ";"
#
#	print "};"
#	print ""

	
def output_node_entry(m):
	"""This is the entry to a node. It schedules the parallel body of the
	work."""

	print "rmd_guid_t node_" + m['label'] + "(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db)"
	print "{"
	print "\trmd_guid_t node_" + m['label'] + "_body_type, node_" + m['label'] + "_body_guid, node_" + m['label'] + "_exit_type;"
	print "\tstruct node_" + m['label'] + "_param *input = (struct node_" + m['label'] + "_param *) db_ptr[0];"
	print "\tstruct node_" + m['label'] + "_param *output;"
	print "\t" + g.variables[m['index']] + " " + m['index'] + ";"
	print ""
	print "\tuint64_t num_iterations = 0;"
	print "\tuint64_t iter = 0;"
	print "\trmd_guid_t *db_id;"
	run.file_debug(1, "\n\tfprintf(stdout, \"node_" + m['label'] + " entry codelet\\n\");")
	run.file_debug(1, "\tfprintf(stdout, \"lower: %d  upper: %d  step: %d\\n\", input->" + g.LOWER + ", input->" + g.UPPER + ", input->" + g.STEP + ");")

	print ""
	#print "\tif (" + m['step'] + " < 0)"
	print "\tif (input->" + g.STEP + " < 0)"
	print "\t\tnum_iterations = (input->" + g.UPPER + " - input->" + g.LOWER + ")/(0 - input->" + g.STEP + ") + 1;"
	print "\telse"
	print "\t\tnum_iterations = (input->" + g.UPPER + " - input->" + g.LOWER + ")/input->" + g.STEP + " + 1;"
	print ""
	print "\trmd_guid_t db_id_guid;"
	print "\trmd_location_t loc;"
	print "\tu64 retval;"
	print "\tloc.type = RMD_LOC_TYPE_RELATIVE;"
	print "\tloc.data.relative.level = RMD_LOCATION_BLOCK;"
	print "\tretval = rmd_db_alloc(&db_id_guid, num_iterations*sizeof(rmd_guid_t), 0, &loc);"
	print "\tassert(0 == GET_STATUS(retval));"
	run.file_debug(2, "\tfprintf(stdout, \"allocated data-block (%ld) for db_id_guid\\n\", db_id_guid.data);")
	#print "\trmd_db_mem((void**)&db_id, db_id_guid);"
	print "\tdb_id = (rmd_guid_t*)(GET_ADDRESS(retval));"
	print ""

	# see if we need the command line parameters. TODO: This assumes the only varialbles named argc or argv are command line variables
	if "argc" in g.args[m['func_name']]['list'] or "argv" in g.args[m['func_name']]['list']:
		print "\tstruct global_param *global = (struct global_param *) RMD_DB_ACQUIRE(input->global_db_guid, 0);"
		print "\trmd_cmd_line_t *cmd_line_ptr = RMD_DB_ACQUIRE(global->cmd_line_db_guid, 0);"
	if "argv" in g.args[m['func_name']]['list']:
		print "\trmd_guid_t argv_db_guid;"
		print "\tchar **argv = (char **) RMD_DB_ALLOC(&argv_db_guid, cmd_line_ptr->argc*sizeof(char*), 0, &loc);" # TODO: this should probably be core local
		print "\tint i;"
		print "\tfor (i = 0; i < cmd_line_ptr->argc; i++)"
		print '\t\targv[i] = (char *) cmd_line_ptr + cmd_line_ptr->argv[i];'
		print ""

	print "\trmd_codelet_create(&node_" + m['label'] + "_exit_type, node_" + m['label'] + "_exit, 0, 0, num_iterations, 1, false, 0);"
	run.file_debug(2, "\tfprintf(stdout, \"created codelet (%ld) for node_" + m['label'] + "_exit_type\\n\", node_" + m['label'] + "_exit_type.data);")
	print "\trmd_codelet_sched(&input->node_" + m['label'] + "_exit_guid, 0, node_" + m['label'] + "_exit_type);"
	run.file_debug(2, "\tfprintf(stdout, \"scheduled codelet (%ld) for node_" + m['label'] + "_exit\\n\", input->node_" + m['label'] + "_exit_guid.data);")
	print ""
	print "\trmd_codelet_create(&node_" + m['label'] + "_body_type, node_" + m['label'] + "_body, 0, 0, 1, 1, false, 0);"
	run.file_debug(2, "\tfprintf(stdout, \"created codelet (%ld) for node_" + m['label'] + "_body_type\\n\", node_" + m['label'] + "_body_type.data);")
	print ""

	print "\tif (input->" + g.STEP + " < 0)"
	print "\t{"
	print "\t\tfor (" + m['index'] + " = input->" + g.UPPER + "; " + m['index'] + " >= input->" + g.LOWER + "; " + m['index'] + " += input->" + g.STEP + ")"
	print "\t\t{"
	print '\t\t\tprintf("step less than one not fully tested\\n");'
#	if g.FILE_TYPE in g.CHPL:
#		print "\t\t\tchpl_exit_all(EXIT_FAILURE);"
#	else:
#		print "\t\t\texit(EXIT_FAILURE);"
	print "\t\t\tloc.type = RMD_LOC_TYPE_RELATIVE;"
	print "\t\t\tloc.data.relative.level = RMD_LOCATION_BLOCK;"
	print "\t\t\tretval = rmd_db_alloc(&db_id[iter], sizeof(struct node_" + m['label'] + "_param), 0, &loc);"
	print "\t\t\tassert(0 ==GET_STATUS(retval));"
	run.file_debug(2, "\t\t\tfprintf(stdout, \"allocated data-block (%ld) for db_id[%ld]\\n\", db_id[iter].data, iter);")
	#print "\t\t\trmd_db_mem((void**)&output, db_id[iter]);"
	print "\t\t\toutput = (struct node_" + m['label'] + "_param *)GET_ADDRESS(retval);"

	print "\t\t\tnode_" + m['label'] + "_param_init(output, input->global_db_guid, input->" + g.LOWER + ", input->" + g.STEP + ", input->" + g.UPPER + ");"
	print "\t\t\toutput->node_" + m['label'] + "_exit_guid = input->node_" + m['label'] + "_exit_guid;"

	# pass on each argument
	for arg_name in g.args[m['func_name']]['list']:
		if arg_name == "argc":
			print "\t\t\toutput->argc = cmd_line_ptr->argc;"
		elif arg_name == "argv":
			print "\t\t\toutput->argv = argv;"
		else:
			print "\t\t\toutput->" + arg_name + " = input->" + arg_name + ";"

		#print '\tprintf("The rmd_cmd_line_db is %ld\\n", db[0].data);'

	print "\t\t\trmd_db_release(db_id[iter]);"
	print ""
	print "\t\t\trmd_codelet_sched(&node_" + m['label'] + "_body_guid, iter, node_" + m['label'] + "_body_type); // TODO: do i need a new guid for each codelet scheduled?"
	run.file_debug(2, "\t\t\tfprintf(stdout, \"scheduled codelet (%ld) for node_" + m['label'] + "_body\\n\", node_" + m['label'] + "_body_guid.data);")
	run.file_debug(2, "\t\t\tfprintf(stdout, \"sending data-block (%ld) to iteration %ld\\n\", db_id[iter].data, iter);")
	print "\t\t\trmd_codelet_satisfy(node_" + m['label'] + "_body_guid, db_id[iter], 0);"
	print "\t\t\titer++;"
	print "\t\t}"
	print "\t}"
	print "\telse"
	print "\t{"
	print "\t\tfor (input->" + m['index'] + " = input->" + g.LOWER + "; input->" + m['index'] + " <= input->" + g.UPPER + "; input->" + m['index'] + " += input->" + g.STEP + ")"
	print "\t\t{"
	print "\t\t\tloc.type = RMD_LOC_TYPE_RELATIVE;"
	print "\t\t\tloc.data.relative.level = RMD_LOCATION_BLOCK;"
	print "\t\t\tretval = rmd_db_alloc(&db_id[iter], sizeof(struct node_" + m['label'] + "_param), 0, &loc);"
	print "\t\t\tassert(0 ==GET_STATUS(retval));"
	run.file_debug(2, "\t\t\tfprintf(stdout, \"allocated data-block (%ld) for db_id[%ld]\\n\", db_id[iter].data, iter);")
	#print "\t\t\trmd_db_mem((void**)&output, db_id[iter]);"
	print "\t\t\toutput = (struct node_" + m['label'] + "_param *)GET_ADDRESS(retval);"

	print "\t\t\tnode_" + m['label'] + "_param_init(output, input->global_db_guid, input->" + g.LOWER + ", input->" + g.STEP + ", input->" + g.UPPER + ");"
	print "\t\t\toutput->node_" + m['label'] + "_exit_guid = input->node_" + m['label'] + "_exit_guid;"

	# pass on each argument
	for arg_name in g.args[m['func_name']]['list']:
		if arg_name == "argc":
			print "\t\t\toutput->argc = cmd_line_ptr->argc;"
		elif arg_name == "argv":
			print "\t\t\toutput->argv = argv;"
		else:
			print "\t\t\toutput->" + arg_name + " = input->" + arg_name + ";"

		#print '\tprintf("The rmd_cmd_line_db is %ld\\n", db[0].data);'

	print "\t\t\trmd_db_release(db_id[iter]);"
	print ""
	print "\t\t\trmd_codelet_sched(&node_" + m['label'] + "_body_guid, iter, node_" + m['label'] + "_body_type); // TODO: do i need a new guid for each codelet scheduled?"
	run.file_debug(2, "\t\t\tfprintf(stdout, \"scheduled codelet (%ld) for node_" + m['label'] + "_body\\n\", node_" + m['label'] + "_body_guid.data);")
	run.file_debug(2, "\t\t\tfprintf(stdout, \"sending data-block (%ld) to iteration %ld\\n\", db_id[iter].data, iter);")
	print "\t\t\trmd_codelet_satisfy(node_" + m['label'] + "_body_guid, db_id[iter], 0);"
	print "\t\t\titer++;"
	print "\t\t}"
	print "\t}"
	print ""

	# see if we need the command line parameters. TODO: This assumes the only varialbles named argc or argv are command line variables
	if "argc" in g.args[m['func_name']]['list'] or "argv" in g.args[m['func_name']]['list']:
		print "\tRMD_DB_RELEASE(global->cmd_line_db_guid);"
		print "\tRMD_DB_RELEASE(input->global_db_guid);"
	if "argv" in g.args[m['func_name']]['list']:
		print "\tRMD_DB_FREE(argv_db_guid);"

	run.file_debug(2, "\tfprintf(stdout, \"freeing data-block (%ld) for db_id_guid in node_" + m['label'] + "_entry\\n\", db_id_guid.data);")
	print "\trmd_db_release(db_id_guid);"
	print "\trmd_db_free(db_id_guid);"
	run.file_debug(2, "\tfprintf(stdout, \"freeing data-block (%ld) for db[0] in node_" + m['label'] + "_entry\\n\", db[0].data);")
	print "\trmd_db_free(db[0]);"
	print ""
	print "\trmd_guid_t ret_guid;"
	print "\tret_guid.data = 0;"
	print "\treturn ret_guid;"
	print "}"
	print ""


def output_node_body(m):
	"""This is the work that gets done by each iteration of the node."""
	print "rmd_guid_t node_" +m['label'] + "_body(uint64_t iteration_number, int n_db, void *db_ptr[], rmd_guid_t *db)"
	print "{"
	run.file_debug(1, '\tfprintf(stdout, "node_' + m['label'] + '_body() codelet iteration %ld\\n", iteration_number);\n')
	run.file_debug(2, "\tfprintf(stdout, \"receiving data-block (%ld) in iteration %ld\\n\", db[0].data, iteration_number);")
	print "\tstruct node_" + m['label'] + "_param *input = (struct node_" + m['label'] + "_param *) db_ptr[0];"
	#print "\tstruct " + m['func_name'] + "_param *ptr;"
	#print "\trmd_db_mem((void**)&ptr, db_ptr[0]);"
	#print ""
	arg_num = 0;
	for arg_name in g.args[m['func_name']]['list']:
		#print "\t" + g.variables[arg_name] + " " + arg_name + " = *(" + g.variables[arg_name] + "*) ptr->" + arg_name + ";"
	#	print "\t" + g.variables[arg_name] + " " + arg_name + " = input->" + arg_name + ";"
		#run.file_debug(1, '\tfprintf(stdout, "' + arg_name + ': %d\\n", input->' + arg_name + ');')
		arg_num += 1

	first = True
	in_args = ""
	for arg_name in g.args[m['func_name']]['list']:
		if first:
			first = False
		else:
			in_args += ", "

		# check to see if we should pass the argument by reference or by value
		in_args += "(" + g.variables[arg_name]
		if g.args[m['func_name']]['dict'][arg_name]['passed_by_reference']:
			in_args += " *)&"
		else:
			in_args += ")"
		in_args += "input->" + arg_name
	print ""
	#print "\t" + m['func_name'] + "(" + m['in_args'] + ");"
	print "\t" + m['func_name'] + "(" + in_args + ");"
	print ""
	print "\tstruct global_param *global = (struct global_param *) RMD_DB_ACQUIRE(input->global_db_guid, 0);"
	#print "\trmd_db_mem((void**)&global, input->global_db_guid);"
	#print "\tstruct node_global_param *input = (struct node_global_param *) input->global_db_guid;"
	print ""
	for arg_name in g.args[m['func_name']]['list']:
		if g.args[m['func_name']]['dict'][arg_name]['passed_by_reference']:
			print "\tglobal->" + arg_name + " = input->" + arg_name + ";"
	print ""
	#print "\trmd_guid_t exit_db_guid;"
	#print "\t" + g.variables[m['cond']] + " *target_ptr;"
	#print "\trmd_db_alloc(&exit_db_guid, sizeof(" + g.variables[m['cond']] + "), 0, COREID_SELF);"
	#run.file_debug(2, "\tfprintf(stdout, \"allocated data-block (%ld) for exit_db_guid\\n\", exit_db_guid.data);")
	#print "\trmd_db_mem((void**)&target_ptr, exit_db_guid);"
	#print "\t*target_ptr = " + m['cond'] + ";"
	#print ""
	#run.file_debug(1, '\tfprintf(stdout, "node_' + m['label'] + '_exit_guid: %ld\\n", node_' + m['label'] + '_exit_guid.data);')
	#print "\trmd_codelet_satisfy(input->node_" + m['label'] + "_exit_guid, exit_db_guid, iteration_number);"
	print "\trmd_codelet_satisfy(input->node_" + m['label'] + "_exit_guid, db[0], iteration_number);"
	#run.file_debug(2, "\tfprintf(stdout, \"freeing data-block (%ld) for db[0] in node_" + m['label'] + "_body\\n\", db[0].data);")
	#print "\trmd_db_release(exit_db_guid);"
	#print "\trmd_db_free(db[0]);"
	print "\trmd_db_release(input->global_db_guid);"
	print ""
	print "\trmd_guid_t ret_guid;"
	print "\tret_guid.data = 0;"
	print "\treturn ret_guid;"
	print "}"
	print ""


def output_node_exit(m):
	"""This is the confluence codelet of a node. All of the nodes of work meet
	here when they are done."""

	print "rmd_guid_t node_" + m['label'] + "_exit(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db)"
	print "{"
	print "\tint i;"
	if "0" in g.targets[m['label']] and len(g.targets[m['label']]) == 1:
		pass
	else:
		print "\trmd_location_t loc;"
		print "\tu64 retval;"
	print "\trmd_guid_t ret_guid;"
	print "\tret_guid.data = 0;"
	print "\tstruct node_" + m['label'] + "_param *input = (struct node_" + m['label'] + "_param *) db_ptr[0];"
	#print "\t" + g.variables[m['cond']] + " target = *(int*) db_ptr[0];"
	#print "\t" + g.variables[m['cond']] + " *target_ptr;"
	#print "\trmd_db_mem((void**)&target_ptr, db_ptr[0]);"
	run.file_debug(1, '\tfprintf(stdout, "node_' + m['label'] + '_exit() codelet\\n");')
	print "\tswitch (input->" + m['cond'] + ")" # TODO: pass this into the codelet
	print "\t{"
	for target in g.targets[m['label']]:
		print "\t\tcase " + target + ":"
		print "\t\t{"
		# TODO: This assumes that a target of 0 is exit
		if target == "0":
			print "\t\t\t// Exit target"
			print "\t\t\tfor (i = 0; i < n_db; i++)"
			print "\t\t\t{"
			run.file_debug(2, "\t\t\t\tfprintf(stdout, \"freeing data-block (%ld) for db[%d] in node_" + m['label'] + "_exit\\n\", db[i].data, i);")
			print "\t\t\t\trmd_db_free(db[i]);"
			print "\t\t\t}"
			run.file_debug(2, "\t\t\tfprintf(stdout, \"freeing data-block (%ld) for global_db_guid in node_" + m['label'] + "_exit\\n\", input->global_db_guid.data);")
			print "\t\t\trmd_db_free(input->global_db_guid);"
			print "\t\t\trmd_complete();"
			print "\t\t\treturn ret_guid;"
		else:
			print "\t\t\trmd_guid_t node_" + target + "_type, node_" + target + "_guid, node_" + target + "_db_guid;"
			print "\t\t\tstruct node_" + target + "_param *output;"
			print "\t\t\tloc.type = RMD_LOC_TYPE_RELATIVE;"
			print "\t\t\tloc.data.relative.level = RMD_LOCATION_BLOCK;"
			print "\t\t\tretval = rmd_db_alloc(&node_" + target + "_db_guid, sizeof(struct node_" + target + "_param), 0, &loc);"
			print "\t\t\tassert(0 == GET_STATUS(retval));"
			run.file_debug(2, "\t\t\tfprintf(stdout, \"allocated data-block (%ld) for node_" + target + "_db_guid\\n\", node_" + target + "_db_guid.data);")
			print "\t\t\toutput = (struct node_" + target + "_param *)GET_ADDRESS(retval);"
			print ""

			# TODO: this should be False, but I'm taking a shortcut for the TODO a couple lines down
			#uses_global = False
			uses_global = True
			for arg_name in g.args[g.functions[target]]['list']:
				if arg_name not in g.args[g.functions[m['label']]]['list']:
					uses_global = True
			# TODO: add a check for the parameters in a the lower:step:upper range to see if they come from global
			if uses_global:
				print "\t\t\tstruct global_param *global = (struct global_param *) RMD_DB_ACQUIRE(input->global_db_guid, 0);"

			init = ""
			lm = re.match("\d+", g.intervals[target]['lower'])
			if lm:
				init += g.intervals[target]['lower'] + ", "
			else:
				if g.intervals[target]['lower'] in g.args[m['func_name']]:
					init += "input->"
				else:
					init  += "global->"
				init += g.intervals[target]['lower'] + ", "
			sm = re.match("\d+", g.intervals[target]['step'])
			if sm:
				init += g.intervals[target]['step'] + ", "
			else:
				if g.intervals[target]['step'] in g.args[m['func_name']]:
					init += "input->"
				else:
					init  += "global->"
				init += g.intervals[target]['step'] + ", "
			um = re.match("\d+", g.intervals[target]['upper'])
			if um:
				init += g.intervals[target]['upper']
			else:
				if g.intervals[target]['upper'] in g.args[m['func_name']]:
					init += "input->"
				else:
					init  += "global->"
				init += g.intervals[target]['upper']

			print "\t\t\t// [lower, step, upper] = [" + init + "]"
			print "\t\t\tnode_"+ target + "_param_init(output, input->global_db_guid, " + init + ");"

			print ""
			for arg_name in g.args[g.functions[target]]['list']:
				if arg_name in g.args[g.functions[m['label']]]['list']:
					print "\t\t\toutput->" + arg_name + " = input->" + arg_name + ";"
				else:
					print "\t\t\toutput->" + arg_name + " = global->" + arg_name + ";"
			print ""
			print "\t\t\trmd_codelet_create(&node_" + target + "_type, node_" + target + ", 0, 0, 1, 1, false, 0);"
			run.file_debug(2, "\t\t\tfprintf(stdout, \"created codelet (%ld) for node_" + target + "_type\\n\", node_" + target + "_type.data);")
			print "\t\t\trmd_codelet_sched(&node_" + target + "_guid, 0, node_" + target + "_type);"
			run.file_debug(2, "\t\t\tfprintf(stdout, \"scheduled codelet (%ld) for node_" + target + " entry\\n\", node_" + target + "_guid.data);")
			print "\t\t\trmd_codelet_satisfy(node_" + target + "_guid, node_" + target + "_db_guid, 0);"
			print "\t\t\trmd_db_release(node_" + target + "_db_guid);"
			print "\t\t\trmd_db_release(input->global_db_guid);"
		print "\t\t\tbreak;"
		print "\t\t}"
	print "\t\tdefault:"
	print "\t\t{"
	print "\t\t\tfprintf(stdout, \"ERROR: Unknown case '%d' in node_" + m['label'] + "\\n\", input->" + m['cond'] + ");"
	if g.FILE_TYPE in g.CHPL:
		print "\t\t\tchpl_exit_all(EXIT_FAILURE);"
	else:
		print "\t\t\texit(EXIT_FAILURE);"
	print "\t\t\tbreak;"
	print "\t\t}"
	print "\t}"
	print ""
	run.file_debug(2, "\tfprintf(stdout, \"freeing data-block (%ld)\\n\", db[0].data);")
	print "\tfor (i = 0; i < n_db; i++)"
	print "\t\trmd_db_free(db[i]);"
	print ""
	print "\treturn ret_guid;"
	print "}"
	print ""


# TODO: change this function so it doesn't take 'm', but the pieces of 'm' that it needs
# TODO: change this so it doesn't generate a single codelet, but 2 or 3. An entry, a body and a confluence codelet
def afl_process_node_procedure(m):
	"""Output the body of a node function"""

	output_node_entry(m)
	output_node_body(m)
	output_node_exit(m)


def process_header():
	"""Outputs the header for the file. E.g., includes libraries."""

	run.process_header()

	print "// Includes for the AFL library"
	print "#include <unistd.h>"
	print '#include "codelet.h"'
	print '#include "rmd_afl_all.h"'
	print ""


def process_variables():
	"""Create a global store for all variables declared for use on shared
	   memory only."""

	print "//--------------------------------------"
	print "// global parameter"
	print "//--------------------------------------"
	print ""
	print "struct global_param"
	print "{"
	print "\trmd_guid_t cmd_line_db_guid;"
	for var in g.variables:
		print "\t" + g.variables[var] + " " + var + ";"
	print "};"
	print ""


def afl_process_global_param_init():
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


def afl_process_node_param_init(func_name, label):
	"""generate the parameter init function for the function in the node"""

	# find the input arguments
	in_args = ""
	for arg_name in g.args[func_name]['list']:
		in_args += ", " + g.variables[arg_name] + " " + arg_name

	print "static void node_" + label + "_param_init(struct node_" + label + "_param *inst, rmd_guid_t g, int64_t l, int64_t s, int64_t u)" #+ in_args + ")"
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


def afl_process_param_init(label, lower, step, upper, func_name, in_args):
	"""process param init"""

	print "//--------------------------------------"
	print "// init functions for node_" + label
	print "//--------------------------------------"
	print ""

	afl_process_node_param_init(func_name, label)
	#afl_process_func_param_init(func_name, label)


def handle_nodes(nodes):
	"""Output the C code for each node in nodes."""

	print "// Map function prototypes"
	for m in nodes:
		afl_process_function_prototype(m['label'], m['func_name'])
	print ""

	for m in nodes:
		afl_process_type(m['label'], m['index'], m['func_name'], m['lower'], m['step'], m['upper'], m['in_args'])

	#afl_process_global_param_init()
	for m in nodes:
		afl_process_param_init(m['label'], m['lower'], m['step'], m['upper'], m['func_name'], m['in_args'])

	for m in nodes:
		afl_process_node_procedure(m)


def process_pil_enter(node):
	"""Output the appropriate code to enter a node"""

	sys.stderr.write("ERROR: pil_enter is not implemented for afl\n")
	sys.exit(1)


def process_main():
	"""Output the main function for the file."""

	label = g.main_node_label
	init = g.main_node_input[0] + ", " + g.main_node_input[1] + ", " + g.main_node_input[2]

	if g.FILE_TYPE in g.CHPL:
		print "// dummy chpl_main function to appease the Chapel and start the AFL runtime"
		print "void chpl_main()"
		print "{"
		print "\treturn;"
	else:
		print "int main(int argc, char **argv)"
		print "{"
		print "\tpil_init(argc, argv);"
		print "\treturn EXIT_SUCCESS;"

	print "}"
	print ""
	print ""

	print "rmd_guid_t main_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db)"
	print "{"
	print '\tprintf("main_codelet()\\n");'
	run.file_debug(1, '\tfprintf(stdout, "Using AFL backend of PIL\\n");')
	#run.set_stack_size()
	print "\trmd_guid_t node_" + label + "_type, node_" + label + "_guid, node_" + label + "_db_guid, global_db_guid;"
	print "\tstruct node_" + label + "_param *input;"
	print "\trmd_location_t loc;"
	print "\tu64 retval;"
	print "\trmd_cmd_line_t *cmd_line_ptr = db_ptr[0];"
	print "\tassert(n_db == 1);"
	print ""
	print "\tint i;"
	print '\tprintf("The rmd_cmd_line_db is %ld\\n", db[0].data);'
	print "\tfor (i = 0; i < cmd_line_ptr->argc; i++)"
	print '\t\tprintf("\\t%s\\n", (char *) cmd_line_ptr + cmd_line_ptr->argv[i]);'
	print ""
	print "\tloc.type = RMD_LOC_TYPE_RELATIVE;"
	print "\tloc.data.relative.level = RMD_LOCATION_BLOCK;"
	print "\tretval = rmd_db_alloc(&node_" + label + "_db_guid, sizeof(struct node_" + label + "_param), 0, &loc);"
	print "\tassert(0 == GET_STATUS(retval));"
	run.file_debug(2, "\tfprintf(stdout, \"allocated data-block (%ld) for node_" + label + "_db_guid\\n\", node_" + label + "_db_guid.data);")
	#print "\trmd_db_mem((void**)&input, node_" + label + "_db_guid);"
	print "\tinput = (struct node_" + label + "_param *)GET_ADDRESS(retval);"

	#in_args = ""
	#for arg_name in g.args[g.main_node_func_name]['list']:
	#	in_args += ", global." + arg_name

	print "\tstruct global_param *global;"
	#print "\tglobal = (struct global_param *) malloc(sizeof(struct global_param));"
	print "\tretval = rmd_db_alloc(&global_db_guid, sizeof(struct global_param), 0, &loc);"
	print "\tassert(0 == GET_STATUS(retval));"
	run.file_debug(2, "\tfprintf(stdout, \"allocated data-block (%ld) for global_db_guid\\n\", global_db_guid.data);")
	print "\tglobal = (struct global_param *)GET_ADDRESS(retval);"
	print "\t//global_param_init();"
	if "argc" in g.args[g.main_node_func_name]['list'] or "argv" in g.args[g.main_node_func_name]['list']:
		print "\tglobal->cmd_line_db_guid = db[0];"
	print ""
	print ""
	print "\tnode_" + label + "_param_init(input, global_db_guid, " + init + ");"#in_args + ");"
	print ""
	print "\trmd_codelet_create(&node_" + label + "_type, node_" + label + ", 0, 0, 1, 1, false, 0);"
	run.file_debug(2, "\tfprintf(stdout, \"created codelet (%ld) for node_" + label + "_type\\n\", node_" + label + "_type.data);")
	print "\trmd_codelet_sched(&node_" + label + "_guid, 0, node_" + label + "_type);"
	run.file_debug(2, "\tfprintf(stdout, \"scheduled codelet (%ld) for node_" + label + " entry\\n\", node_" + label + "_guid.data);")
	print "\trmd_codelet_satisfy(node_" + label + "_guid, node_" + label + "_db_guid, 0);"
	print "\trmd_db_release(node_" + label + "_db_guid);"
	print "\trmd_db_release(global_db_guid);"
	print ""
	print "\trmd_guid_t ret_guid;"
	print "\tret_guid.data = 0;"
	print "\treturn ret_guid;"
	print "}"

