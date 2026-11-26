# A guide to writing RC scripts
I realize now that I never really made a good comprehensive guide on writing service scripts for this init, so it's time to rectify that.

This guide will go over all of the variables and metadata needed to write an RC script.

All RC scripts are just simple shell scripts, placed in /etc/rc.d (by default), and executed by the init system in a specific order.

## Metadata
There are 3 tags that must be specified, and 1 optional.
### PROVIDES: \<name\>
Parsed by rcorder(8), tells it what this script provides.
### REQUIRES: \<name...\>
Parsed by rcorder(8), tells it what needs to run first before running this. Specifying "none" signals that there are no dependencies.
### BEFORE: \<name...\>
Optional, parsed by rcorder(8), tells it to run this script before the scripts specified in this tag.
### KEYWORDS: ...
Parsed by rcorder(8) and rc.subr(8).

There are 3 important keywords for this tag: daemon, oneshot, critical. Anything else specified is ignored by rc.subr(8) and is only used by rcorder(8) if it's called with a -s or -k flag that matches any specified keyword.

daemon and oneshot should not be used together, only one or the other should be specified.
#### daemon
This signals that this script should be put into a service control group, so it can be monitored or stopped later.
#### oneshot
This signals that this script should just be run in init's control group, and the lifecycle of it should be ignored.
#### critical
Optional. If this is specified, the exit code of the executed command is propagated to whoever called the RC script.

As an example, if a script executed by rc(5) returns nonzero (which can only happen if the critical keyword is specified), it won't execute any more scripts, and will signal to init to spawn an emergency shell.
## Variables
All variables here will be of the form var\_ame: required|optional

These variables won't mean anything unless you source rc.subr(8) at the beginning of the script.

A note, all of the \*cmd\_args variables don't get parsed if the corresponding \*cmd variable isn't defined.
### name: required
The name of the service, which will usually match the name specified in PROVIDES.
### rcvar: required
The variable in rc.conf that will gate this script. Usually you should set this to ${name}\_enable.
### use\_cgroup: optional (defaults to yes if not specified)
Should only be either yes or no.

Only parsed if the oneshot keyword is specified. If set to no, the script won't be run in init's control group.

This should only be used by scripts that are executed before the cgroup filesystem is mounted.
### cgroup\_dir: optional
Specifies what directory the control group should be created in.

Very rare that this needs to be set, only useful for daemons like elogind, which can only be supervised by putting it in init's control group.
### start\_msg: optional (defaults to "/ Starting ${name}" if not specified)
The message to be printed when the start command is executed.
### stop\_msg: optional (defaults to "/ Stopping ${name}" if not specified)
The message to be printed when the stop command is executed.
### cmd: optional (though it should absolutely be specified in the majority of scripts)
The actual command to execute. Can either be a string or a shell function.
### cmd\_args: optional
The arguments to pass to the command.
### stop\_cmd: optional
The command to execute when the stop method is invoked. Can either be a string or a shell function.

On oneshot scripts, if this is not specified, stop\_msg doesn't get printed.
### stop\_cmd\_args: optional
The arguments to pass to the stop command.
### status\_cmd: optional
The command to execute when the status method is invoked. Can either be a string or a shell function.
### status\_cmd\_args: optional
The arguments to pass to the status command.
## Functions
The functions will be specified in the same way as the variables.
### run\_rc\_command: required
This will almost always be specified as run\_rc\_command "$1" at the very bottom of the script.
### load\_rc\_config: optional
This is implicitly called in run\_rc\_command, but if you use variables from rc.conf directly in your script, you should still call this right after sourcing rc.subr(8) to be absolutely sure the variables get sourced correctly.

## EXAMPLES
A note, the \#\#\# BEGIN rc-info and \#\#\# END rc-info don't mean anything to any of the metadata parsers, they're completely optional.
### Daemonizing script, using rundaemon(8) provided by this repo
```sh
#!/bin/sh
### BEGIN rc-info
# PROVIDES: thingd
# REQUIRES: EXTRA
# BEFORE: MULTIUSER
# KEYWORDS: daemon
### END rc-info

. /etc/rc.subr
name="thingd"
rcvar="${name}_enable"
start_msg="+ Starting thingd"
cmd="/sbin/rundaemon"
cmd_args="thingd --foreground"

run_rc_command "$1"
```
### oneshot script marked critical
```sh
#!/bin/sh
### BEGIN rc-info
# PROVIDES: important_thing
# REQUIRES: sysinit
# BEFORE: SYSTEM
# KEYWORDS: oneshot critical
### END rc-info

. /etc/rc.subr
load_rc_config
name="important_thing"
rcvar="${name}_enable"
start_msg=": Doing some important tasks"
cmd=do_things
# A note, if you want the start function to exit immediately upon any failure, you have to append " || return 1" to every command within the function.
do_things() {
    mkdir -p /run/important_directory || return 1
    if test "$SOME_IMPORTANT_RCVAR" != "correct"; then
        echo >&2 "You did not do this correctly"
        return 1
    fi
}

run_rc_command "$1"
```
### Pseudo-target (useful as a sort of checkpoint)
```sh
#!/bin/sh
### BEGIN rc-info
# PROVIDES: EXTRA
# REQUIRES: SYSTEM
# BEFORE: NETWORK MULTIUSER
# KEYWORDS: oneshot
### END rc-info

. /etc/rc.subr
name="EXTRA"
rcvar="${name}_enable"
start_msg="Reached target ${name}"

run_rc_command "$1"
```
### An actual script (for elogind, to demonstrate control group behavior)
```sh
#!/bin/sh
### BEGIN rc-info
# PROVIDES: elogind
# REQUIRES: dbus
# BEFORE: EXTRA
# KEYWORDS: daemon
### END rc-info

. /etc/rc.subr
name="elogind"
rcvar="elogind_enable"
cgroup_dir="/sys/fs/cgroup/bltinit/elogind"
start_msg="+ Starting elogind"
cmd="/sbin/rundaemon"
cmd_args="/usr/libexec/elogind/elogind"

run_rc_command "$1"
```
