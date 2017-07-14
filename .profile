
# Get the aliases and functions
PATH=$PATH:$HOME/bin:.			# set command search path
export PATH

HOME=/home/center
export HOME

if [ -z "$LOGNAME" ]; then
	LOGNAME=`logname`		# name of user who logged in
	export LOGNAME
fi


if [ -z "$PWD" ]; then
	PWD=$HOME			# assumes initial cwd is HOME
	export PWD
fi

if [ -f $HOME/.kshrc -a -r $HOME/.kshrc ]; then
	ENV=$HOME/.kshrc		# set ENV if there is an rc file
	export ENV
fi

DEV_HOME=/home/center/dev
export DEV_HOME

# set environment variables
# database
# for oracle
NLS_LANG=american_america.ZHS16GBK;export NLS_LANG
NLS_DATE_FORMAT="YYYYMMDD HH24:MI:SS";export NLS_DATE_FORMAT


PATH=$JAVA_HOME/bin:/usr/bin:/etc:/usr/sbin:/usr/local/bin:/usr/ucb:$HOME/sbin:$HOME/bin:/usr/bin/X11:/sbin:.:$HOME/db2sh:/usr/vacpp/bin

export PATH

if [ -s "$MAIL" ]           # This is at Shell startup.  In normal
then echo "$MAILMSG"        # operation, the Shell checks
fi                          # periodically.


case $LOGNAME in                        # include command number in prompt
root)   PS1='!$' ;;
*)      PS1='$PWD> ' ;;
esac
export PS1


EXINIT="set tabstop=4 nonu showmode autoindent nomagic nowrapscan"
export EXINIT

DBUSER=ccbfca;export DBUSER
DBPWD=ccbfca123;export DBPWD
DBSID=ORADB;export DBSID

DBUSER_CRT=ccbfca;export DBUSER_CRT
DBPWD_CRT=ccbfca123;export DBPWD_CRT
DBSID_CRT=ccbfca;export DBSID_CRT
DBUSER_SYN_CRT=ORADB;export DBUSER_SYN_CRT
DBPWD_SYN_CRT=ccbfca123;export DBPWD_SYN_CRT
ORACLE_SID=ORADB
ORACLE_BASE=/home/oracle/product/10.2.0/db_1
ORACLE_HOME=$ORACLE_BASE
ORA_NLS10=$ORACLE_HOME/nls/data


umask 022

LD_LIBRARY_PATH=$ORACLE_HOME/lib32:$ORACLE_HOME/lib:$HOME/dev/lib
LIBPATH=$HOME/dev/lib
PATH=$ORACLE_HOME/bin:/usr/sbin:/usr/local/bin:/usr/ccs/bin:$PATH
NLS_DATE_FORMAT="yyyy-mm-dd hh24:mi:ss"
BACKUP_PATH=$HOME/backup
export PS1 ORACLE_SID ORACLE_BASE ORACLE_HOME NLS_LANG ORA_NLS10 LD_LIBRARY_PATH NLS_DATE_FORMAT BACKUP_PATH LIBPATH
export PATH
TERM=vt100;export TERM
SHELL_PATH=/etc/paic/shell; export SHELL_PATH
TEMP=$HOME/tmp
export TEMP
EDITOR=vim
export EDITOR

TL_DBPWD_FILE=$HOME/etc/pwd.dat
export TL_DBPWD_FILE


SYS_PARAM_CFG=$HOME/etc/sys.cfg
export SYS_PARAM_CFG

THIS_IP_ADD=10.2.35.120
export THIS_IP_ADD

#alias
alias c='clear'
alias rm='rm -i '
alias mv='mv -i '
alias cp='cp -i '
alias lf='/bin/ls -l'
alias l='/bin/ls -lrt'
alias ls='/bin/ls -CF '
alias la='/bin/ls -aldF '
alias ls='/bin/ls -CF '
alias la='/bin/ls -aldF '
alias lt='/bin/ls -ltr '
alias h='history'
alias tmboot='Governor'

#for dev
alias cds='cd ~/dev/src'
alias cdh='cd ~/dev/include'
alias cdl='cd ~/log'

cd $HOME

#LANG=zh_CN
LANG=en_US
#LANG=Zh_CN.GB18030
export LANG



#banner "center"
echo "  ========= Only For dev     ====================="
echo "  =                                              ="
echo "  = 用户说明：     center                        ="
echo "  =                                              ="
echo "  =             center         常规开发       ="
echo "  =                                              ="
echo "  =========这是开发环境==========================="
echo ""
echo ""
