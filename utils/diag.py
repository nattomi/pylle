import sys
import os.path

if len(sys.argv) == 1:  # display help
    helpmsg = "Basic data diagnostic tool\n"
    helpmsg += "Usage:\npython "
    helpmsg += sys.argv[0] + "/path/to/cso_dir tol"
    sys.exit(helpmsg)
else:
    csodir = sys.argv[1]
    if 3 <= len(sys.argv):
        tol = sys.argv[2]
    else:
        tol = 100 # fallback to default value

i = 0
gap = 0
missing = 0
while True:
    basename = "%08d.cso" % i
    filepath = os.path.join(csodir,basename)
    cond_fail = not os.path.exists(filepath) or not os.path.isfile(filepath)
    if cond_fail:
        missing += 1
        gap += 1
        if gap > tol:
            print "Exceeded tol value of " + str(tol) + ", I give up searching"
            print "Found " + str(missing - tol - 1) + " files missing" 
            break
        else: pass
    else:
        print basename
        if gap > 0:
            gap = 0
    i += 1
