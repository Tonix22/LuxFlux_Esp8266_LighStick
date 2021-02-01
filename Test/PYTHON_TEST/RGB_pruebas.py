import numpy as np

time = "30" #miliseconds
frames = 500

min_val = 400
max_val = 800
delta   = int((max_val-min_val)/3)

w = 0

if w > max_val:
	w = max_val
elif w < min_val:
	w = min_val

f = lambda wv,a,b: 256*np.exp(-((2*wv-(a+b))/(b-a))**2)
#f = lambda wv,a,b: 256*np.exp(-((2*wv-(a+b))/(b-a))**2)
pruebas = np.linspace(min_val-75,max_val+75,frames)

f_rbg = open("../color_patern.txt", "w")

for i in pruebas:
	R = int(f(i,min_val+2*delta,max_val))
	G = int(f(i,min_val+delta,min_val+2*delta))
	B = int(f(i,min_val,min_val+delta))

	if (R > 255) or (G > 255) or (B > 255):
		print( "error with lambda = " + str(i))
	elif (R < 0) or (G < 0) or (B < 0):
		print("\n error with lambda = " + str(i))
	else:
		f_rbg.write("4("+str(R)+","+str(G)+","+str(B)+")"+",4("+str(R)+","+str(G)+","+str(B)+"),"+time+"\n")
		print("wvl %5f nm = \(%3d,%3d,%3d\)" % (i,R,G,B))


f_rbg.close()