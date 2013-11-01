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

	time ./wsperf ws://127.0.0.1:9000 4 100000 1000 > results.json


## Analyze

Analyzing results can be done with `analyze.py` (a quick hack, needs more love). We need a streaming JSON parser, since results are getting big:

	git clone git@github.com:oberstet/ijson.git
	cd ijson
	~/pypy-2.1/bin/easy_install ijson

Make sure to run that baby with PyPy.
