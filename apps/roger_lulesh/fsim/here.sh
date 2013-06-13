#!/bin/sh
set -x
egrep -e '(time\ =|Final)' logs/lulesh.log.brd00.chp00.unt00.blk00.CE.00 \
| sed -e 's/^CONSOLE: XE. >>> //' >/tmp/j
~/xstack/apps/roger_lulesh/utils/hex2sci </tmp/j >/tmp/jj
wc /tmp/j /tmp/jj
diff ~roger/xstack/apps/roger_lulesh/utils/gold.5.hex /tmp/j  | head -5 
diff ~roger/xstack/apps/roger_lulesh/utils/gold.5.sci /tmp/jj | head -5 
wc /tmp/j ~roger/xstack/apps/roger_lulesh/utils/gold.5.hex /tmp/jj ~roger/xstack/apps/roger_lulesh/utils/gold.5.sci 
