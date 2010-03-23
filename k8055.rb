#!/usr/bin/env ruby

__doc__ = """
Ruby version of K8055 command line program

Copyright (C) 2010 by Nathan D. Broadbent

Syntax : ruby k8055.rb [-p:(number)] [-d:(value)] [-a1:(value)] [-a2:(value)]
             [-num:(number) [-delay:(number)] [-dbt1:(value)]
             [-dbt2:(value)] [-reset1] [-reset2] [-debug] [-h|--help]
	-p:(number)	Set board number (0/1/2/3)
	-d:(value)	Set digital output value (bitmask, 8 bits in decimal)
	-a1:(value)	Set analog output 1 value (0-255)
	-a2:(value)	Set analog output 2 value (0-255)
	-num:(number)   Set number of measures (-1 for no read)
	-delay:(number) Set delay between two measures (in msec)
	-dbt1:(value)   Set debounce time for counter 1 (in msec)
	-dbt2:(value)   Set debounce time for counter 2 (in msec)
	-reset1		Reset counter 1
	-reset2		Reset counter 2
	-debug		Activate debug mode
	-h or --help	Print this text

Example : ruby k8055.rb -p:1 -d:147 -a1:25 -a2:203

NOTE:
	Because of the nature of commands sent to the K8055 board, this
	program has no way of knowing previous state of the analog or
	digial outputs, thus each run of this command will clear the previous
	state - and set a new state - of the analog and digital outputs

	See header of libk8055.c for more details of K8055 commands
"""

require 'rubyk8055'
@r = RubyK8055.new


def main(argv)
	# preset values
	p , db = 0,0		# Port, debug
	r1, r2 = 0,0		# reset1, reset2
	dl = -1			# delay
	db1,db2 = -1,-1		# debounce1, debounce2
	a1,a2,d = -1,-1,-1	# analog1, analog2, digital
	nm = 1			# number of times to read, default once

	if argv.include?("-h") or argv.include?("--help")
		print __doc__
		exit
  end

	# not the standard getopts way, but...
	for arg in argv
		if arg[:4] == "-a1:"
		  a1  = int(arg[4:])
		elsif arg[:4] == "-a2:"
		  a2  = int(arg[4:])
		elsif arg[:5] == "-num:"
		  nm  = int(arg[5:])
		elsif arg[:3] == "-d:"
		  d   = int(arg[3:])
		elsif arg[:3] == "-p:"
		  p   = int(arg[3:])
		elsif arg[:7] == "-delay:"
		  dl  = int(arg[7:])
		elsif arg[:6] == "-dbt1:"
		  db1 = int(arg[6:])
		elsif arg[:6] == "-dbt2:"
		  db2 = int(arg[6:])
		elsif arg     == "-reset1"
		  r1  = 1
		elsif arg     == "-reset2"
		  r2  = 1
		elsif arg     == "-debug"
		  db  = 1
		else
			puts __doc__
			exit
	  end
  end

	begin
		# Open device
		k = RubyK8055
    if k.connect p

		  # set requested
		  k.reset_counter(1) if r1  !=  0
		  k.reset_counter(2) if r2  !=  0
		  k.write_all_digital(d) if d   != -1
		  k.set_analog(1,a1) if a1  != -1
		  k.set_analog(2,a2) if a2  != -1
		  k.set_debounce(1, db1) if db1 != -1
		  k.set_debounce(2, db2) if db2 != -1

		  # Now we loop (or not) for the specified number of times
		  # reading all the input values

		  # Each read of all values will take about 8ms, at least
		  # on my setup - so no sense in using delay less than 10
		  dl = 10 if (dl > 0) && (dl < 10)

		  if (nm > 0) && (dl > 0)
			  tst = Time.now
			  while nm > 0
				  ds = str(k)
				  tnow = Time.now
				  tms = (tnow - tst)
				  tst = tnow
				  print str(int(tms+0.4999))+";"+ds
				  sleep((dl-8)/1000.0)	# compensate for read time
				  nm -= 1
    		end
		  elsif nm > 0
			  ds = str(k)
			  print "0;"+ds
    	end

		  k.disconnect
    end
end

main(ARGV)

