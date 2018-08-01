*********
* Notes *
*********

- Read the manual, if you want to create your own programs.

- The manual sometimes refers to the interface of the iOS app.
  For the desktop version read the First Steps in this file before.

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
	macOS: /Users/YourName/Library/Application Support/Inutilis Software/LowRes NX/settings.txt
	Windows: C:\Users\YourName\AppData\Roaming\Inutilis Software\LowRes NX\settings.txt

- Command line arguments:
	These override the options from the settings file.

	"LowRes NX" [-fullscreen yes/no] [-programs path] [program.nx]

	-fullscreen yes/no
	Starts the application in fullscreen mode
	
	-programs path
	Path for the tool programs and virtual disk file.
	By default it's the "programs" folder next to the LowRes NX
	application.

	program.nx
	Name of program to run


***************
* First Steps *
***************

To run a program, just open the LowRes NX application and drag and
drop any .nx file into its window. You can also select LowRes NX as
default application for .nx files, so programs can be started simply
by double clicking them.

For writing your own programs, use any text editor and save your files
with the .nx extension. On Windows make sure the text editor supports
Mac/Linux line ends, otherwise you may see everything in one line.

If you open NX tools (Character Designer, Background Designer, etc.)
directly like other NX programs, they will use the "Disk.nx" file in
the application's "programs" folder for loading and saving their data.

To edit your program's data directly (without the "Disk.nx" file),
run your program in the normal way. Then press Ctrl+M to enter the
Development Menu. There click on "ED" and select a tool. It will use
the current program for its data. When done, press Ctrl+M again to
return to the Development Menu, or Ctrl-R to run your program
directly.
