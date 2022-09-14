# functions shared by (legacy) cruft and cruft-ng
. /usr/lib/cruft/common.sh

# all functions moved here are not needed anymore by cruft-ng
cruft_debug()
{
	local min_level=${1:-1}
	if [ -n "$CRUFT_DEBUG" ] && [ "$CRUFT_DEBUG" -ge $min_level ]; then
		# dash turns into a forkbomb with this :-/
		#PS4='$(date +\>[%Y-%m-%d\ %H:%M:%S.%N])'" [$min_level] "
		set -x
		ulimit -c unlimited
	fi
}

# print default list of all mounted filesystems to scan
cruft_default_scan_fs()
{
	mount | grep -v -E " type (proc|devpts|nfs[1234]?|ncp|coda|(a|smb|ci|ncp|usb|tmp|sys|devtmp|debug|config|auto|security|hugetlb|selinux|trace)fs|fusectl|cgroup|mqueue|pstore|binfmt_misc)" | cut -d\  -f3
}

# set list of fs to scan, for other programs
set_cruft_scan_fs()
{
	: > /var/spool/cruft/DRIVES
	for x in "$@"; do echo $x >> /var/spool/cruft/DRIVES; done
}

# print all mounted filesystems
cruft_all_fs()
{
	mount | cut -d\  -f3
}

# print all mounted filesystems to scan. Obeys what set_scan_drives has set.
cruft_scan_fs()
{
	cat /var/spool/cruft/DRIVES
}

# print all mounted filesystems not to scan. Obeys what set_scan_drives has set.
cruft_noscan_fs()
{
	for fs in $(cruft_all_fs); do
		local yes=0
		for yesfs in $(get_cruft_scan_fs); do
			if [ "$fs" = "$yesfs" ]; then yes=1; fi
		done
		[ "$yes" = "1" ] || echo "$fs"
	done
}

add_prune()
{
	local prunes="$1"; shift
	local ignore="$1"; shift

	if [ -n "$prunes" ]; then
		echo "${prune} -or -wholename $ignore -prune"
	else
		echo "-wholename $ignore -prune"
	fi
}

finish_prunes()
{
	if [ -n "$1" ] ; then
		echo "( $1 ) -or"
	else
		echo
	fi
}

get_prunes_for()
{
	local drive="$1"
	local prune=""
	for ignore in $(get_ignores)
	do
		if is_subdir "$ignore" "$drive"; then
			# $DRIVE is a subdir of $IGNORE
			# no need to scan the drive at all
			echo skip
			return
		elif is_subdir "$drive" "$ignore"; then
			# $IGNORE is a subdir of $DRIVE
			# add it to prune list
			prune=$(add_prune "${prune}" "${ignore}")
		fi
	done
	finish_prunes "${prune}"
}

get_ignores()
{
	if [ -e /var/spool/cruft/IGNORES ] ; then
	    cat /var/spool/cruft/IGNORES
	fi
}

set_ignores()
{
	: > /var/spool/cruft/IGNORES
	for x in "$@"; do echo $x >> /var/spool/cruft/IGNORES; done
}


# This function checks if, and how the argument should be scanned, depending on
# current DRIVES and IGNORES, and either does nothing or runs find command(s)
# suffixed with appropriate options
cruft_find()
{
	exec /usr/lib/cruft/cruft_find "$@"
}

fixup_slashes()
{
	sed 's:/\.$:/:;s:/$::;s:^$:/:'
}

package_has_script()
{
	local pkg="$1"
	local script="$2"
	local ctrl_path_tmp=$(mktemp)
	if ! dpkg-query --control-path "${pkg}" "${script}" >"${ctrl_path_tmp}" 2>/dev/null
	then
		rm -f "${ctrl_path_tmp}"
		# error, most likely ${pkg} is not installed
		return 1
	else
		lines=$(wc -l < "${ctrl_path_tmp}")
		rm -f "${ctrl_path_tmp}"
		if [ "${lines}" -eq 0 ]
		then
			# no path returned
			return 1
		else
			return 0
		fi
	fi
}

package_has_files()
{
	local pkg="$1"
	local list_tmp=$(mktemp)
	if ! dpkg-query --listfiles "${pkg}" >"${list_tmp}" 2>/dev/null
	then
		rm -f "${list_tmp}"
		# error, most likely ${pkg} is not installed
		return 1
	else
		lines=$(wc -l < "${list_tmp}")
		rm -f "${list_tmp}"
		if [ "${lines}" -eq 0 ]
		then
			# has no files
			return 1
		else
			return 0
		fi
	fi
}

package_installed()
{
	local pkg="$1"
	package_has_script "${pkg}" prerm ||
	package_has_script "${pkg}" postrm ||
	package_has_files "${pkg}"
}

# return 0 if file with that name is to be processed
# return 1 if it is to be skipped
process_package()
{
	local name="$1"
	# Check if it is a package name
	if [ "${name}" = "$(echo ${name} | tr '[:lower:]' '[:upper:]')" ] ; then
		# All UPPER_CASE, this is an exception, do not require a
		# package to be installed, process the file unconditionally
		return 0
	elif package_installed "${name}" ; then
		# process a file whose matching package is not completely purged
		return 0
	else
		debug "   skipping ${name} - package not installed"
		return 1
	fi
}
