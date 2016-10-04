from random import random

n, m = map(int, raw_input().split())

print 'p sp', n, m
for i in range(m):
    # row = (' ').join([str(int(random() * 1000)) for j in range(m)])
    # row =  (' ').join([str(j + 1) for j in range(m)])
    print 'a', i+1, i+2, int(random()*400)
