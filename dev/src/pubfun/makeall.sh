#!/usr/bin/ksh

#. ~/.profile

###############################################
#Auth: wucg
#Date: 2011-01-26
#Disc: 底层函数编译脚本
###############################################

#进入批量源程序代码目录
CURDIR="/home/center/dev/src/pubfun"

cd $CURDIR

#公共低层库源程序目录，要优先编译, 先后顺序不能乱改
DIR_LIST="\

"

#优先编译低层库
for DIR_NAME in $DIR_LIST
do
  if [ "a$DIR_NAME" = "a" ];then
     continue
  fi

  #进入目录
  cd $CURDIR/$DIR_NAME

  #如果存在Makefile,则编译
  if [ -f Makefile ]; then
       echo "wait  compileing............ ";
#      make clean all
  fi

  #如果存在makefile, 则编译
  if [ -f makefile ]; then
     echo "wait  compileing............ ";
#     make clean all
  fi

done

cd $CURDIR

ls -1 > makefile_dir.list

while read LINE
do
    cd $CURDIR
    if [ $? -ne 0 ]; then
       echo "cd $LINE err"
       exit 1
    fi

    DIRFLAG=0

    #判断是否是否是底层库目录，如果是，则跳出
    for DIR_NAME in $DIR_LIST
    do
      if [ "a$DIR_NAME" = "a" ];then
        continue
      fi

      if [ "a$DIR_NAME/" = "a$LINE" ];then
        DIRFLAG=1
      fi
    done

    #如果是底层库目录，跳出
    if [ "$DIRFLAG" = "1" ]; then
       continue
    fi

    #判断是否是文件
    if [ -f $LINE ]; then
       echo "$LINE is file"
       continue
    fi

    cd $LINE

    #如果有再有下级目录, 则在下级目录下写个makeall.sh
    if [ -f makeall.sh ]; then
        sh ./makeall.sh
        continue
    fi

    if [ -f Makefile ]; then
       make -f Makefile clean all
       continue
    fi

    if [ -f makefile ]; then
       make clean all
       continue
    fi

done <makefile_dir.list

#进入编译源程序总目录，删除临时文件
cd $CURDIR
rm -f makefile_dir.list

echo "makeall compiled success !!!!"

exit 0;
