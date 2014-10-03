import os, sys

folder = sys.argv[1]

for file in os.listdir(folder + "/usr/share/linuxmint/mintinstall/icons"):
	print "Converting %s to 32x32" % file
	os.popen("convert %(folder)s/usr/share/linuxmint/mintinstall/icons/%(file)s -resize 32x32 %(folder)s/usr/share/linuxmint/mintinstall/icons/%(file)s" % {'folder':folder, 'file':file})
