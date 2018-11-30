import numpy as np
from matplotlib import pyplot as plt

x = np.array([1, 2, 4,8])
bst = np.array([2.79,5.47,9.40,7.82])
avl = np.array([3.95,6.77,10.40,9.83])


fig = plt.figure()
ax1 = fig.add_subplot(111)
ax1.plot(x, bst, '-o', label="BST", markersize=10, linewidth=2)
ax1.plot(x, avl, '-^', label="AVL", markersize=10, linewidth=2)
plt.legend(loc='upper left', shadow=True)
plt.xlabel('Threads', fontsize=14)
plt.ylabel('Throughput (MOps/sec)', fontsize=14)
ax1.tick_params(labelsize=12)
plt.title('9%/1%/90%')
# plt.show()
plt.savefig("conc-9-1-90", dpi=300, bbox_inches="tight")
