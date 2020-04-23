# Remote System Monitor
Progetto del corso di "Laboratorio di Sistemi Operativi" progettato e sviluppato da Ciro Gallo e Monteiro Del Prete, studenti dell'  Università degli Studi di Napoli Federico II, corso di laurea in INFORMATICA (L-31).

# Specifiche
Realizzare un sistema che consenta di monitorare lo stato di più computer connessi in rete. Il sistema deve
essere costituito da tre componenti:
- un server che riceva e raccolga le informazioni riguardanti gli host monitorati;
- un agent che venga eseguito sui singoli host da monitorare ed invii le informazioni sullo stato dell’host
al server;
- un client che interagisca con il server centrale per chiedere informazioni sullo stato degli host.

Si utilizzi il linguaggio C su piattaforma UNIX. I processi comunicano tramite socket TCP.

# Documentazione
La documentazione completa del software è disponibile al [https://docs.google.com/document/d/1Y5Ok6j4xdEeq5L_xVmeziPvZghuUd3JcSD31FKqhnWs/](seguente link) in cui è possibile reperire:
- Guida alla compilazione e all'uso.
- Protocollo di comunicazione client-server.
- Protocollo di comunicazione agent-server.
- Dettagli implementativi.

# Descrizione dettagliata
Il sistema di monitoraggio deve fornire informazioni sullo stato degli host di una rete. L’architettura del sistema
prevede tre componenti: un server, un client ed un agent.

**Il server** riceve periodicamente dagli agent le informazioni sullo stato degli host presenti sulla rete, memorizza tali informazioni insieme al nome simbolico dell’host (o al suo IP qualora questo non sia disponibile) e l’istante in cui le ha ricevute. Qualora il server non riceva informazioni su un dato host per più di 6 secondi dovrà considerare l’host come disconnesso e continuare a memorizzare soltanto l’ultimo istante in cui ha ricevuto dati da esso.
Il server deve inoltre gestire le connessioni dei client che possono richedere le informazioni sullo stato di un dato host connesso e la data e l’ora dell’ultima informazione utile ricevuta da un host disconnesso.
I client e gli agent comunicano con il server collegandosi ad una o più porte TCP fissate ed utilizzando un apposito protocollo interno all’applicazione. All’avvio del server, è possibile specificare tramite riga di comando la porta (o le porte) TCP sulla quale mettersi in ascolto. Il server è di tipo concorrente, ovvero è in grado di servire più client e gestire più agent simultanemente.

**L’agent** è un programma in esecuzione su ogni host da monitorare. Il suo compito è prelevare le informazioni sullo stato del sistema mediante la system call sysinfo e inviarle al server ogni 3 secondi. Le informazioni da inviare sono le seguenti:
- i secondi trascorsi dall’ultimo boot
- la percentuale di memoria fisica disponibile
- il numero di processi in esecuzione.

**Il client** consente all’utente di accedere alle informazioni sugli host della rete. Esso si connette al server (specificando sulla linea di comando l’indirizzo IP e la porta del server) e ottiene una lista di host connessi e disconnessi. A quel punto, l’utente può selezionare ripetutamente uno degli host della lista e ottenere tutte le informazioni sul suo stato, se quell’host risulta connesso, o la data e l’ora dell’ultimo aggiornamento, se quell’host risulta disconnesso.
