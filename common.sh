
# usage:
# is_subdir potential_base potential_subdir
#  0 = success ; if potential_subdir is a subdir of potential_base
#  1 = failure ; otherwise
is_subdir()
{
	dir="$1";shift
	sub="$1";shift
	# remove trailing slash
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
	trail="${sub##$dir}"

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
	prunes="$1"; shift
	ignore="$1"; shift
	
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
	echo "$@" > /var/spool/cruft/IGNORES
}


# This function checks if, and how the argument should be scanned, depending on
# current DRIVES and IGNORES, and echoes a no-op or a find command suffixed
# with appropriate options
cruft_find()
{
	local arg="$1"
	shift
	if [ -n "$1" ]; then
		echo "cruft_find expects only one argument" >&2
	fi
	
	PRUNES=""
	for IGNORE in $(get_ignores); do
		if is_subdir "$IGNORE" "$FTPDIR"; then
			echo ":" "$arg"
			return
		elif is_subdir "$FTPDIR" "$IGNORE"; then
			PRUNES="$(add_prune "$PRUNES" "$IGNORE")"
		fi
	done
	PRUNES=$(finish_prunes "$PRUNES")
	echo find "$arg" $PRUNES
}

