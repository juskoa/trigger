#/bin/bash
function mkln() {
rm -f $2
ln -sf $1/$2 $2
}
if [ `hostname` = "avmes" -o `hostname` = "alidcscom835" ] ;then
  DIR=/home/dl6/local/trigger
  hm=/home/alice/trigger
elif [ `hostname` = "alidcscom188" ] ;then
  DIR=/data/dl/root/usr/local/trigger/stable
  hm=/home/alice/trigger
else
  exit
  DIR=/data/dl/root/usr/local/trigger/devel
  hm=/home/trigger
fi
cd /var/www/html
mkln $DIR/v/vme/dimcdistrib cnames.sorted2
mkln $DIR/v/vme CNTWEB
mkln CNTWEB index.html
rm -f CS
su - -c "mkdir -p $DIR/v/vme/CFG/ctp/DB/fs_auto" trigger
su - -c "mkdir -p $DIR/v/vme/CFG/ctp/DB/fs" trigger
ln -sf $DIR/v/vme/CFG/ctp/DB/COLLISIONS.SCHEDULE CS
mkln $DIR/v/vme/CFG/ctp/DB fs
mkln $DIR/v/vme/CFG/ctp/DB fs_auto
rm -f ltus
ln -sf $DIR/v/vme/PHPMON ltus
su - -c 'cd;mkdir -p CNTRRD/rawcnts' trigger
mkln $hm/CNTRRD htmls
mkln $hm/CNTRRD rawcnts
mkln $hm writeups
