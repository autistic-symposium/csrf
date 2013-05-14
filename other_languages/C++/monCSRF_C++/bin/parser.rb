#!/usr/bin/env ruby

require File.expand_path('lib/log_parser')

# This is the main file that will run the parser on a log file of our choice..
# Usage: we can either run it or calculate benchmark. For this second option the argument is -b.
# The name of the log is the first argument.
# It calls the class lib/runtime.rb.

benchmark = false
benchmark = true if ARGV[1] == '-b'
Runtime.run(ARGV[0], benchmark)
