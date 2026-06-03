#!/usr/bin/env python3
"""
graph.py – Implémentation d’un graphe simple avec listes d’adjacence
Optimisé pour les requêtes de voisinage (O(degré) au lieu de O(nbe)).

Traduction Python du code C original.
"""

import sys
import random
from typing import List, Optional, Tuple

# ----------------------------------------------------------------------
# Classes représentant les structures du C
# ----------------------------------------------------------------------

class Vertex:
    """Sommet du graphe."""
    def __init__(self, name: str):
        self.name = name
        self.id = -1          # sera mis à jour lors de l'ajout au graphe

    def __repr__(self):
        return f"Vertex({self.name}, id={self.id})"

class Edge:
    """Arête entre deux sommets."""
    def __init__(self, name: str, v1: Vertex, v2: Vertex):
        self.name = name
        self.endpoints = [v1, v2]

    def __repr__(self):
        return f"Edge({self.name}, {self.endpoints[0].name} -- {self.endpoints[1].name})"

class AdjList:
    """Liste d'adjacence dynamique (simulée par une liste Python)."""
    def __init__(self):
        self.edges: List[Edge] = []   # correspond à edge** edges, avec taille variable

class Graph:
    """Graphe simple avec listes d'adjacence."""
    def __init__(self, name: str, nbvMAX: int):
        self.name = name
        self.nbvMAX = nbvMAX
        self.nbv = 0
        self.nbeMAX = (nbvMAX * (nbvMAX - 1)) // 2
        self.nbe = 0
        self.vertices: List[Optional[Vertex]] = [None] * nbvMAX
        self.edges: List[Optional[Edge]] = [None] * self.nbeMAX
        self.adj: List[AdjList] = [AdjList() for _ in range(nbvMAX)]

# ---------- Création des éléments ----------

def create_vertex(name: str) -> Vertex:
    """Équivalent de createVertex(char* name)."""
    return Vertex(name)

def create_edge(name: str, v1: Vertex, v2: Vertex) -> Edge:
    """Équivalent de createEdge."""
    return Edge(name, v1, v2)

def create_graph(name: str, nbvMAX: int) -> Graph:
    """Équivalent de createGraph."""
    return Graph(name, nbvMAX)

# ---------- Recherche ----------

def get_vertex(g: Graph, name: str) -> Optional[Vertex]:
    """Équivalent de getVertex."""
    for i in range(g.nbv):
        if g.vertices[i] is not None and g.vertices[i].name == name:
            return g.vertices[i]
    return None

def get_edge(g: Graph, name: str) -> Optional[Edge]:
    """Équivalent de getEdge."""
    for i in range(g.nbe):
        if g.edges[i] is not None and g.edges[i].name == name:
            return g.edges[i]
    return None

# ---------- Ajout ----------

def add_vertex(g: Graph, v: Vertex) -> int:
    """Ajoute un sommet au graphe. Retourne 1 si succès, 0 sinon."""
    if g.nbv >= g.nbvMAX:
        print(f"WARNING: too many vertices in {g.name}, ignoring {v.name}")
        return 0
    g.vertices[g.nbv] = v
    v.id = g.nbv
    g.nbv += 1
    return 1

def vertex_index(g: Graph, v: Vertex) -> int:
    """Retourne l'index du sommet v dans le tableau vertices (id interne)."""
    if v is None:
        return -1
    return v.id

def add_edge(g: Graph, e: Edge) -> int:
    """Ajoute une arête. Retourne 1 si succès, 0 sinon."""
    if g.nbe >= g.nbeMAX:
        print(f"WARNING: too many edges in {g.name}, ignoring {e.name}")
        return 0

    i1 = vertex_index(g, e.endpoints[0])
    i2 = vertex_index(g, e.endpoints[1])
    if i1 == -1 or i2 == -1:
        print(f"WARNING: edge {e.name} references unknown vertices")
        return 0

    g.edges[g.nbe] = e
    g.nbe += 1

    # Ajout dans les listes d'adjacence
    g.adj[i1].edges.append(e)
    g.adj[i2].edges.append(e)
    return 1

# ---------- Suppression ----------

def remove_edge_ptr(g: Graph, e: Edge):
    """Supprime une arête à partir de son pointeur (fonction interne)."""
    i1 = vertex_index(g, e.endpoints[0])
    i2 = vertex_index(g, e.endpoints[1])

    # Retirer des listes d'adjacence
    for idx in (i1, i2):
        adj = g.adj[idx]
        if e in adj.edges:
            adj.edges.remove(e)

    # Retirer du tableau global g.edges
    for i in range(g.nbe):
        if g.edges[i] is e:
            # Décalage
            for j in range(i, g.nbe - 1):
                g.edges[j] = g.edges[j + 1]
            g.edges[g.nbe - 1] = None
            g.nbe -= 1
            break

    # En Python, l'objet e sera garbage-collecté, pas besoin de free().
    # Mais si on veut simuler la libération du nom, on peut supprimer la référence.
    # On ne fait rien de plus.

def delete_edge(g: Graph, e: Edge):
    """Équivalent de deleteEdge(Graph* g, Edge e)."""
    ptr = get_edge(g, e.name)
    if ptr:
        remove_edge_ptr(g, ptr)

def delete_vertex(g: Graph, v: Vertex):
    """Équivalent de deleteVertex."""
    ptr = get_vertex(g, v.name)
    if ptr is None:
        return

    idx = vertex_index(g, ptr)
    if idx == -1:
        return

    # Copie de la liste d'adjacence car elle sera modifiée pendant la suppression
    adj_edges = list(g.adj[idx].edges)  # snapshot

    for e in adj_edges:
        remove_edge_ptr(g, e)

    # Libération de la liste d'adjacence du sommet supprimé (remise à zéro)
    g.adj[idx].edges.clear()

    # Décalage des tableaux vertices et adj
    # On enlève le sommet du tableau
    for i in range(idx, g.nbv - 1):
        g.vertices[i] = g.vertices[i + 1]
        g.adj[i] = g.adj[i + 1]
        if g.vertices[i] is not None:
            g.vertices[i].id = i   # mise à jour de l'id

    g.vertices[g.nbv - 1] = None
    g.adj[g.nbv - 1] = AdjList()   # réinitialise une liste vide
    g.nbv -= 1

# ---------- Voisinage (optimisé) ----------

def get_issued_edges(g: Graph, v: Vertex) -> Tuple[Vertex, List[Edge]]:
    """Retourne (v, liste des arêtes incidentes)."""
    idx = vertex_index(g, v)
    if idx == -1:
        return (v, [])
    return (v, list(g.adj[idx].edges))   # copie pour être cohérent avec l'original

# ---------- Utilitaires ----------

def degrees_tab(g: Graph) -> List[int]:
    """Équivalent de degreesTab."""
    return [len(g.adj[i].edges) for i in range(g.nbv)]

def print_vertex(v: Vertex):
    print(v.name, end="")

def print_edge(e: Edge):
    print(f"{e.name} : {e.endpoints[0].name} -- {e.endpoints[1].name}", end="")

def print_graph(g: Graph):
    print(f'--- Graph "{g.name}" ---')
    print("Vertices:")
    for i in range(g.nbv):
        print("  ", end=""); print_vertex(g.vertices[i]); print()
    print("Edges:")
    for i in range(g.nbe):
        print("  ", end=""); print_edge(g.edges[i]); print()
    print("Adjacency lists:")
    for i in range(g.nbv):
        print(f"  {g.vertices[i].name} -> ", end="")
        for e in g.adj[i].edges:
            print(f"{e.name} ", end="")
        print()
    print("--------")

# ---------- Clonage ----------

def clone_graph(g: Graph) -> Graph:
    """Équivalent de cloneGraph."""
    buff = f"{g.name}_clone"
    clone = create_graph(buff, g.nbvMAX)

    # Tableau de correspondance ancien -> nouveau sommet
    mapping = [None] * g.nbvMAX

    # Cloner les sommets
    for i in range(g.nbv):
        new_v = create_vertex(g.vertices[i].name)
        add_vertex(clone, new_v)
        mapping[i] = new_v

    # Cloner les arêtes en utilisant les nouveaux sommets
    for i in range(g.nbe):
        old_e = g.edges[i]
        new_v1 = mapping[vertex_index(g, old_e.endpoints[0])]
        new_v2 = mapping[vertex_index(g, old_e.endpoints[1])]
        new_e = create_edge(old_e.name, new_v1, new_v2)
        add_edge(clone, new_e)

    return clone

# ---------- Libération (no-op en Python, mais conservée pour l'interface) ----------

def free_graph(g: Graph):
    """En Python, la mémoire est libérée par le garbage collector.
       Cette fonction existe pour compatibilité mais ne fait rien."""
    pass

# ---------- Comptage de triangles ----------

def count_triangle(g: Graph) -> int:
    """Équivalent de countTriangle."""
    n = g.nbv
    if n < 3:
        return 0

    mark = [0] * n
    cur_mark = 0
    triangles = 0

    for i in range(n):
        u = g.vertices[i]
        cur_mark += 1
        # Marquer tous les voisins de u
        for e in g.adj[i].edges:
            neigh = e.endpoints[1] if e.endpoints[0] is u else e.endpoints[0]
            # Trouver l'indice du voisin (optimisable en stockant l'indice dans Vertex)
            idx = -1
            for j in range(n):
                if g.vertices[j] is neigh:
                    idx = j
                    break
            if idx != -1:
                mark[idx] = cur_mark

        # Pour chaque voisin v de u d'indice > i
        for e in g.adj[i].edges:
            v = e.endpoints[1] if e.endpoints[0] is u else e.endpoints[0]
            idx_v = -1
            for j in range(n):
                if g.vertices[j] is v:
                    idx_v = j
                    break
            if idx_v <= i:
                continue

            # Chercher les voisins w de v avec w > idx_v et marqués
            for e2 in g.adj[idx_v].edges:
                w = e2.endpoints[1] if e2.endpoints[0] is v else e2.endpoints[0]
                idx_w = -1
                for j in range(n):
                    if g.vertices[j] is w:
                        idx_w = j
                        break
                if idx_w > idx_v and mark[idx_w] == cur_mark:
                    triangles += 1

    return triangles

# ----------------------------------------------------------------------
# Fonctions utilitaires pour l'export DOT (getBaseIndex, getSuffix, etc.)
# ----------------------------------------------------------------------

def get_base_index(name: str) -> int:
    """Retourne l'indice de base d’un sommet (le premier entier après la lettre)."""
    # Extrait le premier entier rencontré après le premier caractère
    i = 1
    while i < len(name) and not name[i].isdigit():
        i += 1
    if i >= len(name):
        return -1
    j = i
    while j < len(name) and name[j].isdigit():
        j += 1
    return int(name[i:j]) if i < j else -1

def get_suffix(name: str) -> int:
    """Retourne le suffixe _X, ou -1 si absent."""
    if '_' in name:
        parts = name.split('_')
        if len(parts) >= 2 and parts[-1].isdigit():
            return int(parts[-1])
    return -1

def get_rank_group(v: Vertex) -> int:
    """Détermine un identifiant de groupe pour le rank=same.
       Ordre souhaité (bas → haut) : s simples, m_0, m_1, m_2, m_3, s_0, s_1, s_2, ...
    """
    name = v.name
    if name[0] == 's' and '_' not in name:
        return 0                     # s2, s4, … tout en bas
    if name[0] == 'm':
        return 1 + get_suffix(name)  # m_0 -> 1, m_1 -> 2, …
    if name[0] == 's':
        return 10 + get_suffix(name) # s_0 -> 10, s_1 -> 11, …
    return 99

def compare_vertex_name(a: Vertex, b: Vertex) -> int:
    """Comparaison pour trier les sommets d’un même groupe par ordre alphabétique."""
    if a.name < b.name: return -1
    if a.name > b.name: return 1
    return 0

def vertex_order(a: Vertex, b: Vertex) -> int:
    """Ordre de hauteur (suffixe ou type) pour aligner du bas vers le haut."""
    rankA = get_rank_group(a)
    rankB = get_rank_group(b)
    if rankA != rankB:
        return rankA - rankB
    return compare_vertex_name(a, b)

# ----------------------------------------------------------------------
# Exportation en DOT
# ----------------------------------------------------------------------

def generate_dot_file(g: Graph, filename: str):
    """Génère un fichier DOT avec alignement vertical automatique."""
    with open(filename, 'w') as f:
        f.write(f'graph "{g.name}" {{\n')
        f.write('\tlayout=dot;\n')
        f.write('\trankdir=TB;\n')
        f.write('\tsplines=false;\n')
        f.write('\tnewrank=true;\n\n')

        # 1. Regrouper les sommets par groupe de rang
        MAX_GROUP = 50
        groups = [[] for _ in range(MAX_GROUP)]

        for i in range(g.nbv):
            grp = get_rank_group(g.vertices[i])
            if 0 <= grp < MAX_GROUP:
                groups[grp].append(g.vertices[i])

        # Trier chaque groupe par nom
        for grp in range(MAX_GROUP):
            groups[grp].sort(key=lambda v: v.name)

        # Écrire les blocs rank=same
        for grp in range(MAX_GROUP):
            if not groups[grp]:
                continue
            f.write('\t{ rank=same;')
            for v in groups[grp]:
                f.write(f' {v.name};')
            f.write(' }\n')

        # 2. Déclaration des sommets
        f.write('\n')
        for i in range(g.nbv):
            f.write(f'\t{g.vertices[i].name} [shape=circle];\n')

        # 3. Arêtes réelles
        f.write('\n')
        for i in range(g.nbe):
            e = g.edges[i]
            f.write(f'\t{e.endpoints[0].name} -- {e.endpoints[1].name};\n')

        # 4. Arêtes invisibles pour l'alignement vertical
        f.write('\n\t// Contraintes d\'alignement vertical par groupe\n')
        MAX_BASE = 100
        base_lists = [[] for _ in range(MAX_BASE)]

        for i in range(g.nbv):
            base = get_base_index(g.vertices[i].name)
            if 0 <= base < MAX_BASE:
                base_lists[base].append(g.vertices[i])

        for base in range(MAX_BASE):
            if len(base_lists[base]) <= 1:
                continue
            # Trier selon l'ordre vertical (du bas vers le haut)
            base_lists[base].sort(key=lambda v: (get_rank_group(v), v.name))
            # Relier dans l'ordre : le plus haut (dernier) vers le plus bas (premier)
            for k in range(len(base_lists[base]) - 1, 0, -1):
                f.write(f'\t{base_lists[base][k].name} -- {base_lists[base][k-1].name} [style=invis];\n')

        f.write('}\n')
    print(f"Fichier DOT généré : {filename}")

# ----------------------------------------------------------------------
# Fonctions internes pour PathOfTriangle
# ----------------------------------------------------------------------

def edge_exists(g: Graph, v1: Vertex, v2: Vertex) -> bool:
    """Vrai si une arête existe déjà entre v1 et v2."""
    for i in range(g.nbe):
        e = g.edges[i]
        if (e.endpoints[0] is v1 and e.endpoints[1] is v2) or \
           (e.endpoints[0] is v2 and e.endpoints[1] is v1):
            return True
    return False

def share_common_s(g: Graph, a: Vertex, b: Vertex) -> bool:
    """Vrai si a et b ont un voisin commun dont le nom commence par 's'."""
    # Trouver les indices
    idx_a = -1
    idx_b = -1
    for i in range(g.nbv):
        if g.vertices[i] is a:
            idx_a = i
        if g.vertices[i] is b:
            idx_b = i
    if idx_a == -1 or idx_b == -1:
        return False

    # Marquer les voisins de type 's' de a
    marked = set()
    for e in g.adj[idx_a].edges:
        neigh = e.endpoints[1] if e.endpoints[0] is a else e.endpoints[0]
        if neigh.name.startswith('s'):
            marked.add(neigh)

    # Vérifier si b possède un voisin marqué
    for e in g.adj[idx_b].edges:
        neigh = e.endpoints[1] if e.endpoints[0] is b else e.endpoints[0]
        if neigh.name.startswith('s') and neigh in marked:
            return True
    return False

# ----------------------------------------------------------------------
# PathOfTriangle (génération du graphe complexe)
# ----------------------------------------------------------------------

def path_of_triangle(path: int, core: int, maxPerBox: int, maxLR: int) -> Graph:
    """Équivalent de PathOfTriangle."""
    nbVertices = 2 * maxPerBox * path + 2 * maxLR * ((path + 1) // 2)
    g = create_graph("PoT", nbVertices)

    # Tableau des sommets principaux du chemin
    path_vertices = [None] * path

    # Compteur d'arêtes entre paires d'indices pairs consécutifs
    edge_counters = [0] * (path - 1)

    # --- Création du chemin principal (s2, s4, ..., s(2*path)) ---
    for i in range(path):
        idx = 2 * (i + 1)
        name = f"s{idx}"
        path_vertices[i] = create_vertex(name)
        add_vertex(g, path_vertices[i])

    # Arêtes du chemin et leurs sommets M
    for i in range(path - 1):
        left_name = f"s{2*(i+1)}"
        right_name = f"s{2*(i+2)}"
        e = create_edge(f"e_{left_name}_{right_name}", path_vertices[i], path_vertices[i+1])
        add_edge(g, e)

        left  = 2 * (i + 1)
        right = 2 * (i + 2)
        mid   = left + 1
        cnt   = edge_counters[i]
        edge_counters[i] += 1
        m = create_vertex(f"m{mid}_{cnt}")
        add_vertex(g, m)
        add_edge(g, create_edge(f"e_m{mid}_{cnt}_s{left}", m, path_vertices[i]))
        add_edge(g, create_edge(f"e_m{mid}_{cnt}_s{right}", m, path_vertices[i+1]))

    # --- Boîtes (copies) sur les sommets intérieurs ---
    multiplied = [False] * path
    previous_taken = False
    for i in range(1, path - 1):
        if not previous_taken and random.randint(0, 1):
            multiplied[i] = True
            k = random.randint(0, maxPerBox)   # maxPerBox inclus?
            # En C: rand() % maxPerBox (donc 0..maxPerBox-1). Ici on suit.
            k = random.randint(0, maxPerBox - 1) if maxPerBox > 0 else 0
            for j in range(k):
                base_idx = 2 * (i + 1)
                new_v = create_vertex(f"s{base_idx}_{j}")
                add_vertex(g, new_v)

                # Connexion gauche
                add_edge(g, create_edge(f"e_{new_v.name}_s{base_idx-2}", new_v, path_vertices[i-1]))

                left  = base_idx - 2
                right = base_idx
                mid   = left + 1
                pair_idx = i - 1
                cnt = edge_counters[pair_idx]
                edge_counters[pair_idx] += 1
                m_left = create_vertex(f"m{mid}_{cnt}")
                add_vertex(g, m_left)
                add_edge(g, create_edge(f"e_{m_left.name}_s{left}", m_left, path_vertices[i-1]))
                add_edge(g, create_edge(f"e_{m_left.name}_{new_v.name}", m_left, new_v))

                # Connexion droite
                add_edge(g, create_edge(f"e_{new_v.name}_s{base_idx+2}", new_v, path_vertices[i+1]))

                left  = base_idx
                right = base_idx + 2
                mid   = left + 1
                pair_idx = i
                cnt = edge_counters[pair_idx]
                edge_counters[pair_idx] += 1
                m_right = create_vertex(f"m{mid}_{cnt}")
                add_vertex(g, m_right)
                add_edge(g, create_edge(f"e_{m_right.name}_{new_v.name}", m_right, new_v))
                add_edge(g, create_edge(f"e_{m_right.name}_s{right}", m_right, path_vertices[i+1]))
            previous_taken = True
        else:
            previous_taken = False

    # --- Marquage des sommets non multipliés (ajout d'un 'n') ---
    for i in range(path):
        if not multiplied[i]:
            v = path_vertices[i]
            v.name = v.name + "n"   # modification du nom (attention aux références)

    # --- Création des paires L/R pour les sommets marqués ---
    L_pairs = [None] * path   # listes de listes
    R_pairs = [None] * path
    nbLR_list = [0] * path
    if core:
        maxLR = 1

    for i in range(path):
        if not multiplied[i]:
            base_idx = 2 * (i + 1)
            nbLR = random.randint(0, maxLR) if maxLR >= 0 else 0   # rand() % (maxLR+1) -> 0..maxLR
            nbLR_list[i] = nbLR
            if nbLR > 0:
                L_pairs[i] = [None] * nbLR
                R_pairs[i] = [None] * nbLR
            for j in range(nbLR):
                l_idx = base_idx + 1
                r_idx = base_idx - 1

                L = create_vertex(f"l{l_idx}_{j}")
                add_vertex(g, L)
                L_pairs[i][j] = L

                R = create_vertex(f"r{r_idx}_{j}")
                add_vertex(g, R)
                R_pairs[i][j] = R

                add_edge(g, create_edge(f"e_{L.name}_{path_vertices[i].name}", L, path_vertices[i]))
                add_edge(g, create_edge(f"e_{R.name}_{path_vertices[i].name}", R, path_vertices[i]))
                add_edge(g, create_edge(f"e_{L.name}_{R.name}", L, R))

    # --- Nouveaux sommets négatifs pour les sommets marqués (si pas core) ---
    if not core:
        for i in range(path):
            base_idx = 2 * (i + 1)
            nbNew = random.randint(0, maxPerBox - 1) if maxPerBox > 0 else 0  # 0..maxPerBox-1
            for j in range(nbNew):
                neg = create_vertex(f"s{base_idx}_1_{j+1}")
                add_vertex(g, neg)
                for k in range(nbLR_list[i]):
                    chosen = L_pairs[i][k] if random.randint(0, 1) else R_pairs[i][k]
                    add_edge(g, create_edge(f"e_{neg.name}_{chosen.name}", neg, chosen))

    # On compte les triangles avant modifications
    nb_triangles = count_triangle(g)
    g_simple = clone_graph(g)

    # --- Relier les colonnes paires consécutives ---
    for i in range(2, 2*path+1, 2):
        j = i + 2
        if j > 2*path:
            continue
        col_i = []
        col_j = []
        for v in g.vertices:
            if v is None:
                continue
            if not v.name.startswith('s'):
                continue
            base = get_base_index(v.name)
            if base == i:
                col_i.append(v)
            elif base == j:
                col_j.append(v)
        for a in col_i:
            for b in col_j:
                if not edge_exists(g, a, b):
                    add_edge(g, create_edge(f"e_{a.name}_{b.name}", a, b))

    # --- Relier les M_{i-1} et M_{i+1} s'ils n'ont pas de voisin 's' commun ---
    for i in range(2, 2*path+1, 2):
        left_idx = i - 1
        right_idx = i + 1
        if left_idx < 1 or right_idx > 2*path+1:
            continue
        leftMs = []
        rightMs = []
        for v in g.vertices:
            if v is None:
                continue
            if not v.name.startswith('m'):
                continue
            base = get_base_index(v.name)
            if base == left_idx:
                leftMs.append(v)
            elif base == right_idx:
                rightMs.append(v)
        for a in leftMs:
            for b in rightMs:
                if not edge_exists(g, a, b) and not share_common_s(g, a, b):
                    add_edge(g, create_edge(f"e_{a.name}_{b.name}", a, b))

    # --- Relier les colonnes i et j telles que j >= i+3 et (j-i)%3 == 2 ---
    for i in range(2, 2*path+1, 2):
        for j in range(i+3, 2*path+1, 2):
            if (j - i) % 3 != 2:
                continue
            col_i = []
            col_j = []
            for v in g.vertices:
                if v is None:
                    continue
                if not v.name.startswith('s'):
                    continue
                base = get_base_index(v.name)
                if base == i:
                    col_i.append(v)
                elif base == j:
                    col_j.append(v)
            for a in col_i:
                for b in col_j:
                    if not edge_exists(g, a, b):
                        add_edge(g, create_edge(f"e_{a.name}_{b.name}", a, b))

    # --- Suppression aléatoire d'arêtes entre sommets sX_1_Y ---
    remove_rate = 0.1
    if remove_rate > 0.0:
        to_remove = []
        for e in g.edges[:g.nbe]:
            v1 = e.endpoints[0]
            v2 = e.endpoints[1]
            if not (v1.name.startswith('s') and v2.name.startswith('s')):
                continue
            # Test si les deux sont de la forme sX_1_Y
            if '_1_' not in v1.name or '_1_' not in v2.name:
                continue
            if random.random() < remove_rate:
                to_remove.append(e)
        for e in to_remove:
            delete_edge(g, e)

    # Libération des tableaux temporaires (automatique en Python)

    nb_triangles_finaux = count_triangle(g)
    print(f"NOMBRE DE TRIANGLES INITIAUX:{nb_triangles}. NOMBRE DE TRIANGLES FINAUX: {nb_triangles_finaux}")

    return g