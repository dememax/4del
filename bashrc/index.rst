####################
My changes to bashrc
####################

PS1 - primary prompt string
***************************

.. code-block:: bash
   :caption: Overriding/setting history env vars

   # Added by Max on 2025-05-12
   PROMPT_COMMAND=__prompt_command    # Function to generate PS1 after CMDs

   __prompt_command() {
       local EXIT="$?"                # This needs to be first
       PS1='\[\033]0;\u@\h:\w\007\]\[\033[01;32m\]\u@\h\[\033[01;34m\] \w \$\[\033[00m\] '
       if [ $EXIT != 0 ]; then
           PS1=" \[\e[0;31m\]$EXIT!\[\033[00m\]
   $PS1"
       else
           PS1=" \[\e[32m\]✔️\[\e[0m\]
   $PS1"
       fi
   }

Command history
***************

.. code-block:: bash
   :caption: Overriding/setting history env vars

   # Commented by Max on 2025-05-12
   # HISTCONTROL=ignoreboth

   # append to the history file, don't overwrite it
   shopt -s histappend

   # for setting history length see HISTSIZE and HISTFILESIZE in bash(1)
   # Changed by Max on 2025-05-12
   HISTSIZE=8000
   HISTFILESIZE=8000
   HISTTIMEFORMAT="%F %T "

Path
****

.. code-block:: bash
   :caption: Adding my scripts on the path

   # Added by Max on 2025-05-12
   PATH=~/opt/bin:$PATH

Debian/Ubuntu env vars
**********************

.. code-block:: bash
   :caption: Overriding/setting history env vars

   VISUAL=nano
   EDITOR=nano
   PAGER=less
