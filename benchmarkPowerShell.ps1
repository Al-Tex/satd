param (
    [string]$TestDirectory,    # Folder containing files
    [string]$ExecutablePath,  # Path to the executable
    [int]$MaxProcesses  # Number of concurrent processes
)

# Check if input folder exists
if (!(Test-Path -Path $TestDirectory -PathType Container)) {
    Write-Error "Input folder does not exist."
    exit
}

# Get executable name without extension
$ExecutableName = [System.IO.Path]::GetFileNameWithoutExtension($ExecutablePath)
$ExecutableName = $ExecutableName +".exe_results"


# Create a folder named after the executable in the current directory
$BenchmarkFolder = Join-Path -Path $TestDirectory -ChildPath $ExecutableName
if (!(Test-Path -Path $BenchmarkFolder)) {
    New-Item -ItemType Directory -Path $BenchmarkFolder
}

# Create benchmark_results.csv file in the created folder
$BenchmarkFile = Join-Path -Path $BenchmarkFolder -ChildPath "benchmark_results.csv"
if (!(Test-Path -Path $BenchmarkFile)) {
	"Test,Satisfiable?,Time,Memory(B),Memory(KB),Memory(MB),Memory(GB)" | Out-File -FilePath $BenchmarkFile -Encoding UTF8    
#New-Item -ItemType File -Path $BenchmarkFile
}

# Iterate through each file in the input folder
$Files = Get-ChildItem -Path $TestDirectory -File

# Process files in parallel with a maximum number of processes
#$RunningProcesses = @()
$RunningProcesses = New-Object 'System.Collections.Generic.List[System.Diagnostics.Process]'
foreach ($File in $Files) {
    $OutputFile = Join-Path -Path $BenchmarkFolder -ChildPath "benchmark_results.csv"
    #$StartTime = Get-Date

    # Start process
    $Process = Start-Process -FilePath $ExecutablePath -ArgumentList "$($File.FullName) $OutputFile" -PassThru -NoNewWindow
    $RunningProcesses += ,$Process
    #$RunningProcesses.Add($Process)
    # Wait if max processes are running
    while ($RunningProcesses.Count -ge $MaxProcesses) {
        $RunningProcesses = $RunningProcesses | Where-Object { $_.HasExited -eq $false }
        Start-Sleep -Milliseconds 2
    }

    #$EndTime = Get-Date
    #$ExecutionTime = $EndTime - $StartTime

    # Append execution results to benchmark_results.csv
    #"$($File.Name), , $($ExecutionTime.TotalSeconds), , , ," | Out-File -FilePath 0$BenchmarkFile -Append -Encoding UTF8
}

# Wait for all processes to finish
$RunningProcesses | ForEach-Object { $_.WaitForExit() }

Write-Host "Done"
