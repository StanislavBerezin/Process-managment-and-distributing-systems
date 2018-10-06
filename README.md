# TO START

1. To start the files type `make`
2. If made changes to files and wish to see updated app type `make clean` and then `make`, make clean will
   delete the files and the following make will recreate them.

# Specification

A user can either reveal a tile or flag (in case he believes there is a bomb)

# Server:

If you start ./server 12345, then it means a server shall start on port 12345

1. All logic on server side, mines creation etc.
2. Then the grid is send to client, a client makes a move which is send to server
3. Server reads the input and simulates the results and sends the updated grid to client
4. Server is responsible for ensuring only authenticated players to play
5. Time should be used to avoid cheating

# Client:

If you start ./client NAME 12345, then it means NAME for name, and 12345 is the server to connect to
Client should not recieve the coordinates of all mines.

1. Welcome scree asking to enter credentials (only auth users can play), if success proceed to the game, else shut down socket connection
2. Once successful the following should appear:

```
Welcome to minesweeper gaming system.

Please enter a selection:
<1> Play the game
<2> Show leaderboard
<3> Quit

Select option (1-3):
```

# Play the game

```
If <1>{
    should allocate all the memory
    create 9x9 grid tiles
    all should be hidden
    game state could be stored in struct
}
```

And the client could look like this:

```
Remaining mines: 10

    1  2  3  4  5  6  7  8  9
---------------------------------
A |
B |
C |
..
..
..


Chose an option:
<R> Reveal tile
<P> Place flag
<Q> Quit game

Option (R,P,Q):
```

The client eneters X, Y coordinates and the input is sent to server, which checks if that coordinate has already been
used before, if YES then an error is send to client.
If no, then the server checks the number of adjecent bombs, if 0 around then that tile is marked 0, if 3 around, then its marked
as 3.
If a client hits a mines then a game over should be send with coordinates of all mines.

If a client opts to place a flag `+`, a client should provide coordinates and the results send to server
server should compare the coordinates and if it was correct then total number of mines `Remaining mines: 10` should be decremented. If all mines have been flaged then a user wins and client returns to the main menu.

# Show leader board

Should display name, total secs played, how many games won, and how many games played
If no games have been won, then it should display to user empty board saying "No games played"

# Multithreaded programming (84%)

Allow multiple clients to play at the same time. No one should be able to access the leaderboard while other players are playing Critical - Section problem. Can display something "The game is still on"

One potential problem that you must handle in this multithreaded programming task is a socalled
Critical-Section Problem because the leaderboard is a database that may be shared
among multiple concurrent connections, each of which may read or update the database. You
must make sure that there will no connection that is updating the leaderboard while other
connections are reading it. However, it is fine to allow multiple connections to read the
leaderboard concurrently.

This critical-section problem is similar to the Readers-Writers Problem discussed in Lecture 5.
Thus, you may reuse the solution to the Readers-Writers Problem. In this critical-section
problem, each client (connection) may play the role of Reader or Writer at a time. While the
client (connection) plays the Minesweeper game, its role is Writer as at the end of the game, it
will update the leaderboard; while the client (connection) displays the leaderboard, its role is
Reader as it only read data from the leaderboard, but never update the leaderboard.
A second issue to note is the random number generator. As C’s rand() function is not threadsafe,
accesses to this function must be synchronised (e.g. with a mutex).

# Threadpool (100%)

The server will allow up to 10 clients to use the system at the same time. After the maximum
of 10 clients have connected, if a new client tries to connect either the new client will have to
wait for another user to exit or the connection may be dropped.

You will need to implement a thread pool where each incoming connection is handled in a
separate thread. A thread pool reduces the cost of constantly creating and destroying threads to
handle requests. Instead, threads are reused to handle connections. It also limits the number of
incoming connections to guarantee the performance of the server.

You can implement your own Thread Pool reusing the producer consumer pattern from the
weekly practicals, where the main thread accepts incoming sockets and places them in a queue
(the producer) and 10 threads remove sockets from the queue and process the connection (the consumer). You may also use or adapt the complete Thread Pool implementation provided in
the weekly practicals.
