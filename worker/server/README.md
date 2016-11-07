# SNKRS Pop-up Server

You should already have Node.js and npm installed. You also
need to install TypeScript:

    $ npm install -g typescript

Then should should install the npm modules:

	$ npm install

Once that's done, what I'd do is open up two shells to the
`server/` directory and in one of them, run this command:

    $ tsc

That will create a `bin/` directory, and in it, the compiled
JavaScript files. You can leave it running, it will notice
any changes to your TypeScript files and re-compile them.

You can use the second shell to run git commands.

Drag the `server/` folder to the Visual Studio Code icon to
open up the project. You don't need/want to edit any of the
code in the `bin/` directory. Edit the code in the `src/`
directory instead (only).

To try out debugging, click on the `src/main.ts` file in
the "Explore" tab and on line 4, click in the gutter to set
a breakpoint. Then press F5 to start running Node.js. (You
can also click on the bug icon on the far right to open the
debugging screen, and near the top, press the green "play"
icon.)

The app will start running and should pause on line 4 in
`main.ts`. Note that even though Node.js is running the
JavaScript compiled by `tsc`, you're still able to set
breakpoints in the TypeScript source code and debug there.

In the center at the top are the debugger commands. When
you are done debugging, press the "stop" icon to shut
down Node.js.