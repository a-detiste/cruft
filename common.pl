use strict;
use warnings;

# is_subdir potential_base potential_subdir
# 1 = success ; if potential_subdir is a subdir of potential_base or they are
# the same
# 0 = failure ; otherwise
sub is_subdir($$)
{
	my ($dir, $sub) = @_;
	# compact multiple slashes
	$dir =~ s#/+#/#g;
	$sub =~ s#/+#/#g;
	# strip trailing slash unless it's the root dir
	$dir =~ s#/+$## unless $dir eq '/';
	$sub =~ s#/+$## unless $sub eq '/';
	# "the same"
	return 1 if $dir eq $sub;
	# / - special cases, which would need special treatment below
	# every dir is a subdir of /
	return 1 if $dir eq '/';
	# no dir is parent of / (except for itself, but that was caught above)
	return 0 if $sub eq '/';
	
	# see, if $sub begins with $dir
	if ($dir eq substr($sub, 0, length($dir))) {
		# it does, so there are two possibilities:
		if (substr($sub, length($dir), 1) eq '/') {
			# the char in $sub after $dir is a slash (/dir/sub)
			return 1;
		} else {
			# it's something else (/diranother/sub)
			return 0
		}
	} else {
		# if it's not, then $sub cannot possibly be a subdir of $dir
		return 0;
	}
}

sub ignores
{
	my @lines = `cat /var/spool/cruft/IGNORES`;
	chomp @lines;
	return grep { $_ ne '' } @lines;
}

sub scan_fs
{
	my @lines = `cat /var/spool/cruft/DRIVES`;
	chomp @lines;
	return @lines;
}

sub all_fs
{
	my @all = `mount|cut -d\\  -f3`;
	chomp @all;
	push @all, '/' unless grep { '/' eq $_ } @all;
	return @all if @all;
}

sub noscan_fs
{
	my @all = all_fs;
	my @ret;
	my @do = scan_fs;
	foreach my $a (@all) { push @ret, $a unless grep { $_ eq $a } @do };
	return @ret;
}

sub get_direct_fs($@)
{
	my $path = shift;
	my $found = '';
	foreach my $fs (@_) {
		$fs =~ s#/+#/#g;
		$found = $fs if
			is_subdir($fs, $path)
			and
			length($fs) > length($found);
	}
	return $found || undef;
}

sub prunes($@)
{
	my $path = shift;
	my @prunes;
	foreach my $prune (@_) {
		push @prunes, "-path $prune -prune" if is_subdir($path, $prune);
	}
	return "( ".join(" -or ", @prunes)." ) -or" if @prunes;
	return '';
}

1;
