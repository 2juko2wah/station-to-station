**Abstract**
  Slime molds are certain distanly related eukaryotic organisms with a
tangled single-celled and multicellular life cycle, found in settings
with similar environment to those favoured by mushrooms. A specific species
of slime molds called Physarum Polycephalum had been used by Atsushi Tero
and his team at Hokkaido university to demonstrate this unicellular organism
with a completely decenteralized decision making mechanism was capable of
creating efficient networks. Our goal in this research project was to 
replicate this project digitally by using a multi-agent model of slime molds
proposed by Reynolds (2010). The source code for this project is readily
available at: https://www.github.com/2juko2wah/station-to-station.

**The Experiment**
  Tero's experiment is really interesting because on the surface it is 
quite rudimentary. They basically place slime molds in a setting where
city centers and metro stations in Tokyo city are represented by oats
which lovely slime molds adore feasting upon. After letting the slime 
molds crawl around on the surface for 24 hours, they compare tunnels
formed by slime molds between food sources to the actual railroads system
and note that the result was pretty similar to the real deal, sans certain
characteristic differences.

**A literally no-brainer approach to path finding**
  But how can these unicellular organisms without any centeral decision 
making capabilities could replicate the railroad system made by very
Japanese engineers known for their brilliancy and efficiency? The answer
is surprisingly simple. Individually slime molds are only capable of sensing
immediate stimuli those of food sources, lights, certain chemicals and most 
importantly of other members of their species. Being very moldy, they also
tend to leave traces where ever they move in small amounts that keep 
evaporating. As individual slime molds target to the same food source, they
start leaving more and more trails to the said source. Effectively creating 
channels or tunnels other slime molds can follow, while less preferred paths 
evaporate more quickly. This simple mechanism creates very beautiful patterns
but also create the sense of a greater intelligence than there is.

**Frankenstein, or The (Modern) Blob**
  This simple behavior begs the question, is such a simple behaviour can
create highly efficient and desirable networks why shouldnt we try to 
replicate it. Reylonds (2010) exactly aimed at this, creating an agent
based model of slime molds, with a few very simple rules. Each agent is 
assigned a position in the two dimensional plane, a direction of where they are
heading in angles and three sensors placed on the perimeter of a circle whose
center is the position of the agent. The first sensor is directly at the
heading direction while two others are respectively placed at the left and
right of the forward sensor with a certain angle. The three sensors are the
only window an agent has to the plane, so all their interactions are entirely
localized. To decide on the direction to head they basically compare sensory
information from the three sensors. If only the forward sensor is buzzing, 
an agent keeps it straight, if the left sensor outweights the others steers
to left and if the right sensor outweights it steers to right. If none of the
sensors have any reading it keeps its heading direction steady with a certain
randomness added to keep exploration in tact. This simple agent structure is
enough to replicate slime mold, Physarum polycephalum's behaviour to an 
extent. To replicate the Tokyo experiment we also needed a certain way to 
represent stations, to keep our model simple we "implemented" oats by
constantly emmiting signals at certain positions on the map as desired. 
Unlike Tokyo city, Istanbul's terrain and roads are eponymously convoluted
with rivers, crossing roads, hills, sea, and lakes, to model these we 
basically use anti-oats, that constantly emmit negative signals which due to
the linear nature of the underlying maths work without any rework. The 
anti-oat idea can be physically and biologically motivated by slime mold's 
reaction to salt and light which causes them to steer away without a second
thought. With out agents, digital oats and anti-oats we attempt to replicate
a result similar to the Tokyo result on a region of Istanbul's railroad 
system, specifically selected from an area that surround Kadir Has University.
  
