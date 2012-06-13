#!/usr/bin/perl

use strict;
use utf8;
use FileHandle;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

if((@ARGV != 5) and (@ARGV != 6)) {
    print STDERR "Usage: $0 INF INE ANSWER OUTF OUTE [CORRECT]\n";
    exit 1;
}

open INF, "<:utf8", $ARGV[0] or die "Could not open $ARGV[0]";
open INE, "<:utf8", $ARGV[1] or die "Could not open $ARGV[1]";
open INA, "<:utf8", $ARGV[2] or die "Could not open $ARGV[2]";
open OUTF, ">:utf8", $ARGV[3] or die "Could not open $ARGV[3]";
open OUTE, ">:utf8", $ARGV[4] or die "Could not open $ARGV[4]";
my $CORRECT = ($ARGV[5] ? $ARGV[5] : 1);

my ($f, $e, $a);
while(($f = <INF>) and ($e = <INE>) and ($a = <INA>)) {
    chomp $a;
    if($a == $CORRECT) {
        print OUTF $f;
        print OUTE $e;
    }
}
