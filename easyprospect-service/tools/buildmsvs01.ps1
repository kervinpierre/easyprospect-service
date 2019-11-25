# https://github.com/deadlydog/Invoke-MsBuild
#Import-Module -Name "$PSScriptRoot\Invoke-MsBuild.psm1"
Import-Module -Name "Invoke-MsBuild"

## Switch script root.  This should also be the project source root
#$currentPath=Split-Path ((Get-Variable MyInvocation -Scope 0).Value).MyCommand.Path
#Push-Location $currentPath
#[Environment]::CurrentDirectory = $PWD

#N.B. We should be in the $ProjectRoot/build/ directory

# Build
#  /t:Clean,Build 
$buildResult = Invoke-MsBuild -ShowBuildOutputInCurrentWindow `
				-Path "..\easyprospect-service.sln" `
				-Params "/target:Clean;Rebuild      `
				         /verbosity:normal          `
						 /property:Configuration=Debug"

$buildCodeExit = 0

$buildTime = (get-date).ToUniversalTime().ToString("yyyyMMdd-HHmmssfffffffZ")
if($buildResult.BuildSucceeded -eq $true)
{ 
	$buildTime | Set-Content 'build.txt'
	Write-Host "Build completed successfully. " 	
}
elseif ($buildResult.BuildSucceeded -eq $false)
{ 
	Write-Host "Build failed. Check $($buildResult.BuildLogFilePath)." 
	$buildCodeExit = 1
}
else
{ 
	Write-Host "Unsure of result: $($buildResult.Message)" 
	$buildCodeExit = 1 
}

#Pop-Location
#[Environment]::CurrentDirectory = $PWD

if( !$buildCodeExit -eq 0 )
{
	exit $buildCodeExit
}
