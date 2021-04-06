#!/usr/bin/bash
echo "starting msvc environment"
if [[ ! -v VCVARS64 ]]; then
  export VCVARS64='"'$(powershell.exe build_scripts/windows/find_vcvars64.ps1)'"'
fi
# Set Visual Studio environment variables
cmd='WSLENV=VCVARS64/w cmd.exe /C %VCVARS64% \&\& wsl.exe'
if [[ $# -ge 1 ]]; then
  cmd+=" < $1"
fi
echo "running command: $cmd"
eval $cmd
