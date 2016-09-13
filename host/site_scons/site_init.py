import re
import subprocess
def git_tag_version(env):
	pipe=subprocess.Popen(["git","describe"],stdout=subprocess.PIPE)
	stdout,stderr=pipe.communicate()
	version_string=stdout.split("\n")[0]
	version_regex=re.compile(r"(\d+)\.(\d+).(\d+)(?:-(\d+)(?:-(\w+)))?")
	version_match=version_regex.match(version_string)

	# version from git-describe
	if version_match:
		CPPDEFINES=dict(
			VERSION_MAJOR=version_match.group(1),
			VERSION_MINOR=version_match.group(2),
			VERSION_PATCH=version_match.group(3),
			VERSION_HASH="" if version_match.group(5) is None else version_match.group(5)
		)
	# use "custom" if failed
	else:
		CPPDEFINES=dict(
			VERSION_MAJOR="0",
			VERSION_MINOR="0",
			VERSION_PATCH="0",
			VERSION_HASH="custom"
		)
	if CPPDEFINES["VERSION_HASH"] == "":
		VERSION_STRING='{VERSION_MAJOR}.{VERSION_MINOR}.{VERSION_PATCH}'.format(**CPPDEFINES)
	else:
		VERSION_STRING='{VERSION_MAJOR}.{VERSION_MINOR}.{VERSION_PATCH}-{VERSION_HASH}'.format(**CPPDEFINES)
		
	env.Append(VERSION_STRING=VERSION_STRING)
	print("building version: {}".format(env["VERSION_STRING"]))
	CPPDEFINES["VERSION_STRING"]='"{}"'.format(VERSION_STRING)
        env.Append(SUBST_DICT={"%{}%".format(k):v for k,v in CPPDEFINES.iteritems()})

def get_architecture_from_dpkg(env):
	if env["TARGET_ARCH"] is None:
		arch = subprocess.check_output(["dpkg","--print-architecture"])
		env["TARGET_ARCH"]=arch[:-1]
		print("TARGET_ARCH was not set. Reading from dpkg: {}".format(env["TARGET_ARCH"]))

