#!/usr/bin/env python

# Generate the uvw coordinates for all AARTFAAC (zenith-pointing) baselines
# given an input correlator parset.

import sys
import re
from itertools import permutations
from pyrap.measures import measures

def parse_parset(filename):
    reference_position = None
    positions = {}
    for line in open(filename, 'r'):
        line = line.strip()
        if not line:
            # Skip blank lines
            continue
        if line[0] == "#":
            # Skip comments
            continue
        key, value = [x.strip() for x in line.split("=")]
        if key == "Observation.referencePhaseCenter":
            reference_position = eval(value)
        m = re.match(r"PIC\.Core\.(.*)\.position", key)
        if m:
            positions[m.groups()[0]] = eval(value)
    return reference_position, positions

def calculate_uvw(dm, pos1, pos2):
    baseline = dm.baseline(
        "ITRF",
        "%fm" % (pos1[0]-pos2[0]),
        "%fm" % (pos1[1]-pos2[1]),
        "%fm" % (pos1[2]-pos2[2])
    )
    return dm.to_uvw(baseline)['xyz']


if __name__ == "__main__":
    parset_name = sys.argv[1]
    ref, posns = parse_parset(parset_name)

    dm = measures()
    dm.do_frame(dm.epoch("UTC", "today"))
    dm.do_frame(dm.direction("AZEL", "0deg", "90deg"))
    dm.do_frame(dm.position("ITRF", "%fm" % ref[0], "%fm" % ref[1], "%fm" % ref[2]))

    for ant1, ant2 in permutations(posns.keys(), 2):
        print "%s %s %s" % (ant1, ant2, str(calculate_uvw(dm, posns[ant1], posns[ant2])))
