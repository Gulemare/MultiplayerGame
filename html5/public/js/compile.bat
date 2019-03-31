REM Compile all dependencies including protobuf to single script

SET dest=protobuf\js\res
mkdir %dest%
copy protobuf\js\map.js %dest%
copy protobuf\js\message.js %dest%
copy protobuf\js\binary\arith.js %dest%
copy protobuf\js\binary\constants.js %dest%
copy protobuf\js\binary\decoder.js %dest%
copy protobuf\js\binary\encoder.js %dest%
copy protobuf\js\binary\reader.js %dest%
copy protobuf\js\binary\utils.js %dest%
copy protobuf\js\binary\writer.js %dest%

python.exe "closure-library/closure/bin/build/closurebuilder.py" --root=closure-library/ --root=protobuf/js/res/ --root=generated/ --root=src/ --namespace="game.startClient" --output_mode=compiled --compiler_jar=compiler/compiler.jar > client_compiled.js