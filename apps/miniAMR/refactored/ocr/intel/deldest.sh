rm df.sh
fgrep -l eliminated XX* > df.sh
vi -c ":1,\$s/XX/rm XX/" -c ":wq" df.sh
chmod +x df.sh
./df.sh
rm df.sh
