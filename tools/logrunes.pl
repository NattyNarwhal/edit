use strict;
use warnings;

# starting from a trace generated by fuzzlog.pl
# this program computes the number of runes that
# have to be stored in the undo log

my $tot = 0;
while(<>){
	chomp;
	next if(not /-(\d+) (\d+)/); # deletions only
	$tot += $2 - $1;
}

print "Number of runes in log: $tot\n";
