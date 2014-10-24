#!/bin/bash
# Returns the resource string for Torque if $1 = "resources" and "" if $1 = "output"
processors_per_node=32
node_count=1

trap "exit 1" ERR

if [ $1 == "output" ]; then
    echo ""
    exit 0
fi

if [ $1 == "resources" ]; then
    echo "nodes=$node_count:ppn=$processors_per_node"
    exit 0
fi

if [ $1 == "queue" ]; then
    echo -n "reserved"
    exit 0
fi
