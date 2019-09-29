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
		plots = csv.reader(csvfile, delimiter=',')
		for row in plots:
			x.append(float(row[0]))
			y.append(float(row[1]))
			y_err.append(float(row[2]))
			y_target.append(float(row[3]))

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
		plots = csv.reader(csvfile, delimiter=',')
		for row in plots:
			x2.append(float(row[0]))
			y2.append(float(row[4]))
			y3.append(float(row[5]))
			y4.append(float(row[6]))


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
