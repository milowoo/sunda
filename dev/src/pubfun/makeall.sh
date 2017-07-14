#!/usr/bin/ksh

#. ~/.profile

###############################################
#Auth: wucg
#Date: 2011-01-26
#Disc: �ײ㺯������ű�
###############################################

#��������Դ�������Ŀ¼
CURDIR="/home/center/dev/src/pubfun"

cd $CURDIR

#�����Ͳ��Դ����Ŀ¼��Ҫ���ȱ���, �Ⱥ�˳�����Ҹ�
DIR_LIST="\

"

#���ȱ���Ͳ��
for DIR_NAME in $DIR_LIST
do
  if [ "a$DIR_NAME" = "a" ];then
     continue
  fi

  #����Ŀ¼
  cd $CURDIR/$DIR_NAME

  #�������Makefile,�����
  if [ -f Makefile ]; then
       echo "wait  compileing............ ";
#      make clean all
  fi

  #�������makefile, �����
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

    #�ж��Ƿ��Ƿ��ǵײ��Ŀ¼������ǣ�������
    for DIR_NAME in $DIR_LIST
    do
      if [ "a$DIR_NAME" = "a" ];then
        continue
      fi

      if [ "a$DIR_NAME/" = "a$LINE" ];then
        DIRFLAG=1
      fi
    done

    #����ǵײ��Ŀ¼������
    if [ "$DIRFLAG" = "1" ]; then
       continue
    fi

    #�ж��Ƿ����ļ�
    if [ -f $LINE ]; then
       echo "$LINE is file"
       continue
    fi

    cd $LINE

    #����������¼�Ŀ¼, �����¼�Ŀ¼��д��makeall.sh
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

#�������Դ������Ŀ¼��ɾ����ʱ�ļ�
cd $CURDIR
rm -f makefile_dir.list

echo "makeall compiled success !!!!"

exit 0;
