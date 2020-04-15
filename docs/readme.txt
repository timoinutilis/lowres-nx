***************
* First Steps *
***************

Please read the introduction (at least "Getting Started") of the manual.


************
* Controls *
************

- Use any real gamepad or the keyboard:

	Button  Player 1    Player 2
	-------+-----------+--------
	UP      Arrow Up    E
	DOWN    Arrow Down  D
	LEFT    Arrow Left  S
	RIGHT   Arrow Right F
	A       Z/N         Q/Tab
	B       X/M         A/Shift

- More keys:
	Dev Menu     Esc
	Pause        Return/P
	Fullscreen   Ctrl+f
	Fullwidth    Ctrl+w
	View Area    Ctrl+v/Space
	Screenshot
	   large     Ctrl+s
	   original  Ctrl+Shift+s
	Debug        Ctrl+d
	Reload/Run   Ctrl+r
	Eject        Ctrl+e
	Quit         Esc (if disabledev)


************
* Settings *
************

- Settings file:
	A default settings file is created on application start, if none
	exists yet. Available options are the same as for command line
	arguments, but each one is written in its own line and without
	the leading "-" character.
	macOS: /Users/YourName/Library/Application Support/Inutilis Software/LowRes NX/settings.txt
	Windows: C:\Users\YourName\AppData\Roaming\Inutilis Software\LowRes NX\settings.txt

- Command line arguments:
	These override the options from the settings file.

	"LowRes NX" [-option value] [program.nx]

	-fullscreen yes/no
	Start the application in fullscreen mode

	-fullwidth yes/no
	Start the application in fullwidth mode, Ctrl+v or Space key toggles view area
	
	-disabledev yes/no
	Disable the Development Menu, Esc key quits LowRes NX

	program.nx
	Name of the program to run


*********
* Notes *
*********

- Share programs and discuss on: https://lowresnx.inutilis.com

- Development news on Twitter: @timo_inutilis

- Tweet with #LowResNX.

- To-do and bug list on:
  https://github.com/timoinutilis/lowres-nx/issues

- Write to timo@inutilis.com.
