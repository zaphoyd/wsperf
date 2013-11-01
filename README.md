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

	time ./wsperf ws://127.0.0.1:9000 4 64000 2000 > results.json


## Analyze

Analyzing results can be done with `analyze.py` (a quick hack, needs more love). We need a streaming JSON parser, since results are getting big:

	git clone git@github.com:oberstet/ijson.git
	cd ijson
	~/pypy-2.1/bin/easy_install ijson

Make sure to run that baby with PyPy and do multiple runs *without* restarting the server (for server that use JITting compilers to warm up).

## Results

### Autobahn Multicore

Start the server:

	oberstet@corei7-ubuntu:~/scm/AutobahnPython/examples/websocket/echo_multicore$ ~/pypy-2.1/bin/pypy server.py --wsurws://localhost:9000 --workers 4

Start the test:

	oberstet@corei7-ubuntu:~/scm/wsperf$ time ./wsperf ws://127.0.0.1:9000 4 64000 2000 > results.json

	real	0m5.745s
	user	0m12.128s
	sys	0m8.000s
	oberstet@corei7-ubuntu:~/scm/wsperf$ python
	Python 2.7.5 (default, Oct 22 2013, 10:12:53) 
	[GCC 4.6.3] on linux2
	Type "help", "copyright", "credits" or "license" for more information.
	>>> 64000./5.745
	11140.12184508268

Analyze results:

	oberstet@corei7-ubuntu:~/scm/wsperf$ ~/pypy-2.1/bin/pypy analyze.py

	wsperf results - WebSocket Opening Handshake

	Success:     64000
	   Fail:         0

	    Min:    2.5 ms
	     SD:   30.1 ms
	    Avg:   20.2 ms
	 Median:   13.8 ms
	  q90.0:   18.8 ms
	  q95.0:   83.1 ms
	  q99.0:  166.4 ms
	  q99.9:  179.6 ms
	    Max:  187.8 ms

### WebSocket++

Start the server:

	oberstet@corei7-ubuntu:~/scm/websocketpp$ ./build/release/testee_server/testee_server

Start the test:

	oberstet@corei7-ubuntu:~/scm/wsperf$ time ./wsperf ws://127.0.0.1:9002 4 64000 2000 > results.json

	real	0m5.455s
	user	0m11.448s
	sys	0m7.636s


Analyze results:

	oberstet@corei7-ubuntu:~/scm/wsperf$ ~/pypy-2.1/bin/pypy analyze.py 

	wsperf results - WebSocket Opening Handshake

	Success:     63871
	   Fail:       129

	    Min:    0.2 ms
	     SD: 90063.1 ms
	    Avg: 4060.1 ms
	 Median:   13.1 ms
	  q90.0:   15.6 ms
	  q95.0:   17.0 ms
	  q99.0:   26.4 ms
	  q99.9: 2008071.8 ms
	    Max: 2008072.8 ms
