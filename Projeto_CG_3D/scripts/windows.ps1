<#
.SYNOPSIS
    Compila, executa e testa o Visualizador 3D no Windows pelo terminal.

.DESCRIPTION
    Usa o MSBuild do Visual Studio 2026 (localizado com o vswhere) para
    compilar a solucao Projeto_CG_3D.sln.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\windows.ps1 -Acao compilar
    powershell -ExecutionPolicy Bypass -File scripts\windows.ps1 -Acao executar
    powershell -ExecutionPolicy Bypass -File scripts\windows.ps1 -Acao testes
    powershell -ExecutionPolicy Bypass -File scripts\windows.ps1 -Acao autoteste
    powershell -ExecutionPolicy Bypass -File scripts\windows.ps1 -Acao tudo
#>
param(
    [ValidateSet('compilar', 'executar', 'testes', 'autoteste', 'tudo', 'limpar')]
    [string]$Acao = 'executar',

    [ValidateSet('Debug', 'Release')]
    [string]$Configuracao = 'Release'
)

$ErrorActionPreference = 'Stop'
$Raiz = Split-Path -Parent $PSScriptRoot
$Solucao = Join-Path $Raiz 'Projeto_CG_3D.sln'
$Saida = Join-Path $Raiz "build\vs\x64\$Configuracao"

function Get-MSBuild {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path $vswhere) {
        $msbuild = & $vswhere -latest -prerelease -products * -requires Microsoft.Component.MSBuild `
            -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
        if ($msbuild) { return $msbuild }
    }
    $cmd = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    throw 'MSBuild nao encontrado. Instale o Visual Studio 2026 com a carga "Desenvolvimento para desktop com C++".'
}

function Invoke-Compilar {
    $msbuild = Get-MSBuild
    Write-Host "MSBuild: $msbuild"
    & $msbuild $Solucao /m /nologo /verbosity:minimal "/p:Configuration=$Configuracao" '/p:Platform=x64'
    if ($LASTEXITCODE -ne 0) { throw "Falha na compilacao (codigo $LASTEXITCODE)." }
    Write-Host "Compilado em $Saida"
}

function Invoke-Testes {
    & (Join-Path $Saida 'testes.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Testes unitarios falharam.' }
}

function Invoke-Autoteste {
    $pasta = Join-Path $Raiz 'build\autoteste'
    & (Join-Path $Saida 'visualizador3d.exe') --autoteste $pasta
    if ($LASTEXITCODE -ne 0) { throw "Autoteste falhou. Veja $pasta\relatorio.txt" }
    Write-Host "Relatorio e capturas em $pasta"
}

Push-Location $Raiz
try {
    switch ($Acao) {
        'compilar'  { Invoke-Compilar }
        'executar'  { Invoke-Compilar; & (Join-Path $Saida 'visualizador3d.exe') }
        'testes'    { Invoke-Compilar; Invoke-Testes }
        'autoteste' { Invoke-Compilar; Invoke-Autoteste }
        'tudo'      { Invoke-Compilar; Invoke-Testes; Invoke-Autoteste }
        'limpar'    { Remove-Item -Recurse -Force (Join-Path $Raiz 'build\vs') -ErrorAction SilentlyContinue }
    }
}
finally {
    Pop-Location
}
