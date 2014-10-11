###############################################################################
# pil2c
###############################################################################

import run
#from run import file_debug, debug
import g
import sys


#------------------------------------------------------------------------------
# C specific process functions for use by mc
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


def c_process_function_prototype(label, func_name):
	"""Output a function prototype for each node function"""
	sys.stdout.write("//--------------------------------------\n")
	sys.stdout.write("// function prototypes for node_" + label + "\n")
	sys.stdout.write("//--------------------------------------\n")
	sys.stdout.write("\n")

	g.h_file.write("void node_" + label + "();\n")
	sys.stdout.write("void node_" + label + "();\n")


def print_loop_less_than_zero(tabs, m):

	print tabs + "for(" + m['index'] + " = " + m['upper'] + "; " + m['index'] + " >= " + m['lower'] + "; " + m['index'] + " += " + m['step'] + ")"


def print_loop_greater_than_zero(tabs, m):

	print tabs + "for(" + m['index'] + " = " + m['lower'] + "; " + m['index'] + " <= " + m['upper'] + "; " + m['index'] + " += " + m['step'] + ")"


def print_loop_body(tabs, m):
	print tabs + "{"
	print tabs + "\t// we create a new local data_array and set its pointer to be an offset of the global data_array as a way to split the data_array."
	print tabs + "\t// this works because we are on shared memory. the result will be 'merged' for us automatically."
	print tabs + "\tgpp_t _pil_index_array;"
	print tabs + "\t_pil_index_array.guid = index_array.guid; // global index_array"
	print tabs + "\t_pil_index_array.ptr = pil_mem(index_array.guid); // global index_array"
	print tabs + "\tint *_pil_gia = (int *) index_array.ptr; // global ia pointer"
	print tabs + "\tgpp_t *_pil_gda = (gpp_t *) data_array.ptr; // global da pointer"
	print tabs + "\t{"
	print tabs + "\t\tgpp_t index_array; // local index_array"
	print tabs + "\t\tgpp_t data_array; // local data_array"
	print tabs + "\t\tif (_pil_index_array.ptr) { // only split if index_array.ptr is not NULL"
	print tabs + "\t\t\tpil_alloc(&index_array, 2*sizeof(int));"
	print tabs + "\t\t\tint *_pil_lia = (int *) index_array.ptr; // local ia pointer"
	print tabs + "\t\t\tint _pil_iter = (" + m['index'] + " - " + m['lower'] + ") / " + m['step'] + ";"
	print tabs + "\t\t\tint _pil_num = _pil_gia[_pil_iter+1] - _pil_gia[_pil_iter];"
	print tabs + "\t\t\t_pil_lia[0] = 0;"
	print tabs + "\t\t\t_pil_lia[1] = _pil_num;"
	print ""
	print tabs + "\t\t\tsize_t _pil_data_size = _pil_num*sizeof(gpp_t);"
	print tabs + "\t\t\tpil_alloc(&data_array, _pil_data_size);"
	print tabs + "\t\t\tgpp_t *_pil_lda = (gpp_t *) data_array.ptr; // local da pointer"
	print tabs + "\t\t\tmemcpy(_pil_lda, &_pil_gda[_pil_gia[_pil_iter]], _pil_data_size);"
	print ""
	print tabs + "\t\t\tint _pil_index;"
	print tabs + "\t\t\tfor (_pil_index = 0; _pil_index < _pil_num; _pil_index++) {"
	print tabs + "\t\t\t\t_pil_lda[_pil_index].ptr = pil_mem(_pil_lda[_pil_index].guid);"
	print tabs + "\t\t\t}"
	print tabs + "\t\t}"
	print tabs + "\t\t" + m['func_name'] + "(" + m['in_args'] + "); // function call with the new local data_array set appropriately"
	print tabs + "\t}"
	print tabs + "}"


def print_omp_pragma(tabs, m):

	print "#ifdef OMP"
	print tabs + "#pragma omp parallel for private(" + m['index'] + ")"
	print "#endif"


# TODO: change this function so it doesn't take 'm', but the pieces of 'm' that it needs
def c_process_node_procedure(m):
	"""Output the body of a node function"""

	print "void node_" + m['label'] + "()"
	print "{"
	run.file_debug(1, "\tfprintf(stdout, \"node_" + m['label'] + "\\n\");")
	run.file_debug(1, "\tfprintf(stdout, \"lower: %d  upper: %d  step: %d\\n\", " + m['lower'] + ", " + m['upper'] + ", " + m['step'] + ");")
	if "P" in g.variables:
		print "#ifdef OMP"
		print "\tomp_set_num_threads(P);"
		print "#endif"
		print ""

	#print "\tfor(" + m['index'] + " = " + m['lower'] + "; " + m['index'] + " != (" + m['upper'] + "+" + m['step'] + "); " + m['index'] + " += " + m['step'] + ")"
	step_is_int = True
	try:
		step = int(m['step'])
	except ValueError:
		step_is_int = False # step is a variable

	tabs = "\t"
	if step_is_int:
		print_omp_pragma(tabs, m)
		if step < 0:
			print_loop_less_than_zero(tabs, m)
		else:
			print_loop_greater_than_zero(tabs, m)
		print_loop_body(tabs, m)
	else:
		tabs += "\t"
		print "\tif(" + m['step'] + " < 0)"
		print "\t{"
		print_omp_pragma(tabs, m)
		print_loop_less_than_zero(tabs, m)
		print_loop_body(tabs, m)
		print "\t}"
		print "\telse"
		print "\t{"
		print_omp_pragma(tabs, m)
		print_loop_greater_than_zero(tabs, m)
		print_loop_body(tabs, m)
		print "\t}"

	print "\n\t// TODO: need to merge index_array and data_array"

	print ""
	print "\tswitch (" + m['cond'] + ")"
	print "\t{"
	for target in g.targets[m['label']]:
		print "\t\tcase " + target + ":"
		print "\t\t{"
		# TODO: This assumes that a target of 0 is exit
		if target == "0":
			print "\t\t\treturn;"
		else:
			print "\t\t\tnode_" + target + "();"
		print "\t\t\tbreak;"
		print "\t\t}"
	print "\t\tdefault:"
	print "\t\t{"
	print "\t\t\tfprintf(stdout, \"ERROR: Unknown case '%d' in node_" + m['label'] + "\\n\", " + m['cond'] + ");"
	if g.FILE_TYPE in g.CHPL:
		print "\t\t\tchpl_exit_all(EXIT_FAILURE);"
	else:
		print "\t\t\texit(EXIT_FAILURE);"
	print "\t\t\tbreak;"
	print "\t\t}"
	print "\t}"
	print "}"
	print ""


def process_header():
	"""Outputs the header for the file. E.g., includes libraries."""

	run.process_header()

	print "#ifdef OMP"
	print "#include <omp.h>"
	print "#endif"
	print ""


def process_variables():
	"""Create the global variables"""

	run.debug(0,"process_variables")

	print "// Global variables"
	for var_name in g.variables:
		run.debug(4,var_name + "\n")
		declaration = ""
		if var_name in g.arrays:
			#declaration += g.variables[var_name] + " (*" + var_name + ")[" + g.arrays[var_name] + "]"
			declaration += g.variables[var_name] + " " + var_name + "[" + g.arrays[var_name][0] + "][" + g.arrays[var_name][1] + "]"
		else:
			declaration += g.variables[var_name] + " " + var_name
			if var_name in g.initial_values:
				declaration += " = " + g.initial_values[var_name]
		declaration +=  ";"
		print declaration
	print ""


def handle_nodes(nodes):
	"""Output the C code for each node in nodes."""

	g.h_file.write("// Map function prototypes\n")
	sys.stdout.write("// Map function prototypes\n")
	for m in nodes:
		c_process_function_prototype(m['label'], m['func_name'])

	for m in nodes:
		c_process_node_procedure(m)


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
			print "\t\t\t" + arg_name + " = *" + arg_name + "_ptr;"
		else:
			print "\t\t\t" + arg_name + " = va_arg(argptr, " + g.variables[arg_name] + ");"



	#print "#endif"
	print "\t\t\tbreak;"


def process_pil_enter(nodes, node):
	"""Output the appropriate code to enter a node"""

	lower = 0
	step = 0
	upper = 0

	for n in nodes:
		if n['label'] == node['label']:
			lower = n['lower']
			step = n['step']
			upper = n['upper']

	print "#ifdef PIL2C"
	print "\t\t\tif (index_array.guid == NULL_GUID) {"
	print "\t\t\t\tuint64_t _pil_num_iter = 0;"
	print "\t\t\t\t_pil_num_iter = (" + upper + " - " + lower + ") / " + step + " + 1;"
	print "\t\t\t\tpil_alloc(&index_array, (_pil_num_iter+1) * sizeof(int));"
	print "\t\t\t\tint *_pil_ia = (int *) index_array.ptr;"
	print "\t\t\t\tint _pil_index;"
	print "\t\t\t\tfor (_pil_index = 0; _pil_index <= _pil_num_iter; _pil_index++) {"
	print "\t\t\t\t\t_pil_ia[_pil_index] = 0;"
	print "\t\t\t\t}"
	print "\t\t\t}"
	print "\t\t\tnode_" + node['label'] + "();"
	args = g.args[g.functions[node['label']]]
	for arg_name in args['list']:
		if arg_name == g.target_variables[node['label']]:
			continue
		#elif arg_name == 'index_array' or arg_name == 'data_array':
		#	continue
		if g.args[g.functions[node['label']]]['dict'][arg_name]['passed_by_reference']:
			print "\t\t\t*" + arg_name + "_ptr = " + arg_name + ";"
	print "\t\t\tbreak;"
	print "#endif"


def process_main():
	"""Output the main function for the file."""

	if g.FILE_TYPE in g.CHPL:
		print "void chpl_main()"
	else:
		print "int main(int ac, char **av)"
	print "{"
	run.file_debug(1, '\tfprintf(stdout, "Using C backend of PIL\\n");')
#	run.set_stack_size()
	if g.FILE_TYPE in g.CHPL:
		pass
	else:
		#print "\tpil_init(ac, av);"
		for var_name in g.variables:
			if var_name == "argc":
				print "\targc = ac;"
			elif var_name == "argv":
				print "\targv = av;"
	print "\tindex_array.guid = NULL_GUID;"
	print "\tindex_array.ptr = NULL;"
	print "\tdata_array.guid = NULL_GUID;"
	print "\tdata_array.ptr = NULL;"
	if g.PIL_MAIN:
		print "\tpil_main(ac, av);"
	else:
		print "\tnode_" + g.main_node_label + "();"
	if g.FILE_TYPE in g.CHPL:
		print "\treturn;"
	else:
		print "\treturn EXIT_SUCCESS;"
	print "}"
