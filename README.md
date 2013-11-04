# wsperf

**wsperf** is a WebSocket load testing probe.

## Building

You will need to have 2 environment variables set:

  * `BOOSTROOT` pointing to a Boost installation
  * `WSPP_ROOT` pointing to a WebSocket++ source distribution

Note that (for now), you will need the `flow_control2` branch with WebSocket++ checked out.

To build:

	scons

To cleanup

	scons -uc


## Usage

	time ./wsperf ws://127.0.0.1:9000 4 200000 1000 2000 > results.json


## Analyze

Analyzing results can be done with `analyze.py` (a quick hack, needs more love). We need a streaming JSON parser, since results are getting big:

	git clone git@github.com:oberstet/ijson.git
	cd ijson
	~/pypy-2.1/bin/easy_install ijson


## Results

### Autobahn Multicore

Make sure to run **wsperf** against the testee multiple times to allow testee that use a JITting compiler (suche as Autobahn on PyPy) to warm up.

Starting a test run:

	oberstet@corei7-ubuntu:~/scm/wsperf$ time ./wsperf ws://127.0.0.1:9000 8 200000 1000 2000 > results.json

	real	0m15.341s
	user	0m35.104s
	sys	0m20.528s

Analyzing results:

	oberstet@corei7-ubuntu:~/scm/wsperf$ ~/pypy-2.1/bin/pypy analyze.py 

	wsperf results - WebSocket Opening Handshake

	          Duration:     14583 ms
	             Total:    200000
	           Success:    200000
	              Fail:         0
	            Fail %:      0.00
	    Handshakes/sec:     13714

	     Min:       1.2 ms
	      SD:      19.9 ms
	     Avg:      24.8 ms
	  Median:      18.1 ms
	  q90   :      43.2 ms
	  q95   :      71.7 ms
	  q99   :     101.9 ms
	  q99.9 :     117.3 ms
	  q99.99:     124.0 ms
	     Max:     131.7 ms

### Linux Perf

Basic statistics:

	oberstet@corei7-ubuntu:~/scm/wsperf$ sudo perf stat ./wsperf ws://127.0.0.1:9000 8 200000 1000 2000 > results.json

	 Performance counter stats for './wsperf ws://127.0.0.1:9000 8 200000 1000 2000':

	      56397,171142 task-clock                #    3,576 CPUs utilized          
	           177.411 context-switches          #    0,003 M/sec                  
	            23.577 cpu-migrations            #    0,418 K/sec                  
	           595.352 page-faults               #    0,011 M/sec                  
	   186.462.389.466 cycles                    #    3,306 GHz                     [83,25%]
	   144.686.178.873 stalled-cycles-frontend   #   77,60% frontend cycles idle    [83,57%]
	    83.129.352.607 stalled-cycles-backend    #   44,58% backend  cycles idle    [66,67%]
	    84.877.715.262 instructions              #    0,46  insns per cycle        
	                                             #    1,70  stalled cycles per insn [83,35%]
	    16.967.531.711 branches                  #  300,858 M/sec                   [83,40%]
	       673.808.823 branch-misses             #    3,97% of all branches         [83,11%]

	      15,771390852 seconds time elapsed

	oberstet@corei7-ubuntu:~/scm/wsperf$ 


Detailed event recording:

	oberstet@corei7-ubuntu:~/scm/wsperf$ sudo perf record -e cycles,branch-misses,cache-misses ./wsperf ws://127.0.0.1:9000 8 200000 1000 2000 > results.json
	[ perf record: Woken up 119 times to write data ]
	[ perf record: Captured and wrote 30.653 MB perf.data (~1339248 samples) ]

Reporting

	oberstet@corei7-ubuntu:~/scm/wsperf$ sudo perf report --stdio
	# ========
	# captured on: Mon Nov  4 08:03:59 2013
	# hostname : corei7-ubuntu
	# os release : 3.8.0-32-generic
	# perf version : 3.8.13.10
	# arch : x86_64
	# nrcpus online : 8
	# nrcpus avail : 8
	# cpudesc : Intel(R) Core(TM) i7 CPU 920 @ 2.67GHz
	# cpuid : GenuineIntel,6,26,4
	# total memory : 12296476 kB
	# cmdline : /usr/bin/perf_3.8.0-32 record -e cycles,branch-misses,cache-misses ./wsperf ws://127.0.0.1:9000 
	# event : name = cycles, type = 0, config = 0x0, config1 = 0x0, config2 = 0x0, excl_usr = 0, excl_kern = 0, 
	# event : name = branch-misses, type = 0, config = 0x5, config1 = 0x0, config2 = 0x0, excl_usr = 0, excl_ker
	# event : name = cache-misses, type = 0, config = 0x3, config1 = 0x0, config2 = 0x0, excl_usr = 0, excl_kern
	# HEADER_CPU_TOPOLOGY info available, use -I to display
	# HEADER_NUMA_TOPOLOGY info available, use -I to display
	# pmu mappings: cpu = 4, software = 1, tracepoint = 2, uncore = 6, breakpoint = 5
	# ========
	#
	# Samples: 249K of event 'cycles'
	# Event count (approx.): 190156545668
	#
	# Overhead  Command        Shared Object                                                                    
	# ........  .......  ...................  ..................................................................
	#
	     4.68%   wsperf  libc-2.15.so         [.] _int_malloc                                                   
	     3.94%   wsperf  libc-2.15.so         [.] _int_free                                                     
	     3.20%   wsperf  [kernel.kallsyms]    [k] __ticket_spin_lock                                            
	     2.91%   wsperf  libc-2.15.so         [.] malloc                                                        
	     1.86%   wsperf  wsperf               [.] std::__shared_count<(__gnu_cxx::_Lock_policy)2>::~__shared_cou
	     1.84%   wsperf  wsperf               [.] std::__shared_count<(__gnu_cxx::_Lock_policy)2>::__shared_coun
	     1.61%   wsperf  libpthread-2.15.so   [.] pthread_mutex_lock                                            
	     1.28%   wsperf  libc-2.15.so         [.] tolower                                                       
	     1.12%   wsperf  libc-2.15.so         [.] __memcpy_ssse3_back                                           
	     1.09%   wsperf  libstdc++.so.6.0.16  [.] __cxxabiv1::__vmi_class_type_info::__do_dyncast(long, __cxxabi
	     1.06%   wsperf  libc-2.15.so         [.] free                                                          
    ...
