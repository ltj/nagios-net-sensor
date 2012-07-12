# Python Nagios plugin to fetch html formatted
# network sensor data (temperature, humidity or
# light).
# Author: Lars Toft Jacobsen (http://boxed.dk)
# Version 0.1a (still prototyping)
# Software comes with no warranty
# CC BY-SA

import sys, re, urllib.request, argparse

# nagios plugin API return values
OK = 0
WARNING = 1
CRITICAL = 2
UNKNOWN = 3

# default thresholds
t_warn = 25.0
t_crit = 30.0
h_warn = 60.0
h_crit = 65.0

serverpage = 'http://'

def main(type):
	retval = OK
	outstring = 'Status is '
	
	# Establish a connection with the Arduino Ethernet and
	# fetch data. A timeout of 5 seconds is set.
	try:
		f = urllib.request.urlopen(serverpage, timeout=5)
	except:
		# If the URL cannot be opened we return '3' == UNKNOWN
		print('Network error')
		sys.exit(UNKNOWN)
	
	# Decode the byte stream and pick out readings using a
	# regular expression
	result = f.read().decode('utf-8')
	m = re.search('T:(\d+\.\d+),H:(\d+\.\d+),L:(\d+)', result)
	
	if type == 'T':
		# temperature
		temp = float(m.group(1))
		if temp <= t_warn:
			outstring += 'OK - Temperature: SHT11 ' + str(temp) + ' C|temperature=' + str(temp)
		elif t_warn < temp <= t_crit:
			outstring += 'WARNING - Temperature: SHT11 ' + str(temp) + ' C|temperature=' + str(temp)
			retval = WARNING
		elif temp > t_crit:
			outstring += 'CRITICAL - Temperature: SHT11 ' + str(temp) + ' C|temperature=' + str(temp)
			retval = CRITICAL
	elif type == 'H':
		# humidity
		hum = float(m.group(2))
		if hum <= h_warn:
			outstring += 'OK - Humidity: SHT11 ' + str(hum) + ' %|humidity=' + str(hum)
		elif h_warn < hum <= h_crit:
			outstring += 'WARNING - Humidity: SHT11 ' + str(hum) + ' %|humidity=' + str(hum)
			retval = WARNING
		elif hum > h_crit:
			outstring += 'CRITICAL - Humidity: SHT11 ' + str(hum) + ' %|humidity=' + str(hum)
			retval = CRITICAL
	elif type == 'L':
		# light (always OK)
		light = m.group(3)
		outstring += 'OK - Light: TSL2561 ' + light + ' Lux|light=' + light
		
	print(outstring)	
	sys.exit(retval)

if __name__ == "__main__":
	# Option parsing
	parser = argparse.ArgumentParser()
	parser.add_argument("-t", "--type", help="type of data", choices="THL")
	parser.add_argument("-H", "--host", help="host IP", required=True)
	parser.add_argument("-w", "--warning", type=float, help="warning threshold")
	parser.add_argument("-c", "--critical", type=float, help="critical threshold")
	args = parser.parse_args()
	serverpage += args.host
	# Set warning/critical thresholds if provided
	if args.warning != None:
		t_warn = args.warning
		h_warn = args.warning
	if args.critical != None:
		t_crit = args.critical
		h_crit = args.critical
	
	main(args.type)