from monosat import *

import functools
import math
import os
import random
import random
import sys

print("begin encode");

seed = random.randint(1,100000)

random.seed(seed)
print("RandomSeed=" + str(seed))

bv1 = bv(4)
bv2 = bv(4)
bv3 = bv(4)
bv4 = bv(4)



g = Graph()

nodes = []
for n in range(4):
    nodes.append(g.addNode())
    
Assert(g.addEdge(0,1,bv1))
Assert(g.addEdge(0,2,bv2))
Assert(g.addEdge(1,2,bv3))
Assert(g.addEdge(2,3,bv4))

bv5 = bv(4)
bv6 = bv(4)

Assert(g.distance_leq(0,3,bv5))
Assert(Not(g.distance_lt(0,3,bv5)))

#Assert(bv5<6)
#Assert(bv5 > 4)


#Assert(bv6>=5)

Assert(g.distance_leq(1,3,bv6))
Assert(Not(g.distance_lt(1,3,bv6)))

Assert(bv5+bv6==10)
Assert(bv6>4)
bvs=[bv1,bv2,bv3,bv4,bv5,bv6]
#Assert(bv6>6)
#Assert(bv6<10)
result = solve()
print("Result is " + str(result))
if(result):
    for bv in bvs:
        print("bv" + str(bv.getID()) + " = " + str(bv.value()))
        


