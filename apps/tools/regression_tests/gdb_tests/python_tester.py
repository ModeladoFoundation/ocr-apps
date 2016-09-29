import os
import re

blank_line_string = "^(?#__blank_line__)$"
python_string = "__PYTHON_OUTPUT__: "

MAX_UNTIL = 32

# Verbose output
def voutput(*s):
    gdb.write(" ".join(map(str, s)))
    gdb.flush()

def output(*s):
    gdb.write(python_string + " ".join(map(str, s)) + "\n")
    gdb.flush()

def fail_test(*msgs, **kwargs):
    output("FAILED!")

    if "line_no" in kwargs:
        kwargs["line_no"] += 1
        output("test line number: %d" % kwargs["line_no"])

    for msg in msgs:
        output(msg)
    output()
    gdb.execute("quit")

# Returns the output of running a command (minus the lines matched by
# ignore_all_re) as an array of strings (one per line).
def run_command(cmd, line_no, ignore_all_re, error_re):
    voutput("(gdb)", cmd, "\n")

    out_str = gdb.execute(cmd, from_tty=True, to_string=True)
    voutput(out_str)

    # Set the output (minus what is ignored)
    out = []
    for out_line in out_str.split('\n')[:-1]:
        for r in error_re:
            if r.search(out_line):
                fail_test("Error output `%s'"%out_line, line_no=line_no)
        for r in ignore_all_re:
            if r.search(out_line):
                break
        else:
            out.append(out_line)

    return out

def _run_test(testpath, testname):

    found_test = False
    start_of_tests = False
    ignore_all_re = [] #List of regex objects
    error_re = [] #List of regex objects
    out = []
    cmd = "" #The last command

    with open(testpath) as f:
        for line_no, line in enumerate(f):

            line = line.rstrip('\n')

            # Just for consistancy of denoting a blank line
            if line == "out:":
                line = "out: " + blank_line_string
            if line == "out?:":
                line = "out?: " + blank_line_string

            #
            # Check what the command is and execute it
            #
            if found_test:
                # Comment
                if line.startswith("#"):
                    continue

                # Reached the end of the test (start of next test)
                elif line.startswith("test: "):
                    break

                # Found file command
                elif line.startswith("file: "):
                    filepath = line[6:].format(**os.environ)

                    voutput("(gdb) file", filepath, "\n")
                    voutput(gdb.execute("file " + filepath, from_tty=True, to_string=True))

                    voutput("(gdb) exec-file", filepath, "\n")
                    voutput(gdb.execute("exec-file " + filepath, from_tty=True, to_string=True))

                # Found target command
                elif line.startswith("target: "):
                    target_args = line[8:].format(**os.environ)
                    target_args += " -l " + os.environ["LOGS_DIR"]
                    voutput("(gdb) target xstg", target_args, "\n")
                    voutput(gdb.execute("target xstg " + target_args, from_tty=True, to_string=True))

                # Found target arguments
                elif line.startswith("args: "):
                    args = line[8:].format(**os.environ)
                    voutput("(gdb) set args", args, "\n")
                    voutput(gdb.execute("set args " + args, from_tty=True, to_string=True))

                # Execute command
                elif line.startswith("in: "):
                    if out:
                        fail_test("too much output", "unexpected lines: ", *out, line_no=line_no)
                    cmd = line[4:]
                    out = run_command(cmd, line_no, ignore_all_re, error_re)

                # Check output
                elif line.startswith("out:"):
                    cmd = ""
                    if out:
                        regex = re.compile(line[5:])
                        out_line = out.pop(0).rstrip('\n')
                        if not regex.search(out_line):

                            if regex.pattern == blank_line_string:
                                fail_test("unexpected output `%s'"%out_line,
                                          "expected to be a blank line",
                                          line_no=line_no)
                            else:
                                fail_test("unexpected output `%s'"%out_line,
                                          "expected to match `%s'"%regex.pattern,
                                          line_no=line_no)
                    else:
                        fail_test("not enough output",
                                  "expected line to match `%s'"%line,
                                  line_no=line_no)

                # Check for optional output
                elif line.startswith("out?: "):
                    cmd = ""
                    if out:
                        regex = re.compile(line[6:])
                        out_line = out[0].rstrip('\n')
                        if regex.search(out_line):
                            out.pop(0)

                # Ignore all output
                elif line == "ignore-output":
                    cmd = ""
                    out = []

                elif line.startswith("until: "):
                    if not cmd:
                        output("malformed test!")
                        output("`until' not following `in' on test line `%s'" % line)
                    for x in xrange(MAX_UNTIL):
                        regex = re.compile(line[7:])
                        for out_line in out:
                            if regex.search(out_line):
                                break
                        else:
                            # Didn't find the output: run again.
                            out = run_command(cmd, line_no, ignore_all_re, error_re)
                            continue
                        out = []
                        break
                    else:
                        fail_test("Until loop never found `%s'"%regex.pattern, line_no=line_no)

                elif line.startswith("ignore-all: "):
                    ignore_all_re.append(re.compile(line[12:]))

                elif line.startswith("error: "):
                    error_re.append(re.compile(line[7:]))

                # Error in test
                elif line.strip() != "":
                    output("malformed test!")
                    output("Unexpected test line `%s'" % line)
                    gdb.execute("quit")

            elif line == "test: %s" % testname:
                found_test = True

            elif line.startswith("test: "):
                start_of_tests = True

            elif not start_of_tests:
                #
                # Headers to set up global regexs for errors and ignored messages
                #
                if line.startswith("ignore-all: "):
                    ignore_all_re.append(re.compile(line[12:]))

                elif line.startswith("error: "):
                    error_re.append(re.compile(line[7:]))

        if out:
            fail_test("output past end of the test", "unexpected lines: ", *out)

        if found_test:
            output("passed")
        else:
            output("non-existant test!")

        gdb.execute("quit")

def run_test(*args):
    gdb.execute("set confirm off")

    try:
        _run_test(*args)
    except Exception as e:
        output("Error running test!")
        output("Error raised was:")
        output(type(e), e)
        gdb.execute("quit")
