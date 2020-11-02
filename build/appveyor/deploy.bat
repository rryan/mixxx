set USER=mixxx
set HOSTNAME=downloads-hostgator.mixxx.org
set APPVEYOR_DESTDIR=public_html/downloads/builds/appveyor
set SSH_KEY=..\build\certificates\downloads-hostgator.mixxx.org.key
set SSH="ssh -i ${SSH_KEY} -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null"
set DEST_PATH=%APPVEYOR_DESTDIR%/%APPVEYOR_REPO_BRANCH%/
set TMP_PATH=%APPVEYOR_DESTDIR%/.tmp/%APPVEYOR_BUILD_ID%/

echo CWD=%cd%
echo Deploying to %TMP_PATH%, then to %DEST_PATH%.

:: "Unlock" the key by removing its password. This is easier than messing with ssh-agent.
ssh-keygen -p -P %DOWNLOADS_HOSTGATOR_DOT_MIXXX_DOT_ORG_KEY_PASSWORD% -N "" -f %SSH_KEY%

:: Always upload to a temporary path.
rsync -e "%SSH%" --rsync-path="mkdir -p %TMP_PATH% && rsync" -r --delete-after --quiet .\*.exe %USER%@%HOSTNAME%:%TMP_PATH%

:: Move from the temporary path to the final destination.
%SSH% %USER%@%HOSTNAME% "mkdir -p %DEST_PATH% && mv %TMP_PATH%/* %DEST_PATH% && rmdir %TMP_PATH%"
