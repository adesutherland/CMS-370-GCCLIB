REM Copy and build source files to VM370
docker kill vm370
docker container rm vm370
docker run --rm -d -p 3270:3270 -p 8038:8038 -p 3505:3505 --name vm370 adriansutherland/vm370:builder

yata -c -f tmp.txt
echo USERID  CMSUSER > yata.txt
echo :READ  YATA     TXT      >> yata.txt
type tmp.txt >> yata.txt
docker cp yata.txt vm370:/opt/hercules/vm370/io
erase yata.txt
erase tmp.txt
docker exec vm370 bash -c "cd /opt/hercules/vm370/io && yata -x"
docker exec vm370 bash -c "rm /opt/hercules/vm370/io/yata.txt"
docker cp cmsbuild.sh vm370:/opt/hercules/vm370/io
docker cp cmsinstall.sh vm370:/opt/hercules/vm370/io

docker exec vm370 bash -c "cd /opt/hercules/vm370/io && ./cmsbuild.sh"
docker exec vm370 bash -c "cd /opt/hercules/vm370/io && ./cmsinstall.sh"
