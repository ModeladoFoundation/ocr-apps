#!/usr/bin/env python


import getopt, logging, math, multiprocessing, os, re, shutil, sys, time
from datetime import datetime
from operator import itemgetter

# Update the search path for external dependences
sys.path.append('./external')

from junit_xml import TestSuite, TestCase
from filelock import filelock
from RECSortedList import RECSortedList

# Import other objects
from JobObject import JobObject, LocalJobObject, TorqueJobObject, JobTypeObject


# Set up the logger
class MyFilter(object):
    def __init__(self, highLevel):
        self.highLevel = highLevel

    def filter(self, record):
        if record.levelno < self.highLevel:
            return 1
        return 0

logging.getLogger().setLevel(logging.DEBUG)
streamHandlerStdOut = logging.StreamHandler(sys.stdout)
streamHandlerStdErr = logging.StreamHandler()
streamHandlerStdErr.setLevel(logging.WARNING)
streamHandlerStdOut.setLevel(logging.INFO)
streamFormatter = logging.Formatter('%(levelname)-8s %(message)s')
streamHandlerStdErr.setFormatter(streamFormatter)
streamHandlerStdOut.setFormatter(streamFormatter)
streamHandlerStdOut.addFilter(MyFilter(logging.WARNING)) # Leave WARNING and ERROR to stderr
logging.getLogger().addHandler(streamHandlerStdErr)
logging.getLogger().addHandler(streamHandlerStdOut)

# Global variables used to keep track of jobs
"""Initial directories fetched from
GIT. This is the reference directory that
contains the entire checked-out trees for the various repositories
"""
cleanDirectories = dict()

"""Environment directory
This directory contains pre-built executables
for example which are built more infrequently to allow
for more incremental testing"""
envDirectory = None

""""Root" directory that is shared (with LUSTRE)
across the various nodes. All tests will have their
directories created here if needed (absolute path)
"""
sharedRoot = None

""""Root" directory that is private the machine (faster).
All tests will have their directories created here as well
if needed (absolute path)
"""
privateRoot = None

"""Number of seconds the script will sleep before
checking for more jobs to run
"""
sleepInterval = None

"""All jobs that are left to run.
key: Name of the job
value: JobObject
"""
allRemainingJobs = dict()

"""All job types that can be used.
key: Name of the job type
value: JobTypeObject
"""
allJobTypes = dict()

"""Contains a list of all jobs keyed by the length of time
they have been running. Each element of the list is a JobObject of
a running job
"""
allRunningJobs = []

"""Contains the list of all jobs that are ready to run
ordered by their depth (priority). Contains a pair where the
first element is the priority of the ready job and the second
element is the JobObject
"""
allReadyJobs = RECSortedList(key=itemgetter(0))

"""Jobs that are "terminal" (ie: no waiters). The
results of the jobs in this list will be printed out"""
allTerminalJobs = []

"""Jobs based on their exit status"""
allJobsStatus = dict()

"""All running jobs by paths. The dictionary contains:
    - Key: The path where the job is running
    - Value: A pair:
        - 1st Element is True if one of the jobs is not OK with sharing
        - 2nd Element is a count of jobs running in this directory
        - 3rd Element is a count of failed jobs in that directory
"""
allUsedPaths = dict()

"""Global to give a sequential ID to each job
running. This ID is not necessarily consistent across
runs of testrunner but is guaranteed to be unique
for one run of testrunner thereby allowing it to be
used as a distinguishing factor for running jobs (for
example to create uniquely named directories for each
job
"""
jobId = 0

# Check whether a dictionary has the proper format
def checkDict(inputDict, typeStr, fieldTypes, mandatoryFields):
    """Checks whether input matches a certain format. This
    checks to see if the fields are of the proper type
    and if mandatory fields are present once and only once"""

    myLog = logging.getLogger();
    countPresence = [0] * len(mandatoryFields)
    if type(inputDict) != type(dict()):
        myLog.error("%s is not a dictionary", typeStr)
    for k, v in inputDict.items():
        t = fieldTypes.get(k, None)
        if t is not None:
            if type(t) != type(v):
                myLog.error("%s has invalid type for key '%s'" % (typeStr, k))
                return False
            if k in mandatoryFields:
                countPresence[mandatoryFields.index(k)] += 1
        else:
            myLog.error("%s has invalid key '%s'" % (typeStr, k))
            return False
    for i in range(0, len(countPresence)):
        if countPresence[i] == 0:
            myLog.error("%s is missing key '%s'" % (typeStr, mandatoryFields[i]))
            return False
        if countPresence[i] > 1:
            myLog.error("%s has multiple values for key '%s'" % (typeStr, mandatoryFields[i]))
            return False
    return True

# Check if a list of keywords has the keywords in the second list
def hasKeywords(enclosingList, keywords, allKeywords):
    myKw = frozenset(enclosingList)
    theirKw = frozenset(keywords)
    if allKeywords:
        # Test for subset
        return theirKw <= myKw
    else:
        # Check length of intersection
        return len(myKw & theirKw) > 0

# Check whether a job has the proper format
def checkJob(inputDict):
    checkStruct = {'name': "", 'keywords': (), 'depends': (),
                   'jobtype': "", 'run-args': "",
                   'param-args': "",
                   'sandbox': (), 'req-repos': (),
                   'timeout': int(0),
                   'env-vars': {}}
    checkPresence = ('name', 'jobtype',
                     'run-args')
    return checkDict(inputDict, "JobObject", checkStruct, checkPresence)

# Check whether a job type has the proper format
def checkJobType(inputDict):
    checkStruct = {'name': "", 'keywords': (), 'depends': (),
                   'isLocal': False,
                   'run-cmd': "", 'param-cmd': "",
                   'epilogue-cmd': "", 'prologue-cmd': "",
                   'sandbox': (), 'timeout': int(0),
                   'env-vars': {}, 'req-repos': () }
    checkPresence = ('name', 'keywords', 'isLocal', 'run-cmd', 'param-cmd',
                     'sandbox', 'req-repos')
    return checkDict(inputDict, "JobObject type", checkStruct, checkPresence)

# Print usage
class Usage:
    def __init__(self, msg):
        self.msg = msg

# Goes through the list of dependences for a job and updates it
# stripping out __alltype, __type and __skippable
def updateDependsList(jobName, deps, jobTypeToDependentJobs, jobTypeToName):
    myLog = logging.getLogger()
    replaceList = []
    depList = []
    for item in deps:
        splitItem = item.split()
        if splitItem[0] == "__type" and len(splitItem) == 2:
            # In this case, it is "skippable" (ie: if the test fails, it
            # won't block things)
            replaceList.append((splitItem[1], True))
            myLog.debug("Adding '%s' as a skippable job-type dependence to '%s'" % (splitItem[1], jobName))
        elif splitItem[0] == "__alltype" and len(splitItem) == 2:
            myLog.debug("Adding '%s' as a job-type dependence to '%s'" % (splitItem[1], jobName))
            replaceList.append((splitItem[1], False))
        elif splitItem[0] == "__skippable" and len(splitItem) == 2:
            # Add the dependence as a skippable one
            myLog.debug("Adding '%s' as a skippable dependence to '%s'" % (splitItem[1], jobName))
            depList.append((splitItem[1], True))
        else:
            # All other cases, just say that you cannot skip the dependence
            myLog.debug("Adding '%s' as a dependence to '%s'" % (item, jobName))
            depList.append((item, False))
    for item in replaceList:
        t = jobTypeToDependentJobs.get(item[0], ([], False))
        # Make sure the skippable value is correct
        assert(len(t[0]) == 0 or t[1] == item[1])
        jobTypeToDependentJobs[item[0]] = (t[0].append(jobName), item[1])
        if item[0] in jobTypeToName:
            depList = depList + [(k, item[1]) for k in jobTypeToName[item[0]]]
    return depList

# Main loop: loops that continuously looks for
# tasks to run and collects their results. It will return
# when there are no tasks that can be run
def mainLoop():
    global cleanDirectories, envDirectory, sharedRoot, privateRoot, sleepInterval
    global allRemainingJobs, allJobTypes, allRunningJobs, allReadyJobs, allTerminalJobs, allJobsStatus
    global allUsedPaths
    global maxLocalJobs
    global jobId

    localJobCount = 0      # Number of local jobs running
    waitForJobDrain = None # Environment producing job to run
    envJobRunning = False
    envLockFile = filelock.FileLock(os.path.join(envDirectory, ".jenkinstestrunner.lock"),
                           timeout=363, delay=sleepInterval)
    myLog = logging.getLogger()
    myLog.debug("Using lock file '%s'" % (os.path.join(envDirectory, ".jenkinstestrunner.lock")))
    while len(allRemainingJobs) > 0 or len(allReadyJobs) or len(allRunningJobs) or (waitForJobDrain is not None):
        myLog.debug("Looping with %d remaining jobs %s" % (len(allRemainingJobs) + len(allReadyJobs),
                                                           waitForJobDrain is not None and " and %s environment job" % (waitForJobDrain)
                                                           or ""))
        myLog.debug("Have %d blocked jobs and %d ready jobs" %
                    (len(allJobsStatus['blocked']), len(allReadyJobs)))
        needWholeMachine = False;
        # Go over the running jobs and see if they
        # are done
        tempAllRunning = []
        for job in allRunningJobs:
            if not job.poll():
                tempAllRunning.append(job)
                if job.getIsWholeMachine():
                    needWholeMachine = True
            else:
                returnedStatus = job.getStatus()
                assert(returnedStatus & 0x40)
                if returnedStatus == JobObject.DONE_OK:
                    allJobsStatus['ok'].append(job.name)
                else:
                    allJobsStatus['fail'].append(job.name)
                if job.getIsTerminalJob():
                    allTerminalJobs.append(job)
                if job.isLocal():
                    localJobCount -= 1
        allRunningJobs = tempAllRunning

        if waitForJobDrain is not None:
            if len(allRunningJobs):
                if envJobRunning:
                    myLog.debug("Waiting for environment producing job")
                else:
                    myLog.debug("Waiting for %d remaining jobs to drain"
                                % (len(allRunningJobs)))
                time.sleep(sleepInterval)
                continue
            else:
                if envJobRunning:
                    # This means that the environment producing job is done
                    myLog.info("Environment producing job %s finished" % (waitForJobDrain))
                    waitForJobDrain = None
                    envJobRunning = False
                    # Remove the lock file
                    envLockFile.release()
                else:
                    myLog.info("Running environment producing job %s" % (waitForJobDrain))
                    while True:
                        try:
                            envLockFile.acquire()
                            break
                        except filelock.FileLockException:
                            myLog.error("Environment producing job still on hold...")
                    returnedStatus = waitForJobDrain.execute(jobId)
                    jobId = jobId + 1
                    if (returnedStatus > JobObject.BLOCKED_JOB and
                        returnedStatus < JobObject.DONE_OK):
                        myLog.info("Starting %s" % (waitForJobDrain))
                        envJobRunning = True
                        allRunningJobs.append(waitForJobDrain)
                        if waitForJobDrain.isLocal():
                            localJobCount += 1
                    elif returnedStatus > JobObject.DONE_OK:
                        # This means the job did not launch properly
                        myLog.warning("%s failed to launch properly... it will be reported as a failure"
                                      % (waitForJobDrain))
                        if waitForJobDrain.getIsTerminalJob():
                            allTerminalJobs.append(waitForJobDrain)
                        allJobsStatus['fail'].append(waitForJobDrain.name)
                        waitForJobDrain = None
                        # This next statement should be redundant. Being safe
                        envJobRunning = False
                    else:
                        myLog.error("Status returned by execute not allowed for %s (got %d)"
                                    % (waitForJobDrain, returnedStatus))
                        waitForJobDrain = None
                        envJobRunning = False
                    continue
        # End if(waitForJobDrain)

        if waitForJobDrain is not None:
            # If we are here, it means we still have an environment
            # producing job so we hold off
            time.sleep(sleepInterval)
            continue

        # We now go over the jobs that are in allRemainingJobs
        # and distribute them to the other lists as appropriate
        keysToRemove = []
        for k, v in allRemainingJobs.iteritems():
            jobStatus = v.getStatus()
            if (jobStatus == JobObject.UNCONFIGURED_JOB or
                jobStatus == JobObject.WAITING_JOB):
                pass
            elif jobStatus == JobObject.READY_JOB:
                myLog.info("%s is now ready to run" % (v))
                if v.getIsEnvProducer():
                    assert(waitForJobDrain is None);
                    keysToRemove.append(k)
                    waitForJobDrain = v
                    myLog.info("%s is an environment producing job... Will run alone."
                               % (v))
                else:
                    keysToRemove.append(k)
                    allReadyJobs.add((-1*(v.getDepth()), v))
            elif jobStatus == JobObject.BLOCKED_JOB:
                myLog.warning("%s is now blocked -- it will never run." % (v))
                keysToRemove.append(k)
                allJobsStatus['blocked'].append(k)
                if v.getIsTerminalJob():
                    allTerminalJobs.append(v)
            else:
                myLog.warning("Unknown job status for %s: %s" %
                              (v, str(jobStatus)))
        # Now update the allRemainingJobs
        for k in keysToRemove:
            del allRemainingJobs[k]

        if waitForJobDrain is not None:
            # If we are here, it means we found an environment
            # producing job. We loop around to get
            # it scheduled at the top of the loop
            # At any rate, we can't schedule any new jobs
            time.sleep(sleepInterval)
            continue

        # We now have all the jobs in allReadyJobs and
        # we can try to execute them
        # TODO: This is ugly. Is there a better way??!?!
        effectiveEnd = len(allReadyJobs) - 1
        for i in range(0, len(allReadyJobs)):
            if i > effectiveEnd:
                break
            if needWholeMachine:
                break # We can't run any more jobs right now
            if allReadyJobs[i][1].isLocal() and localJobCount == maxLocalJobs:
                # We can't run local jobs anymore
                myLog.debug("Not launching local job %s (too many jobs running)" % (str(allReadyJobs[i][1])))
                continue
            myLog.debug("Trying to run %s" % (str(allReadyJobs[i][1])))
            assert(allReadyJobs[i][1].getIsEnvProducer() is False)
            returnedStatus = allReadyJobs[i][1].execute(jobId)
            jobId = jobId + 1
            if returnedStatus > JobObject.BLOCKED_JOB and returnedStatus < JobObject.DONE_OK:
                myLog.info("Starting %s" % (allReadyJobs[i][1]))
                allRunningJobs.append(allReadyJobs[i][1])
                if allReadyJobs[i][1].getIsWholeMachine():
                    myLog.debug("%s requires the entire machine, not launching others" % (str(allReadyJobs[i][1])))
                    needWholeMachine = True
                if allReadyJobs[i][1].isLocal():
                    localJobCount += 1
            elif returnedStatus > JobObject.DONE_OK:
                # This means the job did not launch properly
                myLog.warning("%s failed to launch properly... it will be reported as a failure" % (str(allReadyJobs[i][1])))
                if allReadyJobs[i][1].getIsTerminalJob():
                    allTerminalJobs.append(allReadyJobs[i][1])
                allJobsStatus['fail'].append(allReadyJobs[i][1].name)
            else:
                myLog.error("Status returned by execute not allowed for %s (got %d)" % (str(allReadyJobs[i][1]), returnedStatus))
                # I don't really know what to do here. It should never happen
            allReadyJobs.pop(i)
            effectiveEnd -= 1

        # At this stage, we have launched all jobs that
        # could be launched. We will sleep before resuming the
        # loop
        time.sleep(sleepInterval)
    # End of while loop on allRemainingJobs

# Importing job definitions
def importFromFile(name, tempJobs, jobTypeToName, alternateJobs):
    global allJobTypes

    myLog = logging.getLogger()

    tFile = __import__(name)
    for k,v in tFile.__dict__.items():
        if k.startswith('job_'):
            # This is a job so we insert it in the allJobs dictionary
            try:
                if tempJobs.has_key(v['name']):
                    raise Usage("Duplicate job name '%s'" % (v['name']))
                myLog.info("Checking job '%s' for correctness" % (v['name']))
                if checkJob(v):
                    # Set up an empty dependence list if it doesn't have one
                    if not v.has_key('depends'):
                        v['depends'] = ()
                    tempJobs[v['name']] = v
                    # Add jobname and jobtype to dictionary
                    if jobTypeToName.has_key(v['jobtype']):
                        jobTypeToName[v['jobtype']].append(v['name'])
                    else:
                        jobTypeToName.setdefault(v['jobtype'],[v['name']])
                else:
                    raise Usage("JobObject '%s' is incorectly formated" % (v['name']))
            except KeyError:
                raise Usage("JobObject '%s' in file '%s' does not define a name" %
                            (k, a))
        elif k.startswith('jobtype_'):
            # This is a job type so we insert it in the allJobTypes dictionary
            try:
                if allJobTypes.has_key(v['name']):
                    raise Usage("Duplicate job type name '%s'" % (v['name']))
                myLog.info("Checking job type '%s' for correctness" % (v['name']))
                if checkJobType(v):
                    allJobTypes[v['name']] = JobTypeObject(v)
                else:
                    raise Usage("JobObject type '%s' is incorectly formated" % (v['name']))
            except KeyError:
                raise Usage("JobObject type '%s' in file '%s' does not define a name" %
                            (k, a))
        elif k.startswith('pick_one_of_'):
            # This means that we need to select one of the jobs that best matches the list
            # of repositories we have available
            alternateJobs[alternateJobs['__count']] = v
            alternateJobs['__count'] = alternateJobs['__count'] + 1


# Main function
def main(argv=None):
    global cleanDirectories, envDirectory, sharedRoot, privateRoot, sleepInterval
    global allRemainingJobs, allJobTypes, allRunningJobs, allReadyJobs, allTerminalJobs
    global allJobsStatus
    global allUsedPaths
    global streamHandlerStdOut
    global maxLocalJobs
    testKeywords = []
    negativeTestKeywords = []

    allJobsStatus['blocked'] = []
    allJobsStatus['ok'] = []
    allJobsStatus['fail'] = []

    KEEP_NONE = 0
    KEEP_FAILURE = 1
    KEEP_ALL = 2

    keepDirs = KEEP_NONE # Which directories to keep after the test
    runRemoteJobs = True # Whether or not to run remote jobs

    myLog = logging.getLogger()

    if argv is None:
        argv = sys.argv

    doHierarchicalResults = False
    maxLocalJobs = int(math.ceil(multiprocessing.cpu_count() / 4.0))
    myLog.info("---- Starting ----")
    tempJobs = dict() # Contains the jobs until we have parsed all the job types
    alternateJobs = dict() # Certain jobs have one version depending on what repos are accessible
                           # This contains the list.
                           # alternateJobs['__count']: an index to index the other fields. Next value to use
                           # alternateJobs[X]: a dict: name -> nameOfAlternate; alternates -> name of alternate versions
                           # alternateJobs['name'] = alternatePicked
    alternateJobs['__count'] = 0
    jobTypeToName =  dict() # Contains list of all job types and jobs belonging to each category
    resultFileName = None
    defaultTimeout = 333 # 5 min for all jobs if no other timeout specified
    try:
        try:
            opts, args = getopt.getopt(argv[1:], "hc:i:e:s:p:t:k:o:dr:", [
                "help", "config=", "initdir=", "envdir=", "shared=", "private=",
                "time=", "repo=", "keyword=", "output=", "full-help", "debug",
                "local-only", "local-max=", "no-clean=", "hier-results", "deftimeout="])
        except getopt.error, err:
            raise Usage(err)
        # Need to parse deftimeout first
        for o, a in opts:
            if o in ('--deftimeout',):
                defaultTimeout = int(a)
                break
        myLog.debug("Default timeout set to %d seconds" % (defaultTimeout))
        JobObject.setGlobals(defaultTimeout=defaultTimeout)
        for o, a in opts:
            if o in ("-h", "--help"):
                raise Usage(\
"""
    -h,--help:      Prints this message
    --full-help:    Prints a detailed message on the format of jobs and job types
    -d,--debug:     Enable debugging
    -c,--config:    Test files to import (without .py extension). The files should contain
                    the description of the tests to run. Can be specified multiple times.
                    By default, the tester will look for a folder named 'jenkins' in the root
                    of each of the repositories and import all.py.
    -r,--repo:      Repository that is accessible to this instance of the test runner (initial directory).
                    It can be specified multiple times (once for each repository). The format is
                    name:path where 'name' is the name of the repository (used in req-repos) and
                    'path' is the path to that repository (absolute path)
    -e,--envdir:    Environment directory containing pre-built binaries. Persistent
                    across test runs (should be shared by LUSTRE)
    -s,--shared:    Root directory to use for "shared" workspaces (shared by LUSTRE)
                    Optional if '--local-only' is specified
    -p,--private:   Root directory to use for "private" (local) workspaces
    -t,--time:      Time to wait before each iteration of the job scheduling algorithm
    -k,--keyword:   (optional) Keywords to use to select the test to run. They keywords specified
                    can either be positive or negative. The test runner will run:
                        - ALL the tests that match ALL the keywords specified positively
                        - EXCEPT those that match a keyword specified negatively
                    To specify a keyword negatively, use ~keyword
                    As an example, if you specify the keywords: foo bar ~baz:
                       - a test with keywords foo bar blah will run
                       - a test with keywords foo will NOT run (does not contain bar)
                       - a test with keywords foo bar baz will NOT run (contains baz)
      If specified multiple times,
                    only the tests that match ALL the keywords will be run.
    -o,--output:    Filename to produce containing the results of the tests
    --hier-results: Output results hierarchically in Jenkins. This means that the top-level
                    "packages" will be the name of the terminating jobs and the "classes"
                    inside will be the name of the jobs that were required along the path
                    to running the terminal job. If not specified, there will be a "root"
                    package and all tests (terminal or not) will be listed as independent
                    classes.
    --local-only:   Runs only local tests (tests that do not require Torque and/or LUSTRE)
    --local-max:    Maximum number of local jobs that will be concurrently running. Defaults to
                    ceil(#CPUs/4).
    --no-clean:     Can be either 'all' or 'failure'. If 'all', all build directories
                    will be maintained after completion of the test. If 'failure', all
                    directories with at least one failure in them will be maintained.
    --deftimeout    Default timeout for jobs without timeouts. Defaults to 333 seconds
""")
            elif o in ('--full-help',):
                raise Usage(\
"""
    Each job type must specify the following arguments:
        - name:        (string) Name of the job type (must be unique)
        - keywords:    (tuple of strings) Used to restrict the tests run. If the user specifies
                       keywords when launching the test harness, only those job types that match the
                       specified keywords will be run
        - isLocal:     (bool) If True, the job will run on the local machine. If False, it will be
                       farmed out by Torque
        - run-cmd:     (string) Command to launch the job.
        - param-cmd:   (string) Command to respond to parameter queries for jobs. The following
                       arguments will be passed:
                           - for all jobs:
                               - output: Returns the absolute path of a XML output file with JUnit
                                         formatted result or the empty string if no such file is
                                         generated (in which case the job itself is the testcase)
                           - for non-local jobs:
                               - resources: Returns the formatted string of resource requirements
                                            for Torque
                               - queue:     Returns the name of the queue to submit jobs to.
                                            Defaults to 'batch'
        - sandbox:     (tuple of strings) See below for an explanation of the sandbox parameters
        - req-repos:   (tuple of strings) Repositories that are required to run this job type. The
                       strings correspond to the name passed in with the "-r" option
    It can also optionally specify:
        - depends:      (tuple of strings) List of jobs all jobs of this type will depend on.
                        See 'depends' in the job description for more detail
        - timeout:      (int) Number of seconds before a job is killed. If non specified or 0,
                        the job will never timeout
        - prologue-cmd: (string) A script to be run right before the job. This is mostly
                        useful for remote jobs. On success, this script should return 0.
                        The system will abort prologue scripts after 5 minutes.
                        Note that this script runs ONCE even for remote jobs taking up
                        multiple machines. It will run on the "first" remote node. Note that
                        prologue and epilogue scripts run in an environment with NO environment
                        variables defined. For convenience, the system will parse the
                        prologue and epilogue script and statically replace all instances
                        of environment variables. It does not do this recursively.
        - epilogue-cmd: (string) Same as 'prologue-cmd' except it executes after the job
                        runs. Note that this can be used for cleanup as it will run
                        even in the case of a failure.
        - env-vars:     (dict) A dictionary of additional environment variables for the
                        job.

    Each job must specify the following arguments:
        - name:        (string) Name of the job (must be unique)
        - jobtype:     (string) Name of the job type of this job
        - run-args:    (string) Arguments for this job (will be concatenated with 'run-cmd'
                       from its job type to actually run the job
    It can optionally specify:
        - keywords:    (tuple) Additional keywords specific to this job. These keywords will be
                       appended to the keywords defined for the jobtype
        - depends:     (tuple of strings): Specifies the dependences this job needs to wait on. A dependence
                       is either skippable or non-skippable. A skippable dependence will not block the execution
                       of this job if it fails. A dependence is described by either:
                         - a job name: this is a normal non-skippable dependence
                         - a skippable job name (syntax: '__skippable <jobname>'): a skippable dependence
                         - a non-skippable job from a list of alternatives (syntax: '__alternate <identifier>'):
                           a non-skippable job picked from a set of alternate that are described in a dictionary
                           called 'pick_one_of_<identifier>'
                         - a skippable job type (syntax: '__type <jobtype>'): a dependence on jobs of the
                           specified job-type. All dependences are skippable
                         - a non-skippable job type (syntax: '__alltype <jobtype>'): a dependence on jobs of the
                           specified job-type. All dependences are non-skippable and the failure of any one
                           job will cause this job to be blocked
                       If the dependence is on a jobtype, this job will wait on all jobs of that
                       jobtype to complete before running. In that case the inheritX arguments are valid
                       but the order of the jobs is undetermined.
                       In all cases, the 'depends' list of a specific job is concatenated with the (optional)
                       'depends' list for the job-type.
        - param-args:  (string) Arguments for this job to get parameter information for this
                       job. Will be concatenated with 'param-cmd' from its job type AFTER
                       the type of parameter required.
        - sandbox:     (tuple of strings) See below for an explanation of the sandbox parameters.
                       This complements the job type's sandbox information for inheritX information
                       and over-rides all other parameters.
        - req-repos:   Additional required repository(ies) for this job. These will be in addition
                       to the ones specified in the jobtype.
        - timeout:     (int) Number of seconds before this job is killed. It overrides anything
                       specified by the job type
        - env-vars:    (dict) A dictionary of additional environment variables for the job. Thes
                       will be added to the ones defined in the job-type; in case of duplicates, th
                       job environment variables will override the ones from the job-type

    Explanation of the parameters of the 'sandbox' argument:
        - The 'sandbox' argument determines the directories this job will be given to run in
          and whether or not it "inherits" data from some of its parent jobs. In the list below,
          a [no] prefix indicates that the job's sandbox parameters can override the job type's
          sandbox parameter. The valid strings are:
            ## Job execution characteristics: Defaults: nosingle, noshareOK ##
            - [no]single:   Specifies whether the job requires the entire machine to run. This only applies to
                            local jobs as the requirements for remote jobs are returned through param-cmd.
            - [no]shareOK:  If inheriting from a parent, specifies whether it is OK to run other jobs in that
                            same directory.
            - writeEnv:     Can only be specified in the job type's sandbox.
                            Will be prioritized over other jobs and will run
                            alone to update the environment.
            ## Execution directories: Defaults: local ##
            - [no]local:    Specifies whether this job requires a local home directory (non-shared over LUSTRE).
                            If specified before 'shared', the initial directory for the job will be the
                            local one. The environment variable JJOB_PRIVATE_HOME will be set to this
                            directory as well as JJOB_START_HOME if this is the starting directory.
            - [no]shared:   Specifies whether this job requires a shared home directory (shared over LUSTRE).
                            Same comments as for 'local'. Sets JJOB_SHARED_HOME.
            - inheritX:     Where X is an integer from 0 to the number of dependences minus 1. This specifies
                            which working directory from a parent this job should inherit. For example, jobs
                            that run regression tests may want to inherit from the job that built the executables.
                            If this option is not specified, the job will get a copy of the initial check-outs (the
                            repositories the job has access to). This option can only be specified in the
                            job's sandbox description.
            ## Execution directories behavior: Defaults: (none) ##
            - [no]copyShared:  If specified, the job will be given a copy of its 'shared' home. This is relevant
                               only when inheritX is specified as, in the default case, a copy of the initial
                               checkout is given
            - [no]emptyShared: If specified, the job will be given an empty 'shared' home.
            - [no]copyLocal:   Same as [no]copyShared for the private home
            - [no]emptyLocal:  Same as [no]emptyShared for the private home


    Environment variables available to the jobs:
        - JJOB_NAME                   Name of the job running
        - JJOB_ID                     A unique identifier (for this job). The identifier is an integer
                                      and will not be consistent across runs of testrunner
        - JJOB_INITDIR_XXXX           Directory containing the initial checkouts. You should not
                                      modify anything in this (this is the "golden" copy for the
                                      tests). This is always local. XXXX corresponds to the all-caps
                                      name of the repository checked out
        - JJOB_ENVDIR                 Path to the environment directory (always shared). You should
                                      not modify this unless you are a writeEnv job
        - JJOB_PRIVATE_HOME           Local (non LUSTRE) directory for this job
                                      if 'local' is specified
        - JJOB_SHARED_HOME            Shared (LUSTRE) directory for this job if
                                      'shared' is specified
        - JJOB_START_HOME             Directory where the job is started.
                                      One of JJOB_SHARED_HOME or JJOB_PRIVATE_HOME
                                      depending on the sandbox flags:
                                        - If only one directory exists, that one is
                                          used
                                        - If both 'shared' and 'local' are specified
                                          will use the PRIVATE_HOME if 'local' is
                                          specified before 'shared' (and not cancelled
                                          out by 'noLocal' prior to 'shared')
        - JJOB_PARENT_PRIVATE_HOME_X  If parent X (X from 0 to number of
                                      dependences minus 1) has a private
                                      home, gives the path (otherwise empty).
                                      This can be the same as JJOB_START_HOME
                                      and JJOB_PRIVATE_HOME if inheritX is
                                      set and noCreateRoot or NOT createRoot
        - JJOB_PARENT_SHARED_HOME_X   Same as JJOB_PARENT_PRIVATE_HOME_X
                                      for the shared directory. Same comments
                                      as well
        - Any environment variables passed using env-vars in the job-type or job.
        - Note that you can use environment variables inside other environment variables (one-level
          of substitution). The rules for substitution are:
            -w $$ will be replaced by $
            - $identifier or ${identifier} will be replaced by the value of 'identifier'
            - Unknown identifiers will be ignored (ie: $foobar if foobar does not refer to
              a valid substitution will be left as $foobar)
          The script will run over the following list of variables as identifiers for substitution:
            - Normal environment variables ($PATH, etc.)
            - The set of environment variables for the job (ie: if you set FOOBAR as a job or
              job-type environment variable, you can use $FOOBAR is another environment variable)

    References:
        - Prologue error codes for Torque: http://docs.adaptivecomputing.com/torque/4-2-8/help.htm#topics/12-appendices/prologueErrorProc.htm
        - Prologue and epilogue scripts for Torque: http://docs.adaptivecomputing.com/torque/4-2-8/help.htm#topics/12-appendices/scriptEnvironment.htm
""")
            elif o in ("-d", "--debug"):
                streamHandlerStdOut.setLevel(logging.DEBUG)
            elif o in ("-c", "--config"):
                (importPath, name) = os.path.split(a)
                myLog.debug("Importing file in '%s' called '%s'" % (importPath, name))
                savePath = list(sys.path)
                sys.path.append(importPath)
                try:
                    importFromFile(name, tempJobs, jobTypeToName, alternateJobs)
                except ImportError:
                    raise Usage("File '%s' could not be imported" % (a))
                sys.path = savePath
            elif o in ("-r", "--repo"):
                splits = a.split(":", 1)
                cleanDirectories[splits[0]] = splits[1]
            elif o in ("-e", "--envdir"):
                envDirectory = a
            elif o in ("-s", "--shared"):
                sharedRoot = a
            elif o in ("-p", "--private"):
                privateRoot = a
            elif o in ("-t", "--time"):
                sleepInterval = int(a)
            elif o in ("-k", "--keyword"):
                if a[0] == '~':
                    negativeTestKeywords.append(a[1:])
                else:
                    testKeywords.append(a)
            elif o in ("-o", "--output"):
                resultFileName = a
            elif o in ("--hier-results",):
                doHierarchicalResults = True
            elif o in ("--no-clean",):
                if a == 'all':
                    keepDirs = KEEP_ALL
                elif a == 'failure':
                    keepDirs = KEEP_FAILURE
                else:
                    raise Usage("Value %s is not valid for '--no-clean'" % (a))
            elif o in ("--local-only",):
                runRemoteJobs = False
            elif o in ("--local-max",):
                maxLocalJobs = int(a)
                if maxLocalJobs < 1:
                    maxLocalJobs = 1
            elif o in ("--deftimeout",):
                # Ignore, we already parsed this
                pass
            else:
                raise Usage("Unhandled option")
        if args is not None and len(args) > 0:
            raise Usage("Extraneous arguments %s" % (args))

        # Now check if we have everything we need
        # TODO: Add permission checks maybe
        if len(cleanDirectories) == 0:
            raise Usage("Need at least one repository")
        else:
            t = dict()
            for k, v in cleanDirectories.iteritems():
                realPath = os.path.realpath(v)
                myLog.debug("Adding repository '%s' with path '%s'" % (k, realPath))
                t[k.upper()] = realPath
                # Look for the all.py file in jenkins in realPath
                savePath = list(sys.path)
                sys.path.append(realPath + "/jenkins")
                try:
                    importFromFile(k + "_all", tempJobs, jobTypeToName, alternateJobs)
                except ImportError:
                    myLog.debug("File '%s_all.py' in repository '%s' not found -- ignoring ('%s')" %
                                (k, k, realPath + "/jenkins"))
                sys.path = savePath
            cleanDirectories = t

        if envDirectory is None:
            raise Usage("Missing environment directory")
        else:
            envDirectory = os.path.realpath(envDirectory)

        if privateRoot is None:
            raise Usage("Missing private root directory")
        else:
            privateRoot = os.path.realpath(privateRoot)

        if runRemoteJobs and (sharedRoot is None):
            raise Usage("Missing shared root directory")
        elif runRemoteJobs:
            sharedRoot = os.path.realpath(sharedRoot)

        if sleepInterval is None:
            myLog.warning("Sleep interval not specified... assuming 5 seconds")
            sleepInterval = 5

        if resultFileName is None:
            myLog.warning("No output filename specified, results will be dumped to stdout")
        else:
            resultFileName = os.path.realpath(resultFileName)

        # Check if we will have circular copy issues
        for k, v in cleanDirectories.iteritems():
            if privateRoot.startswith(v):
                raise Usage("The private root directory cannot be within repository '%s': %s" % (k, v))

        for k, v in cleanDirectories.iteritems():
            if sharedRoot.startswith(v):
                raise Usage("The share root directory cannot be within repository '%s': %s" % (k, v))
    except Usage, msg:
        print >> sys.stderr, msg.msg
        print >> sys.stderr, "For help, use -h"
        return 2

    # Set the globals
    JobObject.setGlobals(cleanDirectories=cleanDirectories, envDirectory=envDirectory,
                         sharedRoot=sharedRoot, privateRoot=privateRoot,
                         allUsedPaths=allUsedPaths, runRemoteJobs=runRemoteJobs)

    myLog.info("Max concurrent local jobs is: %d" % (maxLocalJobs))

    try:
        # At this point we have all the job types and we can do other checks
        # and create the jobs

        repoMissingJobs = dict() # Jobs that don't have all the repos they need. Keeping track of separately
                                 # to properly inform the user

        # First step: filter based on repositories
        toRemoveKeys = []
        for k, v in tempJobs.iteritems():
            jobType = allJobTypes.get(v['jobtype'])
            if not hasKeywords(cleanDirectories.keys(), [j.upper() for j in jobType.req_repos + v.get('req-repos', ())], True):
                myLog.info("Ignoring job '%s' due to lack of repositories" % (v['name']))
                toRemoveKeys.append(k)
        for k in toRemoveKeys:
            repoMissingJobs[k] = tempJobs[k]
            del tempJobs[k]

        # Second step: selecting the best alternative for each job
        toRemoveKeys = []
        if alternateJobs['__count'] > 0:
            for i in range(0, alternateJobs['__count']):
                maxSet = frozenset()
                maxName = None
                for name in alternateJobs[i].get('alternates'):
                    job = tempJobs.get(name, None)
                    if job is None:
                        continue
                    jobType = allJobTypes.get(job['jobtype'])
                    myReqSet = frozenset(jobType.req_repos + job.get('req-repos', ()))
                    if myReqSet > maxSet:
                        if maxName is not None:
                            myLog.debug("Removing job '%s' because there is a better alternative ('%s')" % (maxName, name))
                            toRemoveKeys.append(maxName)
                        maxName = name
                        maxSet = myReqSet
                assert(maxName is not None)
                myLog.info("Picked job '%s' among set of alternates" % (maxName))
                alternateJobs[alternateJobs[i].get('name')] = maxName
            # End for
            for k in toRemoveKeys:
                del tempJobs[k]
        #end if

        # Third step: replicate the job-type's depends list to all the jobs of that type
        # We do this now because in the next step, we start looking at dependences.

        for k, v in tempJobs.iteritems():
            jobType = allJobTypes.get(v['jobtype'])
            # We should have the job type properly
            assert(jobType is not None)
            v['depends'] = jobType.depends + v['depends']
        # end for

        # Fourth step: check for __alternate
        # We do this now because if the job is pushed aside and re-added later
        # we will have issues
        toRemoveKeys = []
        for k, v in tempJobs.iteritems():
            # Replace any "__alternate" dependence
            newList = []
            for item in v['depends']:
                if item.split()[0] == "__alternate" and len(item.split()) == 2:
                    newList.append(alternateJobs.get(item.split()[1], "MISSING_ALTERNATE"))
                else:
                    newList.append(item)
            v['depends'] = tuple(newList)

        # Fifth step: check the keywords. Remove all jobs that don't match the keywords.
        # They may be re-added later as dependences (ie: if another job depends on it)
        toRemoveKeys = []
        sideJobs = dict() # Jobs that don't match keyword restrictions. May be re-added due to dependences
        for k, v in tempJobs.iteritems():
            jobType = allJobTypes.get(v['jobtype'])
            if jobType is None:
                raise Usage("JobObject '%s' uses a job type '%s' which is not defined" % (k, v['jobtype']))
            tkey = jobType.keywords + v.get('keywords', ())
            if not hasKeywords(tkey, testKeywords, True):
                myLog.info("Ignoring job '%s' due to keyword restrictions" % (v['name']))
                toRemoveKeys.append(k)
            elif hasKeywords(tkey, negativeTestKeywords, False):
                myLog.info("Ignoring job '%s' due to negative keyword restrictions" % (v['name']))
                toRemoveKeys.append(k)
        for k in toRemoveKeys:
            sideJobs[k] = tempJobs[k]
            del tempJobs[k]

        # Sixth step: Look at __type dependences
        # This allows us to wait only on the jobs that are currently enabled

        # Keep track of jobs that depend on a given type if
        # a job of that type is re-added
        jobTypeToDependentJobs = dict()
        for k,v in tempJobs.iteritems():
            depList = []
            replaceList = []
            v['depends'] = tuple(updateDependsList(k, v['depends'], jobTypeToDependentJobs, jobTypeToName))
            # Now create the jobs
            jobType = allJobTypes.get(v['jobtype'])
            if jobType.isLocal:
                allRemainingJobs[k] = LocalJobObject(v, jobType, len(v['depends']))
            else:
                allRemainingJobs[k] = TorqueJobObject(v, jobType, len(v['depends']))

        # Do another loop to set up the dependences properly
        changedJobs = True
        ttempJobs = dict()
        while changedJobs:
            changedJobs = False
            toRemoveKeys = []
            for k, v in tempJobs.iteritems():
                if len(v['depends']):
                    waitingJob = allRemainingJobs[k]
                    for dep, isSkippable in v['depends']:
                        signalingJob = allRemainingJobs.get(dep, None)
                        if signalingJob is None:
                            tjob = sideJobs.get(dep, None)
                            if tjob is None:
                                tjob = repoMissingJobs.get(dep, None)
                                if tjob is None:
                                    if isSkippable:
                                        myLog.error("<JobObject '%s'> has unknown dependence '%s' (skipping)" % (k, dep))
                                        waitingJob.satisfyDependence(None, dep, -1)
                                    else:
                                        myLog.error("<JobObject '%s'> has unknown dependence '%s'" % (k, dep))
                                        waitingJob.signalJobDone(-3, False, "<JobObject '%s'> has unknown dependence '%s'" % (k, dep))
                                        toRemoveKeys.append(k)
                                        changedJobs = True
                                else:
                                    myLog.debug("<JobObject '%s'> has a known dependence '%s' but that dependence was removed because of insufficient repository information" % (k, dep))
                                    if isSkippable:
                                        waitingJob.signalJobDone(-3, False, "<JobObject '%s'> has a dependence with insufficient repos '%s'; required %s (skipping)" %
                                                             (k, dep, repr(allJobTypes.get(tjob['jobtype']).req_repos + tjob.get('req-repos', ()))))
                                        waitingJob.satisfyDependence(None, dep, -1)
                                    else:
                                        waitingJob.signalJobDone(-3, False, "<JobObject '%s'> has a dependence with insufficient repos '%s'; required %s" %
                                                             (k, dep, repr(allJobTypes.get(tjob['jobtype']).req_repos + tjob.get('req-repos', ()))))
                                        toRemoveKeys.append(k)
                                        changedJobs = True
                            else:
                                tjob['depends'] = updateDependsList(dep, tjob['depends'], jobTypeToDependentJobs, jobTypeToName)
                                ttempJobs[dep] = tjob
                                jobType = allJobTypes.get(tjob['jobtype'])
                                assert(jobType is not None)
                                if jobType.isLocal:
                                    signalingJob = allRemainingJobs[dep] = \
                                                   LocalJobObject(tjob, jobType, len(tjob['depends']))
                                else:
                                    signalingJob = allRemainingJobs[dep] = \
                                                   TorqueJobObject(tjob, jobType, len(tjob['depends']))
                                changedJobs = True
                                myLog.info("Re-adding job %s due to dependence by %s" % (signalingJob, waitingJob))
                                t = jobTypeToDependentJobs.get(tjob['jobtype'], None)
                                if t is not None:
                                    # This means that there are other jobs depending on '__type' that need to be updated
                                    for jobName in t[0]:
                                        otherWaitingJob = allRemainingJobs[jobName]
                                        myLog.debug("Updating Job %s's list of '__type'/'__alltype' dependence; adding Job %s" % (otherWaitingJob, signalingJob))
                                        slot = otherWaitingJob.extendDependenceList(signalingJob, t[1])
                                        if signalingJob.addWaiter(otherWaitingJob, slot) == False:
                                            # This can happen because the signaling job may have been 'signalJobDone' above
                                            # due to a missing dependence for that job (ie: A -> signalingJob -> otherWaitingJob
                                            # and A is missing; signalingJob will be "satisfied")
                                            myLog.debug("Job %s's dependence %d is being satisfied due to completed producer (%s)"
                                                        % (otherWaitingJob, slot, signalingJob))
                                            otherWaitingJob.satisfyDependence(signalingJob, slot)
                                        # end if addWaiter
                                    # end for jobName
                                # end t is not None
                            # end else (tjob exists)
                        # end signalingJob is None
                        if signalingJob is not None:
                            slot = waitingJob.addDependence(signalingJob)
                            if signalingJob.addWaiter(waitingJob, slot) == False:
                                # This can happen because the signaling job may have been 'signalJobDone' above
                                myLog.debug("Job %s's dependence %d is being satisfied due to completed producer (%s)"
                                            % (waitingJob, slot, signalingJob))
                                waitingJob.satisfyDependence(signalingJob, slot)
                                # In most cases, this will block the job. We don't deal with it here
                                # as it will get removed in the mainloop.
                        # End of signalingJob is None
                    # End of dep in v['depends']
                # end of len(v['depends'])
            # Got all dependence information
            for k in toRemoveKeys:
                # Update the proper status for the job
                returnedStatus = allRemainingJobs[k].getStatus()
                assert(returnedStatus != JobObject.DONE_OK)
                if returnedStatus == JobObject.BLOCKED_JOB:
                    allJobsStatus['blocked'].append(k)
                elif returnedStatus > JobObject.DONE_OK:
                    allJobsStatus['fail'].append(k)
                del allRemainingJobs[k]
            tempJobs = ttempJobs.copy()
            ttempJobs.clear()
        # End of while loop
        del sideJobs

        # Check if we missed some dependence
        toRemoveKeys = []
        for k, v in allRemainingJobs.iteritems():
            if v.getStatus() == JobObject.UNCONFIGURED_JOB:
                myLog.error("<JobObject '%s'> has missing dependences" % (k))
                v.signalJobDone(-3, False, "<JobObject '%s'> has missing dependences" % (k))
                allJobsStatus['blocked'].append(k)
                toRemoveKeys.append(k)
        for k in toRemoveKeys:
            del allRemainingJobs[k]
    except Usage, msg:
        print >> sys.stderr, msg.msg
        print >> sys.stderr, "For help, use -h"
        return 2

    myLog.info("---- Initializing ---")
    # Build the initial list of ready jobs
    totalJobCount = len(allRemainingJobs)
    haveReadyJob = False
    toRemoveKeys = []
    for k, v in allRemainingJobs.iteritems():
        if v.getStatus() == JobObject.READY_JOB:
            haveReadyJob = True
            if v.getIsEnvProducer():
                myLog.info("%s is an environment producing job initially ready to run" % (str(v)))
                # We don't put in allReadyJobs. See mainLoop
            else:
                myLog.info("%s is initially ready to run" % (str(v)))
                allReadyJobs.add((-1*(v.getDepth()), v))
                toRemoveKeys.append(k)
    # Remove things from allRemainingJobs
    for k in toRemoveKeys:
        del allRemainingJobs[k]

    if not haveReadyJob:
        myLog.error("No jobs are initially ready, nothing to execute")
        return 0 # This may not be an "error" as such if no jobs match the keywords

    startTime = datetime.now()
    myLog.info("---- Starting jobs (have %d total jobs) @ %s ----" %
               (totalJobCount, str(startTime)))
    mainLoop()
    endTime = datetime.now()
    myLog.info("---- Finished jobs (took %d seconds: %d terminal jobs; %d blocked jobs; %d failures), now cleaning up ----" %
               ((endTime - startTime).seconds, len(allTerminalJobs), len(allJobsStatus['blocked']), len(allJobsStatus['fail'])))

    myLog.info("---- Failed jobs ----")
    for j in allJobsStatus['fail']:
        myLog.info("%s" % (j))
    myLog.info("--- Successful jobs ----")
    for j in allJobsStatus['ok']:
        myLog.info("%s" % (j))
    myLog.info("--- Blocked jobs ----")
    for j in allJobsStatus['blocked']:
        myLog.info("%s" % (j))
    myLog.info("--------")
    # Output results
    allTestSuites = [job.getTestSuite() for job in allTerminalJobs]
    if resultFileName:
        resultFile = open(resultFileName, "w")
        TestSuite.to_file(resultFile, allTestSuites, encoding='utf-8', doHierarchical=doHierarchicalResults)
    else:
        TestSuite.to_file(sys.stdout, allTestSuites, encoding='utf-8', doHierarchical=doHierarchicalResults)

    # Remove directories (do this now because some of the result files
    # may be in the directories)
    for k, v in allUsedPaths.iteritems():
        assert(not v[0])
        assert(v[1] == 0)
        doRemove = True
        if keepDirs == KEEP_FAILURE:
            # We only keep if there is a failure
            if v[2] > 0:
                myLog.info("Keeping work directory '%s' due to %d jobs failing in it"
                           % (k, v[2]))
                doRemove = False
        elif keepDirs == KEEP_ALL:
            myLog.info("Keeping work directory '%s'" % (k))
            doRemove = False
        if doRemove:
            myLog.debug("Removing work directory '%s'" % (k))
            shutil.rmtree(k)

    myLog.info("---- Done ----")
    if len(allJobsStatus['fail']) > 0 or len(allJobsStatus['blocked']) > 0:
        return 1
    return 0

# Starts the program
if __name__ == '__main__':
    sys.exit(main())
