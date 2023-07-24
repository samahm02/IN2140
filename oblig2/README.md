1. Når load_inodes() kalles bruker den fopen() metoden for å få en fildiskriptor til master_file_table filen, deretter sender den fildiskriptoren til en hjelpemetode.

Hjelpemetoden bruker først malloc for å dynamisk allokere minne til rooten av filsystemet. Deretter brukes fread til å lese inn verdiene til noden. Etter dette kjører koden en for-loop som enten lager en inode for hvert entry, dersom is_dictionairy, eller så vil den laste dataene fra "disken" og legger det inn i noden sin entries. 

2. Eventuelle krav som ikke er oppfylt: Ingen

3. Eventuelle deler av implementasjonen som avviker fra prekoden: Ingen

4. Eventuelle tester som feiler: Ingen