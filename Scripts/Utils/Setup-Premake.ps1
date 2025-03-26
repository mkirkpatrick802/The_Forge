param(
    [string]$Url,
    [string]$Destination
)

# Check if parameters are provided
if (-not $Url -or -not $Destination) {
    Write-Host "Missing parameters."
    exit
}

# Define the path to the premake executable
$premakeExecutable = Join-Path -Path $Destination -ChildPath "premake5.exe"

# Check if Premake is already installed
if (Test-Path $premakeExecutable) {
    Write-Host "Premake is already installed at $Destination. Skipping installation."
    exit
}

# Create the destination directory if it does not exist
if (-not (Test-Path -Path $Destination)) {
    New-Item -Path $Destination -ItemType Directory
}

# Download the zip file
Write-Host "Downloading Premake from $Url..."
$zipPath = Join-Path -Path $Destination -ChildPath "premake.zip"
Invoke-WebRequest -Uri $Url -OutFile $zipPath

# Extract the zip file
Write-Host "Extracting Premake..."
Expand-Archive -Path $zipPath -DestinationPath $Destination -Force

# Clean up the zip file
Remove-Item -Path $zipPath

Write-Host "Premake downloaded and extracted successfully!"
