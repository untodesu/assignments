#### Timer
A `SysTick` based timer ticking each millisecond.

##### Interface description
```

 HH:MM:SS.MMM

 STATUS
```

1. The uppest line is a timer's value in a human-readable format.
2. The lowest value is the timer status, can be either green `COUNT` indicating that the timer is active, or a red `PAUSE` indicating the timer's inactivity.
3. Use joystick LEFT to enable/disable timer counting.
4. Use joystick DOWN to clear the timer's counter.

##### Interface
![](image.jpg)

