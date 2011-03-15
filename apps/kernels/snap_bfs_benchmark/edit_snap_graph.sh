m=`head -n 1  $1|awk '{print $2}'`
ne=`wc -l  $1|awk '{print $1}'`
ne=$(($ne-1))
echo "p $m $ne u u 0" > $2
`cat $1 | grep -v "^p" >> $2`