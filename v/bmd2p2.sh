cd $CTP3_ROOT
#2.5.2024: note clocktansition_dcs.py is used only in p2/dcs environment, while
#          clocktransition.py is used in p2/ecs env
tar -cf - scripts/bin/bmd scripts/bin/bcmserver cmdpy/bmdim pywrap/*.py |ssh root@altri23 'cat - >/p2i/bmdimp2.tgz'
