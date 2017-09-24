#!/usr/bin/python3

import os
import subprocess
import time

# repository is mirrored here:
#
# deb http://users.teledisnet.be/ade15809/ stretch main

if os.uname().nodename == 'pi':
  # avoid wearing SD card + speed-up
  BASE = '/tmp'
  subprocess.check_call(['rsync', '/home/pi/cruft', '/tmp/',
                         '-rti',
                         '--exclude=*.o',
                         '--exclude=.debhelper',
                         '--exclude=debian/cruft',
                         '--exclude=debian/cruft-common',
                         '--exclude=debian/tmp'])
else:
  BASE = '/home/tchet/git'

CRUFT = os.path.join(BASE, 'cruft')

subprocess.check_call(['git', 'checkout', 'debian/changelog'], cwd=CRUFT)

version = subprocess.check_output(['dpkg-parsechangelog',
                                   '-l', os.path.join(CRUFT, 'debian/changelog'),
                                   '-S', 'Version'],
                                  universal_newlines=True).strip()
today = time.strftime('%Y%m%d%H%M')
snapshot = version + '~git' + today

subprocess.check_call(['dch', '-b',
                       '-v', snapshot,
                       "Git snapshot"],
                      cwd=CRUFT)
subprocess.check_call(['debuild', '-us', '-uc', '-b'], cwd=CRUFT)
subprocess.check_call(['git', 'checkout', 'debian/changelog'], cwd=CRUFT)

DEB = 'cruft-common_%s_all.deb' % snapshot

if os.uname().nodename == 'pi':
    subprocess.check_call(['reprepro', 'includedeb', 'stretch', os.path.join('/tmp', DEB)],
                          cwd='/var/www/html/repos')
    subprocess.call(['sitecopy', '-u', 'pool'])
    subprocess.call(['sitecopy', '-u', 'dists'])
else:
    subprocess.check_call(['rsync', os.path.join(BASE, DEB), 'pi@pi:/tmp'])
    subprocess.check_call(['ssh', '-t', 'pi@pi', 'publish', os.path.join('/tmp', DEB)])

for file in ('cruft_%s_a*.build',
             'cruft_%s_a*.buildinfo',
             'cruft_%s_a*.changes',
             'cruft_%s_a*.deb',
             'cruft-common_%s_all.deb',
             'cruft-dbgsym_%s_a*.deb'):
    subprocess.check_call(['rm', '-v', file % snapshot],
                          cwd=BASE)
