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

**Our Method: Frankenstein, or The (Modern) Blob**
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
randomness added to keep exploration in tact. As this steering process keeps 
moving indefinitely, molds leave trails for other molds to follow. To capture 
the chemical process of the trails left by individual molds as they move we 
basically take a two step approach as motivated by Reynolds, at each frame
we multiply the entire grid by a number in the range of (0, 1) to capture
evaporation/decay of chemical trails. To capture its diffusing/spreading,
we basically go over each cell in our grid and check its four cell "Von Neumann" 
neighborhood and take the average of the five cells, four in neighbours and 
the cell itself, creating a relatively convincing form of spread. This simple
agent structure is enough to replicate slime mold, Physarum polycephalum's 
behaviour to an extent. To replicate the Tokyo experiment we also needed a certain way to 
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
  
**A small tangent: A slime mold vs a thousand ants**
TODO

**Results and Conclusion**
  After a few test runs pretty much all simulations of the same parameters
converge to similar networks. Different parameters have varying degrees of 
similar to the real railroad system surrounding Kadir Has University. Despite
the expected varience over the set of parameters, we can easily say some patterns
can be extracted. One of the clearest patterns to extract is the existence of 
redundant roads. Perhaps in direct relation to being a simulation of a biological
being certain stations have multiple paths connecting to their surrounding stations
even when a link is not readily implied topologically, this is a very essential
product of biological beings and more generally that of complex systems that is
called resilience, a system's ability to tolerate faults or errors. In the case
a connection is for any reason unusable agents can simply keep their traffic
busy on the other path. Another intereseting property is fast adaptability, which
is rather hard to compare with the real thing as there is not enough data about 
faulty lines in Istanbul's transportation network and how its treated in real time
but being a complex system, our slime mold system is very quick to find new roads
when the user tries to baricade an existing path using anti-oaths. Now for the rose's
thorns, being an agent based model, it thrives on a certain level of entropy, by 
introducing a certain small value of randomness to the keep steady behaviour in steering
overall more effective paths can be found as more possibilities get explored but this
means our system is deeply non-deterministic though it is entirely possible to strip
away all randomness, in such a case increasing the population of slime molds is more
likely to find more efficient solutions. Which brings us to another drawback of using
an agent based model into what is essentially a path finding problem. We cannot effectively
prove the system's convergence, nor can we prove its efficiency, of course running the 
simulation with the same parameters using different initial conditions, we can have an
idea of its behaviour but we cannot pinpoint exact numbers which makes our simulation
not exactly practical for grand scale usage. That being said our simulation is an 
interesting display how very simple rules can create wildly complex systems, how biology
thrives on it and how the biological process can be transferred to the digital 
quite easily.