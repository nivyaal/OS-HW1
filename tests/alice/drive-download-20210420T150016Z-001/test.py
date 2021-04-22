import signal
import psutil
import sys
import time
import os

pidchange = 1
quitkill = False

def getProcess(name):
    for p in psutil.process_iter(attrs=['name']):
        try:
            if name == p.info['name']:
                return p
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            pass
    return None

def add(pid):
    if pid not in pids:
        global pidchange
        pids[pid] = pidchange
        pidchange = pidchange + 1

def pid_replace(line, idx):
    sline = line.split()
    badpid = sline[idx].strip(":")
    add(badpid)
    if quitkill:
        s = str(pids[badpid]) + ":"
        return line.replace(sline[idx], s)
    return line.replace(str(badpid), str(pids[badpid]))

def parse_output(i):
    out_file = open("test{0}.out".format(i), 'w')

    res = output[i].splitlines()

    quitkill = False
    for line in res:
        if len(line.rstrip()) == 0: continue
        n = line
        if " secs (stopped)" in line:
            n = pid_replace(line, -4)
        elif " secs" in line:
            n = pid_replace(line, -3)
        elif "was sent to pid" in line:
            n = pid_replace(line, -1)
        elif "smash pid" in line:
            n = pid_replace(line, -1)
            out_file.write(
                "vvv showpid should say {0} vvv\n".format(pids[str(pid)]))
        elif " : " in line:
            n = pid_replace(line, -1)
        elif " pts/" in line:
            n = pid_replace(line, 0)
        elif "was stopped" in line or "was killed" in line:
            n = pid_replace(line, -3)
        elif "sending SIGKILL signal to " in line:
            quitkill = True
        elif quitkill:
            n = pid_replace(line, 0)
        n = n + '\n'
        out_file.write(n)

    out_file.close()
    with open("out", 'w') as op:
        op.write('')


def run_test(i):
    in_file = open("test{0}.in".format(i), 'r')
    while True:
        line = in_file.readline()
        if not line:
            break
        if "ctrlz" in line:
            smash.send_signal(signal.SIGTSTP)
            time.sleep(1)
            continue
        if "ctrlc" in line:
            smash.send_signal(signal.SIGINT)
            time.sleep(1)
            continue
        sys.stdout.write(line)
        sys.stdout.flush()
        time.sleep(1)
    in_file.close()
    time.sleep(2)

smash = getProcess("smash")
assert smash != None
pid = smash.pid
pids = {}
add(str(pid))

out_original = open("out", 'r')
output = {}

for test in range(1,8):
    run_test(test)
    output[test] = out_original.read()
    open("out", 'w').close()

for test in range(1,8):
    parse_output(test)

smash.wait()
os.remove("out")
