<#
.SYNOPSIS
    Gera o .zip de entrega com a pasta completa do projeto (criterio 5 do PDF).

.DESCRIPTION
    Inclui codigo-fonte, solucao do Visual Studio, bibliotecas em third_party,
    modelos, texturas e documentacao. Exclui artefatos de compilacao (build/)
    e pastas locais (.git, .vs).

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\empacotar.ps1
#>
$ErrorActionPreference = 'Stop'
$Raiz = Split-Path -Parent $PSScriptRoot
$Nome = Split-Path -Leaf $Raiz
$Destino = Join-Path $Raiz "build\entrega\$Nome.zip"
$Temporario = Join-Path $env:TEMP "empacotar_$Nome"

# Pastas locais e executaveis Linux gerados pelo make (curvas = executavel do projeto 2D antigo).
$excluir = @('build', '.git', '.vs', 'visualizador3d', 'curvas')

if (Test-Path $Temporario) { Remove-Item -Recurse -Force $Temporario }
New-Item -ItemType Directory -Force (Join-Path $Temporario $Nome) | Out-Null
New-Item -ItemType Directory -Force (Split-Path $Destino) | Out-Null

# Copia para uma pasta temporaria para o zip conter a pasta raiz do projeto.
Get-ChildItem -Force $Raiz | Where-Object { $excluir -notcontains $_.Name } | ForEach-Object {
    Copy-Item -Recurse -Force $_.FullName (Join-Path $Temporario $Nome)
}

if (Test-Path $Destino) { Remove-Item -Force $Destino }
Compress-Archive -Path (Join-Path $Temporario $Nome) -DestinationPath $Destino
Remove-Item -Recurse -Force $Temporario

$tamanho = [math]::Round((Get-Item $Destino).Length / 1MB, 1)
Write-Host "Pacote de entrega: $Destino ($tamanho MB)"
