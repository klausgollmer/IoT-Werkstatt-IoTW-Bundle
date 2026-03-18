Param(
  [string]$PortableRoot = (Get-Location).Path,
  [string]$FQBN         = 'esp32:esp32:esp32',
  [switch]$ForceRebuild = $false,
  [switch]$RebuildIfSourcesNewer = $true
)

# =========================== Helper: die Basics ===========================
function Throw-IfMissing([string]$path, [string]$what) {
  if (-not $path -or -not (Test-Path $path)) { throw "$what nicht gefunden: $path" }
}

# pkg/arch aus FQBN ableiten
$__parts = $FQBN -split ':'
if ($__parts.Count -lt 3) { throw "Ungültiges FQBN: $FQBN" }
$pkg  = $__parts[0]
$arch = $__parts[1]

Write-Host ("=== Precompile Core-Libraries (pkg={0}, arch={1}, fqbn={2}) ===" -f $pkg,$arch,$FQBN) -ForegroundColor Cyan

# portable erkennen
if (Test-Path (Join-Path $PortableRoot "arduino\portable")) {
  $PortableRoot = Join-Path $PortableRoot "arduino"
}
$portableDir = Join-Path $PortableRoot "portable"
Throw-IfMissing $portableDir "portable-Verzeichnis"

# Arduino EXE (GUI; der Builder steckt drin)
$ArduinoExe = Join-Path $PortableRoot "arduino_debug.exe"
if (-not (Test-Path $ArduinoExe)) { $ArduinoExe = Join-Path $PortableRoot "arduino.exe" }
Throw-IfMissing $ArduinoExe "arduino(.exe)"

# Plattform-Libs-Root (neueste Version)
function Get-PlatformLibsRoot {
  param($portableDir,$pkg,$arch)
  $root = Join-Path $portableDir "packages\$pkg\hardware\$arch"
  if (-not (Test-Path $root)) { throw "Kein Hardware-Ordner: $root" }
  $ver = Get-ChildItem -Path $root -Directory | Sort-Object Name -Descending | Select-Object -First 1
  if (-not $ver) { throw "Keine Plattform-Version unter $root gefunden" }
  return (Join-Path $ver.FullName "libraries")
}
$CoreLibsRoot = Get-PlatformLibsRoot -portableDir $portableDir -pkg $pkg -arch $arch
Write-Host ("Core-Libs: {0}" -f $CoreLibsRoot)

# ar.exe für jeweilige Toolchain
function Get-ArPath {
  param($portableDir, $pkg)
  $toolsRoot = Join-Path $portableDir "packages\$pkg\tools"
  if (-not (Test-Path $toolsRoot)) { throw "Toolchain nicht gefunden unter $toolsRoot" }
  $allAr = Get-ChildItem -Path $toolsRoot -Recurse -File -Include *-ar.exe -ErrorAction SilentlyContinue
  $prefer = @(
    'xtensa-lx106-elf-ar.exe',        # esp8266
    'xtensa-esp32-elf-ar.exe',        # esp32
    'xtensa-esp32s2-elf-ar.exe',
    'xtensa-esp32s3-elf-ar.exe',
    'riscv32-esp-elf-ar.exe',
    'arm-none-eabi-ar.exe'
  )
  foreach ($name in $prefer) {
    $hit = $allAr | Where-Object { $_.Name -ieq $name } | Select-Object -First 1
    if ($hit) { return $hit.FullName }
  }
  if ($allAr) { return ($allAr | Select-Object -First 1).FullName }
  throw "Kein ar.exe gefunden (unter $toolsRoot)."
}
$ArPath = Get-ArPath -portableDir $portableDir -pkg $pkg
Write-Host ("Archiver: " + $ArPath)

# =========================== Whitelist je Architektur ===========================
$Whitelist = @{
  'esp32' = @{
    'Network'               = @{ Include='Network.h';              LibBase='Network';              CoreFolder='Network';              LdAdd='' }
    'WiFi'                  = @{ Include='WiFi.h';                 LibBase='WiFi';                 CoreFolder='WiFi';                 LdAdd='-lNetwork' }
    'NetworkClientSecure'   = @{ Include='NetworkClientSecure.h';  LibBase='NetworkClientSecure';  CoreFolder='NetworkClientSecure';  LdAdd='-lWiFi -lNetwork' }
    'HTTPClient'            = @{ Include='HTTPClient.h';           LibBase='HTTPClient';           CoreFolder='HTTPClient';           LdAdd='-lNetworkClientSecure -lWiFi -lNetwork' }
    'WebServer'             = @{ Include='WebServer.h';            LibBase='WebServer';            CoreFolder='WebServer';            LdAdd='-lWiFi -lNetwork' }
  };
  'esp8266' = @{
    'ESP8266WiFi'           = @{ Include='ESP8266WiFi.h';          LibBase='ESP8266WiFi';          CoreFolder='ESP8266WiFi';          LdAdd='' }
    'ESP8266HTTPClient'     = @{ Include='ESP8266HTTPClient.h';    LibBase='ESP8266HTTPClient';    CoreFolder='ESP8266HTTPClient';    LdAdd='-lESP8266WiFi' }
    'ESP8266httpUpdate'     = @{ Include='ESP8266httpUpdate.h';    LibBase='ESP8266httpUpdate';    CoreFolder='ESP8266httpUpdate';    LdAdd='-lESP8266HTTPClient -lESP8266WiFi' }
    'ESP8266WebServer'      = @{ Include='ESP8266WebServer.h';     LibBase='ESP8266WebServer';     CoreFolder='ESP8266WebServer';     LdAdd='-lESP8266WiFi' }
  }
}
if (-not $Whitelist.ContainsKey($arch)) { throw "Für Architektur '$arch' ist keine Whitelist definiert." }

# =========================== Ziel- & Workdirs ===========================
$DstLibRoot = Join-Path $PSScriptRoot "core_libraries_precompiled"
New-Item -Force -ItemType Directory -Path $DstLibRoot | Out-Null

$TmpRoot   = Join-Path -Path $portableDir -ChildPath '_netbuild'
$BuildRoot = Join-Path -Path $TmpRoot     -ChildPath 'build'
New-Item -Force -ItemType Directory -Path $TmpRoot,$BuildRoot | Out-Null

# =========================== Build-Helfer ===========================
function Make-DummySketch {
  param([string]$name, [string]$includeHeader, [string]$dir)
  if (-not $name) { throw "Make-DummySketch: name leer" }
  if (-not $includeHeader) { throw "Make-DummySketch: includeHeader leer" }
  if (-not $dir) { throw "Make-DummySketch: dir leer" }

  $skDir = Join-Path $dir $name
  if (Test-Path $skDir) { Remove-Item -Recurse -Force $skDir }
  New-Item -ItemType Directory -Path $skDir | Out-Null
  $lines = @(
    "#include <$includeHeader>",
    "void setup(){}",
    "void loop(){}"
  )
  $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
  [System.IO.File]::WriteAllText((Join-Path $skDir "$name.ino"), ($lines -join "`r`n"), $utf8NoBom)
  return $skDir
}

function Build-With-IDE {
  param($ArduinoExe,$FQBN,$SketchPath,$BuildPath,$SketchbookDir)
  $args = @(
    "--verify",
    "--board",$FQBN,
    "--pref","build.path=$BuildPath",
    "--pref","sketchbook.path=$SketchbookDir",
    "--pref","compiler.warning_level=none",
    $SketchPath
  )
  & $ArduinoExe @args | Out-Host
  return $LASTEXITCODE
}

function Find-ObjectsDir {
  param($buildPath,[string]$libName)
  $candRoot = Join-Path $buildPath "libraries"
  if (-not (Test-Path $candRoot)) { return $null }
  $subs = Get-ChildItem -Directory $candRoot -ErrorAction SilentlyContinue
  $hit = $subs | Where-Object { $_.Name -ieq $libName -or $_.Name -like ("*" + $libName + "*") } | Select-Object -First 1
  if ($hit) { return $hit.FullName }
  $best = $null; $bestCnt = -1
  foreach ($d in $subs) {
    $cnt = (Get-ChildItem -Path $d.FullName -Recurse -Include *.o -File | Measure-Object).Count
    if ($cnt -gt $bestCnt) { $best = $d.FullName; $bestCnt = $cnt }
  }
  return $best
}

function Should-Rebuild {
  param($archivePath, $srcDir, [switch]$ForceRebuild, [switch]$RebuildIfSourcesNewer)
  if ($ForceRebuild) { return $true }
  if (-not (Test-Path $archivePath)) { return $true }
  if ($RebuildIfSourcesNewer) {
    $archTime = (Get-Item $archivePath).LastWriteTimeUtc
    $patterns = @("*.c","*.cpp","*.cc","*.cxx","*.h","*.hpp")
    $files = @()
    if (Test-Path $srcDir) { $files += Get-ChildItem -Path $srcDir -Recurse -File -Include $patterns -ErrorAction SilentlyContinue }
    foreach ($f in $files) { if ($f.LastWriteTimeUtc -gt $archTime) { return $true } }
  }
  return $false
}

function Get-CoreLibVersion {
  param([string]$coreLibRoot)
  $propPath = Join-Path $coreLibRoot 'library.properties'
  if (-not (Test-Path $propPath)) { return $null }
  try {
    $lines = Get-Content -Path $propPath -Encoding UTF8 -ErrorAction Stop
  } catch {
    $lines = Get-Content -Path $propPath -ErrorAction SilentlyContinue
  }
  foreach ($line in $lines) {
    if ($line -match '^\s*#') { continue }
    $m = [regex]::Match($line, '^\s*version\s*=\s*(.*?)\s*$')
    if ($m.Success) {
      $v = $m.Groups[1].Value.Trim("`"","'")
      $v = $v -replace '[^0-9A-Za-z._+\x2D]', ''
      if ($v) { return $v }
    }
  }
  return $null
}

function Write-LibraryProps {
  param(
    [string]$outLibDir,
    [string]$libName,
    [string]$includeHeader,
    [string]$libBase,
    [string]$arch,
    [string]$ldAdd,
    [string]$coreLibRoot
  )
  New-Item -Force -ItemType Directory -Path $outLibDir | Out-Null

  $version = Get-CoreLibVersion -coreLibRoot $coreLibRoot
  if (-not $version) { $version = '1.0.0' }

  $propsOut     = Join-Path $outLibDir "library.properties"
  $includesFile = ($includeHeader -split '[\\/]')[-1]
  $ldflags      = ("-l{0} {1}" -f $libBase, $ldAdd).Trim()

  $lines = @(
    "name=$libName",
    "version=$version",
    "author=precompiled-by-script",
    "maintainer=precompiled-by-script",
    "sentence=Precompiled archive of $libName",
    "paragraph=Generated from Arduino core library objects",
    "category=Other",
    "url=",
    "precompiled=true",
    "includes=$includesFile",
    "architectures=$arch",
    "ldflags=$ldflags"
  )
  $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
  [System.IO.File]::WriteAllLines($propsOut, $lines, $utf8NoBom)

  Write-Host ("  -> library.properties aktualisiert (version={0}, architectures={1}, ldflags={2})" -f $version, $arch, $ldflags) -ForegroundColor DarkCyan
}

# =========================== Leeres Temp-Sketchbook ===========================
$TempSbDir = Join-Path $TmpRoot "__sb"
New-Item -Force -ItemType Directory -Path $TempSbDir | Out-Null
New-Item -Force -ItemType Directory -Path (Join-Path $TempSbDir "libraries") | Out-Null

# =========================== Hauptlauf ===========================
foreach ($kv in $Whitelist[$arch].GetEnumerator() | Sort-Object Key) {
  $libName    = $kv.Key
  $conf       = $kv.Value
  $includeH   = $conf.Include
  $libBase    = $conf.LibBase
  $coreFolder = $conf.CoreFolder
  $ldadd      = $conf.LdAdd

  $coreSrcRoot = Join-Path $CoreLibsRoot $coreFolder
  $coreSrcDir  = Join-Path $coreSrcRoot "src"

  # Zielpfade
  $outLibDir  = Join-Path $DstLibRoot $libName
  $srcDirOut  = Join-Path $outLibDir "src"
  $archDirOut = Join-Path $srcDirOut $arch
  $aPath      = Join-Path $archDirOut ("lib{0}.a" -f $libBase)

  $needsBuild = Should-Rebuild -archivePath $aPath -srcDir $coreSrcRoot -ForceRebuild:$ForceRebuild -RebuildIfSourcesNewer:$RebuildIfSourcesNewer

  if ($needsBuild) {
    Write-Host ("`n>> Baue {0}" -f $libName) -ForegroundColor Cyan

    # Dummy-Sketch → wichtig: echtes FQBN sorgt für gültige pins_arduino.h usw.
    $skName = "__netpre_" + ($libName -replace "[^A-Za-z0-9_]","_")
    $skDir  = Make-DummySketch -name $skName -includeHeader $includeH -dir $BuildRoot

    # eigener Build-Ordner je Lib
    $buildPath = Join-Path $skDir "build"
    New-Item -Force -ItemType Directory -Path $buildPath | Out-Null

    # Build
    $rc = Build-With-IDE -ArduinoExe $ArduinoExe -FQBN $FQBN -SketchPath (Join-Path $skDir ($skName + ".ino")) -BuildPath $buildPath -SketchbookDir $TempSbDir
    if ($rc -ne 0) {
      Write-Host ("Fehler beim Kompilieren von {0}" -f $libName) -ForegroundColor Red
      continue
    }

    # .o suchen & Archiv erstellen
    $objDir = Find-ObjectsDir -buildPath $buildPath -libName $libName
    if (-not $objDir) {
      Write-Host ("Keine Objektdateien für {0} gefunden" -f $libName) -ForegroundColor Yellow
      continue
    }
    $objs = Get-ChildItem -Path $objDir -Recurse -Include *.o -File -ErrorAction SilentlyContinue
    if ($objs.Count -eq 0) {
      Write-Host ("Keine .o erzeugt (evtl. header-only): {0}" -f $libName) -ForegroundColor Yellow
      continue
    }

    New-Item -Force -ItemType Directory -Path $archDirOut | Out-Null
    Push-Location $objDir
    $objList = $objs | ForEach-Object { (Resolve-Path $_.FullName).Path }
    & "$ArPath" "rcs" "$aPath" @($objList | ForEach-Object { "`"$_`"" }) | Out-Host
    Pop-Location

    if (-not (Test-Path $aPath)) {
      Write-Host ("Archiv fehlgeschlagen für {0}" -f $libName) -ForegroundColor Red
      continue
    }

    Write-Host ("OK: {0}  →  {1}" -f $libName, $aPath) -ForegroundColor Green
    Write-Host ("  -> Archivinhalt ({0}):" -f (Split-Path $aPath -Leaf)) -ForegroundColor DarkGray
    & $ArPath t $aPath | ForEach-Object { Write-Host ("     - {0}" -f $_) }

    # Header kopieren (src & Root)
    New-Item -Force -ItemType Directory -Path $srcDirOut | Out-Null
    if (Test-Path $coreSrcDir) {
      robocopy $coreSrcDir $srcDirOut *.h *.hpp *.hh *.H /S /NFL /NDL /NJH /NJS /NC /NS | Out-Null
    }
    robocopy $coreSrcRoot $srcDirOut *.h *.hpp *.hh *.H /NFL /NDL /NJH /NJS /NC /NS | Out-Null
  }
  else {
    Write-Host ("Skip: {0} (Archiv vorhanden & aktuell)" -f $libName) -ForegroundColor DarkGray
    New-Item -Force -ItemType Directory -Path $srcDirOut,$archDirOut | Out-Null
  }

  # Properties schreiben/aktualisieren
  Write-LibraryProps -outLibDir $outLibDir -libName $libName -includeHeader $includeH -libBase $libBase -arch $arch -ldAdd $ldadd -coreLibRoot $coreSrcRoot
}

Write-Host "`nFertig. Vorkompilierte Core-Libs liegen unter:`n$DstLibRoot" -ForegroundColor Cyan
