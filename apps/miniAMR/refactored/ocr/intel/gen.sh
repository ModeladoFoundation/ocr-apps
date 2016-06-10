cp df dforig_withEDT
#grep -v "NOS:EDT" df > dftemp; mv dftemp df
cp df dforig_noEDT
rm df.sh
rm XX*
grep grep df > df.sh
grep -v grep df > dftemp; mv dftemp df
chmod +x df.sh
./df.sh
