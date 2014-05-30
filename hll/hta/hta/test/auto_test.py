import sys
import subprocess

def read_config_file():
    backends = []
    tests = []
    with open('./auto_test_config.txt') as f:
        lines = f.readlines()
        backends = lines[0].split()
        for i in lines[1:]:
            if i.rstrip() and i[0] != '#':
                tests.append(i.rstrip())

    return backends, tests

if __name__ == "__main__":
    backends, tests = read_config_file()
    print backends
    print tests

    errlog = open('./auto_test_result.txt', 'w')

    for b in backends:
        for t in tests:
            argv = t.split()
            argv[0] = './' + argv[0] + '_' + b
            print argv 
            ret = subprocess.call(argv)
            print ret
            if ret != 0:
                errlog.write('FAIL:\t\t' + argv[0] + '(' + str(ret) + ')\n')
            else:
                errlog.write('SUCCESS:\t' + argv[0] + '(' + str(ret) + ')\n')

