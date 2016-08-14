#!/usr/bin/python3

import os
import subprocess
import time

# repository is mirrored here:
#
# deb http://users.teledisnet.be/ade15809/ stretch main

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
subprocess.check_call(['rsync',
                       'cruft-common_%s_all.deb' % snapshot,
                       'pi@pi:/tmp'],
                      cwd=BASE)
subprocess.check_call(['ssh', 'pi@pi', 'publish', '/tmp/cruft-common_%s_all.deb' % snapshot])

publish = """#!/bin/bash

# gpg-agent
. ~/.profile

cd /var/www/html/repos
reprepro includedeb jessie $1
sitecopy -u pool
sitecopy -u dists"""

for file in ('cruft_%s_amd64.build',
             'cruft_%s_amd64.changes',
             'cruft_%s_amd64.deb',
             'cruft-common_%s_all.deb',
             'cruft-dbgsym_%s_amd64.deb'):
    subprocess.check_call(['rm', '-v', file % snapshot],
                          cwd=BASE)
