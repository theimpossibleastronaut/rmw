Thank you for thinking about becoming a contributor!

## First Steps ##
1. You may join the rmw support and development chat in the [Slack workspace](https://join.slack.com/t/removetowaste/shared_invite/enQtMjU3NTA0NTI2OTgzLTkzMzQxNDhjYzlkM2UxMTA2MzJjNWYyZjAyYzkyNWNmZjJmYWZmYjUyODk2NzNkNzBhMzFjOGZkMTg2MzAxMTM)
2. If you're not a programmer but are interested in helping, please feel free to submit bug reports, general feedback, or suggestions about the interface.
4. [fork the rmw repo](https://github.com/andy5995/rmw#fork-destination-box) and clone it to your computer
3. Try building it (don't worry about this if you are working on a patch where testing the binary is not necessary)
4. Try using it. By default, rmw will not use your regular Trash folder, but instead will create and store rmw'ed files in use ~/.trash.rmw. Some tips for testing are at [https://github.com/andy5995/rmw/wiki/Testing](https://github.com/andy5995/rmw/wiki/Testing)
5. Read through the [open issues](https://github.com/andy5995/rmw/issues) and see if anything interests you. If you see something you think you can do, but you're not sure, ask me about specifics.

I'm willing to [answer questions or offer tips](https://join.slack.com/t/removetowaste/shared_invite/enQtMjU3NTA0NTI2OTgzLTkzMzQxNDhjYzlkM2UxMTA2MzJjNWYyZjAyYzkyNWNmZjJmYWZmYjUyODk2NzNkNzBhMzFjOGZkMTg2MzAxMTM) on how to get started on any of the [issues listed](https://github.com/andy5995/rmw/issues). If you get stuck on a patch, I'll be available to discuss on how to move forward.

## Coding Standards ##
My goal is to use [GNU Coding Standards](https://www.gnu.org/prep/standards/html_node/Formatting.html#Formatting) (See ticket [#11](https://github.com/andy5995/rmw/issues/10))

## Credits ##
If you submit a patch, please add yourself (along with your personal link) to [AUTHORS.md](https://github.com/andy5995/rmw/blob/master/AUTHORS.md)

## Pull Requests ##
1. Join the [chat room](https://matrix.to/#/!XeJxcdkywroPaRKKtr:matrix.org) so if any problems arise, you can get support.
1. [Fork the rmw repo](https://github.com/andy5995/rmw/fork)
2. Clone it to your computer
3. When you're ready to work on an issue, be sure you're on the **master** branch. From there, [create a separate branch](https://github.com/Kunena/Kunena-Forum/wiki/Create-a-new-branch-with-git-and-manage-branches) (e.g. issue_32)
4. Make your changes. If you're unsure of some details while you're making edits, you can discuss them with me in the [chat room (Slack workspace)](https://join.slack.com/t/removetowaste/shared_invite/enQtMjU3NTA0NTI2OTgzLTkzMzQxNDhjYzlkM2UxMTA2MzJjNWYyZjAyYzkyNWNmZjJmYWZmYjUyODk2NzNkNzBhMzFjOGZkMTg2MzAxMTM) or the [RMW SourceForge Forum](https://sourceforge.net/p/rmw/discussion/).
5. Commit your changes. [git-cola](https://git-cola.github.io/) is a nice GUI front-end for adding files and entering commit messages (git-cola is probably available from your OS repository).
6. Push the working branch (e.g. issue_32) to your remote fork.
7. Make the pull request (on the [upstream **master** branch](https://github.com/andy5995/rmw/tree/master))
    * Do not merge it with the master branch on your fork. That would result in multiple, or unrelated patches being included in a single PR.
8. If any further changes need to be made, I will discuss them with you.

## Syncing ##
Periodically, you'll need to sync your repo with mine (the upstream). GitHub has instructions for doing this

* [Configuring a remote for a fork](https://help.github.com/articles/configuring-a-remote-for-a-fork/)
  * For step 3 on that page, use https://github.com/andy5995/rmw.git for the URL.
* [Syncing a Fork](https://help.github.com/articles/syncing-a-fork/)
  * On that page, it shows how to merge the **master** branch (steps 4 & 5).

## Contact ##
Up-to-date contact information can be found on the [Home Page](https://github.com/andy5995/rmw/wiki)
