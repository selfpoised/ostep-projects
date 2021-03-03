import matplotlib.pyplot as plt

# Using readlines()
file1 = open('p2.out', 'r')
Lines = file1.readlines()

x3 = []
p3 = []
x4 = []
p4 = []
x5 = []
p5 = []

t3 = 0
t4 = 0
t5 = 0

count = 0
# Strips the newline character
for line in Lines:
    items = line.split(' ')
    pid = int(items[3])
    ticks = int(items[4])

    if pid == 3:
        t3 = ticks
        p3.append(3)
        x3.append(count)
    if pid == 4:
        t4 = ticks
        p4.append(2)
        x4.append(count)
    if pid == 5:
        t5 = ticks
        p5.append(1)
        x5.append(count)

    count += 1

fig = plt.figure()
fig.suptitle('xv6 lottery scheduler process chosen per tick', fontsize=20)
plt.scatter(x3, p3, label='proc1 ticket=30', marker='o')
plt.scatter(x4, p4, label='proc2 ticket=20', marker='x')
plt.scatter(x5, p5, label='proc3 ticket=10', marker='v')
leg = plt.legend(loc='lower right', frameon=True, bbox_to_anchor=(1, 0.2))
plt.show()