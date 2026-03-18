Questo è un progetto di gruppo fatto come parte del core curriculum alla scuola 42 di Firenze, si tratta un'implementazione minimale di un server IRC in C++, è testato utilizzando "Konversation", perciò il comportamento con altri client può variare.

Il progetto si basa su socket UNIX perciò il funzionamento in altri ambienti non è garantito.

# Setup

- compilare con make
- runnare l'eseguibile generato

una volta eseguito il server esso rimarrà in ascolto per connessioni in entrata

- connettere il client

può essere utilizzato Konversation per un UI comoda, altrimenti è possibile testarlo utilizando il tool netcat

## Args

il programma richiede 2 argomenti

- la porta sulla quale il server resterà in ascolto
- la password del server

ex: ./ircserv 4242 pass

# Connessione

se si utilizza Konversation impostare IP e porta tramite UI altrimenti con netcat:

- nc -C 127.0.0.1 4242

a questo punto come da standard IRC verranno richiesti:

- NICK
- USER
- PASSWORD

# Comandi

i comandi implementati sono:

- JOIN
- QUIT
- PRIVMSG
- PART
- MODE
- TOPIC
- KICK

