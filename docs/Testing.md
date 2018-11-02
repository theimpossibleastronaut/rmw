## Using make

The easiest way is to use the tests built into 'make'. See [test/README](https://github.com/theimpossibleastronaut/rmw/tree/master/test) for instructions. To learn the "inner-workings" of rmw, continue reading this page.

## Method 1
By default (specified in the config file) rmw will not perform operations on your regular Trash folder; the configuration option for that is

$HOME/.trash.rmw

rmw will create that folder the first time it is run.

Your personal configuration file will be ~/.config/rmw/config. To create that file, copy etc/rmwrc 
to ~/.config/rmw/ and rename it to `config'.

One method of testing is to change the HOME environmental variable at run time. At the command line, use:

    HOME=$PWD rmw
    (use `./rmw` if the program is not yet in your PATH)

rmw would detect your home directory as your present working directory, and would create the necessary directories. Now create a blank file:

    touch tempfile

And then remove it to your temporary rmw trash folder:

    HOME=$PWD rmw tempfile

tempfile would be moved to $PWD/.trash.rmw/files, and a `.trashinfo` file will be created in $PWD/.trash.rmw/info. The format of the .trashinfo file is the same that your Desktop trash uses:

    [Trash Info]
    Path=/home/andy/src/rmw/tempfile
    DeletionDate=2017-11-17T13:44:04a

Now try

    HOME=$PWD rmw -u

You'll see that tempfile has been restored to its original location.

## Method 2
The following method is a little more complex

In rmw.h, change DATA_DIR
`#define DATA_DIR "/testing/.config/testrmw"`

Then build it.

Run the rebuilt binary to create that folder.
Then copy '/etc/rmwrc' (if it's not there, try /usr/local/etc/ or the rmw source directory (./etc)) into it, and rename the newly-placed 'rmwrc' to 'config'.

Add a WASTE folder to the top of 'config', such as
WASTE=$HOME/testing/.local/share/Trash.test

Comment out any other WASTE folders.

The current development version will create directories and their parents; however, if you're using the last release,
make the directory manually:
`~: mkdir $HOME/testing/.local/share/Trash.test`

**Copy all the files from your regular Trash into the test folder:**
```
cd $HOME/testing/.local/share/Trash.test
cp -a $HOME/.local/share/Trash/* .
```
Now run 'ls'. You should see 2 directories: files, info

Now 'cd' back to $HOME/testing

Create some unnecessary files.
`touch {a..z}`
will create 26 empty files, a-z.

`touch {1..10}`
will create 10 empty files, 1-10

You can now use 'rmw' on these files, or copy real files and directories into ~/testing to try it out.

## Testing the purge feature
 
Setting the RMWTRASH environmental variable at run-time to "fake-year" will write the year 1999 to the DeletionDate string in the .trashinfo file.

    RMWTRASH=fake-year rmw some.txt temp.asc files.doc

Then run rmw with the purge option

    rmw -fg

## Other Environmental Variables

RMW_TESTBUF e.g. `RMW_TESTBUF=20` will make rmw think the maximum allowed string length is 20. As a result, it will test the `bufchk` function to make sure it's working as intended. Most strings in the program will exceed 20 chars so the `bufchk` function should catch the problem.