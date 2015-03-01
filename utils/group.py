# ---------
# SETTINGS
# ---------
sRate = 0.087380

import sys
import os.path
import struct
# import binascii

if len(sys.argv) == 1:  # display help
    helpmsg = "Basic data diagnostic tool\n"
    helpmsg += "Usage:\npython "
    helpmsg += sys.argv[0] + "/path/to/cso_dir groupsize"
    sys.exit(helpmsg)
else:
    csodir = sys.argv[1]
    csodir_base = os.path.basename(csodir)
    groupsize = 4 # fallback to default value
    if 2 < len(sys.argv):
        groupsize = int(sys.argv[2])


def dump_group(group):
    print group
    

class buffer:
    def __init__(self, lines):
        l = len(lines)
        if l < 3:
            raise Exception("List doesn't contain any samples")
        try:
            StartTime = float(lines[0].rstrip())
        except ValueError:
            raise Exception("Start time entry is not valid")
        else:
            self.StartTime = StartTime
        if lines[1] != "start\n":
            raise Exception("'start' entry is not valid")
        samples = []
        stop = False
        for i in range(2,l):
            entry = lines[i]
            if entry == "stop\n":
                stop = True
                break
            else:
                try:
                    s = int(entry.split(' ')[1].rstrip())
                except:
                    raise Exception("data entry is not valid")
                else:
                    samples.append(s)
        if stop == False:
            raise Exception("'stop' entry is missing")
        elif len(samples) == 0:
            raise Exception("List doesn't contain any samples")
        self.samples = samples
        self.times = [StartTime + i*sRate for i in range(len(samples))]

  
i = 0
gi = 0
k = 0
new_group = True

while True:
    if k == groupsize:
        k = 0
        gi += 1 
        fbin_open.close()
        new_group = True
    if new_group:
        fbin = csodir_base + ".%08d" % gi
        new_group = False
    basename = "%08d.cso" % i
    f = os.path.join(csodir,basename)
    try:
        f_open = open(f, 'r')
    except IOError:
        fbin_open.close()            
        sys.exit("File " + basename + " does not exist, exiting")
    else:
        lines = f_open.readlines()
        f_open.close()
        b = buffer(lines)
        # group file will be opened only if at least 
        # 1 buffer was read successfully
        if k == 0:  
            fbin_open = open(fbin, 'wb')
        print "Writing " + basename + " to " + fbin
        for j in range(len(b.samples)):
            values = (b.times[j],b.samples[j])
            s = struct.Struct('=d h')
            s_pack = s.pack(*values)
            fbin_open.write(s_pack)
        k += 1
        i += 1
