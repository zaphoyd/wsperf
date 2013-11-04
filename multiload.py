import os, sys
from twisted.internet import reactor
from twisted.internet.utils import getProcessOutput, getProcessValue
from twisted.internet.defer import DeferredList

wsperf = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'wsperf')

df = []
for i in range(4):
   args = ["ws://127.0.0.1:9000", "0", "50000", "250", "500", "result_%d.json" % i]
   d = getProcessOutput(wsperf, args, os.environ)
   df.append(d)

d = DeferredList(df)

def done(res):
   print res
   reactor.stop()

d.addBoth(done)

reactor.run()
