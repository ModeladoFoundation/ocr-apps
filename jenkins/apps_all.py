import sys
from os.path import dirname, realpath

sys.path.append(dirname(realpath(__file__)) + '/../hll/cnc/jenkins')
sys.path.append(dirname(realpath(__file__)) + '/../apps/jenkins')
from cnc_ocr_all import *
from apps_sub_all import *
