#import ijson
import ijson.backends.python as ijson
#import ijson.backends.yajl2 as ijson
#import ijson.backends.yajl as ijson

n = 0

import sys
import math

res = {'success': 0, 'fail': 0}

res_open_timestamps = []
res_close_timestamps = []

parser = ijson.parse(open('results.json'))
for prefix, event, value in parser:
   n += 1
   if prefix == 'item.failed':
      if value:
         res['fail'] += 1
      else:
         res['success'] += 1

   if prefix == 'item.open':
      res_open_timestamps.append(value)

   if prefix == 'item.close':
      res_close_timestamps.append(value)


def pstat(data):
   r = sorted(data)

   r_cnt = len(data)

   r_min = float(r[0])
   r_median = float(r[len(r)/2])
   r_avg = float(sum(data)) / float(r_cnt)
   r_sd = math.sqrt(sum([((float(x) - r_avg)**2.) for x in data]) / (float(r_cnt) - 1.))
   r_q90 = float(r[-len(r)/10])
   r_q95 = float(r[-len(r)/20])
   r_q99 = float(r[-len(r)/100])
   r_q999 = float(r[-len(r)/1000])
   r_max = float(r[-1])

   print ("    Min: %6.1f ms\n" + \
          "     SD: %6.1f ms\n" + \
          "    Avg: %6.1f ms\n" + \
          " Median: %6.1f ms\n" + \
          "  q90.0: %6.1f ms\n" + \
          "  q95.0: %6.1f ms\n" + \
          "  q99.0: %6.1f ms\n" + \
          "  q99.9: %6.1f ms\n" + \
          "    Max: %6.1f ms\n") % (r_min / 1000.,
                                    r_sd / 1000.,
                                    r_avg / 1000.,
                                    r_median / 1000.,
                                    r_q90 / 1000.,
                                    r_q95 / 1000.,
                                    r_q99 / 1000.,
                                    r_q999 / 1000.,
                                    r_max / 1000.)

print
print "wsperf results - WebSocket Opening Handshake"
print
print "Success: %9d" % res['success']
print "   Fail: %9d" % res['fail']
print
pstat(res_open_timestamps)
print

#pstat(res_close_timestamps)


#  start_array None
# item start_map None
# item map_key tcp_pre_init
# item.tcp_pre_init number 20594
# item map_key tcp_post_init
# item.tcp_post_init number 2
# item map_key open
# item.open number 15831
# item map_key close
# item.close number 38388
# item map_key failed
# item.failed boolean False
# item end_map None

# prefix, event, value