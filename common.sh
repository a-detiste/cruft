
# print default list of all mounted filesystems to scan
cruft_default_scan_fs()
{	
	mount | egrep -v " type (proc|devpts|nfs[1234]?|ncp|coda|(a|smb|ci|ncp|usb|tmp|sys)fs)" | cut -d\  -f3
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

# Checks whether a dir is a subdir of another
# usage:
# is_subdir potential_base potential_subdir
#  0 = success ; if potential_subdir is a subdir of potential_base or they are the same
#  1 = failure ; otherwise
is_subdir()
{
	local dir="$1";shift
	local sub="$1";shift
	# remove trailing slash, unless the dir is root dir itself
	[ / != "$dir" ] && dir="${dir%/}"
	[ / != "$sub" ] && sub="${sub%/}"

	# $sub is the same as $dir
	[ "$dir" = "$sub" ] && return 0

	# / - special cases, which would need special treatment below
	# every dir is a subdir of /
	[ / = "$dir" ] && return 0
	# no dir is parent of / (except for itself, but that was caught above)
	[ / = "$sub" ] && return 1

	# try to remove $dir from beginning of $sub
	local trail="${sub##$dir}"

	if [ "$sub" = "$trail" ] ; then
		# since stripping did not succeed (no change)
		# then $sub does not begin with $dir
		return 1
	else
		# $sub begins with $dir
		# There are two possibilities
		if [ "${trail##/}" != "$trail" ] ; then
			# $tail begins with a slash
			# /dir/sub
			return 0
		else
			# $tail does not begin with slash, so it is not below $dir
			# /diranother/sub
			return 1
		fi
	fi
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

get_ignores()
{
	cat /var/spool/cruft/IGNORES
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
	/usr/lib/cruft/cruft_find "$@"
}

