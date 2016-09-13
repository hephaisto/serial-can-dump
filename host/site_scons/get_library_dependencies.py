import re
import subprocess
import os
import SCons.Script as scons
ADMINDIR="/var/lib/dpkg/info"
regex_objdump=re.compile("\s*NEEDED\s+(.*)$")
regex_libname=re.compile("^(.+?) ([\d\.]+) (.+)$")
TARGET_ARCHITECTURE="amd64"

package_buffer={}

def modify_libname(name):
    return name.replace("_","-")

def build_library_dependencies(target, source, env):
    deps=set()
    for s in source:
        try:
            this_packages=get_library_dependencies(str(s))
            deps=deps.union(this_packages)
        except: pass
    target[0].write(", ".join(deps))

def get_library_dependencies(filename):
    """Returns a list of packages that includes all shared objects included from the given file. Items are formatted for ready use in a "Depends:" field in debian/control files, e.g. they might have version info like (>=1.0.0) attached."""
    # get all shared objects linked from the executable
    libfiles=[]
    out,err=subprocess.Popen(["objdump","-p",filename],stdout=subprocess.PIPE).communicate()
    for line in out.split("\n"):
        match=regex_objdump.match(line)
        if match:
            libfiles.append(match.group(1))
    
    # get package for shared objects
    packages=set()
    for libfile in libfiles:
        try:
            packages.add(get_package_for_libfile(libfile))
        except:
            pass
    
    return packages


def get_package_for_libfile(libfile):
    libname,version=libfile.split(".so.")

    # already found?
    if (libname,version) in package_buffer:
        return package_buffer[(libname,version)]

    # try different shlib info files
    shlibs_file="{}/{}{}.shlibs".format(ADMINDIR,modify_libname(libname),version)
    if not os.path.exists(shlibs_file):
        shlibs_file="{}/{}{}:{}.shlibs".format(ADMINDIR,modify_libname(libname),version,TARGET_ARCHITECTURE)
        if not os.path.exists(shlibs_file):
            raise Exception("no suitable shlibs file found for {} version {}".format(libname,version))

    # parse all entries of shlibs file
    with open(shlibs_file,"r") as f:
        for line in f.readlines():
            match=regex_libname.match(line)
            if match:
                package_buffer[(match.group(1),match.group(2))]=match.group(3)

    # actually check for the queried package
    try:
        return package_buffer[(libname,version)]
    except KeyError:
        raise Exception("package {} version {} not found in {}".format(libname,version,shlibs_file))

