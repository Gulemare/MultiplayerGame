protoc --cpp_out=. protocol.proto
protoc --js_out=library=../html5/public/js/generated/game_protocol,binary:. protocol.proto
pause