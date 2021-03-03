import matplotlib.pyplot as plt

# Using readlines()
file1 = open('p1.out', 'r')
Lines = file1.readlines()

x1 = []
p3 = []
p4 = []
p5 = []

t3 = 0
t4 = 0
t5 = 0

count = 0
# Strips the newline character
for line in Lines:
    x1.append(count)

    count += 1

    items = line.split(' ')
    pid = int(items[3])
    ticks = int(items[4])

    if pid == 3:
        t3 = ticks
    if pid == 4:
        t4 = ticks
    if pid == 5:
        t5 = ticks

    p3.append(t3)
    p4.append(t4)
    p5.append(t5)

fig = plt.figure()
fig.suptitle('xv6 lottery scheduler ticks accumulation', fontsize=20)
plt.plot(x1, p3, label='proc1 ticket=30')
plt.plot(x1, p4, label='proc2 ticket=20')
plt.plot(x1, p5, label='proc3 ticket=10')
leg = plt.legend(loc='lower right', frameon=True)
plt.show()