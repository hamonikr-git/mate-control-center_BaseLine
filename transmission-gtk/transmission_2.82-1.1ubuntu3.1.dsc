-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: transmission
Binary: transmission, transmission-common, transmission-dbg, transmission-cli, transmission-gtk, transmission-qt, transmission-daemon
Architecture: any all
Version: 2.82-1.1ubuntu3.1
Maintainer: Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>
Homepage: http://www.transmissionbt.com/
Standards-Version: 3.9.5
Vcs-Browser: http://anonscm.debian.org/gitweb/?p=collab-maint/transmission.git
Vcs-Git: git://anonscm.debian.org/collab-maint/transmission.git
Build-Depends: debhelper (>= 8), autotools-dev, dh-autoreconf, dh-systemd [linux-any], libgtk-3-dev, libevent-dev (>= 2.0), libglib2.0-dev, libnotify-dev (>= 0.7), libssl-dev, zlib1g-dev, libcurl4-gnutls-dev | libcurl4-dev | libcurl-dev, intltool (>= 0.40), libappindicator3-dev, qt5-qmake, qtbase5-dev, qttools5-dev-tools, qt5-default, libminiupnpc-dev, libnatpmp-dev (>= 20110808), libsystemd-daemon-dev [linux-any], dpkg-dev (>= 1.16.1~)
Package-List: 
 transmission deb net optional
 transmission-cli deb net optional
 transmission-common deb net optional
 transmission-daemon deb net optional
 transmission-dbg deb debug extra
 transmission-gtk deb net optional
 transmission-qt deb net optional
Checksums-Sha1: 
 1081542e2692147e45dde7c310b793abc4f33f2d 3172024 transmission_2.82.orig.tar.xz
 4cedc711f8201aaa0859eacf7c7c9f22fc0eb954 22181 transmission_2.82-1.1ubuntu3.1.debian.tar.gz
Checksums-Sha256: 
 3996651087df67a85f1e1b4a92b1b518ddefdd84c654b8df6fbccb0b91f03522 3172024 transmission_2.82.orig.tar.xz
 ca071676a4f8e9a2f084e3e52044a5af809e7f48c526ffc7ca0ec5fb7a076707 22181 transmission_2.82-1.1ubuntu3.1.debian.tar.gz
Files: 
 a5ef870c0410b12d10449c2d36fa4661 3172024 transmission_2.82.orig.tar.xz
 60049e617c9d9c8ce40f61b59ad8a854 22181 transmission_2.82-1.1ubuntu3.1.debian.tar.gz
Original-Maintainer: Leo Costela <costela@debian.org>

-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1

iQIcBAEBCgAGBQJTwDDKAAoJEGVp2FWnRL6TwPAP/2OQgXm7UEiQitqNKeYBzq0C
bhggr3UhAtvwZfRj34CpBZCI7a/YxSOkbLioDAd+W/qlrLxqGhP0CMLTeCm47qy1
okDP4pxmiCun9cCfzNoFWfsjTQ9xIMuhmW1Rqnv0LGDDsIGqw/KwRaBaTy8mg5k1
UEDcxCjJgTGjufbYqNWMO04XrFP2hJK8BXT2WPIetajU81bOXYHoQZQnSFuEVYUA
1y2drXevMyxImdpXFDHAoE7y/f2f7PRMQA1fsQWOXTLuiK3pFpg15zbclpXlDShs
dPBAFf3+qnKW4e3UDOpj8wIwdvXH7kQ7fyBiI9lc35VfZsOjL+rH7qTek9ahXsqs
EdYpO5pijSeDzDUwcr05URaCV8nCIqykOBnfR4cTc3In7JIw10AnPMXkkhtqMSLA
kuVBJxXSZ5DTkHHaJrwkSS3yx3vojWU/INuuDcVUrvg8oRSgufS5T6kjcwHtlLwJ
oQ7bM3YELgZ7uU4YGamPTA4YnZX1EiO/ZfESiC7ljh9IztTFCxwB4SDtVDG+Wh6D
eYvMlZOahbJ1SCXGpR5N1gqQDMbOfJU/nIr7vxN3491zCIOfzz4W5OHj00GNVvmK
W1z6U2NqlKtK46cjBkPAvWeopfhbQ1NSUq3RdTJ7rv9zv/GDIpIEyxAKqE4/81MG
HmAPoU6Af5mnUFCebhVj
=BVRP
-----END PGP SIGNATURE-----
