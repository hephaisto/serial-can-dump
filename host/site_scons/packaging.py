import SCons.Script as scons
from SCons.Node.Python import Value
import os
from get_library_dependencies import build_library_dependencies

PACKAGE_TMP_DIR="packages"

def make_debian(packagename, files, env, additional_dependencies=""):
	package_file="{}/{}-{}-{}.deb".format(PACKAGE_TMP_DIR,packagename, env["VERSION_STRING"], env["TARGET_ARCH"])
	package_folder=scons.Dir("{}/{}".format(PACKAGE_TMP_DIR,packagename))

	# dirty workaround for http://article.gmane.org/gmane.comp.programming.tools.scons.user/19939
	class DependencyStringWrapper(Value):
		def __init__(*args,**kwargs):
			Value.__init__(*args,**kwargs)
		def get_text_contents(self):
			return self.read()
	dependency_string=DependencyStringWrapper("")
	
	env.Command(target=dependency_string, source=[f[0] for f in files], action=build_library_dependencies)

	controlfile=env.Textfile(target="{}/DEBIAN/control".format(package_folder), source=
		[
			env.File("{}/{}.template".format(PACKAGE_TMP_DIR,packagename)),
			"Version: $VERSION_STRING\nArchitecture: $TARGET_ARCH\nDepends: ",
			dependency_string,
			additional_dependencies,
			"\n"
		],TEXTFILESUFFIX="",LINESEPARATOR="")
	

	package_files=[env.Command("{}{}".format(package_folder,f[1]),f[0],scons.Copy("$TARGET","$SOURCE")) for f in files]
	package_dependencies=[package_folder]+package_files+[controlfile
			,scons.Glob("{}/DEBIAN/*".format(package_folder))
			]
	package_file_node=env.Command(package_file, package_dependencies, "fakeroot dpkg-deb --build $SOURCE $TARGET".format(package_folder,package_file))
	return package_file_node
