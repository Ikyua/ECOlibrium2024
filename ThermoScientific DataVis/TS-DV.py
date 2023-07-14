import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import matplotlib.ticker as ticker
import numpy as np
import pandas as pd
import ipywidgets as widgets
from ipywidgets import interact

df = pd.read_csv('exportdata_2022-11-17-18-36-54.csv')
df['dateTime'] = pd.to_datetime(df['localDate'] + ' ' + df['localTime'])
df.set_index('dateTime', inplace=True)

exclude_columns = ['id', 'instrumentName', 'sensorTemp', 'heaterTemp', 'internalTemp', 'UTCdate', 'UTCtime', 'alarms', 'timeRec', 'localDate', 'localTime']
plot_columns = [col for col in df.columns if col not in exclude_columns]

@interact
def update_plot(columns=widgets.SelectMultiple(options=plot_columns), Zoom=(1, len(df), 1), Position=(0, len(df)-1, 1)):
    Zoom = len(df) - Zoom + 1 
    end = Position + Zoom
    df_sliced = df.iloc[Position:end]
    plt.figure(figsize=(10, 5))
    for column in columns:
        plt.plot(df_sliced.index, df_sliced[column], label=column)

    fmt = mdates.DateFormatter('%Y-%m-%d %H:%M')
    ax = plt.gca()
    ax.xaxis.set_major_formatter(fmt)
    ticks = max(20, int(Zoom/len(df)*25)) 
    ax.xaxis.set_major_locator(ticker.MaxNLocator(ticks))  
    plt.xticks(rotation=45)
    plt.legend()

    plt.title('ThermoScience Indoor Readings')
    plt.xlabel('Date and Time')
    
    if len(columns) == 1:
        if columns[0] == 'pressure':
            plt.ylabel('kPa')
        elif columns[0] == 'rh':
            plt.ylabel('%')
        else:
            plt.ylabel('PPM')
    else:
        plt.ylabel('PPM' if 'pressure' not in columns and 'rh' not in columns else 'Multiple Units')

    plt.show()
