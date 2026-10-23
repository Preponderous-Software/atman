Atman
=====

A first-person game about Advaita Vedanta. You wake in a river village on
a spring afternoon, with a body, a name and a list of chores. At dusk
there are snakes on the path to the ghat. The potter has a wheel. Nothing
here is scored.

Play
----

Windows:  double-click atman.exe. Windows may say it protected your PC,
          because the game is not signed: choose "More info", then
          "Run anyway".
Linux:    ./atman   (needs OpenGL 4.5, which any current Mesa or vendor
          driver has, and glibc 2.34 or newer: Ubuntu 22.04, Debian 12,
          Fedora 35 and later)

Press F1 in the game for the controls, and M to turn the sound off or on.
Click in the window to look around with the mouse; Esc frees the mouse,
and Esc again quits. The NEXT line at the top left says what comes next.

The game saves on its own every two minutes, on F5 and when you quit, and
continues where you left off (F9 loads the last save):

  Windows:  %APPDATA%\Atman\default
  Linux:    ~/.local/share/atman/default  (or $XDG_DATA_HOME/atman/default)

  atman --new          start over (the old save is kept as the previous one)
  atman --slot DIR     keep the save somewhere else
  atman --autopilot --speed 10
                       watch the autopilot play it (it never saves)

More: https://github.com/Preponderous-Software/atman
Play in a browser: https://atman.play.danielstephenson.dev

Atman is MIT-licensed (LICENSE.txt). It is built on the Strata engine;
THIRD-PARTY.md lists what that links in, and their licences.
