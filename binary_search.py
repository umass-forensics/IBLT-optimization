from pyblt import *
import random
import gc
import math

def size(items, hedge, hashes):
    """ Returns the size of an IBLT (in row) given key params """
    if hedge == 0:
        return 0
    items = math.trunc(items*hedge)
    while hashes * (items//hashes) != items:
        items += 1
    return items

def refit(items, hedge, hashes):
    """ Returns a hedge value that results in the same    """
    s = size(items, hedge, hashes)
    assert s == size(items, s/items, hashes)
    return s/items


class Search:
    def __init__(self, entries, goal):
        assert entries > 0
        self.entries = entries
        self.goal = goal
        # this constant is a stop condition for binary search
        # sometimes params have a mean of the desired value. In that
        # case we'll stop when the mean is with a relatively 
        # tight confidence interval
        self.ci_limit = (1-self.goal)/3
        
    def trial(self, hedge, k):
        """ Determine the decode rate of PYBLT's param file using at least 5000 trials.
        Trials stop when 95% confidence interval is met, or mean is within .99999 to 1.00001  """
        trials = 0
        successes = 0
        
        while True:
            p = PYBLT(entries=self.entries, value_size=9, hedge=hedge, num_hashes=k)

            for _ in range(self.entries):
                p.insert(random.getrandbits(32), 0)
            if p.list_entries()[0]:  #if decoded
                successes += 1
            trials += 1
            ssize = p.get_serialized_size()
            del p

            prob = successes/trials
            if successes < trials:
                ci = 1.96*math.sqrt(prob*(1-prob)/trials)
            else:
                ci = -(math.exp(math.log(.05)/trials)-1)

            if  trials >= 5000:
              if ( (prob-ci> self.goal-self.ci_limit and prob+ci<self.goal+self.ci_limit) or
                  ((prob-ci >= self.goal) or (prob+ci < self.goal))):
                return successes, trials, prob, ci, ssize
            if trials % 5000 == 0:
              print(trials, prob-ci, successes/trials, prob+ci, ci, self.ci_limit)
              
    def find_hedge_for_k(self, k, best_size=None):
        """ Binary search for best hedge value given items and number of hashvalues k
        best_size: if passed in, we prune the search when the smallest size is larger 
        """
        assert k >= 2
        high = 20.0 # starting high point binary search (code will double if too low)
        # for some desired decode rates, 20 will be too low for a starting value
        low = 0.0  # min hedge to consider
        epsilon = 0 # difference in number of rows between high and low hedge (in terms of IBLT rows)
        vals = defaultdict(list)
        
        # Loop until low and high are the same. We move either low up, or high down, each iteration
        # In some cases, we'll move high up. 
        while size(self.entries, high, k) - size(self.entries, low, k) > epsilon:
            # Print progress as we go
            print("%3d: %3f(%d)\t%3f\t(%3d)\t%3f(%d)" %
                  (k, low, size(self.entries, low, k),
                   (high+low)/2, size(self.entries, (high+low)/2, k),
                   high, size(self.entries, high, k)))

            hedge = refit(self.entries, (high+low)/2, k)
            # Keep track of which results we've completed.
            if vals[size(self.entries, hedge, k)] == list():
                vals[size(self.entries, hedge, k)] = self.trial(hedge, k)

            successes, trials, prob, ci, ssize = vals[size(self.entries, hedge, k)]

            if prob+ci <= self.goal or (prob-ci> self.goal-self.ci_limit and 
                                            prob+ci<self.goal+self.ci_limit):
                # failure, try going higher
                old_low = low
                low = refit(self.entries, (high+low)/2, k)
                # we've got to increase by at least 1 (i.e.,k rows in the IBLT)
                while refit(self.entries, old_low, k) == refit(self.entries, low, k):
                    low += 1
            
            # success if we are above the goal, or we are within a tight confidence interval
            elif (prob-ci > self.goal):
                # success, try going lower
                old_high = high
                high = refit(self.entries, (high+low)/2, k)
                # if lower isn't any lower, then we are done
                if refit(self.entries, old_high, k) == refit(self.entries, high, k):
                    # we are done
                    break
                    
        # assert for debugging
        assert prob-ci >= self.goal or prob-ci > self.goal-self.ci_limit
        return hedge, successes, trials


    def run(self):
        ran = random.getrandbits(32)
        with open('results-%s-%s-%s.csv' % (self.entries, self.goal, ran), 'w') as fd:
            # Start with k=3 and see if we can prune higher values as they run
            print("entries, ", self.entries, ran)
            h, successes, trials = self.find_hedge_for_k(4)
            iblt_size = size(self.entries, h, 4)
            best = iblt_size
            print("items=%d, goal=%f, k=%d, hedge=%f, size=%d, success=%d, trials=%d" % (self.entries, self.goal, 
            4, h, iblt_size, successes, trials))
            fd.write("%d, %f, %d, %f, %d, %d, %d\n"% (self.entries, self.goal, 4, h, iblt_size, successes, trials))
            fd.flush() # since they runs can take a while

            # Run 3...13 where k!=4 now
            for k in list(range(3,4))+list(range(5,13)):
                h, successes, trials = self.find_hedge_for_k(k, best_size=best)
                if h is not None:
                    iblt_size = size(self.entries, h, k)
                    print("%d, %f, %d, %f, %d, %d, %d"% (self.entries, self.goal, 
                    k, h, iblt_size, successes, trials))
                    fd.write("%d, %f, %d, %f, %d, %d, %d\n"% (self.entries, self.goal, k, h, iblt_size, successes, trials))
                    fd.flush() # since they runs can take a while
                    best = min(best, iblt_size)
                else:
                    print("Pruned trial")



items = int(sys.argv[1]) + int(sys.argv[2])
# desired decode rate
desired_prob = float(sys.argv[3])/float(sys.argv[4])

assert items >= 1
assert desired_prob >0 
assert desired_prob <1
Search(items, desired_prob).run()
