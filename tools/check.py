#!/usr/bin/python3

import os
import subprocess

current_packages = set()
proc = subprocess.Popen(['apt-cache', 'pkgnames'],
                        universal_newlines=True,
                        stdout=subprocess.PIPE)
for line in proc.stdout:
    current_packages.add(line.rstrip())

# Add RaspBian
current_packages.add('raspberrypi-bootloader')
current_packages.add('libraspberrypi0')

# Google repository
current_packages.add('google-earth-stable')

# Add Stable, remove cache file to trigger download
if not os.path.isfile('tools/Packages_amd64'):
    subprocess.check_call(['ben', 'download',
                           '--archs', 'amd64',
                           '--suite', 'stable'],
                          cwd='tools')
    os.unlink('tools/Sources')

with open('tools/Packages_amd64', 'r', encoding='utf8') as p:
    for line in p:
        if line.startswith('Package:'):
            current_packages.add(line.split(':', 1)[1].strip())


filters = set()
proc = subprocess.Popen(['ls', 'filters-unex'],
                        universal_newlines=True,
                        stdout=subprocess.PIPE)
for line in proc.stdout:
    filters.add(line.rstrip())


explain = set()
proc = subprocess.Popen(['ls', 'explain'],
                        universal_newlines=True,
                        stdout=subprocess.PIPE)
for line in proc.stdout:
    if line == line.lower():
        explain.add(line.rstrip())

unknown_filters = filters - current_packages
print('Filters for unknown packages', sorted(unknown_filters))

unknown_explain = explain - current_packages
print('Explain scripts for unknown packages', sorted(unknown_explain))
