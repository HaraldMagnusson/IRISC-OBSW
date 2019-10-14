import matplotlib.pyplot as plt
import csv

plot1 = 1
plot2 = 1

if plot1 is 1:
	x = []
	y = []
	y_err = []
	y_target = []

	print('1st plot:\n')

	with open('simdata.txt','r') as csvfile:
		plots = csv.DictReader(csvfile, delimiter=',')
		i = 0;
		for row in plots:
			if i == 0:			
				start_time = float(row["sim time"])
				i += 1
			x.append(float(row["sim time"]) - start_time)
			y.append(float(row["current pos"]))
			y_err.append(float(row["pos error"]))
			y_target.append(float(row["target pos"]))

	plt.figure(1)
	plt.plot(x,y, label='position')
	plt.plot(x,y_err, label='error')
	plt.plot(x,y_target, label='target')
	plt.legend()
	plt.grid(b=True, which='major', color='#eeeeee', linestyle='-')
	plt.tight_layout()
	plt.show()


if plot2 is 1:
	x2 = []
	y2 = []
	y3 = []
	y4 = []

	print('2nd plot:\n')

	with open('simdata.txt','r') as csvfile:
		plots = csv.DictReader(csvfile, delimiter=',')
		i = 0;
		for row in plots:
			if i == 0:			
				start_time = float(row["sim time"])
				i += 1
			x2.append(float(row["sim time"]) - start_time)
			y2.append(float(row["integral"]))
			y3.append(float(row["derivative"]))
			y4.append(float(row["proportional"]))


	plt.figure(2)
	plt.subplot(3,1,1)
	plt.plot(x2, y2, label='Integral')
	plt.ylabel('Integral')
	plt.grid(b=True, which='major', color='#eeeeee', linestyle='-')

	plt.subplot(3,1,2)
	plt.plot(x2, y3, label='Derivative')
	plt.ylabel('Derivative')
	plt.grid(b=True, which='major', color='#eeeeee', linestyle='-')
	
	plt.subplot(3,1,3)
	plt.plot(x2, y4, label='Proportional')
	plt.ylabel('Proportional')
	plt.grid(b=True, which='major', color='#eeeeee', linestyle='-')
	
	plt.tight_layout()
	# plt.legend(['Integral', 'Derivative', 'Proportional'])
	plt.show()
