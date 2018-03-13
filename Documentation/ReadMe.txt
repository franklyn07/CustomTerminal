How to compile Terminal:

1) Copy presblock.c and terminal_final.c to desktop
2) Enter the standard linux terminal
4) Change current working directory to desktop
3) Enter the following commands:
	gcc -o presblock presblock.c
	gcc -o terminal Terminal_final.c -lncurses
4) After the exectubales are compiled, enter the following command:
	./terminal
5) Terminal should pop up