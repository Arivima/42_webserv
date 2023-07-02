#!/usr/bin/bash

BODY="Hello from Bash!";
BODYLEN=${#BODY};
DELIMITER="\r\n";

echo "200 OK "$SERVER_PROTOCOL$DELIMITER
echo "Content-Type: text/plain"$DELIMITER
echo "Content-Length: "$BODYLEN$DELIMITER
echo $DELIMITER$DELIMITER
echo $BODY
