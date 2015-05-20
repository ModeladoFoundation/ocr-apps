import sys

#def add_preds(old_list, new_list):
#	"""flatten old list into new_list"""
#
#	new_list = [item for sublist in old_list for item in sublist]
#	#reduce(lambda x,y: x+y,old_list)


def graph_bfs(graph, label):
	"""label nodes of graph in BFS order starting with node label"""

	#sys.stderr.write("graph_bfs(" + label + ")\n")

	changed = False
	node = graph[label]
	bfs_depth = node.get_bfs_depth() + 1
	for s in node.get_succ():
		succ = graph[s]
		if succ.get_bfs_depth() > bfs_depth:
			changed = True
			succ.set_bfs_depth(bfs_depth)
			#graph_bfs(graph, s)

	if changed:
		for s in node.get_succ():
			graph_bfs(graph, s)


def match_forks_to_joins_dfs_helper(graph, label, fork_stack):
	"""traverse the graph in DFS order, matching merges to joins"""

	#print label, fork_stack
	#graph[label].output()

	if graph[label].has_fork():
		#print "node", label, "has fork"
		fork_stack.append(label)

	if graph[label].has_merge():
		#print "node", label, "has merge"
		graph[label].set_matching_fork(fork_stack[-1])
		fork_stack = fork_stack[:-1]

	# DFS traversal
	for s in graph[label].get_succ():
		if not graph[s].get_dfs_has_been_visited():
			graph[s].set_dfs_has_been_visited(True)
			match_forks_to_joins_dfs_helper(graph, s, fork_stack)

	if graph[label].has_fork():
		#print "trying to match fork ", label, " with merge"
		for node in graph:
			if graph[node].get_matching_fork() == label:
				graph[label].set_matching_merge(node)
				#print "node", label, "has a matching merge at", node


def match_forks_to_joins(graph):
	"""Traverse the graph and mark fork nodes to their joins, and join nodes to their forks"""

	root = "0"
	fork_stack = []
	#print "DFS traversal:"
	#print root, fork_stack
	for s in graph[root].get_succ():
		graph[s].set_dfs_has_been_visited(True)
		match_forks_to_joins_dfs_helper(graph, s, fork_stack)

	#for node in graph:
	#	graph[node].output()


def compute_dominance(graph):
	"""find the dominators in a graph"""

	# dominator of the start node is the start itself
	#Dom(n0) = {n0}
	graph["0"].add_dominator("0")


	# for all other nodes, set all nodes as the dominators
	#for each n in N - {n0}
	#	Dom(n) = N;
	nodes = graph.keys()
	for l in graph:
		if l == "0":
			continue
		for n in nodes:
			graph[l].add_dominator(n)
	#for l in graph:
	#	graph[l].output()
	#sys.stderr.write("\n\n")

	# iteratively eliminate nodes that are not dominators
	#while changes in any Dom(n)
	#	for each n in N - {n0}:
	#		Dom(n) = {n} union with intersection over Dom(p) for all p in pred(n)
	dom_0 = graph["0"].get_dominators()
	changed = True
	while changed:
		changed = False

		for n in nodes:

#			sys.stderr.write("\nprocessing node: " + n + "\n")
#
#			sys.stderr.write("\t0:      ")
#			graph["0"].print_dominators()
#			sys.stderr.write("\n")
#
#			sys.stderr.write("\t" + n + " original: ")
#			graph[n].print_dominators()
#			sys.stderr.write("\n")

			if n == "0":
				continue

			# intersection of predecessors dominators
			preds = graph[n].get_pred()
			#new_set = graph[preds["0"]].get_dominators()
			new_set = []
			for d in graph[preds[0]].get_dominators():
				new_set.append(d)
			#add_preds(graph[preds["0"]].get_dominators(), new_set)
			for p in preds:
#				sys.stderr.write("\t\tp: " + p + "\n")
#				sys.stderr.write("\t\t\t")
#				graph[p].print_dominators()
#				sys.stderr.write("\n")
				for d in new_set:
					if d not in graph[p].get_dominators():
						new_set.remove(d)

#			sys.stderr.write("\t0:      ")
#			graph["0"].print_dominators()
#			sys.stderr.write("\n")

			# union with n
			if n not in new_set:
				new_set.append(n)

#			sys.stderr.write("\t0:      ")
#			graph["0"].print_dominators()
#			sys.stderr.write("\n")

			# continue until no changes
			if set(new_set) != set(graph[n].get_dominators()):
#				sys.stderr.write("\t" + n + " changed ")
#				for i in new_set:
#					sys.stderr.write(i + " ")
#				sys.stderr.write("\n")
				changed = True
				graph[n].set_dominators(new_set)

#				sys.stderr.write("\t" + n + " new:      ")
#				graph[n].print_dominators()
#				sys.stderr.write("\n")

#			sys.stderr.write("\t0:      ")
#			graph["0"].print_dominators()
#			sys.stderr.write("\n")
#		sys.stderr.write("-----------------------\n")

	# label nodes in bfs order
	for l in graph:
		if l == "0":
			graph[l].set_bfs_depth(0)
		else:
			graph[l].set_bfs_depth(sys.maxint)

	# label the graph in BFS order
	graph_bfs(graph, "0")

	for n in graph:
		graph[n].set_idom("0")

	for n in graph:
		for d in graph[n].get_dominators():
			idom = graph[n].get_idom()
			if d != n and graph[d].get_bfs_depth() > graph[idom].get_bfs_depth():
				graph[n].set_idom(d)

	#for n in graph:
	#	if graph[n].has_merge():
	#		idom = graph[n].get_idom()
	#		if not graph[idom].has_fork():
	#			sys.stderr.write("ERROR: node " + n + " has merge point, but its idom " + idom + " is not a fork point.\n")


#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
class GraphNode:

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
	def __init__(self, label, pred, succ):

		self.label = label
		self.real_pred = pred
		self.pred = self.flatten(pred)
		self.real_succ = succ
		self.succ = self.flatten(succ)
		self.dominators = []
		self.idom = "-1"
		self.bfs_depth = sys.maxint
		self.matching_fork = "0"
		self.matching_merge = "0"
		self.dfs_has_been_visited = False

		self.merge = False
		for i in pred:
			if type(i) is list:
				self.merge = True

		self.fork = False
		for i in succ:
			if type(i) is list:
				self.fork = True


	def flatten(self, l):
		"""flatten list l"""

		#return [item for sublist in l for item in sublist]

		new_list = []

		for i in l:
			if type(i) is list:
				for j in i:
					new_list.append(j)
			else:
				new_list.append(i)
		return new_list


	def print_list(self, l):
		"""pretty print a list"""

		sys.stderr.write("[")
		first = True
		for i in l:
			if first:
				first = False
			else:
				sys.stderr.write(", ")
			sys.stderr.write(i)
		sys.stderr.write("]")


	def print_succ(self):
		"""otput the successors of this node"""

		self.print_list(self.succ)


	def print_pred(self):
		"""otput the predecessors of this node"""

		self.print_list(self.pred)


	def print_dominators(self):
		"""otput the dominators of this node"""

		self.print_list(self.dominators)


	def output(self):
		"""output the entire node contents"""

		sys.stderr.write(self.label + ": ")

		self.print_pred()
		sys.stderr.write(" ")
		self.print_succ()

		sys.stderr.write(" dom: ")
		self.print_dominators()

		sys.stderr.write(" idom: ")
		sys.stderr.write(self.idom)

		sys.stderr.write(" bfs_depth: ")
		sys.stderr.write(str(self.bfs_depth))

		sys.stderr.write(" has_fork: ")
		sys.stderr.write(str(self.fork))
		sys.stderr.write(" has_merge: ")
		sys.stderr.write(str(self.merge))

		sys.stderr.write(" matching_fork: ")
		sys.stderr.write(self.matching_fork)
		sys.stderr.write(" matching_merge: ")
		sys.stderr.write(self.matching_merge)

		sys.stderr.write("\n")


	def get_pred(self):
		"""return the list of successors for the node"""

		return self.pred


	def get_succ(self):
		"""return the list of successors for the node"""

		return self.succ


	def add_pred(self, label):
		"""add a label to the list of predecessors"""

		self.pred.append(label)


	def add_succ(self, label):
		"""add a label to the list of successors"""

		self.succ.append(label)


	def get_dominators(self):
		"""return the dominator list for this node"""

		return self.dominators


	def set_dominators(self, dom):
		"""assign the list dom to the list of dominators"""

		self.dominators = dom


	def add_dominator(self, label):
		"""add a dominator to the list of dominators for the node"""

		if label not in self.dominators:
			self.dominators.append(label)


	def get_idom(self):
		"""return this nodes strict dominator"""

		return self.idom


	def set_idom(self, dom):
		"""set this nodes strict dominator"""

		self.idom = dom


	def set_bfs_depth(self, depth):
		"""set the BFS depth for this node to depth"""

		self.bfs_depth = depth


	def get_bfs_depth(self):
		"""return the BFS depth of this node"""

		return self.bfs_depth


	def has_merge(self):
		"""return true if this node is a merge point"""

		return self.merge


	def has_fork(self):
		"""return true if this node is a fork point"""

		return self.fork

	def set_dfs_has_been_visited(self, value):
		"""set the value for dfs_has_been_visited"""

		self.dfs_has_been_visited = value

	def get_dfs_has_been_visited(self):
		"""return the value for dfs_has_been_visited"""

		return self.dfs_has_been_visited

	def set_matching_fork(self, label):

		self.matching_fork = label

	def get_matching_fork(self):

		return self.matching_fork;

	def set_matching_merge(self, label):

		self.matching_merge = label

	def get_matching_merge(self):

		return self.matching_merge
