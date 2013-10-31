Ubuntu 12.04 LTS:

	export BOOST_ROOT=$HOME/build/boost_1_55_0b1_gcc
	export LD_LIBRARY_PATH=$BOOST_ROOT/stage/lib:$LD_LIBRARY_PATH

	g++ -std=c++0x -O3 -D_WEBSOCKETPP_CPP11_STL_ -I../websocketpp/ -I$BOOST_ROOT wsperf.cpp -L$BOOST_ROOT/stage/lib -lssl -lcrypto -lboost_system -lpthread -o wsperf


Ubuntu 12.04 LTS (package manager boost 1.48: libboost-system1.48-dev):
Ubuntu 13.10 (package manager boost 1.53: libboost-all-dev):

    g++ -std=c++0x -O3 -D_WEBSOCKETPP_CPP11_STL_ -I../websocketpp/ wsperf.cpp -lssl -lcrypto -lboost_system -lpthread -o wsperf

Mac OS X 10.9 / XCode, clang

    clang++ -std=c++0x -stdlib=libc++ -I../websocketpp -isystem ../boost_1_53_0_libcpp -O2 ../boost_1_53_0_libcpp/stage/lib/libboost_system.a -D_WEBSOCKETPP_CPP11_STL_ wsperf.cpp -lssl -lcrypto -o wsperf

