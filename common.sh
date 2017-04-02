# this function is called by all the scripts in explain/
# but only really useful when called from main cruft pgm
cruft_debug(){
	:
}

debug()
{
	if [ -z "$CRUFT_DEBUG" ] ; then return ; fi
	echo "$(date +">[%Y-%m-%d %H:%M:%S.%N] [0]")" "$@" >&2
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

get_ignores()
{
	:
}

cruft_find()
{
	# stub
	find "$@"
}
