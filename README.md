Primul README.md
Proiect PSO 2022 FUSE

Commiters: rmocanu001 qRanubis

Surse informare: 
1. [Definitia fuse.h](https://github.com/libfuse/libfuse/blob/master/include/fuse.h)
2. [Tutorial 1](https://www.maastaar.net/fuse/linux/filesystem/c/2016/05/21/writing-a-simple-filesystem-using-fuse/)
3. [Tutorial 2](https://www.maastaar.net/fuse/linux/filesystem/c/2019/09/28/writing-less-simple-yet-stupid-filesystem-using-FUSE-in-C/)
4. [Stackoverflow thread](https://stackoverflow.com/questions/23208634/writing-a-simple-filesystem-in-c-using-fuse)
5. [youtube tutorial](https://www.youtube.com/watch?v=LZCILvr5tUk)

Scop proiect: Realizarea unui sistem de fisiere in user interface, cu cel putin functionalitatile de baza specifice sistemului de fisire implenatat in kernel. De asemenea intelegera profunda a sistemului de fisire specific Linux.

Detalii implementare:
- folosim in main functia fuse_main() care initializeaza sistemul FUSE in mountpoint-ul primit ca argument.
- avem functia fs_getattr in care atribuim fisierelor si directoarelor atributele necesare Similar to stat().
- restul functiilor sunt implementate conform contextului felului in care a fost gandita pastrarea numelor fisirelor/directorelor noastre.

Probleme intalnite:

* Avem nevoie de o structura arborescenta in care sa putem salva instantele directorelor pentru a putea implementa link, rename, etc...

* In momentul de fata putem accesa doar 2 layere de directoare sau fisiere deoarece denumiriile le salvam intr-o matice [n][2];

* Am implementat din tutorialul LSYS FUSE tot ce era posibil limitati fiind de faptul ca nu am creeat o structura in care sa salvam diverse informatii;
