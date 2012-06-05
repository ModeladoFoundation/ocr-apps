#!/bin/sh
fgrep -e '>>>' /tmp/roger*/*.CE.00 | sed 's/^.*CONSOLE://'
fgrep -e '>>>' logs/*.CE.00 | sed 's/^.*CONSOLE://'
