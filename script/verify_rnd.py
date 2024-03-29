import inspect
import logging
import random
import re
import subprocess
import sys

logger = logging.getLogger()
logger.addHandler(logging.StreamHandler(sys.stdout))
logger.setLevel(logging.DEBUG)

# numbers larger than 2000 silently fail
sys.setrecursionlimit(2000)

subprocess.check_call("make")


def dbg(a):
    info = inspect.getframeinfo(inspect.currentframe().f_back)
    logger.debug(f"{info.filename}:{info.function}:{info.lineno}: {repr(a)}")


def remove(s, i):
    s = list(s)
    del s[i]
    return s


def check_tuples(x):
    if isinstance(x, tuple):
        for y in x:
            check_tuples(y)
        return
    if isinstance(x, list):
        raise ValueError(x)


def imp(x, y):
    return "|", ("~", x), y


def size(x):
    if type(x) in (list, tuple):
        n = 0
        for y in x:
            n += size(y)
        return n
    return 1


######################################## printing

outf = None


def pr(x):
    if x is not str:
        x = str(x)
    outf.write(x)


def prargs(x):
    pr("(")
    for i in range(1, len(x)):
        if i > 1:
            pr(",")
        prterm(x[i])
    pr(")")


def need_parens(x, parent):
    if not parent:
        return
    if x[0] in ("&", "<=>", "|"):
        return parent[0] in ("&", "<=>", "?", "!", "~", "|")


def prterm(x, parent=None):
    if isinstance(x, tuple):
        o = x[0]
        # infix
        if o == "=":
            prterm(x[1])
            pr("=")
            prterm(x[2])
            return
        if o in ("&", "<=>", "|"):
            if need_parens(x, parent):
                pr("(")
            assert len(x) == 3
            for i in range(1, len(x)):
                if i > 1:
                    pr(f" {o} ")
                prterm(x[i], x)
            if need_parens(x, parent):
                pr(")")
            return

        # prefix/infix
        if o == "~":
            pr("~")
            prterm(x[1], x)
            return

        # prefix
        if o in ("?", "!"):
            pr(o)
            pr("[")
            v = x[1]
            for i in range(len(v)):
                if i:
                    pr(",")
                y = v[i]
                pr(y)
            pr("]:")
            prterm(x[2], x)
            return
        pr(o)
        prargs(x)
        return
    pr(x)


formnames = 0


def prformula(x):
    global formnames
    formnames += 1
    pr("tff")
    pr("(")

    # name
    pr(formnames)
    pr(", ")

    # role
    pr("plain")
    pr(", ")

    # content
    prterm(x)

    # end
    pr(").\n")


def write_tmp(xs):
    global formnames
    global outf
    formnames = 0
    outf = open("tmp.p", "w")
    for x in xs:
        prformula(x)
    outf.close()


######################################## make terms


def atom():
    atoms = ["$true", "$false", "a", "b", "c", "d"]
    return random.choice(atoms)


def mk(depth):
    if depth == 0 or random.randrange(100) < 30:
        return atom()
    ops = {
        "<=>": 2,
        "&": 2,
        "|": 2,
        "~": 1,
    }
    o = random.choice(list(ops.keys()))
    n = ops[o]
    r = [o]
    for i in range(n):
        r.append(mk(depth - 1))
    return tuple(r)


######################################## top level


def solved(r):
    if r == "Unsatisfiable":
        return 1
    if r == "Satisfiable":
        return 1


def run_eprover(filename):
    cmd = ["bin/eprover", "--generated-limit=10000", filename]
    p = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    stdout, stderr = p.communicate()
    stdout = str(stdout, "utf-8")
    stderr = str(stderr, "utf-8")
    if stderr:
        print(stderr, end="")
        exit(1)
    if p.returncode not in (0, 1, 8):
        raise Exception(str(p.returncode))
    if "Proof found" in stdout:
        return "Unsatisfiable"
    if "No proof found" in stdout:
        return "Satisfiable"


def run_ayane(filename):
    cmd = ["./ayane", "-t3", filename]
    p = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    stdout, stderr = p.communicate()
    stdout = str(stdout, "utf-8")
    stderr = str(stderr, "utf-8")
    if stderr:
        print(stderr, end="")
        exit(1)
    if p.returncode:
        raise Exception(str(p.returncode))
    m = re.search(r"SZS status (\w+) for \w+", stdout)
    if not m:
        raise Exception(stdout)
    return m[1]


def check(xs):
    write_tmp(xs)
    r_eprover = run_eprover("tmp.p")
    if not solved(r_eprover):
        return

    write_tmp(xs)
    r_ayane = run_ayane("tmp.p")
    if not solved(r_ayane):
        return

    if r_eprover != r_ayane:
        print(r_eprover)
        print(r_ayane)
        exit(1)


for i in range(10000):
    xs = [mk(5)]
    print(xs)
    check(xs)
