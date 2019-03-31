
# Dowload and unzip closure compiler, rename it to compiler.jar
wget "http://dl.google.com/closure-compiler/compiler-latest.zip" -outfile compiler.zip
Expand-Archive -LiteralPath compiler.zip -DestinationPath "./compiler" -Force
Remove-Item compiler/compiler.jar -ErrorAction Ignore
Get-ChildItem compiler/*.jar | Rename-Item -NewName compiler.jar
Remove-Item compiler.zip