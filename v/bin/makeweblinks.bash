#/bin/bash
function mkln() {
rm -f $2
ln -sf $1/$2 $2
}
if [ `hostname` = "alidcscom188" ] ;then
  DIR=/data/dl/root/usr/local/trigger/stable
  hm=/home/alice/trigger
else
  DIR=/data/dl/root/usr/local/trigger/devel
  hm=/home/trigger
fi
cd /var/www/html
mkln $DIR/v/vme/dimcdistrib cnames.sorted2
mkln $DIR/v/vme CNTWEB
mkln CNTWEB index.html
rm -f CS
ln -sf $DIR/v/vme/CFG/ctp/DB/COLLISIONS.SCHEDULE CS
mkln $DIR/v/vme/CFG/ctp/DB fs
mkln $DIR/v/vme/CFG/ctp/DB fs_auto
rm -f ltus
ln -sf $DIR/v/vme/PHPMON ltus
mkln $hm/CNTRRD htmls
mkln $hm/CNTRRD rawcnts
mkln $hm writeups
