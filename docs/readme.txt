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
the tool's folder for loading and saving their data.

To edit your program's data directly (without the "Disk.nx" file),
run your program in the normal way. Then press Esc to enter the
Development Menu. There click on "ED" and select a tool. It will use
the current program for its data. When done, press Esc again to
return to the Development Menu, or Ctrl+R to run your program
directly.


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
	
	-disabledev yes/no
	Disable the Development Menu, ESC key quits LowRes NX

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
