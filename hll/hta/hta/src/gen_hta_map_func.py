# This python script generates HTA_map functions that take different 
# number of input arguments

def gen_list_with_name(name, n, start = 1, postfix=''):
    l = []
    for i in range(n):
        l.append(name + str(i+start) + postfix)
    return l

def gen_map_op_type(nh, ns):
    assert nh > 0
    s = 'H'+str(nh)
    if ns > 0:
        s += 'S'+str(ns)
    s += 'Op'
    return s

def gen_map_op_name(nh, ns):
    assert nh > 0
    s = 'h'+str(nh)
    if ns > 0:
        s += 's'+str(ns)
    s += 'op'
    return s

def gen_hta_args_list(nh):
    return gen_list_with_name("HTA *h", nh);

def gen_scalar_args_list(ns):
    return gen_list_with_name("void *s", ns);

# =======================================================
# Generate HTA_map function headers
# =======================================================

def gen_map_header(nh, ns = 0):
    op_type = gen_map_op_type(nh, ns)
    op_name = gen_map_op_name(nh, ns)
    hlist = gen_hta_args_list(nh)
    slist = gen_scalar_args_list(ns)
 
    s = 'void HTA_map_' + op_name[:-2] + '(int level, ' + op_type + ' ' + op_name + ', ' + ", ".join(hlist + slist) + ')'
    return s
    
def gen_map_exec_header(nh, ns = 0):
    op_type = gen_map_op_type(nh, ns)
    op_name = gen_map_op_name(nh, ns)
    hlist = gen_hta_args_list(nh)
    slist = gen_scalar_args_list(ns)
 
    s = 'void _HTA_map_' + op_name[:-2] + '_exec(uint32_t *target_id, gpp_t index_array, gpp_t data_array, int bound, int idx, int level, ' + op_type + ' ' + op_name + ')'
    return s

def gen_node_decl(node_id, nh, ns = 0):
    op_name = gen_map_op_name(nh, ns)
 
    s = "node(" + str(node_id) + ", idx, [0:1:bound], target_id, [0], _HTA_map_" + op_name[:-2] + "_exec(&target_id, index_array, data_array, bound, idx, level, " + op_name + "))"
    return s
# =======================================================
# Generate HTA_map function bodies
# =======================================================

def gen_code_line(newstr, indent = 1, newline = True):
    s = ("    " * indent) + newstr
    if newline:
        s += '\n'
    return s

def gen_map_function(node_id, nh, ns = 0):
    s = ""
    s += gen_code_line(gen_map_header(nh, ns), 0)
    s += gen_code_line("{", 0)
    hlist = gen_list_with_name("h", nh);
    slist = gen_list_with_name("s", ns);
    s += gen_code_line("int bound;")
    s += gen_code_line("int idx = 0;")
    s += gen_code_line("gpp_t index_array, data_array;")
    s += gen_code_line("pil_init(&index_array, &data_array);")
    s += gen_code_line("ASSERT(" + " && ".join(hlist + slist) + ");")
    s += gen_code_line("ASSERT(level <= HTA_LEAF_LEVEL(h1) && \"Mapped level is limited to less than leaf level\");")

    # when map to root level, execute the map function sequentially
    s += gen_code_line("if(level == 0) { // map at one tile only")
    s += gen_code_line(gen_map_op_name(nh, ns) + "(" + ", ".join(hlist + slist) + ");", 2)
    s += gen_code_line("return;", 2)
    s += gen_code_line("}")
    
    # get first level tiles and map to them in parallel
    s += gen_code_line("bound = Tuple_count_elements(h1->tiling, 1);")
    s += gen_code_line("int sz = HTA_get_scalar_size(h1);")

    assert ns < 2  # ns > 2 not supported FIXME
    if ns > 0:
        s += gen_code_line("gpp_t s_darray[bound];")
        s += gen_code_line("for(int i = 0; i < bound; i++) {")
        s += gen_code_line("    pil_alloc(&s_darray[i], sz);")
        s += gen_code_line("    memcpy(s_darray[i].ptr, s1, sz);")
        s += gen_code_line("}")
    
    # tile collection
    # countvars = gen_list_with_name("count", nh)
    # havars = gen_list_with_name("ha", nh)
    # s += gen_code_line("int " + ", ".join([(x + "=0") for x in countvars]) + ";")
    # s += gen_code_line("HTA " + ", ".join([("*" + x +"[bound]") for x in havars])  + ";")
    # for i in range(nh):
    #     s += gen_code_line("HTA_collect_tiles(level, " + hlist[i] + ", " + havars[i] + ", &" + countvars[i] + ");")
    # s += gen_code_line("ASSERT(" + " && ".join([x + " == bound" for x in countvars]) + ");")

    # gpps calculation
    s += gen_code_line("int total_num_gpps = 0;")
    for i in range(nh):
        s += gen_code_line("total_num_gpps += get_num_gpps(" + hlist[i] + ", " + hlist[i] + "->tiles, 1, bound);")
        #s += gen_code_line("total_num_gpps += get_num_gpps(" + hlist[i] + ", " + havars[i] + ", level, bound);")

    # for scalar values FIXME
    if ns > 0:
        s += gen_code_line("total_num_gpps += bound;");
    s += gen_code_line("pil_alloc(&index_array, (bound+1)*sizeof(int));")
    s += gen_code_line("pil_alloc(&data_array, total_num_gpps*sizeof(gpp_t));")
    s += gen_code_line("int processed = 0;")
    s += gen_code_line("gpp_t *ptr_darray = (gpp_t *) data_array.ptr;")
    s += gen_code_line("int *ptr_iarray = (int *) index_array.ptr;")
    s += gen_code_line("for(int i = 0; i < bound; i++) {")
    s += gen_code_line("ptr_iarray[i] = processed;", 2)
    for i in range(nh):
        s += gen_code_line("processed += _pack_HTA(ptr_darray + processed, " + hlist[i] + "->tiles[i]);", 2);
        #s += gen_code_line("processed += _pack_HTA(ptr_darray + processed, " + havars[i] + "[i]);", 2);
    # for scalar values FIXME
    if ns > 0:
        s += gen_code_line("ptr_darray[processed] = s_darray[i];", 2)
        s += gen_code_line("processed++;", 2)
    s += gen_code_line("}")

    s += gen_code_line("ASSERT(processed == total_num_gpps);")
    s += gen_code_line("ptr_iarray[bound] = processed;")
    s += gen_code_line("pil_enter(" + str(node_id) + ", 6, index_array, data_array, bound-1, idx, level-1, " + gen_map_op_name(nh, ns) + ");")
    #s += gen_code_line("pil_enter(" + str(node_id) + ", 5, index_array, data_array, bound-1, idx, " + gen_map_op_name(nh, ns) + ");")
    # for scalar values FIXME
    if ns > 0:
        s += gen_code_line("for(int i = 0; i < bound; i++) {")
        s += gen_code_line("pil_free(s_darray[i]);", 2)
        s += gen_code_line("}")
    s += gen_code_line("pil_free(data_array);")
    s += gen_code_line("pil_free(index_array);")
    s += gen_code_line("}", 0)
    return s

def gen_map_exec_function(nh, ns = 0):
    s = ""
    s += gen_code_line(gen_map_exec_header(nh, ns), 0)
    s += gen_code_line("{", 0)
    hlist = gen_list_with_name("h", nh);
    slist = gen_list_with_name("s", ns);
    s += gen_code_line("HTA " + ", ".join(["*" + x for x in hlist]) + ";")
    # unpacking
    s += gen_code_line("gpp_t *da = (gpp_t *)data_array.ptr;")
    s += gen_code_line("ASSERT(da);")
    s += gen_code_line("// unpack before sequentially execute the opeartor")
    s += gen_code_line("int unpacked = _unpack_HTA(da, &h1);")
    for i in range(nh)[1:]:
        s += gen_code_line("unpacked += _unpack_HTA(da + unpacked, &" + hlist[i] + ");")
    # for scalar values FIXME
    if ns > 0:
        s += gen_code_line("void *s1 = ((gpp_t*)data_array.ptr)[unpacked].ptr;")
        s += gen_code_line("unpacked++;")

    # debug check
    s += gen_code_line("#ifdef DEBUG", 0)
    s += gen_code_line("int *ptr_iarray = (int*) index_array.ptr;")
    s += gen_code_line("int num_gpps = ptr_iarray[1] - ptr_iarray[0];")
    s += gen_code_line("ASSERT(unpacked == num_gpps);")
    s += gen_code_line("#endif", 0)

    # leaf tile condition
    s += gen_code_line("if(h1->height == 1) {")
    s += gen_code_line(gen_map_op_name(nh, ns) + "(" + ", ".join(hlist + slist) + ");", 2)
    s += gen_code_line("}")
    # non-leaf tile condition (a subtree is mapped)
    s += gen_code_line("else { // height > 1")
    # tile collection
    countvars = gen_list_with_name("count", nh)
    havars = gen_list_with_name("ha", nh)
    s += gen_code_line("int num_tiles = Tuple_count_elements(h1->tiling, level);", 2)
    s += gen_code_line("int " + ", ".join([(x + "=0") for x in countvars]) + ";", 2)
    s += gen_code_line("HTA " + ", ".join([("*" + x +"[num_tiles]") for x in havars])  + ";", 2)
    for i in range(nh):
        s += gen_code_line("HTA_collect_tiles(level, " + hlist[i] + ", " + havars[i] + ", &" + countvars[i] + ");", 2)
    s += gen_code_line("ASSERT(" + " && ".join([x + " == num_tiles" for x in countvars]) + ");", 2)
    s += gen_code_line("for(int i = 0; i < num_tiles; i++) {", 2)
    s += gen_code_line(gen_map_op_name(nh, ns) + "(" + ", ".join([x + "[i]" for x in havars] + slist) + ");", 3)
    s += gen_code_line("}", 2)
    s += gen_code_line("}")
    # clean up if necessary
    s += gen_code_line("*target_id = 0;")
    s += gen_code_line("}", 0)
    return s
    

if __name__ == "__main__":

    for j in range(2):
        for i in range(1, 6):
            s = gen_map_function((j+1)*100 + i*10, i, j)
            print s,
            s = gen_map_exec_function(i, j)
            print s,

    for j in range(2):
        for i in range(1, 6):
            s = gen_node_decl((j+1)*100 + i * 10, i, j)
            print s

