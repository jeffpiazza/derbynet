#! /usr/bin/perl

# The logs from derby-timer.jar include details of polling for the timer's start
# gate (for those timers that report the start gate state).  This scripts
# filters out the majority of that polling, to make it easier to see the rest of
# derby-timer.jar's interactions.

# State:
# 0  START
# 1  Have read command
# 2  Have read command and reply
# 3  Have read command, reply, and matched command

my $state = 0;

sub process_line {
    my $line = shift;
    if ($state == 0) {  # Start
        if ($line =~ /^+.*S.*--> (.*)$/) {
            $cmd = $1;
            $state = 1;
        }
        print "$line\n";
    } elsif ($state == 1) {  # Have read command
        if ($line =~ /^+.*S.*<-- (.*)$/) {
            $reply = $1;
            $state = 2;
            $count = 0;
            print "$line\n";
        } else {
            $state = 0;
            process_line($line);
        }
    } elsif ($state == 2) {  # Have read command and reply
        if ($line =~ /^+.*S.*--> (.*)$/ && $1 eq $cmd) {
            $state = 3;
            $held = $line;
        } else {
            if ($count > 0) {
                print "... $count more exchanges like this\n";
            }
            $state = 0;
            process_line($line);
        }
    } else {  # Have read command and reply and matched command
        if ($line =~ /^+.*S.*<-- (.*)$/ && $1 eq $reply) {
            $count = $count + 1;
            $state = 2;
        } else {
            if ($count > 0) {
                print "... $count more exchanges like this\n";
            }
            $state = 0;
            process_line($held);
            process_line($line);
        }
    }
}

while (my $line = <>) {
    chomp $line;
    process_line($line);
}
