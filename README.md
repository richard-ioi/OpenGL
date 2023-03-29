*Travail réalisé dans le cadre d'un projet étudiant en deuxième année du cycle ingénieur (E4) à ESIEE Paris dans la filière Informatique & Applications*

# OpenGL

Simple projet étudiant OpenGL incluant :
- Plusieurs objets différents, avec plusieurs shaders différents
- Etre capable de charger des objets au format OBJ en utilisant la bibliothèque TinyOBJLoader, en gérant les matériaux (couleurs ambiantes, diffuses, speculaires)
- Avoir de l’illumination avec l’eq. de Phong ou Blinn-Phong

La solution .sln a été créée avec la dernière version de Visual Studio 2022, apparemment pas ouvrable avec la version 2019.
Nous avons implémenté deux objets (.obj) dans la scène avec un shader chacun :
- Un teapot (théière) de couleur rouge (couleur de base des vertex) utilisant le ColorShader, simple shader qui donne bêtement la couleur à l'objet (tout rouge).
- Un penguin, texturé utilisant le TextureShader, shader qui implémente les textures et l'illumination diffuse de phong.
- Vous pourrez vous apercevoir que l'on a tenté de mettre en place une matrice MVP pour pouvoir déplacer une caméra, toutes les matrices sont là. Cependant, nous avons eu du mal avec la ViewMatrix où il semble y avoir une erreur de calcul que nous ne pouvons pas résoudre par manque de temps. Nous avons donc retiré la ViewMatrix dans l'équation finale MVP translation * view * worldmatrix qui devient translation * worldmatrix.
- Nous avons donc "simulé" une caméra en exécutant une translation sur tous les objets de la scène, en réalité la caméra reste donc au point de vue (0,0,0) mais tous les objets bougent en même temps, ce qui donne l'illusion d'une caméra qui bouge dans la scène.
- Pour pouvoir déplacer la "caméra" il faut donc utiliser les flèches du clavier (x,y) ainsi que le scroll de la souris (z).
