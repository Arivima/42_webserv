#!/usr/bin/bash

BODY="Hello from Bash!";
BODYLEN=${#BODY};
DELIMITER="\r\n";
CR=$(printf '\r');
LF=$(printf '\n');

echo -e $SERVER_PROTOCOL" 200 OK"$CR$LF;
echo -e "Content-Type: text/plain"$CR$LF;
echo -e "Content-Length: "$BODYLEN$CR$LF;
echo -e $CR$LF;
echo -e $BODY;
