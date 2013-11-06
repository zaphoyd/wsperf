clean:
	rm -f result*.json
	sudo rm -f perf*

test_ab:
	pypy wsperf.py --wsuri ws://127.0.0.1:9000 --workers 4 --threads 0 --conns 50000 --lowmark 250 --highmark 500

test_ws:
	pypy wsperf.py --wsuri ws://127.0.0.1:9002 --workers 4 --threads 0 --conns 50000 --lowmark 250 --highmark 500

test_netty:
	pypy wsperf.py --wsuri ws://127.0.0.1:9999 --workers 4 --threads 0 --conns 50000 --lowmark 250 --highmark 500

analyze:
	pypy wsperf.py --workers 4 --skiprun

help:
	pypy wsperf.py --help
