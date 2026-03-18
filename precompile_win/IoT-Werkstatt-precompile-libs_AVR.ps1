<# IoT-Werkstatt: Precompile Arduino Libraries to .a (Arduino IDE 1.8.19 portable, NO arduino-cli)
   Windows 11 + ESP32 / ESP8266 / AVR Core (portable)

   Struktur:
     <ROOT>\arduino\arduino.exe
     <ROOT>\precompile_win\libraries_2precompile\...
     <ROOT>\precompile_win\libraries_precompiled\...

   Verwendung:
     cd <ROOT>   # z.B. E:\IoTW
     .\IoT-Werkstatt-precompile-libs.ps1 -FQBN "esp32:esp32:makey_v1"
     .\IoT-Werkstatt-precompile-libs.ps1 -FQBN "esp8266:esp8266:octopus"
     .\IoT-Werkstatt-precompile-libs.ps1 -FQBN "arduino:avr:uno"

   Verhalten:
     - Kompiliert alle Libs aus libraries_2precompile für das angegebene FQBN.
     - Erzeugt .a im Ziel: precompile_win\libraries_precompiled\<Lib>\src\<arch>\lib<Lib>.a
       * arch = esp32, esp8266 oder avr (aus dem FQBN)
     - Kopiert Header (*.h, *.hpp) mit.
     - Schreibt library.properties mit:
         precompiled=true
         architectures=esp32,esp8266,avr   (fest)
         ldflags=-l<LibName>
     - Nutzt KnownHeaders (Whitelist) als erste Wahl für den Dummy-Include.
#>

param(
  [string]$PortableRoot,                # <— kein Default mehr hier!
  [string]$FQBN = "esp32:esp32:makey_v1",
  [switch]$IncludeBuiltin,
  [switch]$ForceRebuild,
  [switch]$RebuildIfSourcesNewer
)

# Fallback für PortableRoot setzen (nach dem param-Block!)
if (-not (Get-Variable PSScriptRoot -Scope Script -ErrorAction SilentlyContinue)) {
  $script:PSScriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
}
if (-not $PortableRoot -or [string]::IsNullOrWhiteSpace($PortableRoot)) {
  $PortableRoot = $PSScriptRoot
}

$pkg  = ($FQBN -split ':')[0]   # "esp32", "esp8266", "arduino"
$arch = ($FQBN -split ':')[1]   # "esp32", "esp8266", "avr"

# --- Arduino portable Pfade automatisch erkennen ---
if (Test-Path (Join-Path $PortableRoot "arduino\portable")) {
  $PortableRoot = Join-Path $PortableRoot "arduino"
}

$portableDir = Join-Path $PortableRoot "portable"
if (-not (Test-Path $portableDir)) {
  throw "portable\-Ordner nicht gefunden in $PortableRoot"
}

# --- Arduino-EXE finden ---
$ArduinoExe = Join-Path $PortableRoot "arduino_debug.exe"
if (-not (Test-Path $ArduinoExe)) { $ArduinoExe = Join-Path $PortableRoot "arduino.exe" }
if (-not (Test-Path $ArduinoExe)) { throw "arduino(_debug).exe nicht gefunden in $PortableRoot" }

# --- Library-Verzeichnisse ---
$SrcLibRoot      = Join-Path $PSScriptRoot "libraries_2precompile"
$DstLibRoot      = Join-Path $PSScriptRoot "libraries_precompiled"

Write-Host "SrcLibRoot = $SrcLibRoot"
Write-Host "DstLibRoot = $DstLibRoot"

if (-not (Test-Path $SrcLibRoot)) { throw "Quellverzeichnis 'libraries_2precompile' nicht gefunden unter $SketchbookDir" }
New-Item -Force -ItemType Directory -Path $DstLibRoot | Out-Null

# --- Toolchain: ar.exe finden ---
function Get-ArPath {
  param($portableDir, $pkg)

  $allAr = @()

  # 1) Normal: portable\packages\<pkg>\tools
  $toolsRoot = Join-Path $portableDir "packages\$pkg\tools"
  if (Test-Path $toolsRoot) {
    $allAr += Get-ChildItem -Path $toolsRoot -Recurse -File -Include *-ar.exe,avr-gcc-ar.exe,avr-ar.exe -ErrorAction SilentlyContinue
  }

  # 2) Fallback nur für AVR (pkg=arduino): hardware\tools\avr im IDE-Ordner
  if ($pkg -eq 'arduino') {
    $ideRoot    = Split-Path $portableDir -Parent   # ...\arduino
    $hwToolsDir = Join-Path $ideRoot "hardware\tools\avr"
    if (Test-Path $hwToolsDir) {
      $allAr += Get-ChildItem -Path $hwToolsDir -Recurse -File -Include avr-gcc-ar.exe,avr-ar.exe -ErrorAction SilentlyContinue
    }
  }

  if (-not $allAr -or $allAr.Count -eq 0) {
    $msg = "Kein ar.exe gefunden (gesucht unter '$toolsRoot'"
    if ($pkg -eq 'arduino') {
      $ideRoot    = Split-Path $portableDir -Parent
      $msg += " und '" + (Join-Path $ideRoot "hardware\tools\avr") + "'"
    }
    $msg += ")."
    throw $msg
  }

  $prefer = @(
    "xtensa-esp-elf-ar.exe",
    "xtensa-esp32-elf-ar.exe",
    "riscv32-esp-elf-ar.exe",
    "xtensa-lx106-elf-ar.exe",
    "avr-gcc-ar.exe",
    "avr-ar.exe"
  )
  foreach ($name in $prefer) {
    $hit = $allAr | Where-Object { $_.Name -ieq $name } | Select-Object -First 1
    if ($hit) { return $hit.FullName }
  }
  return ($allAr | Select-Object -First 1).FullName
}

# Aufruf:
$ArPath = Get-ArPath -portableDir $portableDir -pkg $pkg
Write-Host ("Archiver: " + $ArPath)

# --- bekannte Haupt-Header (Whitelist, hat Vorrang) ---
$KnownHeaders = @{
  # WICHTIG: BusIO niemals *_Register.h – sonst Doppeldefinitionen
  "Adafruit BusIO"            = "Adafruit_I2CDevice.h"
  "Adafruit_BusIO"            = "Adafruit_I2CDevice.h"

  "Adafruit GFX Library"      = "Adafruit_GFX.h"
  "Adafruit_GFX_Library"      = "Adafruit_GFX.h"
  "Adafruit SH110X"           = "Adafruit_SH110X.h"
  "Adafruit_SH110X"           = "Adafruit_SH110X.h"
  "Adafruit BME680 Library"   = "Adafruit_BME680.h"
  "Adafruit_BME680_Library"   = "Adafruit_BME680.h"
  "Adafruit NeoPixel"         = "Adafruit_NeoPixel.h"
  "Adafruit_NeoPixel"         = "Adafruit_NeoPixel.h"
  "BME68x Sensor library"     = "bme68xLibrary.h"
  "BME68x_Sensor_library"     = "bme68xLibrary.h"
  "ESP32Encoder"              = "ESP32Encoder.h"

  # Deine Beispiele:
  "IoTW_APDS9999"             = "IoTW_APDS9999.h"
  "IOTW_ADPS9999"             = "IoTW_APDS9999.h"
  "IoTW_Config"               = "IoTW_config.h"
  "ESP8266Audio"              = "AudioOutputI2S.h"
  "Adafruit_Unified_Sensor"   = "Adafruit_Sensor.h"
  "Phyphox_BLE"               = "phyphoxBle.h" 
}

# --- Arbeitsordner ---
$TmpRoot   = Join-Path $portableDir "_libbuild"
$BuildRoot = Join-Path $TmpRoot "build"
New-Item -Force -ItemType Directory -Path $BuildRoot | Out-Null

# --- Helpers ---
function Normalize-Name([string]$s) { return ($s -replace "[^A-Za-z0-9]", "_") }

function Supports-TargetArch {
  param($props, [string]$arch)

  $arch = $arch.ToLower()

  # Kein Feld -> wie "*": wir erlauben den Build
  if (-not $props.ContainsKey('architectures') -or [string]::IsNullOrWhiteSpace($props['architectures'])) {
    return $true
  }

  $raw  = $props['architectures'].ToLower()
  $list = $raw.Replace(';',',').Split(',') |
          ForEach-Object { $_.Trim() } | Where-Object { $_ }

  if ($list -contains '*')        { return $true }
  if ($list -contains $arch)      { return $true }
  if ($arch -eq 'esp32' -and ($list -contains 'espressif32')) { return $true }

  return $false
}

function Read-LibraryProps {
  param($libDir)
  $file = Join-Path $libDir "library.properties"
  $props = @{}
  if (Test-Path $file) {
    Get-Content $file | ForEach-Object {
      if ($_ -match "^\s*#") { return }
      $kv = $_.Split("=",2)
      if ($kv.Count -eq 2) { $props[$kv[0].Trim()] = $kv[1].Trim() }
    }
  }
  return $props
}

function Guess-IncludeHeader {
  param($libDir,$props)
  $libFolder = Split-Path $libDir -Leaf
  $libName = if ($props.ContainsKey("name") -and $props["name"]) { $props["name"] } else { $libFolder }

  # 0) KnownHeaders (Vorrang)
  if ($KnownHeaders.ContainsKey($libName)) {
    $cand = $KnownHeaders[$libName]
    $src  = Join-Path $libDir "src"
    if (Test-Path (Join-Path $src  $cand)) { return $cand }
    if (Test-Path (Join-Path $libDir $cand)) { return $cand }
    $hit = Get-ChildItem -Path $src -Recurse -File -Include $cand -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($hit) { return $hit.Name }
  }

  # 1) library.properties -> includes
  if ($props.ContainsKey("includes") -and $props["includes"]) {
    return ($props["includes"].Split(",")[0].Trim())
  }

  # 2) src/*.h, *.hpp
  $src = Join-Path $libDir "src"
  if (Test-Path $src) {
    $hdr = Get-ChildItem -Path $src -Recurse -File -Include *.h,*.hpp -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($hdr) { return $hdr.Name }
  }

  # 3) Root-Header
  $hdr2 = Get-ChildItem -Path $libDir -File -Include *.h,*.hpp -ErrorAction SilentlyContinue | Select-Object -First 1
  if ($hdr2) { return $hdr2.Name }

  # 4) Fallback: erster Header irgendwo
  $hdr3 = Get-ChildItem -Path $libDir -Recurse -File -Include *.h,*.hpp -ErrorAction SilentlyContinue | Select-Object -First 1
  if ($hdr3) { return (Split-Path $hdr3 -Leaf) }

  return $null
}

function Make-DummySketch {
  param($name, $includeHeader, $dir)
  $skDir = Join-Path $dir $name
  if (Test-Path $skDir) { Remove-Item -Recurse -Force $skDir }
  New-Item -ItemType Directory -Path $skDir | Out-Null

  $lines = @()
  $lines += "#define STR_HELPER(x) #x"
  $lines += "#define STR(x) STR_HELPER(x)"
  $lines += ("#include <{0}>" -f $includeHeader)
  $lines += "void setup(){}"
  $lines += "void loop(){}"

  Write-Host "`n--- Generierter Sketch für $name ---" -ForegroundColor Yellow
  $lines | ForEach-Object { Write-Host $_ }
  Write-Host "--- Ende des Sketches ---`n" -ForegroundColor Yellow

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

function Parse-Architectures {
  param([string]$s)
  if (-not $s) { return @() }
  $s = $s.Trim().ToLower()
  if ($s -eq '*') { return @('*') }

  # normal: Komma/Whitespace getrennt
  $parts = $s -split '[,\s;]+' | Where-Object { $_ }

  # Sonderfall: zusammengeklebte Tokens wie "esp32esp8266"
  if ($parts.Count -le 1 -and $s -notmatch ',') {
    if ($s -match 'esp32' -and $s -match 'esp8266') {
      $parts = @('esp32','esp8266')
    }
  }
  return $parts
}

function Find-ObjectsDir {
  param($buildPath,$libDir,$props)
  $libFolder = Split-Path $libDir -Leaf
  $candidatesRoot = Join-Path $buildPath "libraries"
  if (-not (Test-Path $candidatesRoot)) { return $null }

  $namesToTry = @()
  $namesToTry += (Normalize-Name $libFolder)
  if ($props.ContainsKey("name") -and $props["name"]) { $namesToTry += (Normalize-Name $props["name"]) }

  $subdirs = Get-ChildItem -Directory $candidatesRoot -ErrorAction SilentlyContinue
  foreach ($n in $namesToTry) {
    $hit = $subdirs | Where-Object { $_.Name -ieq $n -or $_.Name -like ("*" + $n + "*") } | Select-Object -First 1
    if ($hit) { return $hit.FullName }
  }

  # Fallback: Unterordner mit den meisten .o
  $best = $null; $bestCount = -1
  foreach ($d in $subdirs) {
    $cnt = (Get-ChildItem -Path $d.FullName -Recurse -Include *.o -File | Measure-Object).Count
    if ($cnt -gt $bestCount) { $best = $d.FullName; $bestCount = $cnt }
  }
  return $best
}

function Should-Rebuild {
  param($archivePath, $libDir, $ForceRebuild, $RebuildIfSourcesNewer)

  if ($ForceRebuild) { return $true }
  if (-not (Test-Path $archivePath)) { return $true }

  if ($RebuildIfSourcesNewer) {
    $archiveTime = (Get-Item $archivePath).LastWriteTimeUtc
    $srcDir = Join-Path $libDir "src"
    $patterns = @("*.c","*.cpp","*.cc","*.cxx","*.h","*.hpp")
    $files = @()
    if (Test-Path $srcDir) { $files += Get-ChildItem -Path $srcDir -Recurse -File -Include $patterns -ErrorAction SilentlyContinue }
    $files += Get-ChildItem -Path $libDir -File -Include $patterns -ErrorAction SilentlyContinue
    foreach ($f in $files) {
      if ($f.LastWriteTimeUtc > $archiveTime) { return $true }
    }
    return $false
  }

  return $false
}

# --- Lauf über alle Quell-Libs ---
$libs = Get-ChildItem -Directory $SrcLibRoot | Sort-Object FullName
if ($libs.Count -eq 0) { Write-Host "Keine Libraries in $SrcLibRoot gefunden."; exit 0 }

# --- Temp-Sketchbook, das alle libraries_2precompile als "libraries" sichtbar macht ---
$TempSbDir   = Join-Path $TmpRoot "__sb"
$TempLibsDir = Join-Path $TempSbDir "libraries"
New-Item -Force -ItemType Directory -Path $TempLibsDir | Out-Null

# Für jede Quell-Library eine Junction in $TempLibsDir anlegen (einmalig, idempotent)
Get-ChildItem -Directory $SrcLibRoot | ForEach-Object {
  $name = $_.Name
  $link = Join-Path $TempLibsDir $name
  if (-not (Test-Path $link)) {
    New-Item -ItemType Junction -Path $link -Target $_.FullName | Out-Null
  }
}
Write-Host ("Sketchbook (build): {0}" -f $TempSbDir) -ForegroundColor DarkCyan

foreach ($lib in $libs) {
  try {
    $props = Read-LibraryProps $lib.FullName

    $name = if ($props.ContainsKey("name") -and $props["name"]) { $props["name"] } else { $lib.Name }

    # Architektur der Quelle respektieren
    if (-not (Supports-TargetArch -props $props -arch $arch)) {
       $srcArch = if ($props.ContainsKey('architectures')) { $props['architectures'] } else { '(leer)' }
       $msg = ("Skip: {0} (Quelle architectures='{1}' unterstuetzt Ziel '{2}' nicht)" -f $name, $srcArch, $arch)
       Write-Host $msg -ForegroundColor DarkGray
       continue
    }

    # Nur für den Ziel-Ordner: Leerzeichen (und „kritische“ Zeichen) durch _ ersetzen
    $folderName = ($name -replace '\s','_') -replace '[^A-Za-z0-9._-]','_'

    $outLibDir = Join-Path $DstLibRoot $folderName
    $srcDirOut  = Join-Path $outLibDir "src"

    # vor der Verwendung von $archDirOut einfügen:
    $targetArch = $arch
    if ($arch -eq 'avr') {
     $targetArch = 'atmega328p'
    }

    $archDirOut = Join-Path $srcDirOut $targetArch



    # $archDirOut = Join-Path $srcDirOut  $arch    # esp32 / esp8266 / avr

    # Bibliotheksbasisname für -l… bleibt wie gehabt (keine Leerzeichen / Sonderzeichen)
    $libBase = ($name -replace "[^A-Za-z0-9_]", "")

    $aPath = Join-Path $archDirOut ("lib{0}.a" -f $libBase)

    $header = Guess-IncludeHeader $lib.FullName $props
    if (-not $header) {
      Write-Host ("Überspringe '{0}' (kein Header gefunden)" -f $name) -ForegroundColor Yellow
      continue
    }

    if (-not (Should-Rebuild -archivePath $aPath -libDir $lib.FullName -ForceRebuild:$ForceRebuild -RebuildIfSourcesNewer:$RebuildIfSourcesNewer)) {
      Write-Host ("Skip: {0} (Archiv vorhanden & aktuell)" -f $name) -ForegroundColor DarkGray
      continue
    }

    Write-Host ("`n>> Baue {0} (Header: <{1}>)" -f $name, $header)
    $skName = "__pre_" + (Normalize-Name $lib.Name)
    $skDir  = Make-DummySketch -name $skName -includeHeader $header -dir $BuildRoot
    $buildPath = Join-Path $skDir "build"
    New-Item -Force -ItemType Directory -Path $buildPath | Out-Null

    $rc = Build-With-IDE -ArduinoExe $ArduinoExe -FQBN $FQBN -SketchPath (Join-Path $skDir ($skName + ".ino")) -BuildPath $buildPath -SketchbookDir $TempSbDir
    if ($rc -ne 0) {
      Write-Host ("Fehler beim Kompilieren von {0}" -f $name) -ForegroundColor Red
      continue
    }

    $objDir = Find-ObjectsDir -buildPath $buildPath -libDir $lib.FullName -props $props
    if (-not $objDir) {
      Write-Host ("Keine Objektdateien für {0} gefunden (kein passender libraries\-Unterordner)" -f $name) -ForegroundColor Yellow
      continue
    }
    $objs = Get-ChildItem -Path $objDir -Recurse -Include *.o -File -ErrorAction SilentlyContinue
    if ($objs.Count -eq 0) {
      Write-Host ("Keine .o Dateien erzeugt (evtl. header-only): {0}" -f $name) -ForegroundColor Yellow
      continue
    }

    # Zielordner jetzt anlegen (nur bei Erfolg)
    New-Item -Force -ItemType Directory -Path $archDirOut | Out-Null

    # Archiv erstellen
    Push-Location $objDir
    $objList = $objs | ForEach-Object { (Resolve-Path $_.FullName).Path }
    & "$ArPath" "rcs" "$aPath" @($objList | ForEach-Object { "`"$_`"" }) | Out-Host
    Pop-Location

    if (-not (Test-Path $aPath)) {
      Write-Host ("Archiv fehlgeschlagen für {0}" -f $name) -ForegroundColor Red
      continue
    }

    Write-Host ("  -> Archivinhalt ({0}):" -f (Split-Path $aPath -Leaf)) -ForegroundColor DarkGray
    & $ArPath t $aPath | ForEach-Object { Write-Host ("     - {0}" -f $_) }

    # Header kopieren
    New-Item -Force -ItemType Directory -Path $srcDirOut | Out-Null
    $srcOrig = Join-Path $lib.FullName "src"
    if (Test-Path $srcOrig) {
      robocopy $srcOrig $srcDirOut *.h *.hpp *.hh *.H /S /NFL /NDL /NJH /NJS /NC /NS | Out-Null
    }
    # ggf. auch Root-Header kopieren
    robocopy $lib.FullName $srcDirOut *.h *.hpp *.hh *.H /NFL /NDL /NJH /NJS /NC /NS | Out-Null

    # --- Examples kopieren (optional aus <lib>/examples und <lib>/src/examples) ---
    $dstExamples = Join-Path $outLibDir "examples"
    $srcExamplesCandidates = @(
      (Join-Path $lib.FullName "examples"),
      (Join-Path $lib.FullName "src\examples")
    )

    foreach ($ex in $srcExamplesCandidates) {
      if (Test-Path $ex) {
        New-Item -Force -ItemType Directory -Path $dstExamples | Out-Null
        # kopiert komplette Struktur, lässt Binär-/Build-Artefakte weg
        robocopy $ex $dstExamples *.* /E `
          /XF *.o *.obj *.a *.elf *.bin *.hex *.map `
          /XD .git .github build CMakeFiles _deps .pio `
          /NFL /NDL /NJH /NJS /NC /NS | Out-Null
      }
    }

    # --- library.properties zusammenführen/schreiben ---
    $propsOutPath = Join-Path $outLibDir "library.properties"

    $existing = @{}
    if (Test-Path $propsOutPath) {
      Get-Content $propsOutPath | ForEach-Object {
        if ($_ -match '^\s*#') { return }
        $kv = $_.Split('=',2)
        if ($kv.Count -eq 2) { $existing[$kv[0].Trim()] = $kv[1].Trim() }
      }
    }

    # Arch-Liste mergen (wird nur noch geloggt)
    $archList = Parse-Architectures ($existing['architectures'])
    if ($archList -contains '*') {
      $archList = @('*')
    } else {
      $archList += $arch
      $archList = $archList |
        ForEach-Object { $_.Trim().ToLower() } |
        Where-Object { $_ } |
        Select-Object -Unique |
        Sort-Object
    }
    $archValue = ($archList -join ',')

    # includes nur als Dateiname (ohne Pfad)
    $includesFile = if ($props.ContainsKey('includes') -and $props['includes']) {
      ($props['includes'])
    } else {
      (($header -split '[\\/]' )[-1])
    }

    # Version aus Quelle oder Default
    $version  = if ($props.ContainsKey("version") -and $props["version"]) { $props["version"] } else { "1.0.0" }

    # Properties aufbauen (vorhandene sinnvolle Felder respektieren)
    $merged = [ordered]@{
      'name'          = if ($existing.name)      { $existing.name }      else { $name }
      'version'       = if ($existing.version)   { $existing.version }   else { $version }
      'author'        = if ($existing.author)    { $existing.author }    else { 'precompiled-by-script' }
      'maintainer'    = if ($existing.maintainer){ $existing.maintainer }else { 'precompiled-by-script' }
      'sentence'      = if ($existing.sentence)  { $existing.sentence }  else { "Precompiled archive of $name" }
      'paragraph'     = if ($existing.paragraph) { $existing.paragraph } else { 'Generated from existing library objects' }
      'category'      = if ($existing.category)  { $existing.category }  else { 'Other' }
      'url'           = if ($existing.url)       { $existing.url }       else { '' }
      'precompiled'   = 'true'
      'includes'      = if ($existing.includes)  { $existing.includes }  else { $includesFile }
      'architectures' = 'esp32,esp8266,avr'   # <-- fest auf drei Plattformen
      'ldflags'       = "-l$libBase"
    }

    # Stabil & ohne abgeschnittenes erstes Zeichen schreiben (UTF-8 ohne BOM)
    $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllLines(
      $propsOutPath,
      ($merged.GetEnumerator() | ForEach-Object { "$($_.Key)=$($_.Value)" }),
      $utf8NoBom
    )

    # Log für schnelle Kontrolle
    Write-Host ("  -> library.properties: architectures={0}" -f 'esp32,esp8266,avr') -ForegroundColor DarkCyan

    Write-Host ("OK: {0}" -f $name) -ForegroundColor Green
  }
  catch {
    Write-Host ("Fehler bei {0}: {1}" -f $lib.Name, $_.Exception.Message) -ForegroundColor Red
  }
}

Write-Host "`nFertig. Vorkompilierte Libraries liegen in:`n$DstLibRoot"
