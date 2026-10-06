# Pre-build helper: generates the app tile / splash PNGs into Images/.
# Runs on the developer machine (PowerShell 5.1+, System.Drawing from .NET Framework).
Add-Type -AssemblyName System.Drawing

$dir = Join-Path $PSScriptRoot 'Images'
New-Item -ItemType Directory -Force -Path $dir | Out-Null

function New-CatIcon([int]$size, [string]$path) {
    $bmp = New-Object System.Drawing.Bitmap $size, $size
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias

    $orange = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(245, 180, 92))
    $dark   = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(58, 44, 34))
    $pink   = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(255, 199, 199))

    $s = [float]$size
    # ears
    $g.FillPolygon($orange, [System.Drawing.PointF[]]@(
        [System.Drawing.PointF]::new($s*0.18, $s*0.45),
        [System.Drawing.PointF]::new($s*0.30, $s*0.08),
        [System.Drawing.PointF]::new($s*0.45, $s*0.38)))
    $g.FillPolygon($orange, [System.Drawing.PointF[]]@(
        [System.Drawing.PointF]::new($s*0.55, $s*0.38),
        [System.Drawing.PointF]::new($s*0.70, $s*0.08),
        [System.Drawing.PointF]::new($s*0.82, $s*0.45)))
    $g.FillPolygon($pink, [System.Drawing.PointF[]]@(
        [System.Drawing.PointF]::new($s*0.25, $s*0.38),
        [System.Drawing.PointF]::new($s*0.31, $s*0.17),
        [System.Drawing.PointF]::new($s*0.40, $s*0.35)))
    $g.FillPolygon($pink, [System.Drawing.PointF[]]@(
        [System.Drawing.PointF]::new($s*0.60, $s*0.35),
        [System.Drawing.PointF]::new($s*0.69, $s*0.17),
        [System.Drawing.PointF]::new($s*0.75, $s*0.38)))
    # face
    $g.FillEllipse($orange, $s*0.12, $s*0.30, $s*0.76, $s*0.62)
    # eyes
    $g.FillEllipse($dark, $s*0.30, $s*0.52, $s*0.10, $s*0.14)
    $g.FillEllipse($dark, $s*0.60, $s*0.52, $s*0.10, $s*0.14)

    $g.Dispose()
    $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
}

New-CatIcon 44  (Join-Path $dir 'Square44x44Logo.png')
New-CatIcon 150 (Join-Path $dir 'Square150x150Logo.png')
New-CatIcon 50  (Join-Path $dir 'StoreLogo.png')

# splash screen 620x300
$bmp = New-Object System.Drawing.Bitmap 620, 300
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.Clear([System.Drawing.Color]::FromArgb(253, 246, 233))
$g.Dispose()
$bmp.Save((Join-Path $dir 'SplashScreen.png'), [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()

Write-Host "DesktopPet: app images generated in $dir"
