*********
* Notes *
*********

- Read the manual, especially the introduction!

- The manual sometimes refers to the interface of the iOS app.
  On macOS/Windows just use any text editor, there is no included one.
  Press Ctrl+M to show the Development Menu, from where you can use
  the designer tools.

- Share programs and discuss on: https://lowresnx.inutilis.com

- Development news on Twitter: @timo_inutilis

- Tweet with #LowResNX.

- To-do and bug list on:
  https://github.com/timoinutilis/lowres-nx/issues

- Write to timo@inutilis.com.
  
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
	Quit         Esc
	Pause        Return/P
	Fullscreen   Ctrl+f
	Debug        Ctrl+d
	Dev Menu     Ctrl+m
	Reload/Run   Ctrl+r
	Exit         Ctrl+e

- Settings file:
	A default settings file is created on application start, if none
	exists yet. Available options are the same as for command line
	arguments, but each one is written in its own line and without
	the leading "-" character.
	macOS: /Users/YourName/Library/Application Support/com.inutilis/LowResNX/settings.txt

- Command line arguments:
	These override the options from the settings file.

	"LowRes NX" [-fullscreen yes/no] [-programs path] [program.nx]

	-fullscreen yes/no
	Starts the application in fullscreen mode
	
	-programs path
	Path for the tool programs and virtual disk file. Terminate it
	with a path separator (slash or back-slash).
	By default it's the "programs" folder next to the LowRes NX
	application.

	program.nx
	Name of program to run
