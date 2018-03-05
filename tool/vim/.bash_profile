# .bash_profile

# Get the aliases and functions
if [ -f ~/.bashrc ]; then
	. ~/.bashrc
fi

# User specific environment and startup programs

PATH=/home/zhangxiang17/project/tool/scp:/home/zhangxiang17/install/bin:$PATH:$HOME/bin
export C_INCLUDE_PATH=/home/zhangxiang17/install/include/:$C_INCLUDE_PATH
export LD_LIBRARY_PATH=/home/zhangxiang17/install/lib:$LD_LIBRARY_PATH

export PATH
export PS1='\[\033[01;32m\]\u@\t\[\033[00m\]:\[\033[01;36m\]\W\[\033[00m\]\$ '                                                 
export CLICOLOR=1                                                                                                              
export LSCOLORS=bxfxhxhxgxhxhxgxgxbxbx
