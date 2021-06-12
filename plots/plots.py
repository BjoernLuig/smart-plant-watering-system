# -*- coding: utf-8 -*-
"""
@author: Bjoren Luig
"""

import requests
import numpy as np
import matplotlib.pyplot as plt

ip = "192.168.2.114"
url = f"http://{ip}/data"

data = requests.get(url).text.replace('-',' ').replace('T',' ').replace(':',' ').split('\n')
year,month,day,hour,minute,moisture1,moisture2,light,ultrasonic = np.genfromtxt(data,unpack=True,skip_header=1)

def getDate(i): return(f'{day[i]:.0f}.{month[i]:.0f}.{year[i]:.0f}')

plt.figure(figsize = (7,5))
dayZero = np.atleast_1d(day[0])
plt.plot(day-dayZero+hour/24.+minute/1440.,moisture1,label='moisture sensor 1 (%)',color='limegreen')
plt.plot(day-dayZero+hour/24.+minute/1440.,moisture2,label='moisture sensor 2 (%)',color='forestgreen')
plt.plot(day-dayZero+hour/24.+minute/1440.,light,label='light sensor (%)',color='gold')
plt.plot(day-dayZero+hour/24.+minute/1440.,ultrasonic,label='ultrasonic sensor (cm)',color='blue')
plt.title('all sensors')
plt.xlabel(f'days since {getDate(0)}')
plt.ylabel('sensor value')
plt.grid()
plt.legend()
plt.savefig(f'all-sensors-since-{getDate(0)}.jpg',dpi=300)
plt.show()

plt.figure(figsize = (7,5))
plt.errorbar(range(24),
             [moisture1[hour == h].mean() for h in range(24)],
             yerr = [moisture1[hour == h].std() for h in range(24)],
             fmt = '.',label='moisture sensor 1 (%)',color='limegreen')
plt.errorbar(range(24),
             [moisture2[hour == h].mean() for h in range(24)],
             yerr = [moisture2[hour == h].std() for h in range(24)],
             fmt = '.',label='moisture sensor 2 (%)',color='forestgreen')
plt.errorbar(range(24),
             [light[hour == h].mean() for h in range(24)],
             yerr = [light[hour == h].std() for h in range(24)],
             fmt = '.',label='light sensor (%)',color='gold')
plt.errorbar(range(24),
             [ultrasonic[hour == h].mean() for h in range(24)],
             yerr = [ultrasonic[hour == h].std() for h in range(24)],
             fmt = '.',label='ultrasonic sensor (cm)',color='blue')
plt.title('daycycle')
plt.xlabel(f'days since {getDate(0)}')
plt.ylabel('sensor value')
plt.grid()
plt.legend()
plt.savefig(f'daycycle-since-{getDate(0)}.jpg',dpi=300)
plt.show()