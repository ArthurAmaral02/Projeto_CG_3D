"""Gera as texturas de exemplo da pasta texturas/ (procedurais, sem direitos autorais).

As imagens ja estao no repositorio; este script so e necessario para recria-las.
Requer: pip install pillow numpy

    python scripts/gerar_texturas.py
"""

from pathlib import Path

import numpy as np
from PIL import Image

PASTA = Path(__file__).resolve().parent.parent / "texturas"
TAM = 512


def madeira() -> Image.Image:
    """Madeira com veios (JPG)."""
    y, x = np.mgrid[0:TAM, 0:TAM] / TAM
    rng = np.random.default_rng(7)
    distorcao = np.sin(y * 2 * np.pi * 2) * 0.025 + np.sin(y * 2 * np.pi * 5 + 1.3) * 0.006
    aneis = np.sin((x + distorcao) * 2 * np.pi * 9) * 0.5 + 0.5
    fibra = rng.normal(0, 0.05, (TAM, 1)).repeat(TAM, axis=1)
    t = np.clip(aneis * 0.7 + fibra + 0.15, 0, 1)[..., None]
    claro = np.array([196, 140, 84])
    escuro = np.array([112, 66, 32])
    rgb = escuro + (claro - escuro) * t
    return Image.fromarray(rgb.astype(np.uint8), "RGB")


def xadrez() -> Image.Image:
    """Quadriculado com numeracao visual de orientacao (BMP)."""
    y, x = np.mgrid[0:TAM, 0:TAM]
    casas = ((x // 64) + (y // 64)) % 2
    rgb = np.where(casas[..., None] == 1, [235, 235, 225], [40, 90, 170]).astype(np.uint8)
    # Faixa vermelha no topo e verde a esquerda: mostram a orientacao (u, v).
    rgb[:16, :, :] = [200, 40, 40]
    rgb[:, :16, :] = [40, 160, 60]
    return Image.fromarray(rgb, "RGB")


def marmore() -> Image.Image:
    """Marmore com veios (PNG)."""
    rng = np.random.default_rng(3)
    y, x = np.mgrid[0:TAM, 0:TAM] / TAM
    ruido = np.zeros((TAM, TAM))
    for oitava in range(1, 6):
        n = 2 ** (oitava + 1)
        grade = rng.random((n + 1, n + 1))
        grade[-1, :] = grade[0, :]
        grade[:, -1] = grade[:, 0]
        img = Image.fromarray((grade * 255).astype(np.uint8)).resize((TAM, TAM), Image.BICUBIC)
        ruido += np.asarray(img) / 255.0 / (2 ** oitava)
    veios = np.abs(np.sin((x * 3 + y * 2 + ruido * 2.5) * np.pi * 2)) ** 0.35
    t = veios[..., None]
    rgb = np.array([90, 92, 100]) + (np.array([238, 236, 230]) - np.array([90, 92, 100])) * t
    return Image.fromarray(rgb.astype(np.uint8), "RGB")


def ambiente_por_do_sol() -> Image.Image:
    """Mapa de reflexao esferico alternativo: ceu de por do sol sobre o mar (PNG)."""
    s, t = np.meshgrid(np.linspace(-1, 1, TAM), np.linspace(1, -1, TAM))
    r2 = np.clip(s * s + t * t, 0, 1)
    nz = np.sqrt(1 - r2)
    rx, ry, rz = 2 * nz * s, 2 * nz * t, 2 * nz * nz - 1
    ceu = np.stack([0.98 - ry * 0.7, 0.55 - ry * 0.25, 0.30 + ry * 0.5], axis=-1)
    sol = np.exp(-((rx - 0.0) ** 2 + (ry - 0.12) ** 2 + (rz + 0.95) ** 2) * 40)[..., None]
    mar = np.stack([0.10 + 0 * ry, 0.18 + 0 * ry, 0.30 + 0 * ry], axis=-1) * (1 + ry[..., None])
    rgb = np.where(ry[..., None] >= 0, ceu + sol * np.array([1.0, 0.9, 0.6]), mar)
    return Image.fromarray((np.clip(rgb, 0, 1) * 255).astype(np.uint8), "RGB")


def main() -> None:
    PASTA.mkdir(exist_ok=True)
    madeira().save(PASTA / "madeira.jpg", quality=90)
    xadrez().save(PASTA / "xadrez.bmp")
    marmore().save(PASTA / "marmore.png")
    ambiente_por_do_sol().save(PASTA / "ambiente_por_do_sol.png")
    print("Texturas geradas em", PASTA)


if __name__ == "__main__":
    main()
