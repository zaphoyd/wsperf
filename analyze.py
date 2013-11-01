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

tcp_pre_init_min = None
tcp_pre_init_max = None
total_duration = None

parser = ijson.parse(open('results.json'))
for prefix, event, value in parser:
   n += 1
   #print prefix, event, value
   #if n > 10:
   #   sys.exit(0)
   if True:
      if prefix == 'total_duration':
         total_duration = value
      if prefix == 'connection_stats.item.tcp_pre_init':
         if tcp_pre_init_min is None or value < tcp_pre_init_min:
            tcp_pre_init_min = value
         if tcp_pre_init_max is None or value > tcp_pre_init_max:
            tcp_pre_init_max = value

      if prefix == 'connection_stats.item.failed':
         if value:
            res['fail'] += 1
         else:
            res['success'] += 1

      if prefix == 'connection_stats.item.open':
         res_open_timestamps.append(value)

      if prefix == 'connection_stats.item.close':
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
   r_q9999 = float(r[-len(r)/10000])
   r_max = float(r[-1])

   print ("     Min: %9.1f ms\n" + \
          "      SD: %9.1f ms\n" + \
          "     Avg: %9.1f ms\n" + \
          "  Median: %9.1f ms\n" + \
          "  q90   : %9.1f ms\n" + \
          "  q95   : %9.1f ms\n" + \
          "  q99   : %9.1f ms\n" + \
          "  q99.9 : %9.1f ms\n" + \
          "  q99.99: %9.1f ms\n" + \
          "     Max: %9.1f ms\n") % (r_min / 1000.,
                                     r_sd / 1000.,
                                     r_avg / 1000.,
                                     r_median / 1000.,
                                     r_q90 / 1000.,
                                     r_q95 / 1000.,
                                     r_q99 / 1000.,
                                     r_q999 / 1000.,
                                     r_q9999 / 1000.,
                                     r_max / 1000.)

res['total'] = res['success'] + res['fail']

print
print "wsperf results - WebSocket Opening Handshake"
print
print "          Duration: %9d ms" % (float(total_duration) / 1000.)
print "             Total: %9d" % res['total']
print "           Success: %9d" % res['success']
print "              Fail: %9d" % res['fail']
print "            Fail %%: %9.2f" % (100. * float(res['fail']) / float(res['total']))
print "    Handshakes/sec: %9d" % int(round((float(res['success']) / (float(total_duration) / 1000000.))))
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