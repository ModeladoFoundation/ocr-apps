###############################################################################
# g.py
#
# This file contains a set of globals and constants that are used throughout
# the system.
###############################################################################

#------------------------------------------------------------------------------
# Command Line Parameters
#------------------------------------------------------------------------------

# OUTPUT sets the type of code to output. Set to sequential C by default.
OUTPUT = "C"

# DEBUG_LEVEL sets the amount of debug information to print. 0 will print
# almost nothing while 9 will print everything available.
DEBUG_LEVEL = 0

# DEBUG sets whether or not to output debugging statements into the generated
# code.
FILE_DEBUG_LEVEL = 0

# NUM_THREADS sets the number of threads the runtime system should start
NUM_THREADS = "1"

# the basename of the file
FILE = ""

# the type of the file we are reading. E.g., a 'chpl' file generated from the
# Chapel front end.
FILE_TYPE = "default"

# this variable sets whether or not there is a pil_main() in the code. If so,
# we need to process the file differently. We assume not, that the code is
# emitted by a compiler.
PIL_MAIN = False

# the memory model assumed to output code for. Choices are "shared" and "dist".
MEM = "shared"


# flag to denote if we are to use the R-Stream compiler
RSTREAM = False


#------------------------------------------------------------------------------
# Constants
#------------------------------------------------------------------------------

# Valid Chapel FILE_TYPEs
CHPL = ['chpl']

# Valid default FILE_TYPES
DEFAULT = ['default']

# the regular expression that specifies a node call
MAP_RE = "node\s*\(\s*(\d+)\s*,\s*(\w+)\s*,\s*\[(\w+):(\w+):(\w+)\]\s*,\s*(\w+)\s*,\s*\[(.*)\]\s*,\s*((\w+)\(\s*(.*)\s*\)\s*\))"
#        "node    (   label   ,   index   ,     [lower:step:upper]    ,    key    ,   [targets]  ,    func_name(in_args) )
NW_RECV_RE = "(\s*)pil_recv\s*\(\s*(\w+)\s*,\s*([&]{0,1})\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*\)\s*;.*"

# the index variable we shall use if the user specifies a NULL index variable
INDEX = "__pil_unique_index_variable_name"
INDEX_TYPE = "int32_t"

# the name of the loop index varables to use. They must be unique enough to not
# clash with user defined varaiables.
LOWER = "_lower"
STEP = "_step"
UPPER = "_upper"

stacksize = str(64 * 1024 * 1024)


#------------------------------------------------------------------------------
# Globals
#------------------------------------------------------------------------------

# globals to keep track of the main node in the file.
first_node = True
main_node_label = ""
main_node_input = []
main_node_func_name = ""

# A dictionary of dictionaries of all of the arguments for functions indexed by
# function name, input/output args, and argument name.
#     args[func_name]['list'] = ['foo', 'bar', 'etc']
#     args[func_name]['dict'] = {'foo': {'type': 'int', 'modifier': '*'}, 'bar': {...}}
args = {}
variables = {INDEX: INDEX_TYPE} # the types of each variable indexed by name
initial_values = {} # the initial values of each variable indexed by name
arrays = {} # the sizes of constant arrays

# TODO: this is saving info from each node. I'm starting to think I might as
# well save it all.
functions = {} # the function each node calls
intervals = {} # the interval of each node
target_variables = {} # the name of the target variable for each function
targets = {} # the list of targets of each node
indices = {} # the index variable name for each node

nodes = {} # the _pil_nodes that are called
nodelets = {} # the _pil_nodelets that are called
nw_calls = {} # the nw calls inside of nodes and nodelets
context_variables = {} # variables declared with _pil_context


# the name of the header file to output to
header_file_name = ""
h_file = None # the file handle for the header file

func_lines = {} # the body functions to be output later
pil_main_lines = [] # the contents of the pil_main function to be output later

nodes_entered = [] # the nodes that are entered with a pil_enter() call
