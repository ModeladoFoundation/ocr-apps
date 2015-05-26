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

    s = 'void _HTA_map_' + op_name[:-2] + '_exec(uint32_t *target_id, gpp_t index_array, gpp_t data_array, int pid, int level, ' + op_type + ' ' + op_name + ')'
    return s

def gen_node_decl(node_id, nh, ns = 0):
    op_name = gen_map_op_name(nh, ns)

    s = "node(" + str(node_id) + ", pid, idx, [0:1:0], target_id, [0], [0], _HTA_map_" + op_name[:-2] + "_exec(&target_id, index_array, data_array, pid, level, " + op_name + "))"
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
    hlist = gen_list_with_name("h", nh);
    slist = gen_list_with_name("s", ns);
    tlist = gen_list_with_name("t", nh);

    s = ""
    s += gen_code_line(gen_map_header(nh, ns), 0)
    s += gen_code_line("{", 0)

    s += gen_code_line("ASSERT(" + " && ".join(hlist + slist) + ");")
    s += gen_code_line("ASSERT(level <= HTA_LEAF_LEVEL(h1) && \"Mapped level is limited to less than leaf level\");")

    # when map to root level, execute the map function sequentially
    s += gen_code_line("if(level == 0) { // map at one tile only")
    s += gen_code_line(gen_map_op_name(nh, ns) + "(" + ", ".join(hlist + slist) + ");", 2)
    s += gen_code_line("return;", 2)
    s += gen_code_line("}")


    # gpps calculation
    s += gen_code_line("int total_num_gpps = 0;")
    for i in range(nh):
        s += gen_code_line("for(int i = 0; i < " + hlist[i] + "->num_tiles; i++) {")
        s += gen_code_line("HTA *t = "+hlist[i]+"->tiles[i];", 2)
        s += gen_code_line("if(t->pid == t->home) {", 2)
        s += gen_code_line("    total_num_gpps += _count_gpps(t);", 2)
        s += gen_code_line("}", 2)
        s += gen_code_line("}")

    s += gen_code_line("if(total_num_gpps == 0) return; // not owned tiles to compute")

    assert ns < 2  # ns > 2 not supported FIXME
    if ns > 0:
        s += gen_code_line("int sz = HTA_get_scalar_size(h1);")
        s += gen_code_line("gpp_t s_gpp;")
        s += gen_code_line("pil_alloc(&s_gpp, sz);")
        s += gen_code_line("memcpy(s_gpp.ptr, s1, sz);")

    # for scalar values FIXME
    if ns > 0:
        s += gen_code_line("total_num_gpps += 1;");

    # GPP Array
    s += gen_code_line("ASSERT(total_num_gpps < 1024 && \"Hard limit on the number of gpps\");")
    s += gen_code_line("// allocate data array and index array using pil_alloc")
    s += gen_code_line("gpp_t index_array, data_array;")
    s += gen_code_line("pil_init(&index_array, &data_array); ")
    s += gen_code_line("pil_alloc(&index_array, (2)*sizeof(int)); ")
    s += gen_code_line("pil_alloc(&data_array, total_num_gpps*sizeof(gpp_t)); ")
    s += gen_code_line("int processed = 0;")
    s += gen_code_line("gpp_t *ptr_darray = (gpp_t *) data_array.ptr;")
    s += gen_code_line("int *ptr_iarray = (int *) index_array.ptr;")
    s += gen_code_line("ptr_iarray[0] = 0;")
    # for scalar values FIXME
    if ns > 0:
        s += gen_code_line("// pack scalar gpp at the beginning of the data array")
        s += gen_code_line("ptr_darray[0] = s_gpp;");
        s += gen_code_line("processed += 1;");
    # HTA tiles
    s += gen_code_line("// pack HTA tiles")
    s += gen_code_line("for(int i = 0; i < h1->num_tiles; i++) {")
    for i in range(nh):
        s += gen_code_line("HTA *" + tlist[i] + " = " + hlist[i] + "->tiles[i];", 2)
    s += gen_code_line("if(t1->pid == t1->home) {", 2)
    for i in range(nh):
        if i >= 1:
            s += gen_code_line("ASSERT("+ tlist[i] +"->pid == "+ tlist[i] +"->home);", 3)
        s += gen_code_line("processed += _pack_HTA(ptr_darray + processed, "+ tlist[i]+");", 3)
    s += gen_code_line("}", 2)  #if
    s += gen_code_line("}")     #for

    s += gen_code_line("ASSERT(processed == total_num_gpps);")
    s += gen_code_line("ptr_iarray[1] = processed;")
    s += gen_code_line("pil_enter(" + str(node_id) + ", h1->pid, 5, index_array, data_array, h1->pid, level-1, " + gen_map_op_name(nh, ns) + ");")

    # for scalar values FIXME
    if ns > 0:
        s += gen_code_line("pil_free(s_gpp);")

    s += gen_code_line("GPP_ARRAY_FINALIZE")
    s += gen_code_line("}", 0)
    return s

def gen_map_exec_function(nh, ns = 0):
    hlist = gen_list_with_name("h", nh);
    slist = gen_list_with_name("s", ns);
    tlist = gen_list_with_name("t", nh);

    s = ""
    s += gen_code_line(gen_map_exec_header(nh, ns), 0)
    s += gen_code_line("{", 0)

    s += gen_code_line("HTA " + ", ".join(["*" + x for x in hlist]) + ";")
    s += gen_code_line("gpp_t *da = (gpp_t *)data_array.ptr;")
    s += gen_code_line("ASSERT(da);")
    # for scalar values FIXME
    if ns > 0:
        s += gen_code_line("void *s1 = da[0].ptr;")
    if ns > 0:
        s += gen_code_line("int unpacked = 1;")
    else:
        s += gen_code_line("int unpacked = 0;")
    s += gen_code_line("// unpack before sequentially execute the opeartor")
    s += gen_code_line("int *ptr_iarray = (int*) index_array.ptr;")
    s += gen_code_line("int num_gpps = ptr_iarray[1] - ptr_iarray[0];")
    # unpacking
    s += gen_code_line("while(unpacked != num_gpps) { // has something to do")
    for i in range(nh):
        s += gen_code_line("unpacked += _unpack_HTA(da + unpacked, &" + hlist[i] + ");", 2)

    # leaf tile condition
    s += gen_code_line("if(h1->height == 1) {", 2)
    s += gen_code_line("    if(h1->pid == h1->home) {", 2)
    for i in range(nh):
        if i >= 1:
            s += gen_code_line("ASSERT("+ hlist[i] +"->pid == "+ hlist[i] +"->home);", 4)
    s += gen_code_line(gen_map_op_name(nh, ns) + "(" + ", ".join(hlist + slist) + ");", 4)
    s += gen_code_line("    }", 2)
    s += gen_code_line("}", 2)

    # non-leaf tile condition (a subtree is mapped)
    s += gen_code_line("else { // height > 1", 2)
    # tile collection
    countvars = gen_list_with_name("count", nh)
    havars = gen_list_with_name("ha", nh)
    s += gen_code_line("int num_tiles = 1;", 3)
    # compute # of tiles at the mapped level
    s += gen_code_line("HTA* t = h1;", 3)
    s += gen_code_line("int leveldown = level;", 3)
    s += gen_code_line("while(leveldown != 0) {",3)
    s += gen_code_line("    num_tiles *= Tuple_product(&t->tiling);", 3)
    s += gen_code_line("    t = t->tiles[0];", 3)
    s += gen_code_line("    leveldown--;", 3)
    s += gen_code_line("}", 3)

    s += gen_code_line("int " + ", ".join([(x + "=0") for x in countvars]) + ";", 3)
    s += gen_code_line("HTA " + ", ".join([("*" + x +"[num_tiles]") for x in havars])  + ";", 3)
    for i in range(nh):
        s += gen_code_line("HTA_collect_tiles(level, " + hlist[i] + ", " + havars[i] + ", &" + countvars[i] + ");", 3)
    s += gen_code_line("ASSERT(" + " && ".join([x + " == num_tiles" for x in countvars]) + ");", 3)
    s += gen_code_line("for(int i = 0; i < num_tiles; i++) {", 3)
    s += gen_code_line(gen_map_op_name(nh, ns) + "(" + ", ".join([x + "[i]" for x in havars] + slist) + ");", 4)
    s += gen_code_line("}", 3) #for
    s += gen_code_line("}", 2) #if
    s += gen_code_line("}") #while
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

