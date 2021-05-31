
import numpy as np

w = float(input("wavelength [nm]: "))

min_val = 370
max_val = 690
delta   = int((max_val-min_val)/3)

if w > max_val:
	w = max_val
elif w < min_val:
	w = min_val

f = lambda wv,a,b: 256*np.exp(-((2*wv-(a+b))/(b-a))**2)

R = int(f(w,min_val+2*delta,max_val))
G = int(f(w,min_val+delta,min_val+2*delta))
B = int(f(w,min_val,min_val+delta))

print((R,G,B))

