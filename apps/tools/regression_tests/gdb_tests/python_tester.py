import os
import re

python_string = "__PYTHON_OUTPUT__: "

def output(*s):
    gdb.write(python_string + " ".join(map(str, s)) + "\n")

def fail_test(*msgs, **kwargs):
    output("FAILED!")

    if "line_no" in kwargs:
        kwargs["line_no"] += 1
        output("test line number: %d" % kwargs["line_no"])

    for msg in msgs:
        output(msg)
    output()
    gdb.execute("quit")

def _run_test(testpath, testname):

    found_test = False
    out = []

    with open(testpath) as f:
        for line_no, line in enumerate(f):

            line = line.rstrip('\n')

            # Just for consistancy of denoting a blank line
            if line == "out:":
                line = "out: "
            if line == "out?:":
                line = "out?: "

            if found_test:
                if line.startswith("#"):
                    continue

                # Reached the end of the test (start of next test)
                elif line.startswith("test: "):
                    break

                # Found file command
                elif line.startswith("file: "):
                    filepath = line[6:].format(**os.environ)
                    gdb.execute("file " + filepath)
                    gdb.execute("exec-file " + filepath)

                # Found target command
                elif line.startswith("target: "):
                    target_args = line[8:].format(**os.environ)
                    target_args += " -l " + os.environ["LOGS_DIR"]
                    gdb.execute("target xstg " + target_args)

                # Found target arguments
                elif line.startswith("args: "):
                    args = line[8:].format(**os.environ)
                    gdb.execute("set args " + args)

                # Execute command
                elif line.startswith("in: "):
                    if out:
                        fail_test("too much output", "unexpected lines: ", *out, line_no=line_no)
                    cmd = line[4:]

                    gdb.write("(gdb) %s\n" % cmd)

                    out_str = gdb.execute(cmd, to_string=True)
                    out = out_str.split('\n')[:-1]

                    gdb.write(out_str)

                # Check output
                elif line.startswith("out:"):
                    if out:
                        regex = re.compile(line[5:])
                        out_line = out.pop(0).rstrip('\n')
                        if not regex.search(out_line):
                            fail_test("unexpected output `%s'"%out_line,
                                      "expected to match `%s'"%regex.pattern,
                                      line_no=line_no)
                    else:
                        fail_test("not enough output",
                                  "expected line to match `%s'"%line,
                                  line_no=line_no)

                # Check for optional output
                elif line.startswith("out?:"):
                    if out:
                        regex = re.compile(line[6:])
                        out_line = out[0].rstrip('\n')
                        if regex.search(out_line):
                            out.pop(0)

                # Ignore all output
                elif line == "ignore-output":
                    out = []

                # Error in test
                elif line.strip() != "":
                    output("malformed test!")
                    output("Unexpected test line `%s'" % line)

            elif line == "test: %s" % testname:
                found_test = True

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
