* create an @action decorator and apply it to action methods in states
* consider defining (typed?) decorators for lmdb.Dbi instances?
* make sessionid.sig unique for each session (so we can reset/invalidate just one session at a time)

* [DONE] Material UI step 3
* [DONE] Extract async and transient handling into a class
* [NEEDS TESTING] Implement non crash proof version
* [LATER (if at all)] Add insecure web socket
* [NEEDS TESTING] Add NaCl
* Enable multiple app types
* Develop all the statecharts and think through how the systems will be configured
* Implement and test the session and model layer
* Implement and test the GrapheQL queries
* Implement the admin UIs and iOS app
* Configure the systems

* Start QueryProcessor threads child processes automatically (in server_main.ts)
  * re-spawn QueryProcessor worker sockets when they exit
  * make sure they exit when their websocket connection dies
* Need an AuditingTracer for admin sessions that stores the email and date/time of each action

* How about a Node.js "console" app for certain admin actions that can only be run locally?

* Server Dropbox images
