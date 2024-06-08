# This folder is dedicated to the code for Infrared (IR) signal detection

IR_v1 contains the code that can analyze the frequency of the IR signal by analyzing the time between state changes of the digital input. /n
The interval time has a unit of microseconds and on the Arduino board, the resolution is 4 microseconds, which is enough to distinguish between 571 $Hz$ and 353 $Hz$.

This is a digital signal analysis so it can also be used to analyze the Radio wave signal with envelope detection and comparator output.
