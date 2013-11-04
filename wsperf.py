import os, sys, argparse
from twisted.internet import reactor
from twisted.internet.utils import getProcessOutput, getProcessValue
from twisted.internet.defer import DeferredList

import analyze



if __name__ == '__main__':

   default_wsperf = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'wsperf')

   parser = argparse.ArgumentParser(description = 'wsperf test driver')

   parser.add_argument('--wsuri', dest = 'wsuri', type = str, default = 'ws://127.0.0.1:9000', help = 'The WebSocket URI the testee is listening on, e.g. ws://127.0.0.1:9000.')
   parser.add_argument('--workers', dest = 'workers', type = int, default = 4, help = 'Number of wsperf worker processes to spawn.')
   parser.add_argument('--threads', dest = 'threads', type = int, default = 0, help = 'Number of wsperf worker threads to spawn at each worker [0: run on main thread, >0: spawn that many background worker threads].')

   parser.add_argument('--conns', dest = 'conns', type = int, default = 50000, help = 'Number of WebSocket connections to open from each worker.')
   parser.add_argument('--lowmark', dest = 'lowmark', type = int, default = 250, help = 'Low watermark for each worker.')
   parser.add_argument('--highmark', dest = 'highmark', type = int, default = 500, help = 'High watermark for each worker.')

   parser.add_argument('--resultfile', dest = 'resultfile', type = str, default = r'result_%d.json', help = 'Result file pattern.')

   parser.add_argument('--wsperf', dest = 'wsperf', type = str, default = default_wsperf, help = 'Full path to wsperf executable.')

   parser.add_argument('--skiprun', dest = 'skiprun', action = "store_true", default = False, help = 'Skip test run.')
   parser.add_argument('--skipanalyze', dest = 'skipanalyze', action = "store_true", default = False, help = 'Skip analyze results.')

   options = parser.parse_args()

   resultfiles = [(options.resultfile % i) for i in xrange(options.workers)]

   if options.skiprun:
      ## here we don't start a reactor.
      
      if not options.skipanalyze:
         analyze.printResults(resultfiles)

   else:
      df = []
      for i in range(options.workers):

         args = [options.wsuri,
                 str(options.threads),
                 str(options.conns),
                 str(options.lowmark),
                 str(options.highmark),
                 options.resultfile % i]

         ## run wsperf executable
         d = getProcessOutput(options.wsperf, args, os.environ)

         ## accumulate any output
         df.append(d)

      d = DeferredList(df, consumeErrors = True)

      def onok(res):
         if not options.skipanalyze:
            analyze.printResults(resultfiles)
         reactor.stop()

      def onerr(err):
         print err
         reactor.stop()

      d.addCallbacks(onok, onerr)

      reactor.run()
