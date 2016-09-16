from random import random

n, m = map(int, raw_input().split())

print n, m
for i in range(n):
    # row = (' ').join([str(int(random() * 1000)) for j in range(m)])
    row = (' ').join([str(j + 1) for j in range(m)])
    print row
