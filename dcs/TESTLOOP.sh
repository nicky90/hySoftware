#!/bin/sh

if [ $# -eq 2 ]
then
  # orbhandler
  cd /home/nsoas/DATA/$1/; rm -rf $2
elif [ $# -eq 3 ]
then
  # chnhandler
  echo
elif [ $# -eq 4 ]
then
  # filehandler
  cd /home/nsoas/DATA/$1/$2/$3
  filename=$4
  prename=${filename:0:32}
  mv $4 ${prename}.dat
  ls -l ${prename}.dat | awk '{print $9":"$5}' > ${prename}.fin
  mv ${prename}.dat ${prename}.fin /home/nsoas/DATA/$1
else
  # wrong args
  echo 
fi

exit 0
