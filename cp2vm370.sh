#!/bin/sh
# Copy and build source files to VM370

# STOP MSYS2 rewriting directory paths in the docker container
export MSYS2_ARG_CONV_EXCL="vm370:;/opt"

docker kill vm370
docker pull adriansutherland/vm370:builder
docker run --rm -d -p 3270:3270 -p 8038:8038 -p 3505:3505 --name vm370 adriansutherland/vm370:builder

yata -c -f tmp.txt
echo "USERID  CMSUSER\n:READ  ARCHIVE  YATA    " > archive.yata
cat tmp.txt >> archive.yata
docker cp archive.yata vm370:/opt/hercules/vm370/io
rm archive.yata
rm tmp.txt
docker exec vm370 bash -c "cd /opt/hercules/vm370/io && yata -x"
docker exec vm370 bash -c "rm /opt/hercules/vm370/io/archive.yata"

# Tools
yata -c -d tools -f tmp.txt
echo "USERID  CMSUSER\n:READ  ARCHIVE  YATA    " > archive.yata
cat tmp.txt >> archive.yata
docker exec vm370 bash -c "mkdir /opt/hercules/vm370/io/tools"
docker cp archive.yata vm370:/opt/hercules/vm370/io/tools
rm archive.yata
rm tmp.txt
docker exec vm370 bash -c "cd /opt/hercules/vm370/io/tools && yata -x"
docker exec vm370 bash -c "rm /opt/hercules/vm370/io/tools/archive.yata"

docker cp cmsbuild.sh vm370:/opt/hercules/vm370/io
docker exec vm370 bash -c "cd /opt/hercules/vm370/io && ./cmsbuild.sh"
