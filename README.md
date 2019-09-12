# Custom Terminal

A custom terminal built in C. It mainly has 4 panels;
- Date and Time Panel: showing the date and time of 3 different countries
- Alarm Panel: randomly sends a signal which is then caught and displays an appropriate colour
- Output Panel: displays the returned data from the terminal, as well as being able to hold the history of past commands
- Input Panel: where the user is able to interact with the terminal, making use of standard cmd commands as well as custom ones as described in the **documentation**.

Makes use of;
- ncurses
- signals
- concurrent processes (fork)
- shared memory segments
- system calls
- etc.
