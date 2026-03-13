# reNut-Launcher
A lightweight launcher designed for rexglue-based projects.

reNut-Launcher is built to work with most rexglue projects with minimal setup.  
In most cases, you only need to configure the `.ini` file and adjust a few constants in the code (such as the number of backgrounds).

## Features

- Automatically fetches the latest EXE from a specified GitHub repository
- Retrieves rexglue CVAR definitions for startup parameters
- Displays recent commits
- Displays the contributors under a credits tab
- Built-in version checking that notifies users when updates are available
- Iso extraction

## Supported CVAR Types

Currently supported:
- `REXCVAR_DEFINE_BOOL`
- `REXCVAR_DEFINE_INT32`

Support for additional CVAR types will be added as reNut evolves.

## Building from source
Ive made a simple premake script and setup.bat that will automatically setup the .sln for Visual Studio 2022+

##
If the user has no Game/assets folder it will prompt them to select the iso for the game.
<img width="1283" height="755" alt="image" src="https://github.com/user-attachments/assets/fd41bae3-510b-46dd-a851-264d702347bf" />

While the Iso is extracting it will show this, Ideally we will eventually have a progress bar.
<img width="1285" height="759" alt="image" src="https://github.com/user-attachments/assets/920a7179-a21e-432d-b38e-ed0097f27af0" />

Once it detects that the Game/assets/*.xex exist and that the exe is absent it will show a download button.
<img width="1284" height="758" alt="image" src="https://github.com/user-attachments/assets/ea98c6e2-da60-404b-9dc0-95a0c6a8f2b1" />

While downloading it will show the progress.
<img width="1283" height="757" alt="image" src="https://github.com/user-attachments/assets/41c08710-82d6-4472-a686-f62954e93e85" />

If the game is up-to-date it will show the launch button.
<img width="1285" height="741" alt="image" src="https://github.com/user-attachments/assets/05974320-4048-4a97-87ca-76512bedf36c" />

While the game is running the launcher will not let you launch again until you close the game.
<img width="1867" height="1007" alt="image" src="https://github.com/user-attachments/assets/447364a8-5e9d-4f48-9390-26f2e8335776" />

If the launcher detects that the current verison is out of date it will prompt the user to update.
<img width="1321" height="787" alt="image" src="https://github.com/user-attachments/assets/f9264022-a479-4554-9197-1a76ecbc5cd6" />

