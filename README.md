Ubuntu 12.04 LTS:

	export BOOST_ROOT=$HOME/build/boost_1_55_0b1_gcc
	export LD_LIBRARY_PATH=$BOOST_ROOT/stage/lib:$LD_LIBRARY_PATH
	
	g++ -std=c++0x -O3 -D_WEBSOCKETPP_CPP11_STL_ -I../websocketpp/ -I$BOOST_ROOT wsperf.cpp -L$BOOST_ROOT/stage/lib -lssl -lcrypto -lboost_system -lpthread -o wsperf
