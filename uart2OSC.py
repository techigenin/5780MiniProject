# This project was written by Michael James
# For the ECE 5780 - Mini Project
# Spring 2018
import serial
from socket import *
from time import sleep



serverPort = 8000
serverPort2 = 8001
serverSocket = socket(AF_INET, SOCK_DGRAM)
clientAddress = '192.168.1.60'
clientAddress2 = '192.168.1.101'


getMessage = {1 : 'Message 1',
2 : 'Message 2',
3 : 'Message 3',
4 : 'Message 4',
5 : 'Message 5',
6 : 'Message 6',
7 : 'Message 7',
8 : 'Message 8',
9 : 'Message 9',
10 : 'Message 10',
11 : 'Message 11',
12 : 'Message 12',
13 : 'Message 13',
14 : 'Message 14',
15 : 'Message 15',
16 : 'Message 16',
17 : 'Message 17',
18 : 'Message 18',
19 : 'Message 19',
20 : 'Message 20',
21 : 'Message 21',
22 : 'Message 22',
23 : 'Message 23',
24 : 'Message 24',
25 : 'Message 25',
26 : 'Message 26',
27 : 'Message 27',
28 : 'Message 28',
29 : 'Message 29',
30 : 'Message 30',
31 : 'Message 31',
32 : '/beyond/cue/1/1/livecontrol/',
33 : '/beyond/general/StartCue "1,2"',
34 : '/beyond/general/StartCue 1,3',
35 : '/beyond/general/StartCue 1,4',
36 : '/beyond/general/StartCue 1,5',
37 : '/beyond/general/StartCue 1,6',
38 : '/beyond/general/StartCue 1,7',
39 : '/beyond/general/StartCue 1,8',
40 : '/beyond/general/StartCue 1,9',
41 : 'Message 41',
42 : 'Message 42',
43 : 'Message 43',
44 : 'Message 44',
45 : 'Message 45',
46 : 'Message 46',
47 : 'Message 47',
48 : 'Message 48',
49 : 'Message 49',
50 : 'Message 50',
51 : 'Message 51',
52 : 'Message 52',
53 : 'Message 53',
54 : 'Message 54',
55 : 'Message 55',
56 : 'Message 56',
57 : 'Message 57',
58 : 'Message 58',
59 : 'Message 59',
60 : 'Message 60',
61 : 'Message 61',
62 : 'Message 62',
63 : 'Message 63'}

def myFunction(data):
	print data
	if data < 41 and data > 0: # barring weird errant Data
		serverSocket.sendto(getMessage[data], (clientAddress, serverPort))
	elif data <64 and data >=41:
		serverSocket.sendto(getMessage[data], (clientAddress2, serverPort2))
serverSocket.bind(('', serverPort))	
	
ser = serial.Serial("/dev/ttyS0", 9600, timeout = 0.1)
while True: 
	receivedData = ser.read(1)
	ser.reset_input_buffer()
	if receivedData != '':
		myFunction(ord(receivedData))
