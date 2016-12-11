
class Thread():
    
    def __init__(self, name, rec_cpu, nice):
        self.name = name
        self.rec_cpu = rec_cpu
        self.nice = nice
        self.prior = 0

    def __str__(self):
        return "'%s' with rcpu %f, nice value %d, prior %d" % (self.name, self.rec_cpu, self.nice, self.prior)
    
# Constants defined in various headers
PRI_MIN = 0
PRI_MAX = 63
TIMER_FREQ = 100
TIME_SLICE = 4

load_avg = 0 # global parameter indicating system's overall load

# format: [name, recent_cpu, niceness]
threads = [ Thread('A', 0.0, 0), Thread('B', 0.0, 1), Thread('C', 0.0, 2) ] 

running_index = None
ready_threads = len(threads)

TICKS = 40
print ("timer        recent_cpu    priority    thread\n" + 
       "ticks   A      B      C    A   B   C   to run")

for tick in range(TICKS):

    if tick % TIMER_FREQ == 0:
        # recalc load avg and recent cpu
        load_avg = (59/60) * load_avg + (1/60) * ready_threads
        # update rcpu
        for t in threads:
            t.rec_cpu = (2*load_avg)/(2*load_avg + 1) * t.rec_cpu + t.nice
        
    if tick % TIME_SLICE == 0:
        # recalc prior
        for t in threads:
            t.prior = int( PRI_MAX - (t.rec_cpu / 4) - (t.nice * 2) )
            t.prior = max(PRI_MIN, t.prior)
            t.prior = min(PRI_MAX, t.prior)

        # decide which thread to run, ensuring round robin
        running = max( enumerate(threads), key = lambda tp: (tp[1].prior, tp[0]) )[1]
        threads.remove(running)
        threads = [running] + threads

    running.rec_cpu += 1 
    
    if tick % 4 == 0:
        recent_cpu = [0] * 3
        priority = [0] * 3
        for i, t in enumerate(sorted(threads, key = lambda t: t.name)):
            recent_cpu[i] = t.rec_cpu
            priority[i] = t.prior
        print ( "{:2d}  {:>6.2f} {:>6.2f} {:>6.2f}  {:3d} {:3d} {:3d}  {:>4s}". \
                format(tick, *recent_cpu, *priority, running.name) )


