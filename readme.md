#CITS project
- The goal of this project is to develop a server application to manage the data and permit queries about bus and train routes, such as those in the Transperth transport network. By successfully completing the project, you will have a greater understanding of the standard TCP and UDP protocols running over IP, communication between web-browsers and application programs using HTTP and HTML, and will have developed a simple text-based protocol to make and reply to queries for distributed information.
- no station server should ever contain all knowledge about the whole network, timetabling data, or network connections. Each station's timetabling data, recorded in one textfile for each station, may change at any time (for example, if a bus breaks down, its next trip will be cancelled). Every query arriving at a station server (via a UDP/IP datagram) should be answered using the current up-to-date timetable information for that station.
- It should be tested on a linux environment Compiling should be done either by using make or with the command below cc -std=c99 project.c -o project
[Running the Server] Once the project.c file is finished compiling,
- Each process receives a small number of command-line arguments, providing its station's name, its unique port for TCP/IP-based communication with a web-browser, its unique port for UDP/IP-based communication with other stations using datagrams, the UDP/IP-based port(s) of directly connected (neighbour) stations. 

shell>  ./station Warwick-Stn 2401 2408 2560 2566 .... &  

shell>  ./station.py Greenwood-Stn 2402 2560 2567 2408 .... &

which indicates that the first process (a compiled C program) will manage the data of the station named "Warwick-Stn", will receive queries from web-browsers using TCP/IP port 2401, will receive datagrams from other stations using UDP/IP port 2408, and that "Warwick-Stn" is 'physically adjacent' to 2 other stations that are receiving station-to-station datagrams on UDP/IP ports 2560 and 2566.