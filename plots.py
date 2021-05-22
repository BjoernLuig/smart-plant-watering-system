# -*- coding: utf-8 -*-
"""
@author: Bjoren Luig
"""

import numpy as np
import requests
import datetime
import matplotlib.pyplot as plt

url = "http://192.168.2.114/data"
names = ['moisture sensor 1 (%)','moisture sensor 2 (%)','light sensor (%)','water level distance (cm)']
colors = ['limegreen','forestgreen','gold','blue']

response = requests.get(url).text.split('\n')[:-1]
dates = []
days = []
times = []
data = {}
for name in names: data[name] = []
for line in response:
    line = line.split(' ')
    date = line.pop(0).replace('-',':').replace('T',':').split(':')
    date = datetime.datetime(int(date[0]),int(date[1]),int(date[2]),int(date[3]),int(date[4]))
    dates.append(date)
    day = date-dates[0]
    day = day.days+day.seconds/86400.0
    days.append(day)
    times.append(date.hour)
    for i in range(len(data)): data[names[i]].append(float(line[i]))

print(data)

plt.figure(figsize = (7,5))
for i in [0,1,2]:
    plt.plot(times,data[names[i]],'.',label=names[i],color=colors[i])
title = f'day cycle {dates[0].year}-{dates[0].month}-{dates[0].day} to {dates[-1].year}-{dates[-1].month}-{dates[-1].day}'
plt.title(title)
plt.xlabel('hour')
plt.ylabel('sensor value')
plt.ylim(0,100)
plt.xlim(-0.5,23.5)
plt.grid()
plt.legend()
plt.savefig(f'plots/{title}.jpg',dpi=300)

plt.figure(figsize = (7,5))
for i in range(len(names)):
    plt.plot(days,data[names[i]],label=names[i],color=colors[i])
title = f'all data {dates[0].year}-{dates[0].month}-{dates[0].day} to {dates[-1].year}-{dates[-1].month}-{dates[-1].day}'
plt.title(title)
plt.xlabel(f'days since {dates[0].year}-{dates[0].month}-{dates[0].day}')
plt.ylabel('sensor value')
plt.ylim(0,100)
plt.grid()
plt.legend()
plt.savefig(f'plots/{title}.jpg',dpi=300)

plt.show()

def strToDays(text):
    text = str(text)
    date = text.replace('-',':').replace('T',':').split(':')
    print(np.array(data,dtype='int'))
    date = datetime.datetime(*np.array(data,dtype='int'))
    #print(date)
    return(0.)



print(np.genfromtxt(url,converters = {0: strToDays}))