import time

start = time.time()

for i in range(100000000):
    pass

end = time.time()
print("Temps ecoule pour la boucle: {:.6f} secondes".format(end - start))