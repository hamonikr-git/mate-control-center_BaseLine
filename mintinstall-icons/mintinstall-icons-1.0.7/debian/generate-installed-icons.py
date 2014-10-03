import os, sys

folder = sys.argv[1]

os.system("rm -rf %s/usr/share/linuxmint/mintinstall/installed/*" % folder)

for file in os.listdir(folder + "/usr/share/linuxmint/mintinstall/icons"):	
	print "Adding installed emblem to %s" % file
	os.popen("composite -gravity south-east %(folder)s/usr/share/linuxmint/mintinstall/emblem-installed.png %(folder)s/usr/share/linuxmint/mintinstall/icons/%(file)s %(folder)s/usr/share/linuxmint/mintinstall/installed/%(file)s" % {'folder':folder, 'file':file})
