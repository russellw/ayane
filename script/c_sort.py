import re

import common


# SORT
def block(v, dent, i):
    # end
    if common.indent(v, i) < dent or re.match(r"\s*//$", v[i]):
        return

    # skip comments
    while not re.match(r"\s*//$", v[i]) and re.match(r"\s*//", v[i]):
        i += 1

    # there should be no more leading blank lines
    if i < len(v):
        assert v[i]

    # braced block, probably a function definition
    if v[i].endswith("{"):
        while not (common.indent(v, i) == dent and re.match(r"\s*}$", v[i])):
            i += 1

    return i + 1


def f(v):
    i = 0
    while i < len(v):
        if not re.match(r"\s*// SORT$", v[i]):
            i += 1
            continue

        dent = common.indent(v, i)
        i += 1

        j = i
        r = []
        while 1:
            while j < len(v) and not v[j]:
                j += 1
            k = block(v, dent, j)
            if not k:
                break
            r.append(v[j:k])
            j = k
        assert r

        r.sort(key=key)

        # if we have at least one multiline block, separate them by blank lines
        for w in r:
            if len(w) > 1:
                for w in r[:-1]:
                    w.append("")
                break

        v[i:j] = common.cat(r)

        i = j + 1


def key(v):
    for s in v:
        if not re.match(r"\s*//", s):
            return s
    raise Exception(v)


common.modify_files(f, common.args_c_files())
