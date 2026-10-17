import chipwhisperer as cw
import subprocess
import sys
import numpy as np
import matplotlib.pyplot as plt
import time

##########################################################################################################################################
subprocess.run(["make", "PLATFORM=CW308_STM32F3"], check=True)
##########################################################################################################################################
scope = cw.scope()
target = cw.target(scope)
scope.default_setup()

##########################################################################################################################################
prog = cw.programmers.STM32FProgrammer
cw.program_target(scope, prog, "puf_sram-CW308_STM32F3.hex")

##################### Parametres

TAILLE_BLOC = 32

amount_of_lines = 512      # TAILLE_ZONE
bits_per_line = 8
number_of_tests = 100

assert amount_of_lines % TAILLE_BLOC == 0, "TAILLE_BLOC divisible par amount_of_lines"
NB_BLOCS = amount_of_lines // TAILLE_BLOC

def cycle_alimentation():
    scope.io.target_pwr = False
    time.sleep(1)   #temsp de dechargement des capas parasites

    scope.io.target_pwr = True

    time.sleep(1)
    target.flush()


def lect_bloc(num_bloc):
    octets_num = num_bloc.to_bytes(1, byteorder = 'big')
    target.simpleserial_write('r', octets_num)

    reponse = target.simpleserial_read('w', TAILLE_BLOC, timeout = 500)

    if reponse is None or len(reponse) != TAILLE_BLOC:
        raise RuntimeError(f"reponse pas valide du bloc {num_bloc}")

    return reponse

test_data = []

for test in range(number_of_tests):
    campaign_matrix = []

    cycle_alimentation()

    for num_bloc in range(NB_BLOCS):
        bloc_recu = lect_bloc(num_bloc)

        for octet_recu in bloc_recu:
            matrix_row = []

            bit_mask = 0x01
            for bit in range(bits_per_line):
                matrix_row.append((octet_recu & bit_mask) == bit_mask)
                bit_mask = bit_mask*2
            campaign_matrix.append(matrix_row)

    test_data.append(campaign_matrix)

    print(f"allumage {test + 1}/{number_of_tests} termine ({len(campaign_matrix)} octets lus)")

print(len(test_data))

############### TRAITEMENT ###############

#test_data[num_test][num_octet][bit]

SEUIL_FIABLE = 0.95
BITS_PAR_LIGNE_AFFICHAGE = 64

data = np.array(test_data, dtype=bool)
n_tests, n_octets, n_bits = data.shape

nb_uns = data.sum(axis=0)
p1 = nb_uns / n_tests
fiabilite = np.maximum(p1, 1 - p1)
valeur_stable = p1 > 0.5

def vers_carte(m):
    flat = m.reshape(-1)
    nb_lignes = int(np.ceil(flat.size / BITS_PAR_LIGNE_AFFICHAGE))
    pad = nb_lignes * BITS_PAR_LIGNE_AFFICHAGE - flat.size
    flat = np.concatenate([flat.astype(float), np.full(pad, np.nan)])
    return flat.reshape(nb_lignes, BITS_PAR_LIGNE_AFFICHAGE)

carte_fiab = vers_carte(fiabilite)

# 0 = 0, 1 = 1, 2 = instable
toujours0 = (nb_uns == 0)
toujours1 = (nb_uns == n_tests)
categorie = np.full(nb_uns.shape, 2)
categorie[toujours0] = 0
categorie[toujours1] = 1
carte_cat = vers_carte(categorie)

total_bits = n_octets * n_bits
nb_parfaits = int((toujours0 | toujours1).sum())
nb_fiables = int((fiabilite >= SEUIL_FIABLE).sum())
print(f"Bits analysés            : {total_bits} ({n_octets} octets, {n_tests} allumages)")
print(f"Bits 100 % stables       : {nb_parfaits} ({100*nb_parfaits/total_bits:.1f} %)"
      f"  [toujours 0 : {int(toujours0.sum())} | toujours 1 : {int(toujours1.sum())}]")
print(f"Bits fiables (>= {SEUIL_FIABLE:.0%}) : {nb_fiables} ({100*nb_fiables/total_bits:.1f} %)")

from matplotlib.colors import ListedColormap
fig, axes = plt.subplots(1, 3, figsize=(18, 6))

im = axes[0].imshow(carte_fiab, cmap="RdYlGn", vmin=0.5, vmax=1.0, interpolation="nearest")
axes[0].set_title("Fiabilité de chaque bit\n(0.5 = aléatoire, 1.0 = toujours identique)")
fig.colorbar(im, ax=axes[0], fraction=0.046)

cmap_cat = ListedColormap(["#2b6cb0", "#e53e3e", "#cbd5e0"])
axes[1].imshow(carte_cat, cmap=cmap_cat, vmin=0, vmax=2, interpolation="nearest")
axes[1].set_title(f"Bits stables sur {n_tests} allumages\nbleu = 0 | rouge = 1 | gris = instable")

axes[2].hist(fiabilite.reshape(-1), bins=np.linspace(0.5, 1.0, 26), color="#38a169", edgecolor="black")
axes[2].axvline(SEUIL_FIABLE, color="red", linestyle="--", label=f"seuil {SEUIL_FIABLE:.0%}")
axes[2].set_xlabel("Fiabilité du bit")
axes[2].set_ylabel("Nombre de bits")
axes[2].set_title("Répartition de la fiabilité")
axes[2].legend()

for ax in axes[:2]:
    ax.set_xlabel(f"Position du bit (colonne = bit n°, {BITS_PAR_LIGNE_AFFICHAGE//8} octets par ligne)")
    ax.set_ylabel(f"Ligne (octets {BITS_PAR_LIGNE_AFFICHAGE//8}×n)")

plt.tight_layout()
plt.savefig("carte_fiabilite_sram.png", dpi=150)
plt.show()

bits_fiables = [(o, b, int(valeur_stable[o, b]))
                for o in range(n_octets) for b in range(n_bits)
                if fiabilite[o, b] >= SEUIL_FIABLE]
print(f"{len(bits_fiables)} bits fiables retenus ; exemples : {bits_fiables[:5]}")


##########################################################################################################################################
try:
    scope.dis()
except Exception:
    pass

print("Déconnexion réussie.")
